// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

// Please use an Arduino IDE 1.6.8 or greater

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <string.h>
#include <DallasTemperature.h>
#include <OneWire.h>

#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>
#include <AzureIoTUtility.h>

#include "config.h"
#include "serialReader.h"
#include "message.h"
#include "iothub.h"
#include "iothubClient.h"
#include "lights.h"
#include "viewState.h"

bool messagePending = false;
bool isRazzle = false;
bool messageSending = true;
int interval = INTERVAL;

static char connectionString[] = DEVICE_CONNECTION_STRING;
static char ssid[] = IOT_CONFIG_WIFI_SSID;
static char pass[] = IOT_CONFIG_WIFI_PASSWORD;

void initWifi()
{
    // Attempt to connect to Wifi network:
    Serial.printf("Attempting to connect to SSID: %s.\r\n", ssid);

    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)
    {
        // Get Mac Address and show it.
        // WiFi.macAddress(mac) save the mac address into a six length array, but the endian may be different. The huzzah board should
        // start from mac[0] to mac[5], but some other kinds of board run in the oppsite direction.
        uint8_t mac[6];
        WiFi.macAddress(mac);
        Serial.printf("You device with MAC address %02x:%02x:%02x:%02x:%02x:%02x connects to %s failed! Waiting 10 seconds to retry.\r\n",
                      mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ssid);
        WiFi.begin(ssid, pass);
        delay(10000);
    }
    Serial.printf("Connected to wifi %s.\r\n", ssid);
}

void initTime()
{
    time_t epochTime;
    configTime(0, 0, "pool.ntp.org", "time.nist.gov");

    while (true)
    {
        epochTime = time(NULL);

        if (epochTime == 0)
        {
            Serial.println("Fetching NTP epoch time failed! Waiting 2 seconds to retry.");
            delay(2000);
        }
        else
        {
            Serial.printf("Fetched NTP epoch time is: %lu.\r\n", epochTime);
            break;
        }
    }
}

// static char *serializeToJson(LightState *state)
// {
//     Serial.println("Serializing state");
//     char *result;

//     result = (char *)malloc(MESSAGE_MAX_LEN * sizeof(char));

//     StaticJsonBuffer<MESSAGE_MAX_LEN> jsonBuffer;
//     JsonObject &root = jsonBuffer.createObject();
//     JsonObject &colors = jsonBuffer.createObject();
//     colors["Color"] = NULL;

//     for (int i = 0; i < state->colorCount; i++)
//     {
//         JsonObject &color = jsonBuffer.createObject();
//         LightColor *curColor = state->colors + i;

//         char colorName[6];
//         strcpy(colorName, "Color");
//         strcat(colorName, String(i).c_str());

//         color["Red"] = curColor->Red;
//         color["Green"] = curColor->Green;
//         color["Blue"] = curColor->Blue;

//         colors[colorName] = color;
//     }

//     root["colors"] = colors;
//     root["colorCount"] = state->colorCount;
//     root["lightRows"] = state->lightRows;
//     root["brightness"] = state->brightness;
//     root["mode"] = state->mode;

//     root.printTo(result, MESSAGE_MAX_LEN);

//     Serial.println(result);
//     return result;
// }

static IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle;
//static LightState state;
static Light viewState;

OneWire oneWire(TEMP_PIN);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

void setup()
{
    pinMode(LED_PIN, OUTPUT);

    initSerial();
    delay(2000);

    initWifi();
    initTime();
    initSensor();
    initLights(&viewState);
    sensors.begin();

    /*
    * Break changes in version 1.0.34: AzureIoTHub library removed AzureIoTClient class.
    * So we remove the code below to avoid compile error.
    */

    iotHubClientHandle = IoTHubClient_LL_CreateFromConnectionString(connectionString, MQTT_Protocol);
    if (iotHubClientHandle == NULL)
    {
        Serial.println("Failed on IoTHubClient_CreateFromConnectionString.");
        while (1)
            delay(1000);
    }

    IoTHub_Init();

    IoTHubClient_LL_SetOption(iotHubClientHandle, "product_info", "HappyPath_AdafruitFeatherHuzzah-C");
    IoTHubClient_LL_SetMessageCallback(iotHubClientHandle, receiveMessageCallback, NULL);
    IoTHubClient_LL_SetConnectionStatusCallback(iotHubClientHandle, connection_status_callback, NULL);
    IoTHubClient_LL_SetDeviceMethodCallback(iotHubClientHandle, deviceMethodCallback, NULL);
    IoTHubClient_LL_SetDeviceTwinCallback(iotHubClientHandle, twinCallback, &viewState);

    Serial.println("Finished setup");
    char *reportedProperties = viewState.serializeToJson(); //"{ 'device_property': 'new_value'}"; //

    IoTHubClient_LL_SendReportedState(iotHubClientHandle, (const unsigned char *)reportedProperties, strlen(reportedProperties), reportedStateCallback, NULL);
    free(reportedProperties);
}

static int messageCount = 1;
void loop()
{
    if (!messagePending && messageSending)
    {
        char messagePayload[MESSAGE_MAX_LEN];
        sensors.requestTemperatures();
        bool temperatureAlert = readMessage(messageCount, sensors.getTempCByIndex(0), viewState.getPowerConsumption(), messagePayload);
        sendMessage(iotHubClientHandle, messagePayload, temperatureAlert);
        messageCount++;
        if (viewState.mode != RazzleDazzle)
        {
            delay(interval);
        }
        else
        {
            doRazzle();
        }
    }
    else if (viewState.mode == RazzleDazzle)
    {
        doRazzle();
    }

    IoTHubClient_LL_DoWork(iotHubClientHandle);

    delay(10);
}

static void doRazzle()
{
    long targetMillis = millis() + interval;

    while (targetMillis > millis())
    {
        viewState.progressRazzle();
        updateLights();

        delay(viewState.razzleDelay);
    }
}

static void reportedStateCallback(int status_code, void *userContextCallback)
{
    (void)userContextCallback;
    printf("Device Twin reported properties update completed with result: %d\r\n", status_code);
}

static void connection_status_callback(IOTHUB_CLIENT_CONNECTION_STATUS result, IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason, void *user_context)
{
    (void)reason;
    (void)user_context;
    // This sample DOES NOT take into consideration network outages.
    if (result == IOTHUB_CLIENT_CONNECTION_AUTHENTICATED)
    {
        Serial.print("The device client is connected to iothub\r\n");
    }
    else
    {
        Serial.printf("The device client has been disconnected: %d, %d\r\n", result, reason);
    }
}
