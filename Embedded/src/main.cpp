#include <ESP8266WiFi.h>
#include <Wire.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>

#include "FWAP.h"
#include "FWAPSecrets.h"

#include "debug.h"
#include "FWAPDB.h"
#include "FWAPPMS.h"
#include "FWAPLightSensor.h"
#include "FWAPTempSensor.h"
#include "FWAPMotionSensor.h"

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

  ArduinoOTA.setHostname(HOSTNAME);
  ArduinoOTA.setPassword(OTA_PASS);
  ArduinoOTA.begin(true);

  _fwapDB = new FWAPDB(INFLUXDB_HOST, "FWAP");
  _systemDB = new FWAPDB(INFLUXDB_HOST, "Systems");

  setupFWAPPMS(_fwapDB);
  setupLightMeter(_fwapDB);
  setupTempSensor(_fwapDB);
  setupMotionSensor(_fwapDB);
}

#define DB_INTERVAL 60 * 1000
static unsigned long lastDBMillis = 0;
void loop() {
  ArduinoOTA.handle();

  if (millis() - lastDBMillis > DB_INTERVAL) {
    lastDBMillis = millis();
    
    // Report uptime to Influx
    InfluxData uptime("uptime");
    uptime.addTag("host", HOSTNAME);
    uptime.addValue("uptime", millis());
    _systemDB->write(uptime);
  }

  loopPMS();
  loopLightMeter();
  loopTempSensor();
  loopMotionSensor();

  delay(10);
}
