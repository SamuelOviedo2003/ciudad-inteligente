import serial

ser1 = serial.serial_for_url('rfc2217://localhost:4000', baudrate=115200)
ser2 = serial.serial_for_url('rfc2217://localhost:4001', baudrate=115200)

while True:
    data = ser1.read(ser1.in_waiting)
    if data:
        print(data)
        ser2.write(data)

    data = ser2.read(ser2.in_waiting)
    if data:
        ser1.write(data)
