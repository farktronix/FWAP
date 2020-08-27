#include <Arduino.h>
#include "FWAPPMS.h"
#include "debug.h"
#include "FWAP.h"
#include "PMS.h"

struct PMSAccum {
    uint32_t AE_UG_1_0;
    uint32_t AE_UG_2_5;
    uint32_t AE_UG_10_0;

    uint32_t numSamples;
};

PMS pms(Serial);
PMS::DATA pmsData;
PMSAccum accumulatedData;

FWAPDB *_db;

void setupFWAPPMS(FWAPDB *db) {
    Serial.begin(9600);
    //Serial.swap();
    
    _db = db;
}

#define UPDATE_INTERVAL REPORT_INVERVAL
static unsigned long lastMillis = 0;
void loopPMS() {
    if (pms.readUntil(pmsData)) {
        accumulatedData.AE_UG_1_0 += pmsData.PM_AE_UG_1_0;
        accumulatedData.AE_UG_2_5 += pmsData.PM_AE_UG_2_5;
        accumulatedData.AE_UG_10_0 += pmsData.PM_AE_UG_10_0;

        accumulatedData.numSamples++;
    }
    if ((millis() - lastMillis) > UPDATE_INTERVAL) {
        lastMillis = millis();

        if (accumulatedData.numSamples > 0) {
            float ug10 = (float)accumulatedData.AE_UG_1_0 / accumulatedData.numSamples;
            float ug25 = (float)accumulatedData.AE_UG_2_5 / accumulatedData.numSamples;
            float ug100 = (float)accumulatedData.AE_UG_10_0 / accumulatedData.numSamples;
            
            debug("PM 1.0 (ug/m3): " + String(ug10));
            debug("PM 2.5 (ug/m3): " + String(ug25));
            debug("PM 10.0 (ug/m3): " + String(ug100));

            InfluxData pm10("airquality");
            pm10.addTag("sensor", "PMS5003");
            pm10.addValue("pm10", ug10);
            _db->prepare(pm10);

            InfluxData pm25("airquality");
            pm25.addTag("sensor", "PMS5003");
            pm25.addValue("pm25", ug25);
            _db->prepare(pm25);

            InfluxData pm100("airquality");
            pm100.addTag("sensor", "PMS5003");
            pm100.addValue("pm100", ug100);
            _db->prepare(pm100);

            if (!_db->write()) {
                debug("Couldn't write results to InfluxDB");
            }

            memset(&accumulatedData, 0, sizeof(accumulatedData));
        }
    }
}