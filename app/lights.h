#ifndef JODAY_LIGHTS
#define JODAY_LIGHTS

#include <Adafruit_NeoPixel.h>
#include "message.h"

#define LED_PIN 14
#define TEMP_PIN 12
#define LED_COUNT 1

void initLights(Light *state);
void razzle();
void updateLights();
void turnOn();
void turnOff();

#endif
