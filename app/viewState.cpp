#include <ArduinoJson.h>
#include "viewState.h"

static int debug_index = 0;

Light::Light()
{
    lightRows = LIGHT_ROWS;
    lights = new uint8_t[LIGHT_ROWS]{5, 6, 5, 5, 5};
    brightness = DEFAULT_BRIGHTNESS;
    colors[0] = RED_COLOR;
    colors[1] = YELLOW_COLOR;
    colorCount = 2;
    razzleDelay = 1;
    mode = LightsOff;

    lightBrightness = new uint8_t *[LIGHT_ROWS];

    for (uint8_t i = 0; i < lightRows; i++)
    {
        lightBrightness[i] = new uint8_t[lights[i]];

        for (uint8_t j = 0; j < lights[i]; j++)
        {
            lightBrightness[i][j] = 0;
        }
    }
}

Light::~Light()
{
    delete[] lights;

    for (uint8_t i = 0; i < lightRows; i++)
    {
        delete[] lightBrightness[i];
    }

    delete[] lightBrightness;
}

int Light::getLightCount()
{
    int total = 0;

    for (uint8_t i = 0; i < lightRows; i++)
    {
        total += *(lights + i);
    }

    return total;
}

float Light::getPowerConsumption()
{
    float total = 0.0;

    for (uint8_t i = 0; i < lightRows; i++)
    {
        int index = i % colorCount;
        LightColor curColor = colors[index];

        for (uint8_t j = 0; j < lights[i]; j++)
        {
            uint8_t red = lightBrightness[i][j] * curColor.Red / 255;
            uint8_t green = lightBrightness[i][j] * curColor.Green / 255;
            uint8_t blue = lightBrightness[i][j] * curColor.Blue / 255;

            float tempRand = random(988, 1012) / 1000.0;
            total += 20.0 * (red / 255.0) * tempRand;
            tempRand = random(988, 1012) / 1000.0;
            total += 20.0 * (green / 255.0) * tempRand;
            tempRand = random(988, 1012) / 1000.0;
            total += 20.0 * (blue / 255.0) * tempRand;
        }
    }

    return total;
}

void Light::setLights(Adafruit_NeoPixel *strip)
{
    //Serial.println("Setting lights in Light object");
    uint8_t lightIndex = 0;

    for (uint8_t i = 0; i < lightRows; i++)
    {
        int index = i % colorCount;
        LightColor curColor = colors[index];

        for (uint8_t j = 0; j < lights[i]; j++)
        {
            uint8_t red = lightBrightness[i][j] * curColor.Red / 255;
            uint8_t green = lightBrightness[i][j] * curColor.Green / 255;
            uint8_t blue = lightBrightness[i][j] * curColor.Blue / 255;

            //Serial.printf("Light %d set to (%d, %d, %d)\r\n", lightIndex, red, green, blue);
            strip->setPixelColor(lightIndex++, strip->Color(red, green, blue));
        }
    }

    strip->show();
}

void Light::resetRazzle(uint8_t val)
{
    step = 0;
    for (uint8_t i = 0; i < lightRows; i++)
    {
        for (uint8_t j = 0; j < lights[i]; j++)
        {
            lightBrightness[i][j] = val;
        }
    }
}

void Light::progressRazzle()
{
    if (step == 0)
    {
        if (lightBrightness[0][0] == 0 && lightBrightness[1][0] == 255)
        {
            step++;
            return;
            //Serial.printf("Current Step: %d\r\n", step);
        }

        for (uint8_t i = 0; i < lightRows; i++)
        {
            for (uint8_t j = 0; j < lights[i]; j++)
            {
                if (i % 2 == 0)
                {
                    lightBrightness[i][j] = std::max(lightBrightness[i][j] - 1, 0);
                }
                else
                {
                    lightBrightness[i][j] = std::min(lightBrightness[i][j] + 1, 255);
                }
            }
        }
    }
    else if (step < 12)
    {
        if ((lightBrightness[0][0] == 255 && step % 2 == 1) || (lightBrightness[0][0] == 0 && step % 2 == 0))
        {
            step++;
            return;
            //Serial.printf("Current Step: %d\r\n", step);
        }

        bool up = step % 2 == 1;

        for (uint8_t i = 0; i < lightRows; i++)
        {
            for (uint8_t j = 0; j < lights[i]; j++)
            {
                if (i % 2 == 0)
                {
                    lightBrightness[i][j] += up ? 1 : -1;
                }
                else
                {
                    lightBrightness[i][j] = 255 - lightBrightness[i - 1][0];
                }
            }
        }
    }
    else if (step == 12)
    {
        if (lightBrightness[0][0] == 0 && lightBrightness[1][0] == 0)
        {
            step++;
            return;
            //Serial.printf("Current Step: %d\r\n", step);
        }

        for (uint8_t i = 0; i < lightRows; i++)
        {
            for (uint8_t j = 0; j < lights[i]; j++)
            {
                lightBrightness[i][j] = std::max(lightBrightness[i][j] - 1, 0);
            }
        }
    }
    else if (step < 15)
    {
        if (lightBrightness[lightRows - 1][lights[lightRows - 1] - 1] >= 255 && step % 2 == 1)
        {
            step++;
            return;
            //Serial.printf("Current Step: %d\r\n", step);
        }
        if (lightBrightness[0][0] <= 0 && step % 2 == 0)
        {
            step++;
            return;
            //Serial.printf("Current Step: %d\r\n", step);
        }
        bool up = step % 2 == 1;

        if (up)
        {
            for (uint8_t i = 0; i < lightRows; i++)
            {
                for (uint8_t j = 0; j < lights[i]; j++)
                {
                    if (lightBrightness[i][j] < 255)
                    {
                        lightBrightness[i][j] = std::min(lightBrightness[i][j] + 5, 255);
                        return;
                    }
                }
            }
        }
        else
        {
            for (uint8_t i = lightRows - 1; i >= 0; i--)
            {
                for (uint8_t j = lights[i] - 1; j < lights[i]; j--)
                {

                    if (lightBrightness[i][j] > 0)
                    {
                        lightBrightness[i][j] = std::max(lightBrightness[i][j] - 5, 0);
                        if (j != debug_index)
                        {
                            debug_index = j;
                            //Serial.printf("Debug Index: %d, i: %d, lights[i]: %d\r\n", debug_index, i, lights[i]);
                        }
                        return;
                    }
                }
                //Serial.printf("Current row: %d, first val: %d\r\n", i, lightBrightness[i][0]);
            }
        }
    }
    else
    {
        step = 0;
    }
}

char *Light::serializeToJson()
{
    Serial.println("Serializing state");
    char *result;

    result = (char *)malloc(MESSAGE_MAX_LEN * sizeof(char));

    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    JsonObject &jsonColors = jsonBuffer.createObject();
    jsonColors["Color"] = NULL;

    for (int i = 0; i < colorCount; i++)
    {
        JsonObject &color = jsonBuffer.createObject();
        LightColor curColor = colors[i];

        char colorName[6];
        strcpy(colorName, "Color");
        strcat(colorName, String(i).c_str());

        color["Red"] = curColor.Red;
        color["Green"] = curColor.Green;
        color["Blue"] = curColor.Blue;

        jsonColors[colorName] = color;
    }

    root["colors"] = colors;
    root["colorCount"] = colorCount;
    root["lightRows"] = lightRows;
    root["totalLights"] = getLightCount();
    root["brightness"] = brightness;
    root["mode"] = mode;
    root["lightCount"] = NULL;

    root.printTo(result, MESSAGE_MAX_LEN);

    Serial.println(result);
    return result;
}