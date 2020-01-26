#ifndef _FWAP_DEBUG_H
#define _FWAP_DEBUG_H

#include <Arduino.h>

void debugSetup();

// Prints message and a newline
void debug(String msg);

// Prints message and no newline
void debugPrint(String msg);

#endif