import serial
import os
import sys

if not len(sys.argv) >1:
    print("Port not specified")
    print("Available ports:")
    os.system('python -m serial.tools.list_ports')
    exit(1)

ser = serial.Serial(sys.argv[1], 9600, timeout=16)

for x in range(3):
    data = ser.readline();
    if data and len(str(data,'ASCII').strip()) > 0 and str(data,'ASCII').strip()[0] == '{':
        print(str(data,'ASCII').strip())
        exit(0)

exit(1)
