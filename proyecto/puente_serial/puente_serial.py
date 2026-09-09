#!/usr/bin/env python3
"""Puente serial bidireccional entre dos maquetas ESP32 conectadas por USB al mismo equipo.

Reenvia linea por linea (terminada en \n) lo que llega por un puerto hacia el otro,
en ambos sentidos, usando dos hilos. Util para que las dos maquetas "se hablen"
sin cablear un UART cruzado ni usar ESP-NOW.

Uso:
    python3 puente_serial.py --a /dev/cu.usbmodem312201 --b /dev/cu.usbmodem312301
"""

import argparse
import sys
import threading
import time

import serial


def reenviar(origen: serial.Serial, destino: serial.Serial, nombre: str) -> None:
    while True:
        try:
            linea = origen.readline()
        except serial.SerialException:
            print(f"[{nombre}] puerto cerrado, deteniendo", file=sys.stderr)
            return
        if not linea:
            continue
        print(f"[{nombre}] {linea!r}")
        destino.write(linea)


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--a", required=True, help="puerto serial de la maqueta A")
    parser.add_argument("--b", required=True, help="puerto serial de la maqueta B")
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()

    puerto_a = serial.Serial(args.a, args.baud, timeout=1)
    puerto_b = serial.Serial(args.b, args.baud, timeout=1)
    time.sleep(2)  # el ESP32 se reinicia al abrir el puerto serial

    hilo_ab = threading.Thread(target=reenviar, args=(puerto_a, puerto_b, "A->B"), daemon=True)
    hilo_ba = threading.Thread(target=reenviar, args=(puerto_b, puerto_a, "B->A"), daemon=True)
    hilo_ab.start()
    hilo_ba.start()

    print(f"Puente activo: {args.a} <-> {args.b} @ {args.baud} baud. Ctrl+C para salir.")
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\nCerrando puente...")
    finally:
        puerto_a.close()
        puerto_b.close()


if __name__ == "__main__":
    main()
