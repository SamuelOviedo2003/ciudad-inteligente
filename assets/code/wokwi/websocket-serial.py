import serial
import websocket
import json
import threading

id = "myid"  # Puede ser cualquir número, pero debe ser único para cada usuario
# Verificar el envío de mensajes a través del url https://ws.davinsony.com/{id}

ser = serial.serial_for_url('rfc2217://localhost:4000', baudrate=115200)
wss_url = f"wss://ws.davinsony.com/{id}"


def on_message(ws, message):
    data = json.loads(message)
    print(f"Received message: {data}")
    ser.write((data["msg"]+"\n").encode())


def on_error(ws, error):
    print(f"WebSocket error: {error}")


def on_close(ws):
    print("WebSocket connection closed")


def serial_to_websocket():
    while True:
        line = ser.readline().decode().strip()
        if line:
            if not "Recibido" in line:
                ws.send(json.dumps({"msg": line, "to": id, "from": id}))


def on_open(ws):
    print("WebSocket connection opened : "+wss_url)
    threading.Thread(target=serial_to_websocket, daemon=True).start()


ws = websocket.WebSocketApp(wss_url,
                            on_message=on_message,
                            on_error=on_error,
                            on_close=on_close,
                            on_open=on_open)

ws.run_forever()
