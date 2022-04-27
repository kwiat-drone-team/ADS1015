import serial

ser = serial.Serial(port="/dev/ttyUSB0", baudrate=57600, parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE, bytesize=serial.EIGHTBITS)
print("here")
print(ser.read(1024))
print("there")