#include "debug.h"
#include "FWAPTempSensor.h"
#include "FWAP.h"

#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_Si7021.h>

#if TEMP_SENSOR_BMP280
void configureBMPTempSensor(Adafruit_BMP280 *sensor, uint8_t address = 0x76);

#if ONBOARD_BMP_ENABLE
Adafruit_BMP280 onboardBMP;
#endif

#if DANGLING_BMP_ENABLE
Adafruit_BMP280 danglingBMP;
#endif

#endif // TEMP_SENSOR_BMP280

#if TEMP_SENSOR_SI7021
void configureSiTempSensor(Adafruit_Si7021 *sensor);

Adafruit_Si7021 onboardSi;
#endif // TEMP_SENSOR_SI7021

#define SEALEVELPRESSURE_HPA (1013.25)

static FWAPDB *_db;
static bool tempSensorConfigured = false;

#if TEMP_SENSOR_BMP280
void configureBMPTempSensor(Adafruit_BMP280 *sensor, uint8_t address) {
    unsigned status;
    status = sensor->begin(address, 0x58);
    if (!status) {
        debug("Could not find a valid BMP280 sensor, check wiring, address, sensor ID! " + String(address, HEX));
    } else {
        tempSensorConfigured = true;
    }

    sensor->setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X1,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X1,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_OFF,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */
}
#endif // TEMP_SENSOR_BMP280

#if TEMP_SENSOR_SI7021
void configureSiTempSensor(Adafruit_Si7021 *sensor) {
    unsigned status;
    status = sensor->begin();
    if (!status) {
        debug("Could not find a valid Si7021 sensor, check wiring, address, sensor ID!");
    } else {
        tempSensorConfigured = true;
    }
    sensor->reset();
    sensor->heater(false);
}
#endif // TEMP_SENSOR_SI7021

void setupTempSensor(FWAPDB *db) {
    _db = db;

#if TEMP_SENSOR_BMP280
#if ONBOARD_BMP_ENABLE
    configureBMPTempSensor(&onboardBMP, 0x76);
#endif
#if DANGLING_BMP_ENABLE
    configureBMPTempSensor(&danglingBMP, 0x77);
#endif
#endif // TEMP_SENSOR_BMP280

#if TEMP_SENSOR_SI7021
    configureSiTempSensor(&onboardSi);
#endif
}

#define UPDATE_INTERVAL REPORT_INVERVAL
static unsigned long lastMillis = 0;

#define RECONFIGURE_INTERVAL (60 * 60 * 24)
static unsigned long lastReconfigure = 0;

#if TEMP_SENSOR_BMP280
void reportFromSensor(Adafruit_BMP280 *sensor, String sensorLocation) {
        if (sensor->takeForcedMeasurement()) {
            float temperature = sensor->readTemperature();
            float pressure = sensor->readPressure();
            float altitude = sensor->readAltitude(SEALEVELPRESSURE_HPA);

            InfluxData temperatureDB("weather");
            temperatureDB.addTag("sensor", "BMP280");
            temperatureDB.addTag("sensorLocation", sensorLocation);
            temperatureDB.addValue("temperature", temperature);
            _db->prepare(temperatureDB);

            InfluxData pressureDB("weather");
            pressureDB.addTag("sensor", "BMP280");
            pressureDB.addTag("sensorLocation", sensorLocation);
            pressureDB.addValue("pressure", pressure);
            _db->prepare(pressureDB);

            if (!_db->write()) {
                debug("Couldn't write results to InfluxDB");
            }

            debugPrint("===TEMP SENSOR " + sensorLocation);
            debugPrint("Temperature = " + String(temperature, 2));
            debug(" *C");
            debugPrint("Pressure = " + String(pressure / 100.0F, 4));
            debug(" hPa");
            debugPrint("Approx. Altitude = " + String(altitude, 2));
            debug(" m");
        }
}
#endif

#if TEMP_SENSOR_SI7021
void reportFromSensor(Adafruit_Si7021 *sensor, String sensorLocation) {
    float temperature = sensor->readTemperature();
    float humidity = sensor->readHumidity();

    InfluxData temperatureDB("weather");
    temperatureDB.addTag("sensor", "Si7021");
    temperatureDB.addTag("sensorLocation", sensorLocation);
    temperatureDB.addValue("temperature", temperature);
    _db->prepare(temperatureDB);

    InfluxData humidityDB("weather");
    humidityDB.addTag("sensor", "Si7021");
    humidityDB.addTag("sensorLocation", sensorLocation);
    humidityDB.addValue("humidity", humidity);
    _db->prepare(humidityDB);

    if (!_db->write()) {
        debug("Couldn't write results to InfluxDB");
    }

    debugPrint("===TEMP SENSOR " + sensorLocation);
    debugPrint("Temperature = " + String(temperature, 2));
    debug(" *C");
    debugPrint("Humidity = " + String(humidity, 2));
    debug("%");
}
#endif // TEMP_SENSOR_SI7021

void loopTempSensor() {
    if (!tempSensorConfigured) return;

    if (millis() < lastMillis) lastMillis = 0;
    if (millis() < lastReconfigure) lastReconfigure = 0;

    if (millis() - lastMillis > UPDATE_INTERVAL) {
        lastMillis = millis();

        #if TEMP_SENSOR_BMP280
        #if ONBOARD_BMP_ENABLE
        reportFromSensor(&onboardBMP, "onboard");
        #endif
        #if DANGLING_BMP_ENABLE
        reportFromSensor(&danglingBMP, "dangling");
        #endif
        #endif // TEMP_SENSOR_BMP280

        #if TEMP_SENSOR_SI7021
        reportFromSensor(&onboardSi, "onboard");
        #endif
    }

    if (millis() - lastReconfigure > RECONFIGURE_INTERVAL) {
        #if TEMP_SENSOR_BMP280
        #if ONBOARD_BMP_ENABLE
            configureTempSensor(&onboardBMP, 0x76);
        #endif
        #if DANGLING_BMP_ENABLE
            configureTempSensor(&danglingBMP, 0x77);
        #endif
        #endif // TEMP_SENSOR_BMP280

        #if TEMP_SENSOR_SI7021
            configureSiTempSensor(&onboardSi);
        #endif
    }
}