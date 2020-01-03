#!/usr/local/bin/python3

from MPL3115A2 import MPL3115A2
from Si7021 import Si7021
import time
import logging

#logging.basicConfig(level=logging.DEBUG)

mpl = MPL3115A2(1)
si = Si7021(1)

def readMPL():
    print("🗻  Altitude is %.3f" % mpl.altitude)
    print("🌬  Pressure is %.2f" % mpl.pressure)
    temp = mpl.temperature
    print("🌡  Temp is %.3f°C (%.3f°F)" % (temp, (temp * 1.8 + 32.0)))

def readSi():
    temp = si.temp
    print("🌡 Temp is %.3f°C (%.3f°F)" % (temp, (temp * 1.8 + 32.0)))
    print("🌫 Relative humidity is %0.2f%%" % si.humidity)

while True:

    print("-----")
    readMPL()
    readSi()
    print("-----")
    time.sleep(1)
