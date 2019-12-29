#!/usr/local/bin/python3
#
# MPL3115A2 Driver for Raspberry Pi
# 
# This code is heavily copied from the Adafruit CircuitPython code (https://github.com/adafruit/Adafruit_CircuitPython_MPL3115A2)
#  and adapted for use with the smbus library. This was mostly done as an exercise for me, since re-typing code is a good way to learn it.

from smbus2 import SMBus
from NOAA import NOAA
import logging
import time

#pylint: disable=bad-whitespace
# Internal constants:
_MPL3115A2_ADDRESS                   = 0x60

_MPL3115A2_REGISTER_STATUS           = 0x00
_MPL3115A2_REGISTER_PRESSURE_MSB     = 0x01
_MPL3115A2_REGISTER_PRESSURE_CSB     = 0x02
_MPL3115A2_REGISTER_PRESSURE_LSB     = 0x03
_MPL3115A2_REGISTER_TEMP_MSB         = 0x04
_MPL3115A2_REGISTER_TEMP_LSB         = 0x05

_MPL3115A2_REGISTER_DR_STATUS        = 0x06
_MPL3115A2_OUT_P_DELTA_MSB           = 0x07
_MPL3115A2_OUT_P_DELTA_CSB           = 0x08
_MPL3115A2_OUT_P_DELTA_LSB           = 0x09
_MPL3115A2_OUT_T_DELTA_MSB           = 0x0A
_MPL3115A2_OUT_T_DELTA_LSB           = 0x0B

_MPL3115A2_WHOAMI                    = 0x0C

_MPL3115A2_BAR_IN_MSB                = 0x14
_MPL3115A2_BAR_IN_LSB                = 0x15

_MPL3115A2_REGISTER_STATUS_TDR       = 0x02
_MPL3115A2_REGISTER_STATUS_PDR       = 0x04
_MPL3115A2_REGISTER_STATUS_PTDR      = 0x08

_MPL3115A2_PT_DATA_CFG               = 0x13
_MPL3115A2_PT_DATA_CFG_TDEFE         = 0x01
_MPL3115A2_PT_DATA_CFG_PDEFE         = 0x02
_MPL3115A2_PT_DATA_CFG_DREM          = 0x04

_MPL3115A2_CTRL_REG1                 = 0x26
_MPL3115A2_CTRL_REG2                 = 0x27
_MPL3115A2_CTRL_REG3                 = 0x28
_MPL3115A2_CTRL_REG4                 = 0x29
_MPL3115A2_CTRL_REG5                 = 0x2A

_MPL3115A2_CTRL_REG1_SBYB            = 0x01
_MPL3115A2_CTRL_REG1_OST             = 0x02
_MPL3115A2_CTRL_REG1_RST             = 0x04
_MPL3115A2_CTRL_REG1_RAW             = 0x40
_MPL3115A2_CTRL_REG1_ALT             = 0x80
_MPL3115A2_CTRL_REG1_BAR             = 0x00

_MPL3115A2_CTRL_REG1_OS1             = 0x00
_MPL3115A2_CTRL_REG1_OS2             = 0x08
_MPL3115A2_CTRL_REG1_OS4             = 0x10
_MPL3115A2_CTRL_REG1_OS8             = 0x18
_MPL3115A2_CTRL_REG1_OS16            = 0x20
_MPL3115A2_CTRL_REG1_OS32            = 0x28
_MPL3115A2_CTRL_REG1_OS64            = 0x30
_MPL3115A2_CTRL_REG1_OS128           = 0x38

_MPL3115A2_REGISTER_STARTCONVERSION  = 0x12
#pylint: enable=bad-whitespace

