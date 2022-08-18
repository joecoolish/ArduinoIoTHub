#ifndef JODAY_VIEWSTATE
#define JODAY_VIEWSTATE

#include <Adafruit_NeoPixel.h>
#include "config.h"

#define LIGHT_ROWS 5

typedef enum LIGHT_MODE
{
    LightsOff = 1,
    LightsOn = 2,
    RazzleDazzle = 3
} LightMode;

typedef struct COLOR
{
    uint8_t Red;
    uint8_t Green;
    uint8_t Blue;

    COLOR()
    {
        Red = 0;
        Green = 0;
        Blue = 0;
    }

    COLOR(uint8_t r, uint8_t g, uint8_t b)
    {
        Red = r;
        Green = g;
        Blue = b;
    }
} LightColor;

class Light
{
public:
    LightColor colors[32];
    uint8_t colorCount;
    uint8_t lightRows;
    uint8_t *lights;
    uint8_t brightness;
    long razzleDelay;
    LightMode mode;

    Light();
    ~Light();
    int getLightCount();
    float getPowerConsumption();
    void resetRazzle(uint8_t val);
    void progressRazzle();
    void setLights(Adafruit_NeoPixel *strip);
    char *serializeToJson();

private:
    uint8_t **lightBrightness;
    int step;
};

#endif
