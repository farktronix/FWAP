#include "debug.h"
#include <BH1750.h>
#include "FWAPDB.h"
#include "FWAP.h"

BH1750 lightMeter(0x23);

static FWAPDB *_db;
static bool lightSensorConfigured = false;

void setupLightMeter(FWAPDB *db) {
    _db = db;
    if(lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
        debug("Configured light meter");
        lightSensorConfigured = true;
    } else {
        debug("Failed to configure light meter");
    }
    // Do one dummy read to get the sensor running
    lightMeter.readLightLevel();
}

// Datasheet says that in high res mode the updates are 180ms apart at most
#define UPDATE_INTERVAL 180
static unsigned long lastUpdateMillis = 0;


#define DB_INTERVAL REPORT_INVERVAL
static unsigned long lastDBMillis = 0;
static float summedSamples = 0;
static unsigned long numSamples = 0;

void loopLightMeter() {
    if (!lightSensorConfigured) return;
    
    if (millis() < lastDBMillis) lastDBMillis = 0;
    if (millis() < lastUpdateMillis) lastUpdateMillis = 0;

    if (millis() - lastUpdateMillis > UPDATE_INTERVAL) {
        lastUpdateMillis = millis();
        float lux = lightMeter.readLightLevel();
        //if (numSamples % 10  == 0) debug("Light meter is " + String(lux,4));

        summedSamples += lux;
        numSamples++;
    }

    if (millis() - lastDBMillis > DB_INTERVAL) {
        lastDBMillis = millis();

        float lux = summedSamples / (float)numSamples;

        debug("Reporting lux value to db: " + String(lux, 4));

        InfluxData light("sensors");
        light.addTag("sensor", "BH1750");
        light.addValue("lux", summedSamples / (float)numSamples);
        _db->write(light);

        summedSamples = 0;
        numSamples = 0;
    }
}