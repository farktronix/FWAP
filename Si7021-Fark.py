#!/usr/local/bin/python3

from smbus2 import SMBus
import time
import logging

_Si7021_ADDRESS                     = 0x40

_Si7021_RESET                       = 0xFE

_Si7021_READ_TEMP_HOLD              = 0xE3
_Si7021_READ_TEMP_NOHOLD            = 0xF3

_Si7021_READ_HUMID_HOLD             = 0xE5
_Si7021_READ_HUMID_NOHOLD           = 0xF5

_Si7021_READ_PREV_TEMP              = 0xE0

_Si7021_WRITE_USER_REG              = 0xE6
_Si7021_READ_USER_REG               = 0xE7

_Si7021_USER_REG_12RH_14TMP         = 0x00
_Si7021_USER_REG_8RH_12TMP          = 0x01
_Si7021_USER_REG_10RH_13TMP         = 0x80
_Si7021_USER_REG_11RH_11_TMP        = 0x81
_Si7021_USER_REG_VDD_STATUS         = 0x40
_Si7021_USER_REG_HEATER_ENABLE      = 0x04

_Si7021_WRITE_HEATER_CONTROL        = 0x51
_Si7021_READ_HEATER_CONTROL         = 0x11

class Si7021:
    def __init__(self, i2cBusID, address=_Si7021_ADDRESS):
        logging.debug("🌟  Si7021: Hello, World!")

        self.bus = SMBus(i2cBusID)
        if self.bus is None:
            raise Exception("Couldn't init I2C bus with identifier %x" % i2cBusID)
        self.address = address
       
        logging.debug("⚡️  Checking voltage...")
        ureg = self.bus.read_byte_data(self.address, _Si7021_READ_USER_REG)
        if (ureg & _Si7021_USER_REG_VDD_STATUS) == _Si7021_USER_REG_VDD_STATUS:
            raise Excception("Si7021 is experiencing low voltage")
        
        self.bus.write_byte_data(self.address, _Si7021_WRITE_USER_REG, _Si7021_USER_REG_12RH_14TMP)

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        logging.info("🌅  Shutting down. Goodbye!")
        self.bus.close()

    # Taken from Adafruit_CircuitPython_SI7021
    def _crc(self, data):
        crc = 0
        for byte in data:
            crc ^= byte
            for _ in range(8):
                if crc & 0x80:
                    crc <<= 1
                    crc ^= 0x131
                else:
                    crc <<= 1
        return crc

    @property
    def humidity(self):
        data = self.bus.read_i2c_block_data(self.address, _Si7021_READ_HUMID_HOLD, 3)
        humid = (data[0] << 8) | data[1]
        if data[2] != self._crc(data[:2]):
            #raise ValueError("CRC mismatch")
            print("CRC Mismatch")
        return min(100.0, ((humid * 125) >> 16) - 6.0)

    @property
    def temp(self):
        data = self.bus.read_i2c_block_data(self.address, _Si7021_READ_TEMP_HOLD, 3)
        temp = (data[0] << 8) | data[1]
        if data[2] != self._crc(data[:2]):
            #raise ValueError("CRC mismatch")
            print("CRC Mismatch")
        return min(100.0, ((temp * 175.72) / 65536) - 46.85)
