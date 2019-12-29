#!/usr/local/bin/python3

from MPL3115A2 import MPL3115A2
import time

chip = MPL3115A2(1)
while True:
    print("-----")
    print("🗻  Altitude is %.3f" % chip.altitude)
    print("🌬  Pressure is %.2f" % chip.pressure)
    temp = chip.temperature
    print("🌡  Temp is %.3f°C (%.3f°F)" % (temp, (temp * 1.8 + 32.0)))
    print("-----")
    time.sleep(1)
