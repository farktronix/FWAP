#include <Arduino.h>

void debugSetup() {
    Serial1.begin(115200);
    while(!Serial1);  
}

void debug(String msg) {
    Serial1.println(msg);
}

void debugPrint(String msg) {
    Serial1.print(msg);
}
