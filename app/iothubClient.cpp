#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <WiFiUdp.h>

#include <AzureIoTHub.h>
#include <AzureIoTProtocol_MQTT.h>

#include "iothub_device_client.h"
#include "parson.h"

#include "config.h"
#include "message.h"
#include "iothubClient.h"
#include "lights.h"

static WiFiClientSecure sslClient; // for ESP8266

const char *onSuccess = "\"Successfully invoke device method\"";
const char *notFound = "\"No method found\"";

/*
 * The new version of AzureIoTHub library change the AzureIoTHubClient signature.
 * As a temporary solution, we will test the definition of AzureIoTHubVersion, which is only defined
 *    in the new AzureIoTHub library version. Once we totally deprecate the last version, we can take
 *    the #ifdef out.
 * Break changes in version 1.0.34: AzureIoTHub library removed AzureIoTClient class.
 * So we remove the code below to avoid compile error.
 */

/*
 * #ifdef AzureIoTHubVersion
 * static AzureIoTHubClient iotHubClient;
 * void initIoThubClient()
 * {
 *     iotHubClient.begin(sslClient);
 * }
 * #else
 * static AzureIoTHubClient iotHubClient(sslClient);
 * void initIoThubClient()
 * {
 *     iotHubClient.begin();
 * }
 * #endif
 */

void initClient(IOTHUB_DEVICE_CLIENT_HANDLE iotHubClientHandle)
{
    IoTHubDeviceClient_GetTwinAsync(iotHubClientHandle, getCompleteDeviceTwinOnDemandCallback, NULL);
}

void getCompleteDeviceTwinOnDemandCallback(DEVICE_TWIN_UPDATE_STATE update_state, const unsigned char *payLoad, size_t size, void *userContextCallback)
{
    (void)update_state;
    (void)userContextCallback;
    printf("GetTwinAsync result:\r\n%.*s\r\n", (int)size, payLoad);
}

void blinkLED()
{
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
}

static void sendCallback(IOTHUB_CLIENT_CONFIRMATION_RESULT result, void *userContextCallback)
{
    if (IOTHUB_CLIENT_CONFIRMATION_OK == result)
    {
        Serial.println("Message sent to Azure IoT Hub.");
        blinkLED();
    }
    else
    {
        Serial.println("Failed to send message to Azure IoT Hub.");
    }
    messagePending = false;
}

void sendMessage(IOTHUB_CLIENT_LL_HANDLE iotHubClientHandle, char *buffer, bool temperatureAlert)
{
    IOTHUB_MESSAGE_HANDLE messageHandle = IoTHubMessage_CreateFromByteArray((const unsigned char *)buffer, strlen(buffer));
    if (messageHandle == NULL)
    {
        Serial.println("Unable to create a new IoTHubMessage.");
    }
    else
    {
        MAP_HANDLE properties = IoTHubMessage_Properties(messageHandle);
        Map_Add(properties, "temperatureAlert", temperatureAlert ? "true" : "false");
        Serial.printf("Sending message: %s.\r\n", buffer);
        if (IoTHubClient_LL_SendEventAsync(iotHubClientHandle, messageHandle, sendCallback, NULL) != IOTHUB_CLIENT_OK)
        {
            Serial.println("Failed to hand over the message to IoTHubClient.");
        }
        else
        {
            messagePending = true;
            Serial.println("IoTHubClient accepted the message for delivery.");
        }

        IoTHubMessage_Destroy(messageHandle);
    }
}

void start()
{
    Serial.println("Start sending temperature and humidity data.");
    messageSending = true;
}

void stop()
{
    Serial.println("Stop sending temperature and humidity data.");
    messageSending = false;
}

IOTHUBMESSAGE_DISPOSITION_RESULT receiveMessageCallback(IOTHUB_MESSAGE_HANDLE message, void *userContextCallback)
{
    IOTHUBMESSAGE_DISPOSITION_RESULT result;
    const unsigned char *buffer;
    size_t size;
    if (IoTHubMessage_GetByteArray(message, &buffer, &size) != IOTHUB_MESSAGE_OK)
    {
        Serial.println("Unable to IoTHubMessage_GetByteArray.");
        result = IOTHUBMESSAGE_REJECTED;
    }
    else
    {
        /*buffer is not zero terminated*/
        char *temp = (char *)malloc(size + 1);

        if (temp == NULL)
        {
            return IOTHUBMESSAGE_ABANDONED;
        }

        strncpy(temp, (const char *)buffer, size);
        temp[size] = '\0';
        Serial.printf("Receive C2D message: %s.\r\n", temp);
        free(temp);
        blinkLED();
    }
    return IOTHUBMESSAGE_ACCEPTED;
}

int deviceMethodCallback(const char *methodName, const unsigned char *payload, size_t size, unsigned char **response, size_t *response_size, void *userContextCallback)
{
    Serial.printf("Try to invoke method %s.\r\n", methodName);
    const char *responseMessage = onSuccess;
    int result = 200;

    if (strcmp(methodName, "start") == 0)
    {
        start();
    }
    else if (strcmp(methodName, "stop") == 0)
    {
        stop();
    }
    else if (strcmp(methodName, "turnOn") == 0)
    {        
        turnOn();
    }
    else if (strcmp(methodName, "razzle") == 0)
    {
        razzle();

    }
    else if (strcmp(methodName, "turnOff") == 0)
    {
        turnOff();
    }
    else if (strcmp(methodName, "memory") == 0)
    {
        Serial.print("ESP.getBootMode(); ");
        Serial.println(ESP.getBootMode());
        Serial.print("ESP.getSdkVersion(); ");
        Serial.println(ESP.getSdkVersion());
        Serial.print("ESP.getBootVersion(); ");
        Serial.println(ESP.getBootVersion());
        Serial.print("ESP.getChipId(); ");
        Serial.println(ESP.getChipId());
        Serial.print("ESP.getFlashChipSize(); ");
        Serial.println(ESP.getFlashChipSize());
        Serial.print("ESP.getFlashChipRealSize(); ");
        Serial.println(ESP.getFlashChipRealSize());
        Serial.print("ESP.getFlashChipSizeByChipId(); ");
        Serial.println(ESP.getFlashChipSizeByChipId());
        Serial.print("ESP.getFlashChipId(); ");
        Serial.println(ESP.getFlashChipId());
        Serial.print("ESP.getFreeHeap(); ");
        Serial.println(ESP.getFreeHeap());
        Serial.print("ESP.getCpuFreqMHz(); ");
        Serial.println(ESP.getCpuFreqMHz());
    }
    else
    {
        Serial.printf("No method %s found.\r\n", methodName);
        responseMessage = notFound;
        result = 404;
    }

    *response_size = strlen(responseMessage);
    *response = (unsigned char *)malloc(*response_size);
    strncpy((char *)(*response), responseMessage, *response_size);

    return result;
}

void twinCallback(
    DEVICE_TWIN_UPDATE_STATE updateState,
    const unsigned char *payLoad,
    size_t size,
    void *userContextCallback)
{
    printf("Device Twin update received (state=%d, size=%d): \r\n", updateState, size);
    char *temp = (char *)malloc(size + 1);
    for (int i = 0; i < size; i++)
    {
        temp[i] = (char)(payLoad[i]);
    }
    temp[size] = '\0';
    parseTwinMessage(temp, (Light *)userContextCallback);
    free(temp);
}
