#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

#include "config.h"
#include "message.h"
#include "lights.h"

void initSensor()
{
    // use SIMULATED_DATA, no sensor need to be inited
    Serial.println("Sensors initialized");
}

float readTemperature()
{
    return random(20, 30);
}

float readHumidity()
{
    return random(30, 40);
}

bool readMessage(int messageId, float temperature, float power, char *payload)
{
    StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
    JsonObject &root = jsonBuffer.createObject();
    root["deviceId"] = DEVICE_ID;
    root["messageId"] = messageId;
    bool temperatureAlert = false;

    // NAN is not the valid json, change it to NULL
    if (std::isnan(temperature))
    {
        root["temperature"] = NULL;
    }
    else
    {
        root["temperature"] = temperature;
        if (temperature > TEMPERATURE_ALERT)
        {
            temperatureAlert = true;
        }
    }

    if (std::isnan(power))
    {
        root["power"] = NULL;
    }
    else
    {
        root["power"] = power;
    }
    root.printTo(payload, MESSAGE_MAX_LEN);
    return temperatureAlert;
}

void parseTwinMessage(char *message, Light *state)
{
    Serial.println(message);

    bool isTurnOn = false;
    StaticJsonBuffer<MESSAGE_MAX_LEN * 3> jsonBuffer;
    JsonObject &root = jsonBuffer.parseObject(message);
    if (!root.success())
    {
        Serial.printf("Parse %s failed.\r\n", message);
        return;
    }

    JsonObject &desired = root["desired"];

    if (desired["interval"].success())
    {
        interval = desired["interval"];
    }
    else if (root.containsKey("interval"))
    {
        interval = root["interval"];
    }

    if (desired["brightness"].success())
    {
        state->brightness = desired["brightness"];
    }
    else if (root.containsKey("brightness"))
    {
        state->brightness = root["brightness"];
    }

    if (desired["colorCount"].success())
    {
        isTurnOn = state->colorCount != desired["colorCount"];
        state->colorCount = desired["colorCount"];
    }
    else if (root.containsKey("colorCount"))
    {
        isTurnOn = state->colorCount != root["colorCount"];
        state->colorCount = root["colorCount"];
    }

    if (desired["colors"].success())
    {
        int i = 0;
        char colorName[6];
        strcpy(colorName, "Color");
        strcat(colorName, String(i).c_str());

        while (desired["colors"][colorName].success() && i < state->colorCount)
        {
            JsonObject &newColor = desired["colors"][colorName];
            LightColor *color = state->colors + i;
            color->Red = newColor["Red"];
            color->Green = newColor["Green"];
            color->Blue = newColor["Blue"];

            strcpy(colorName, "Color");
            strcat(colorName, String(++i).c_str());
        }
        isTurnOn = true;
    }
    else if (root.containsKey("colors"))
    {
        int i = 0;
        char colorName[6];
        strcpy(colorName, "Color");
        strcat(colorName, String(i).c_str());

        while (root["colors"][colorName].success() && i < state->colorCount)
        {
            JsonObject &newColor = root["colors"][colorName];
            LightColor *color = state->colors + i;
            color->Red = newColor["Red"];
            color->Green = newColor["Green"];
            color->Blue = newColor["Blue"];

            strcpy(colorName, "Color");
            strcat(colorName, String(++i).c_str());
        }
        isTurnOn = true;
    }

    if (desired["mode"].success())
    {
        if (desired["mode"] == (int)LightsOff)
        {
            isTurnOn = false;
        }
        else
        {
            isTurnOn = desired["mode"] != (int)state->mode || isTurnOn;
        }
        state->mode = (LightMode)((int)desired["mode"]);
    }
    else if (root.containsKey("mode"))
    {
        if (root["mode"] == (int)LightsOff)
        {
            isTurnOn = false;
        }
        else
        {
            isTurnOn = root["mode"] != (int)state->mode || isTurnOn;
        }
        state->mode = (LightMode)((int)root["mode"]);
    }

    if (desired["razzleDelay"].success())
    {
        state->razzleDelay = desired["razzleDelay"];
    }
    else if (root.containsKey("razzleDelay"))
    {
        state->razzleDelay = root["razzleDelay"];
    }

    if (isTurnOn)
    {
        turnOn();
    }
}
