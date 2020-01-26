#include "debug.h"
#include "FWAPTempSensor.h"
#include "FWAP.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // I2C

#define SEALEVELPRESSURE_HPA (1013.25)

static FWAPDB *_db;
static bool tempSensorConfigured = false;

void setupTempSensor(FWAPDB *db) {
    _db = db;

    unsigned status;
    status = bmp.begin(0x76, 0x58);
    if (!status) {
        debug("Could not find a valid BMP280 sensor, check wiring, address, sensor ID!");
    } else {
        tempSensorConfigured = true;
    }

    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

#define UPDATE_INTERVAL 5 * 1000
static unsigned long lastMillis = 0;
void loopTempSensor() {
    if (!tempSensorConfigured) return;

    if (millis() < lastMillis) lastMillis = 0;

    if (millis() - lastMillis > UPDATE_INTERVAL) {
        InfluxData temperature("weather");
        temperature.addTag("host", HOSTNAME);
        temperature.addTag("sensor", "BMP280");
        temperature.addValue("temperature", bmp.readTemperature());
        _db->prepare(temperature);

        lastMillis = millis();
        debugPrint("Temperature = " + String(bmp.readTemperature(), 2));
        debug(" *C");

        debugPrint("Pressure = " + String(bmp.readPressure() / 100.0F, 4));
        debug(" hPa");

        InfluxData pressure("weather");
        pressure.addTag("host", HOSTNAME);
        pressure.addTag("sensor", "BMP280");
        pressure.addValue("pressure", bmp.readPressure());
        _db->prepare(pressure);

        debugPrint("Approx. Altitude = " + String(bmp.readAltitude(SEALEVELPRESSURE_HPA), 2));
        debug(" m");

        if (!_db->write()) {
            debug("Couldn't write results to InfluxDB");
        }
    }
}