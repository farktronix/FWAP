#include <ESP8266WiFi.h>
#include <Wire.h>
#include <ESP8266mDNS.h>

#ifdef OTA_UPDATES
#include <ArduinoOTA.h>
#endif

#include "FWAP.h"
#include "FWAPSecrets.h"

#include "debug.h"
#include "FWAPDB.h"
#include "FWAPPMS.h"
#include "FWAPLightSensor.h"
#include "FWAPTempSensor.h"
#include "FWAPMotionSensor.h"
#include "FWAPSwitch.h"

static FWAPDB *_fwapDB;
static FWAPDB *_systemDB;

void setup() {
  debugSetup();

  WiFi.mode(WIFI_STA);
  WiFi.hostname(HOSTNAME);
  WiFi.begin (MY_SSID, MY_PASSWORD);

  Wire.begin();

  debug("MAC: ");
  debug(WiFi.macAddress());

#if INIT_WIFI
  Serial.print("Waiting for WiFi to connect...");
  while ( WiFi.status() != WL_CONNECTED ) {
    delay ( 10 );
    debugPrint(".");
  }
  debug("\nConnected!");

  debug("IP: ");
  debug(WiFi.localIP().toString());

  if (!MDNS.begin(HOSTNAME)) {
    debug("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
#endif

#ifdef OTA_UPDATES
  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASS);
  ArduinoOTA.begin(true);
#endif

  _fwapDB = new FWAPDB(INFLUXDB_HOST, "FWAP");
  _systemDB = new FWAPDB(INFLUXDB_HOST, "Systems");

#if PMS_ENABLE
  setupFWAPPMS(_fwapDB);
#endif

#if LIGHT_SENSOR_ENABLE
  setupLightMeter(_fwapDB);
#endif

#if TEMP_SENSOR_ENABLE
  setupTempSensor(_fwapDB);
#endif

#if MOTION_SENSOR_ENABLE
  setupMotionSensor(_fwapDB);
#endif

#if REED_SWITCH_ENABLE
  setupReedSwitch(_fwapDB);
#endif
}

#define DB_INTERVAL 60 * 1000
static unsigned long lastDBMillis = 0;
void loop() {
#ifdef OTA_UPDATES
  ArduinoOTA.handle();
#endif

  if (millis() - lastDBMillis > DB_INTERVAL) {
    lastDBMillis = millis();
    
    // Report uptime to Influx
    InfluxData uptime("uptime");
    uptime.addValue("uptime", millis());
    _systemDB->write(uptime);
  }

#if PMS_ENABLE
  loopPMS();
#endif

#if LIGHT_SENSOR_ENABLE
  loopLightMeter();
#endif

#if TEMP_SENSOR_ENABLE
  loopTempSensor();
#endif

#if MOTION_SENSOR_ENABLE
  loopMotionSensor();
#endif

#if REED_SWITCH_ENABLE
  loopReedSwitch();
#endif

  yield();
}
