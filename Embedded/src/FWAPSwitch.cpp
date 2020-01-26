#include <Arduino.h>
#include "FWAPSwitch.h"
#include "debug.h"

static FWAPDB *_db;

#define REED_SWITCH_PIN D6

void setupReedSwitch(FWAPDB *db) {
    _db = db;

    pinMode(REED_SWITCH_PIN, INPUT_PULLUP);
}

void loopReedSwitch() {
    int switchState = digitalRead(REED_SWITCH_PIN);

    if (switchState == 1) {
        debug("Switch activated");
    }
}