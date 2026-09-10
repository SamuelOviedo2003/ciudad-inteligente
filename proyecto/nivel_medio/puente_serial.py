#!/usr/bin/env python3
"""
Puente Serial <-> Internet para el nivel medio de la Ciudad Autoadaptable.

Hace tres cosas en paralelo, y es justamente esto lo que la rubrica de nivel
medio pide ("comunicacion serial con el computador para recibir y enviar
informacion de internet, que amplifique las capacidades de autoadaptabilidad
del sistema"):

1. Lee la telemetria que nivel_medio.ino imprime por Serial (modo activo,
   fase, sensores) y la reenvia como JSON a un endpoint HTTP, simulando un
   dashboard/log remoto.
2. Consulta cada cierto tiempo el clima real (API publica de Open-Meteo, sin
   API key) y le manda al ESP32 el comando LLUVIA=1 / LLUVIA=0 cuando cambia,
   para que el semaforo ajuste su tiempo de amarillo con un dato que la
   maqueta no puede medir por si misma.
3. Conecta esta maqueta con la OTRA maqueta de ciudad autoadaptable a traves
   de internet (no USB directo, a diferencia del puente maqueta-a-maqueta de
   proyecto/puente_serial/): publica el conteo local de vehiculos en un
   topico de ntfy.sh (gratis, sin cuenta) y lee de vuelta el de la otra
   maqueta, mandandoselo al ESP32 como DET_REMOTO=<n>. Con eso el semaforo
   puede extender su verde si la otra interseccion de la ciudad esta
   congestionada, aunque las dos maquetas esten en computadores distintos en
   cualquier parte con internet.

En los tres casos la informacion que llega no la puede medir la maqueta por
si misma (clima real, o el estado de una maqueta en otro computador) -- eso
es lo que "amplifica" su autoadaptabilidad frente a nivel bajo.

Uso:
    pip install pyserial
    python3 puente_serial.py [puerto_serial] [id_maqueta]

Si no se indica el puerto, se usa PUERTO_DEFECTO (el simulador Wokwi con
rfc2217ServerPort=4001, ver wokwi.toml). Con una ESP32 real, usar el puerto
serie del sistema operativo, ej. "/dev/tty.usbserial-0001" o "COM5".
`id_maqueta` debe ser distinto en cada una de las dos maquetas (por defecto
"A" / cambiar a "B" en la otra), para que cada una ignore sus propios
mensajes al leer el topico compartido.
"""

import sys
import time
import json
import threading
import urllib.request
import urllib.error

import serial

# --- Configuracion ---
PUERTO_DEFECTO = "rfc2217://localhost:4001"  # ver proyecto/nivel_medio/wokwi.toml
BAUDRATE = 115200
ID_MAQUETA_DEFECTO = "A"  # cambiar a "B" al correrlo en la otra maqueta

# Endpoint de internet para reenviar telemetria. Dejar en None para solo
# imprimir en consola (recomendado hasta tener un endpoint propio, ej. un
# "bin" gratuito en https://requestcatcher.com o https://webhook.site) --
# no usar aqui una URL de un tercero sin permiso.
ENDPOINT_TELEMETRIA = None  # ej: "https://TU-SUBDOMINIO.requestcatcher.com/telemetria"

# Ubicacion para consultar clima real (por defecto: Medellin)
LATITUD = 6.2442
LONGITUD = -75.5812
INTERVALO_CLIMA_S = 30  # cada cuanto se consulta el clima real
INTERVALO_PING_S = 2    # cada cuanto se le avisa al ESP32 que el puente sigue vivo

# Canal internet <-> internet entre las dos maquetas de ciudad, via ntfy.sh
# (HTTP simple, sin cuenta ni API key -- ver https://ntfy.sh). El topico es
# publico y adivinable por cualquiera: cambiar por uno propio del equipo
# (ej. incluir su usuario de GitHub) para no chocar con otros grupos del
# curso usando el mismo nombre por defecto.
TOPIC_RED = "isa262-ciudad-autoadaptable-SamuelOviedo2003"
# Limites de ntfy.sh (docs.ntfy.sh/publish/#limitations): 60 peticiones de
# rafaga por IP, luego se repone 1 cada 5 s, y 250 mensajes publicados por dia.
# Publicar una vez por segundo (una version anterior lo hacia) agota la rafaga
# en un minuto y el cupo diario en cuatro. Por eso: se consulta cada 10 s, se
# publica solo cuando cambia el conteo (nunca mas seguido que cada 5 s) y, si
# no cambia, un latido cada 2 min para que la otra maqueta sepa que seguimos
# vivos (el ESP32 descarta el conteo remoto si no le llega nada en 5 min).
INTERVALO_RED_S = 10             # cada cuanto se revisa si la otra maqueta mando algo nuevo
INTERVALO_MIN_PUBLICACION_S = 5  # separacion minima entre publicaciones
INTERVALO_LATIDO_S = 120         # republicar aunque el conteo no haya cambiado