class MPL3115A2:
    def __init__(self, i2cBusID, address=_MPL3115A2_ADDRESS, fetchPressure=True):
        # If fetchPressure is True then this library will send requests to geolocate the current IP and fetch weather data from the nearest station using the NOAA API.

        logging.debug("🌟  Hello, World!")

        # Bring up SMBus
        self.bus = SMBus(i2cBusID)
        if self.bus is None:
            raise Exception("Couldn't init I2C bus with identifier %x" % i2cBusID)
        self.address = address

        # Check that we can speak to the chip and that we're dealing with the right chip
        whoami = self.bus.read_byte_data(self.address, _MPL3115A2_WHOAMI)
        if whoami != 0xC4:
            raise Exception("I2C device found at address %0x but it is not a MPL3115A2 chip (whoami returned %0x)" % (self.address, whoami))

        logging.debug("📟  Connected to MPL3115A2 chip")

        # Kick off a reset so that we know the configuration of the device
        # I'm not sure this is the best idea, but it's the simplest, and it's what the Adafruit library does so I'm copying it for now
        logging.debug("⚡️  Initiating a reset...")
        # Ignore write errors since the chip might reset in the middle of our message
        try:
            self.bus.write_byte_data(self.address, _MPL3115A2_CTRL_REG1, _MPL3115A2_CTRL_REG1_RST)
        except OSError:
            pass
        self._pollForReg1(_MPL3115A2_CTRL_REG1_RST)
        logging.debug("💫  Chip has been reset")

        logging.debug("😶  Configuring chip...")
        self._ctrlReg1 = (_MPL3115A2_CTRL_REG1_ALT | _MPL3115A2_CTRL_REG1_OS128)
        self._writeReg1()

        logging.debug("🌦  Fetching weather data from NOAA...")
        pressure = 0
        self.noaa = NOAA()
        if fetchPressure is True:
            self._updatePressure()
            # TODO: Set a timer for a recurring pressure update

        self.bus.write_byte_data(self.address, _MPL3115A2_PT_DATA_CFG, (_MPL3115A2_PT_DATA_CFG_TDEFE | _MPL3115A2_PT_DATA_CFG_PDEFE | _MPL3115A2_PT_DATA_CFG_DREM))

        logging.debug("✅  Chip activated!")
        

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        logging.info("🌅  Shutting down. Goodbye!")
        self.bus.close()

    def _writeReg1(self, requestData=False):
        if requestData is True:
            self._ctrlReg1 |= _MPL3115A2_CTRL_REG1_OST
        else:
            self._ctrlReg1 &= ~_MPL3115A2_CTRL_REG1_OST

        logging.debug("⚙️ Setting control register to %x" % self._ctrlReg1)

        self.bus.write_byte_data(self.address, _MPL3115A2_CTRL_REG1, self._ctrlReg1)


    def _pollForReg1(self, mask):
        while self.bus.read_byte_data(self.address, _MPL3115A2_CTRL_REG1) & mask > 0:
            time.sleep(0.01)

    def _pollForStatus(self, mask):
        while self.bus.read_byte_data(self.address, _MPL3115A2_REGISTER_STATUS) & mask == 0:
            time.sleep(0.01)

    def _updatePressure(self):
        try:
            pressure = self.noaa.getCurrentSeaLevelPressure() 
            logging.debug("⛈🌡 Pressure fetched from NOAA is %dPa" % pressure)
            self.seaLevelPressure = pressure
        except Exception as e:
            logging.info("🚨  Error fetching pressure data from NOAA: {}".format(e))

    # ----
    # Temperature
    @property
    def temperature(self):
        self._pollForStatus(_MPL3115A2_REGISTER_STATUS_TDR)
        tempBytes = self.bus.read_i2c_block_data(self.address, _MPL3115A2_REGISTER_TEMP_MSB, 2)
        return (tempBytes[0]) + ((tempBytes[1]>>4 & 0x0F)/16.0)

    # ----

    # ----
    # Altitude 
    @property
    def altitude(self):
        # Wait for any previous measurements to finish
        self._pollForReg1(_MPL3115A2_CTRL_REG1_OST)

        if (self._ctrlReg1 & _MPL3115A2_CTRL_REG1_ALT) != _MPL3115A2_CTRL_REG1_ALT:
            self._ctrlReg1 |= _MPL3115A2_CTRL_REG1_ALT
            self._writeReg1()

        # Request a sensor read
        self._writeReg1(requestData=True)

        # Wait for PDR to be set
        self._pollForStatus(_MPL3115A2_REGISTER_STATUS_PDR)
        altitudeBytes = self.bus.read_i2c_block_data(self.address, _MPL3115A2_REGISTER_PRESSURE_MSB, 3)
        logging.debug("🗻  Altitude data read from chip is %s" % (",".join([hex(i) for i in altitudeBytes])))

        # The reading is a 20 bit value. The first two bytes are the meters (in 2's complement) and the third byte is the fractions of a meter, stored in the top four bits (not 2's complement)
        return (altitudeBytes[0]<<8 | altitudeBytes[1]) + ((altitudeBytes[2]>>4 & 0x0F)/16.0)

    @property
    def seaLevelPressure(self):
        pressureBytes = self.bus.read_i2c_block_data(self.address, _MPL3115A2_BAR_IN_MSB, 2)
        return ((pressureBytes[0] << 8) | (pressureBytes[1])) << 1

    @seaLevelPressure.setter
    def seaLevelPressure(self, pressure):
        pressure //= 2
        pressureLSB = pressure & 0xFF
        pressureMSB = (pressure >> 8) & 0xFF
        logging.debug("🌡 Writing pressure data of %dPa (%0x|%0x)" % (pressure, pressureMSB, pressureLSB))
        self.bus.write_i2c_block_data(self.address, _MPL3115A2_BAR_IN_MSB, [pressureMSB, pressureLSB])

    # ----

    # ----
    # Pressure
    @property
    def pressure(self):
        # Wait for any previous measurements to finish
        self._pollForReg1(_MPL3115A2_CTRL_REG1_OST)

        if (self._ctrlReg1 & _MPL3115A2_CTRL_REG1_ALT) == _MPL3115A2_CTRL_REG1_ALT:
            self._ctrlReg1 &= ~_MPL3115A2_CTRL_REG1_ALT
            self._writeReg1()

        # Request a sensor read
        self._writeReg1(requestData=True)

        # Wait for PDR to be set
        self._pollForStatus(_MPL3115A2_REGISTER_STATUS_PDR)
        pressureBytes = self.bus.read_i2c_block_data(self.address, _MPL3115A2_REGISTER_PRESSURE_MSB, 3)
        logging.debug("🌬   Pressure data read from chip is %s" % (",".join([hex(i) for i in pressureBytes])))

        # The reading is a 20 bit value. The first two bytes are the meters (in 2's complement) and the third byte is the fractions of a meter, stored in the top four bits (not 2's complement)
        return (pressureBytes[0]<<10 | pressureBytes[1]<<2 | ((pressureBytes[2]&0xC0 >> 6) & 0x03a)) + ((pressureBytes[2]>>4 & 0x03)/4.0)
