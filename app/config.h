// Physical device information for board and sensor

#ifndef IOT_CONFIGS_H
#define IOT_CONFIGS_H

/**
 * WiFi setup
 */
#define IOT_CONFIG_WIFI_SSID "(NO SSID)"
#define IOT_CONFIG_WIFI_PASSWORD "password"

/**
 * Find under Microsoft Azure IoT Suite -> DEVICES -> <your device> -> Device Details and Authentication Keys
 * String containing Hostname, Device Id & Device Key in the format:
 *  "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"    
 */
#define DEVICE_CONNECTION_STRING "HostName=<host_name>;DeviceId=<device_id>;SharedAccessKey=<device_key>"

// The protocol you wish to use should be uncommented
//

#define DEVICE_ID "joday-r2d2-sweater"
#define DHT_TYPE DHT22

// Pin layout configuration
#define DHT_PIN 2

#define TEMPERATURE_ALERT 30

// Interval time(ms) for sending message to IoT Hub
#define INTERVAL 2000

// If don't have a physical DHT sensor, can send simulated data to IoT hub
#define SIMULATED_DATA false

// EEPROM address configuration
#define EEPROM_SIZE 512

// SSID and SSID password's length should < 32 bytes
// http://serverfault.com/a/45509
#define SSID_LEN 32
#define PASS_LEN 32
#define CONNECTION_STRING_LEN 256

#define MESSAGE_MAX_LEN 256

#define DEFAULT_BRIGHTNESS 100

#define HALF_WHITE    \
    {                 \
        128, 128, 128 \
    }
#define RED_COLOR \
    {             \
        255, 0, 0 \
    }
#define YELLOW_COLOR \
    {                \
        255, 255, 0  \
    }
#define GREEN_COLOR \
    {               \
        0, 255, 0   \
    }
#define BLUE_COLOR \
    {              \
        0, 0, 255  \
    }

extern bool messagePending;
extern bool messageSending;
extern bool isRazzle;
extern int interval;

#endif /* IOT_CONFIGS_H */