# Varios hilos escriben al mismo puerto serie; el lock evita que dos comandos
# se mezclen en una sola linea.
lock_serial = threading.Lock()


def escribir(ser, texto):
    with lock_serial:
        ser.write(texto.encode())


def abrir_serial(puerto):
    return serial.serial_for_url(puerto, baudrate=BAUDRATE, timeout=1)


def parsear_telemetria(linea):
    """Convierte 'modo=NORMAL fase=A dur=5.0 co2=612 ...' en un dict."""
    datos = {}
    for parte in linea.strip().split(" "):
        if "=" not in parte:
            continue
        clave, valor = parte.split("=", 1)
        try:
            datos[clave] = float(valor) if "." in valor else int(valor)
        except ValueError:
            datos[clave] = valor
    return datos


def enviar_a_internet(datos):
    if not ENDPOINT_TELEMETRIA:
        return
    cuerpo = json.dumps(datos).encode("utf-8")
    peticion = urllib.request.Request(
        ENDPOINT_TELEMETRIA,
        data=cuerpo,
        headers={"Content-Type": "application/json"},
        method="POST",
    )
    try:
        urllib.request.urlopen(peticion, timeout=5)
    except urllib.error.URLError as e:
        print(f"[INTERNET] no se pudo enviar telemetria: {e}")


def consultar_lluvia_real():
    url = (
        "https://api.open-meteo.com/v1/forecast"
        f"?latitude={LATITUD}&longitude={LONGITUD}&current=precipitation&timezone=auto"
    )
    try:
        with urllib.request.urlopen(url, timeout=5) as resp:
            data = json.loads(resp.read())
            precipitacion = data.get("current", {}).get("precipitation", 0)
            return precipitacion > 0
    except (urllib.error.URLError, json.JSONDecodeError, KeyError) as e:
        print(f"[CLIMA] no se pudo consultar el clima real: {e}")
        return None


def publicar_conteo_local(id_maqueta, det):
    """Publica el conteo local de vehiculos en el topico compartido de ntfy.sh."""
    url = f"https://ntfy.sh/{TOPIC_RED}"
    cuerpo = f"origen={id_maqueta} det={det}".encode("utf-8")
    peticion = urllib.request.Request(url, data=cuerpo, method="POST")
    try:
        urllib.request.urlopen(peticion, timeout=5)
    except urllib.error.URLError as e:
        print(f"[RED] no se pudo publicar en ntfy.sh: {e}")


def hilo_lector(ser, id_maqueta):
    """ESP32 -> PC: lee telemetria, la reenvia a internet y publica el conteo
    local para que la otra maqueta lo vea (ver hilo_red), respetando los
    limites de ntfy.sh: solo cuando cambia, o un latido cada INTERVALO_LATIDO_S."""
    ultimo_det = None
    ultima_publicacion = None  # None = "nunca se ha publicado" (no usar 0.0: time.monotonic()
    while True:                # puede arrancar cerca de 0 segun la plataforma, y con 0.0 como
        try:                   # centinela la primera publicacion nunca pasaba el filtro de 5 s)
            linea = ser.readline().decode(errors="ignore").strip()
        except serial.SerialException:
            break
        if not linea or linea == "PONG":
            continue
        datos = parsear_telemetria(linea)
        if not datos:
            continue
        print(f"[ESP32 -> PC] {datos}")
        enviar_a_internet(datos)
        if "det" not in datos:
            continue
        ahora = time.monotonic()
        nunca_publicado = ultima_publicacion is None
        cambio = datos["det"] != ultimo_det
        latido = (not nunca_publicado) and (ahora - ultima_publicacion >= INTERVALO_LATIDO_S)
        gap_cumplido = nunca_publicado or (ahora - ultima_publicacion >= INTERVALO_MIN_PUBLICACION_S)
        if (nunca_publicado or cambio or latido) and gap_cumplido:
            publicar_conteo_local(id_maqueta, datos["det"])
            ultimo_det = datos["det"]
            ultima_publicacion = ahora


