#!/usr/bin/env python3
"""
Puente Serial <-> Internet para el nivel medio de la Ciudad Autoadaptable.

Hace dos cosas en paralelo, y es justamente esto lo que la rubrica de nivel
medio pide ("comunicacion serial con el computador para recibir y enviar
informacion de internet, que amplifique las capacidades de autoadaptabilidad
del sistema"):

1. Lee la telemetria que nivel_medio.ino imprime por Serial (modo activo,
   fase, sensores) y la reenvia como JSON a un endpoint HTTP, simulando un
   dashboard/log remoto.
2. Consulta cada cierto tiempo el clima real (API publica de Open-Meteo, sin
   API key) y le manda al ESP32 el comando LLUVIA=1 / LLUVIA=0 cuando cambia,
   para que el semaforo ajuste su tiempo de amarillo con un dato que la
   maqueta no puede medir por si misma (esto es lo que "amplifica" su
   autoadaptabilidad: informacion que no viene de sus propios sensores).

Uso:
    pip install pyserial
    python3 puente_serial.py [puerto_serial]

Si no se indica el puerto, se usa PUERTO_DEFECTO (el simulador Wokwi con
rfc2217ServerPort=4001, ver wokwi.toml). Con una ESP32 real, usar el puerto
serie del sistema operativo, ej. "/dev/tty.usbserial-0001" o "COM5".
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
BAUDRATE = 9600

# Endpoint de internet para reenviar telemetria. Dejar en None para solo
# imprimir en consola (recomendado hasta tener un endpoint propio, ej. un
# "bin" gratuito en https://requestcatcher.com o https://webhook.site) --
# no usar aqui una URL de un tercero sin permiso.
ENDPOINT_TELEMETRIA = None  # ej: "https://TU-SUBDOMINIO.requestcatcher.com/telemetria"

# Ubicacion para consultar clima real (por defecto: Bogota)
LATITUD = 4.711
LONGITUD = -74.0721
INTERVALO_CLIMA_S = 30  # cada cuanto se consulta el clima real
INTERVALO_PING_S = 2    # cada cuanto se le avisa al ESP32 que el puente sigue vivo


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


def hilo_lector(ser):
    """ESP32 -> PC: lee telemetria y la reenvia a internet."""
    while True:
        try:
            linea = ser.readline().decode(errors="ignore").strip()
        except serial.SerialException:
            break
        if not linea or linea == "PONG":
            continue
        datos = parsear_telemetria(linea)
        if datos:
            print(f"[ESP32 -> PC] {datos}")
            enviar_a_internet(datos)


def hilo_clima(ser):
    """Internet -> PC -> ESP32: clima real que la maqueta no puede medir."""
    lluvia_actual = None
    while True:
        lluvia_real = consultar_lluvia_real()
        if lluvia_real is not None and lluvia_real != lluvia_actual:
            lluvia_actual = lluvia_real
            comando = f"LLUVIA={1 if lluvia_actual else 0}\n"
            ser.write(comando.encode())
            print(f"[PC -> ESP32] {comando.strip()} (clima real de internet)")
        time.sleep(INTERVALO_CLIMA_S)


def hilo_ping(ser):
    """Le avisa al ESP32 que el puente sigue vivo (se ve en el LCD)."""
    while True:
        ser.write(b"PING\n")
        time.sleep(INTERVALO_PING_S)


def main():
    puerto = sys.argv[1] if len(sys.argv) > 1 else PUERTO_DEFECTO
    print(f"[PUENTE] abriendo {puerto} @ {BAUDRATE} baudios")
    ser = abrir_serial(puerto)

    threading.Thread(target=hilo_lector, args=(ser,), daemon=True).start()
    threading.Thread(target=hilo_clima, args=(ser,), daemon=True).start()
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
