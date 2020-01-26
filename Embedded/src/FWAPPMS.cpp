#include <Arduino.h>
#include "FWAPPMS.h"
#include "debug.h"
#include "FWAP.h"
#include "PMS.h"

PMS pms(Serial);
PMS::DATA dataInProcess;
PMS::DATA lastData;
bool dataOk = false;

FWAPDB *_db;

void setupFWAPPMS(FWAPDB *db) {
    Serial.begin(9600);
    Serial.swap();
    
    _db = db;
}

#define UPDATE_INTERVAL 60 * 1000
static unsigned long lastMillis = 0;
void loopPMS() {
    if (pms.read(dataInProcess)) {
        lastData = dataInProcess;
        dataOk = true;
    }
    if (dataOk && millis() - lastMillis > UPDATE_INTERVAL) {
        lastMillis = millis();
        
        debug("PM 1.0 (ug/m3): " + String(lastData.PM_AE_UG_1_0));
        debug("PM 2.5 (ug/m3): " + String(lastData.PM_AE_UG_2_5));
        debug("PM 10.0 (ug/m3): " + String(lastData.PM_AE_UG_10_0));

        InfluxData pm10("airquality");
        pm10.addTag("host", HOSTNAME);
        pm10.addTag("sensor", "PMS5003");
        pm10.addValue("pm10", lastData.PM_AE_UG_1_0);
        _db->prepare(pm10);

        InfluxData pm25("airquality");
        pm25.addTag("host", HOSTNAME);
        pm25.addTag("sensor", "PMS5003");
        pm25.addValue("pm25", lastData.PM_AE_UG_2_5);
        _db->prepare(pm25);

        InfluxData pm100("airquality");
        pm100.addTag("host", HOSTNAME);
        pm100.addTag("sensor", "PMS5003");
        pm100.addValue("pm100", lastData.PM_AE_UG_10_0);
        _db->prepare(pm100);

        if (!_db->write()) {
            debug("Couldn't write results to InfluxDB");
        }
   }
}