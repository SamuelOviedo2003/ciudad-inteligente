#!/usr/bin/env python3
"""Graba en vivo la telemetria de las dos placas fisicas a archivos, con
marca de tiempo, para poder revisar que paso mientras se prueba a mano.
Uso: python3 monitor_placas.py <puerto1> <puerto2>"""
import serial
import sys
import threading
import time


def grabar(port, out_path):
    with open(out_path, "w") as f:
        while True:
            try:
                ser = serial.Serial(port, 115200, timeout=2)
            except Exception as e:
                f.write(f"[{time.strftime('%H:%M:%S')}] ERROR abriendo {port}: {e}\n")
                f.flush()
                time.sleep(2)
                continue
            print(f"[monitor] {port} -> {out_path}")
            while True:
                try:
                    linea = ser.readline().decode(errors="ignore").strip()
                except Exception as e:
                    f.write(f"[{time.strftime('%H:%M:%S')}] DESCONECTADO: {e}\n")
                    f.flush()
                    break
                if linea:
                    f.write(f"[{time.strftime('%H:%M:%S.%f')[:-3]}] {linea}\n")
                    f.flush()
            ser.close()
            time.sleep(1)  # reintentar (placa se puede reiniciar sola)


if __name__ == "__main__":
    puerto1, puerto2 = sys.argv[1], sys.argv[2]
    t1 = threading.Thread(target=grabar, args=(puerto1, "placa1.log"), daemon=True)
    t2 = threading.Thread(target=grabar, args=(puerto2, "placa2.log"), daemon=True)
    t1.start()
    t2.start()
    while True:
        time.sleep(1)
