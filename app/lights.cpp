#include "lights.h"

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
Light *_state;

void initLights(Light *state)
{
    Serial.println("Initializing Lights");
    _state = state;
    strip.begin();                          // INITIALIZE NeoPixel strip object (REQUIRED)
    strip.show();                           // Turn OFF all pixels ASAP
    strip.setBrightness(state->brightness); // Set BRI
    turnOff();
}

void razzle()
{
    turnOn();
    _state->mode = RazzleDazzle;
}

void turnOn()
{
    Serial.println("Turning lights on");
    _state->resetRazzle(255);

    if (_state->mode == LightsOff)
    {
        Serial.println("Setting light mode to 'LightsOn'");
        _state->mode = LightsOn;
    }

    if (_state->mode == RazzleDazzle)
    {
        Serial.println("Setting light mode to 'LightsOn' and resetting RazzleDazzle");
        _state->mode = LightsOn;
    }

    if (_state->getLightCount() != strip.numPixels())
    {
        Serial.printf("Updating lightCount.  Old: %d, new: %d\r\n", strip.numPixels(), _state->getLightCount());
        strip.updateLength(_state->getLightCount());
    }

    if (strip.getBrightness() != _state->brightness)
    {
        Serial.printf("Updating brightness.  Old: %d, new: %d\r\n", strip.getBrightness(), _state->brightness);
        strip.setBrightness(_state->brightness);
    }

    updateLights();
}

void updateLights()
{
    _state->setLights(&strip);
}

void turnOff()
{
    Serial.println("Turning lights off");
    _state->mode = LightsOff;
    _state->resetRazzle(0);

    if (_state->getLightCount() != strip.numPixels())
    {
        Serial.printf("Updating lightCount.  Old: %d, new: %d\r\n", strip.numPixels(), _state->getLightCount());
        strip.updateLength(_state->getLightCount());
    }

    strip.clear();
    for (int i = 0; i < strip.numPixels(); i++)
    {
        strip.setPixelColor(i, strip.Color(0, 0, 0));
        strip.show();
        delay(50);
    }
}
