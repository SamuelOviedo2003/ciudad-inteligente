#!/usr/bin/env python3
"""Pruebas del puente_serial.py de nivel medio sin hardware ni internet:
se reemplazan urlopen y el puerto serial por dobles y se cuenta que hace."""
import io
import json
import sys
import types
import urllib.request

sys.path.insert(0, sys.argv[1])  # carpeta proyecto/nivel_medio
import puente_serial as ps  # noqa: E402

fallas = 0


def check(cond, msg):
    global fallas
    print(("  PASS " if cond else "  FAIL ") + msg)
    if not cond:
        fallas += 1


class SerialFalso:
    def __init__(self, lineas):
        self.lineas = list(lineas)
        self.escrito = []

    def readline(self):
        if not self.lineas:
            raise ps.serial.SerialException("fin")
        return self.lineas.pop(0).encode()

    def write(self, b):
        self.escrito.append(b.decode())


peticiones = []


def urlopen_falso(req, timeout=None):
    url = req.full_url if hasattr(req, "full_url") else req
    peticiones.append(url)
    if "ntfy.sh" in url and "/json?poll=1" in url:
        # historial de 12 h del topico: 3 mensajes viejos de la otra maqueta
        cuerpo = "\n".join(
            json.dumps({"id": f"id{i}", "time": 1000 + i, "event": "message", "topic": "x", "message": f"origen=B det={d}"})
            for i, d in enumerate([5, 1, 6])
        )
        return io.BytesIO(cuerpo.encode())
    return io.BytesIO(b"{}")


urllib.request.urlopen = urlopen_falso

print("== parseo ==")
d = ps.parsear_telemetria("modo=CONG1+ECO fase=A dur=8.00 co2=1200 cny1=1 det=2 lluvia=0")
check(d["modo"] == "CONG1+ECO" and d["dur"] == 8.0 and d["det"] == 2 and d["co2"] == 1200, "parsea clave=valor con tipos")
check(ps.parsear_telemetria("PONG") == {}, "PONG no produce datos")

print("== hilo_lector: cuantas peticiones HTTP por linea de telemetria ==")
peticiones.clear()
lineas = [f"modo=NORMAL fase=A dur=5.00 det={i % 2}\n" for i in range(60)]  # 60 s de telemetria (1 linea/s)
ps.hilo_lector(SerialFalso(lineas), "A")
publicaciones = [u for u in peticiones if "ntfy.sh" in u]
print(f"  60 lineas de telemetria -> {len(publicaciones)} POST a ntfy.sh")
check(len(publicaciones) <= 12, "publica en ntfy.sh como mucho 1 vez cada 5 s (limite ntfy.sh: 60 rafaga + 1 cada 5 s, 250 msg/dia) (FALLA = 1 POST por segundo)")

print("== hilo_lector: conteo constante ==")
peticiones.clear()
ps.hilo_lector(SerialFalso([f"modo=NORMAL det=2\n" for _ in range(30)]), "A")
publicaciones = [u for u in peticiones if "ntfy.sh" in u]
print(f"  30 lineas con det=2 -> {len(publicaciones)} POST")
check(len(publicaciones) == 1, "con el conteo sin cambios publica una sola vez (el latido es cada 2 min)")

print("== hilo_red: arranque con since=all y luego un mensaje nuevo ==")
peticiones.clear()
ser = SerialFalso([])
import time as _t
pasadas = [0]

def dormir_contando(_s):
    pasadas[0] += 1
    if pasadas[0] == 2:
        raise KeyboardInterrupt

_t.sleep = dormir_contando
try:
    ps.hilo_red(ser, "A")
except KeyboardInterrupt:
    pass
print(f"  peticiones: {peticiones}")
print(f"  comandos escritos al ESP32: {ser.escrito}")
check(peticiones[0].endswith("since=all") and peticiones[1].endswith("since=id2"), "la segunda consulta usa el ultimo id visto")
check(ser.escrito == ["DET_REMOTO=5\n", "DET_REMOTO=1\n", "DET_REMOTO=6\n"], "el arranque no reenvia el historial; solo la segunda pasada manda DET_REMOTO")

print(f"\n-> {'OK' if fallas == 0 else str(fallas) + ' fallas'}")
sys.exit(1 if fallas else 0)
