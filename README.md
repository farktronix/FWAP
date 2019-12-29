# FWAP
**F**arkas **W**eather **A**nalysis for raspberry **P**i

This code fetches data from [MPL3115A2](datasheets/MPL3115A2.pdf) and [Si7021](datasheets/Si7021.pdf) breakout boards, both built by Adafruit.

The MPL3115A2 library is more or less a copy of the [Adafruit CircuitPython MPL3115A2](https://github.com/adafruit/Adafruit_CircuitPython_MPL3115A2) library, but written using [smbus2](https://github.com/kplindegaard/smbus2) so that it can run on Linux.

## Example

To read from the MPL3115A2 chip:

```python
from MPL3115A2 import MPL3115A2

chip = MPL3115A2(1)
print("-----")
print("🗻  Altitude is %.3f" % chip.altitude)
print("🌬  Pressure is %.2f" % chip.pressure)
temp = chip.temperature
print("🌡  Temp is %.3f°C (%.3f°F)" % (temp, (temp * 1.8 + 32.0)))
print("-----")
```

#### Output
```
-----
🗻  Altitude is 36.438
🌬  Pressure is 101764.50
🌡  Temp is 18.500°C (65.300°F)
-----
```