def hilo_clima(ser):
    """Internet -> PC -> ESP32: clima real que la maqueta no puede medir."""
    lluvia_actual = None
    while True:
        lluvia_real = consultar_lluvia_real()
        if lluvia_real is not None and lluvia_real != lluvia_actual:
            lluvia_actual = lluvia_real
            comando = f"LLUVIA={1 if lluvia_actual else 0}\n"
            escribir(ser, comando)
            print(f"[PC -> ESP32] {comando.strip()} (clima real de internet)")
        time.sleep(INTERVALO_CLIMA_S)


def hilo_red(ser, id_maqueta):
    """Internet -> PC -> ESP32: conteo de la OTRA maqueta, via ntfy.sh.

    ntfy.sh permite "poll" (traer los mensajes nuevos desde un punto) en vez
    de mantener una conexion abierta, asi que basta con revisar cada
    INTERVALO_RED_S segundos. Se guarda el id del ultimo mensaje visto (mas
    confiable que un timestamp, que probamos y a veces no devuelve nada en
    topicos recien creados) y se usa como "since" en la siguiente consulta,
    para no reprocesar mensajes viejos en cada ciclo. La primera consulta pide
    "since=all" solo para conocer el id mas reciente: en esa pasada NO se
    actua, porque el topico guarda 12 h de historial y reenviarlo al ESP32
    seria darle un conteo de hace horas. Cada linea de la respuesta es un JSON
    de ntfy con el mensaje publicado (ver publicar_conteo_local) en el campo
    "message"; se ignoran los mensajes publicados por esta misma maqueta.
    """
    ultimo_id = "all"
    arrancando = True
    while True:
        url = f"https://ntfy.sh/{TOPIC_RED}/json?poll=1&since={ultimo_id}"
        try:
            with urllib.request.urlopen(url, timeout=10) as resp:
                for linea_bytes in resp:
                    linea = linea_bytes.decode(errors="ignore").strip()
                    if not linea:
                        continue
                    try:
                        evento = json.loads(linea)
                    except json.JSONDecodeError:
                        continue
                    ultimo_id = evento.get("id", ultimo_id)
                    if arrancando:
                        continue  # solo aprender el ultimo id, sin actuar sobre el historial
                    mensaje = evento.get("message", "")
                    datos = parsear_telemetria(mensaje)
                    if datos.get("origen") and datos["origen"] != id_maqueta and "det" in datos:
                        comando = f"DET_REMOTO={datos['det']}\n"
                        escribir(ser, comando)
                        print(f"[PC -> ESP32] {comando.strip()} (otra maqueta, via internet)")
            arrancando = False
        except urllib.error.URLError as e:
            print(f"[RED] no se pudo consultar ntfy.sh: {e}")
        time.sleep(INTERVALO_RED_S)


def hilo_ping(ser):
    """Le avisa al ESP32 que el puente sigue vivo (se ve en el LCD)."""
    while True:
        escribir(ser, "PING\n")
        time.sleep(INTERVALO_PING_S)


def main():
    puerto = sys.argv[1] if len(sys.argv) > 1 else PUERTO_DEFECTO
    id_maqueta = sys.argv[2] if len(sys.argv) > 2 else ID_MAQUETA_DEFECTO
    print(f"[PUENTE] abriendo {puerto} @ {BAUDRATE} baudios (maqueta '{id_maqueta}')")
    ser = abrir_serial(puerto)

    threading.Thread(target=hilo_lector, args=(ser, id_maqueta), daemon=True).start()
    threading.Thread(target=hilo_clima, args=(ser,), daemon=True).start()
    threading.Thread(target=hilo_red, args=(ser, id_maqueta), daemon=True).start()
    threading.Thread(target=hilo_ping, args=(ser,), daemon=True).start()

    print("[PUENTE] corriendo. Ctrl+C para salir.")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n[PUENTE] cerrando...")
        ser.close()


if __name__ == "__main__":
    main()
