#include "debug.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "FWAPMotionSensor.h"
#include "FWAP.h"

#define HCSR501_PIN D5
#define MOTION_DELAY 10000

static unsigned long lastMotionDetected = 0;

static HTTPClient http;
static FWAPDB *_db;

void setupMotionSensor(FWAPDB *db) {
    pinMode(HCSR501_PIN, INPUT);

    _db = db;

    http.setReuse(true);
}

void loopMotionSensor() {
    int buttonState = digitalRead(HCSR501_PIN);

    unsigned long now = millis();
    if (buttonState == 1) {
        if ((now - lastMotionDetected) > MOTION_DELAY) {
            lastMotionDetected = now;

            WiFiClient client;
            http.begin(client, "http://homekit.home.rkas.net:16242/motion");
            int httpCode = http.GET();
            http.end();

            debug("Motion posted to server with response " + String(httpCode));

            InfluxData motion("sensors");
            motion.addTag("host", HOSTNAME);
            motion.addTag("sensor", "HC-SR501");
            motion.addValue("motion", 1);
            _db->write(motion);
        }
    }
}