#include "debug.h"
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include "FWAPMotionSensor.h"
#include "FWAP.h"

#define HCSR501_PIN1 D5
#define HCSR501_PIN2 D6
#define MOTION_DELAY 10000

static unsigned long lastMotionDetected = 0;

static HTTPClient http;
static FWAPDB *_db;

void setupMotionSensor(FWAPDB *db) {
    pinMode(HCSR501_PIN1, INPUT);
    pinMode(HCSR501_PIN2, INPUT);

    _db = db;

    http.setReuse(true);
}

void loopMotionSensor() {
    int motionState1 = digitalRead(HCSR501_PIN1);
    int motionState2 = digitalRead(HCSR501_PIN2);

    unsigned long now = millis();
    if (motionState1 == 1 || motionState2 == 1) {
        if ((now - lastMotionDetected) > MOTION_DELAY) {
            lastMotionDetected = now;

            InfluxData motion("sensors");

            WiFiClient client;
            if (motionState1 == 1) {
                http.begin(client, "http://homekit.home.rkas.net:16242/motion/livingRoom");
                motion.addTag("position", "livingRoom");
            } else {
                http.begin(client, "http://homekit.home.rkas.net:16242/motion/frontDoor");
                motion.addTag("position", "frontDoor");
            }
            int httpCode = http.GET();
            http.end();

            debug("Motion posted to server with response " + String(httpCode));

            motion.addTag("sensor", "HC-SR501");
            motion.addValue("motion", 1);
            _db->write(motion);
        }
    }
}