#!/usr/local/bin/python3

from MPL3115A2 import MPL3115A2
from si7021 import Si7021
from pms5003 import PMS5003
from smbus import SMBus

import influxdb_client
from influxdb_client import InfluxDBClient

import time
import logging

hostname="indoors"

logging.basicConfig(level=logging.DEBUG)

mpl = MPL3115A2(1, fetchPressure=False)
si = Si7021(SMBus(1))
pms5003 = PMS5003(device='/dev/ttyAMA0', baudrate=9600, pin_enable=22, pin_reset=27)

influxdb = InfluxDBClient(url="http://filtr.home.rkas.net:9999", token="dyuhAG11e2qX7dAvsZx9DvmZT8kG006pgyaTnYQ62_I9uwHitjy7PnGW8gLEZctZGCLKbgqcsJKOuJYNfEvGnA==")
influx_write_client = influxdb.write_api()

def readMPL():
    #print("🗻  Altitude is %.3f" % mpl.altitude)
    pressure = mpl.pressure
    temp = mpl.temperature
    print("🌬  Pressure is %.2f" % pressure)
    print("🌡  Temp is %.3f°C (%.3f°F)" % (temp, (temp * 1.8 + 32.0)))
    return [f"weather,host={hostname},sensor=MPL3115A2 pressure={pressure}",
            f"weather,host={hostname},sensor=MPL3115A2 temperature={temp}"]

def readSi():
    (humidity, temp) = si.read()
    print("🌡 Temp is %.3f°C (%.3f°F)" % (temp, (temp * 1.8 + 32.0)))
    print("🌫 Relative humidity is %0.2f%%" % humidity)
    return [f"weather,host={hostname},sensor=Si7021 humidity={humidity}",
            f"weather,host={hostname},sensor=Si7021 temperature={temp}"]

def readPMS():
    pmsdata = pms5003.read()
    pm10 = pmsdata.pm_ug_per_m3(1.0)
    pm25 = pmsdata.pm_ug_per_m3(2.5)
    pm100 = pmsdata.pm_ug_per_m3(10)
    print("✨ PM1.0 ug/m3: %d" % pm10)
    print("✨ PM2.5 ug/m3: %d" % pm25)
    print("✨ PM10 ug/m3: %d" % pm100)
    return [f"airquality,host={hostname},sensor=PMS5003 pm10={pm10}",
            f"airquality,host={hostname},sensor=PMS5003 pm25={pm25}",
            f"airquality,host={hostname},sensor=PMS5003 pm100={pm100}"]

while True:

    print("-----")
    datapoints = []
    try:
        datapoints += readMPL()
    except Exception as e:
        print(f"Exception: {e}")
        pass
    try:
        datapoints += readSi()
    except:
        print(f"Exception: {e}")
        pass
    try:
        datapoints += readPMS()
    except:
        print(f"Exception: {e}")
        pass

    print("Writing datapoints:\n%s" % ",\n".join(datapoints))

    influx_write_client.write("FWAP", "farkhome", datapoints) 

    print("-----")
    time.sleep(60)
