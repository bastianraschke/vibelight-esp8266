#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
    #include <avr/power.h>
#endif


/*
 * Connection configuration
 *
 */

#define WIFI_SSID               ""
#define WIFI_PASSWORD           ""

#define MQTT_CLIENTID           "Vibelight Device 1.0"
#define MQTT_SERVER             "mqtt.sicherheitskritisch.de"
#define MQTT_PORT               8883
#define MQTT_USERNAME           ""
#define MQTT_PASSWORD           ""

#define MQTT_CHANNEL_SWITCH     "/vibelight/api/1.0/"
#define MQTT_CHANNEL_STATUS     "/vibelight/api/1.0/status/"

// Try to connect N times and reset chip if limit is exceeded
// #define CONNECTION_RETRIES      3

#define PIN_STATUSLED           LED_BUILTIN

#define PIN_NEOPIXELS           5 // GPIO5 = D1
#define NEOPIXELS_COUNT         4


WiFiClientSecure secureWifiClient = WiFiClientSecure();
PubSubClient MQTTClient = PubSubClient(secureWifiClient);
Adafruit_NeoPixel neopixelStrip = Adafruit_NeoPixel(NEOPIXELS_COUNT, PIN_NEOPIXELS, NEO_GRB + NEO_KHZ800);

/*
 * Neopixel effects
 *
 */

void neopixel_off()
{
    for (uint16_t i = 0; i < neopixelStrip.numPixels(); i++)
    {
        neopixelStrip.setPixelColor(i, 0);
    }

    neopixelStrip.show();
}

void neopixel_showSingleColorScene(const uint32_t color)
{
    for (uint16_t i = 0; i < neopixelStrip.numPixels(); i++)
    {
        neopixelStrip.setPixelColor(i, color);
    }

    neopixelStrip.show();
}

void neopixel_showMixedColorScene(const uint32_t color1, const uint32_t color2)
{
    const uint16_t neopixelCount = neopixelStrip.numPixels();
    const uint16_t middleOfLEDStripe = neopixelCount / 2;

    for (uint16_t i = 0; i < middleOfLEDStripe; i++)
    {
        neopixelStrip.setPixelColor(i, color1);
    }

    for (uint16_t i = middleOfLEDStripe; i < neopixelCount; i++)
    {
        neopixelStrip.setPixelColor(i, color2);
    }

    neopixelStrip.show();
}

void neopixel_showRainbowScene(const uint32_t color1, const uint32_t color2)
{
    const uint16_t neopixelCount = neopixelStrip.numPixels();

    for(uint16_t i = 0; i < neopixelCount; i++)
    {
        float percentage = _map(i, 0.0f, (float) neopixelCount, 0.0f, 1.0f);

        uint8_t color1_r = (color1 >> 16) & 0xFF;
        uint8_t color1_g = (color1 >>  8) & 0xFF;
        uint8_t color1_b = (color1 >>  0) & 0xFF;

        uint8_t color2_r = (color2 >> 16) & 0xFF;
        uint8_t color2_g = (color2 >>  8) & 0xFF;
        uint8_t color2_b = (color2 >>  0) & 0xFF;

        uint8_t r = (color1_r * percentage) + (color2_r * (1 - percentage));
        uint8_t g = (color1_g * percentage) + (color2_g * (1 - percentage));
        uint8_t b = (color1_b * percentage) + (color2_b * (1 - percentage));

        Serial.printf("stepColor %02X%02X%02X\n", r, g, b);

        uint32_t stepColor = neopixelStrip.Color(r, g, b);
        neopixelStrip.setPixelColor(i, stepColor);
    }

    neopixelStrip.show();
}

float _map(float x, float in_min, float in_max, float out_min, float out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


void _MQTTCallback(char* topic, byte* payload, unsigned int length)
{
    if (!topic || !payload)
    {
        Serial.println("Invalid argument (nullpointer) given!");
    }
    else
    {
        Serial.printf("Message arrived on channel: %s\n", topic);

        /*
         * Example payload:
         * 0AABBCCDDEEFF
         *
         * Scene effect: 0
         * Color 1 (as hexadecimal RGB value): AABBCC
         * Color 2 (as hexadecimal RGB value): DDEEFF
         *
         */
        char* payloadAsCharPointer = (char*) payload;

        char lightSceneEffect = payloadAsCharPointer[0];
        uint32_t color1 = _getRGBColorFromPayload(payloadAsCharPointer, 1);
        uint32_t color2 = _getRGBColorFromPayload(payloadAsCharPointer, 1 + 6);

        Serial.printf("Scene effect: %c\n", lightSceneEffect);
        Serial.printf("Color 1: %06X\n", color1);
        Serial.printf("Color 2: %06X\n", color2);

        showScene(lightSceneEffect, color1, color2);
        saveSceneToEEPROM(lightSceneEffect, color1, color2);
    }
}

uint32_t _getRGBColorFromPayload(char* payload, uint8_t startPosition)
{
    uint32_t rgbColor = 0x000000;

    if (!payload)
    {
        Serial.println("Invalid argument (nullpointer) given!");
    }
    else
    {
        // Pre-initialized char array (length = 7) with terminating null character:
        char rbgColorString[7] = { '0', '0', '0', '0', '0', '0', '\0' };
        strncpy(rbgColorString, payload + startPosition, 6);

        // Convert hexadecimal RGB color strings to decimal integer
        uint32_t convertedRGBColor = strtol(rbgColorString, NULL, 16);

        // Verify that the given color values are in a valid range
        if ( convertedRGBColor >= 0x000000 && convertedRGBColor <= 0xFFFFFF )
        {
            rgbColor = convertedRGBColor;
        }
    }

    return rgbColor;
}

void showScene(char lightSceneEffect, uint32_t color1, uint32_t color2)
{
    switch(lightSceneEffect)
    {
        case '0':
        {
            neopixel_off();
        }
        break;

        case '1':
        {
            neopixel_showSingleColorScene(color1);
        }
        break;

        case '2':
        {
            neopixel_showMixedColorScene(color1, color2);
        }
        break;

        case '3':
        {
            neopixel_showRainbowScene(color1, color2);
        }
        break;

        // Add more effects if desired:
        // ...
    }
}

void saveSceneToEEPROM(char lightSceneEffect, uint32_t color1, uint32_t color2)
{

}

void showLastSceneFromEEPROM()
{
    // showScene(char lightSceneEffect, uint32_t color1, uint32_t color2);
}

void blinkStatusLED(const int times)
{
    for (int i = 0; i < times; i++)
    {
        // Enable LED
        digitalWrite(PIN_STATUSLED, LOW);
        delay(100);

        // Disable LED
        digitalWrite(PIN_STATUSLED, HIGH);
        delay(100);
    }
}

void setupPins()
{
    pinMode(PIN_STATUSLED, OUTPUT);
}

void setupNeopixels()
{
    neopixelStrip.begin();
    neopixelStrip.setBrightness(255);

    showLastSceneFromEEPROM();
}

void setupWifi()
{
    Serial.printf("Connecting to %s\n", WIFI_SSID);

    // Disable Wifi access point mode
    WiFi.mode(WIFI_STA);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        // Blink 2 times when connecting
        blinkStatusLED(2);

        delay(500);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi connected.");

    Serial.print("Obtained IP address: ");
    Serial.println(WiFi.localIP());
}

void setupMQTT()
{
    MQTTClient.setServer(MQTT_SERVER, MQTT_PORT);
    MQTTClient.setCallback(_MQTTCallback);
}

void connectMQTT()
{
    if (MQTTClient.connected() == true)
    {
        return ;
    }

    #ifdef CONNECTION_RETRIES
        uint8_t retries = CONNECTION_RETRIES;
    #endif

    while ( MQTTClient.connected() == false )
    {
        Serial.print("Attempting MQTT connection... ");

        if (MQTTClient.connect(MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD) == true)
        {
            Serial.println("Connected.");

            // (Re)subscribe on topic
            MQTTClient.subscribe(MQTT_CHANNEL_SWITCH);

            // Enable onboard LED permanently
            digitalWrite(PIN_STATUSLED, LOW);
        }
        else
        {
            Serial.printf("Connection failed! Error code: %s\n", MQTTClient.state());

            // Blink 3 times for indication of failed MQTT connection
            blinkStatusLED(3);

            Serial.println("Try again in 5 seconds...");
            delay(5000);
        }

        #ifdef CONNECTION_RETRIES
            retries--;

            if (retries == 0)
            {
                Serial.print("Connection failed too often. Resetting chip...");

                // Reset chip
                ESP.restart();
            }
        #endif
    }
}

void setup()
{
    Serial.begin(115200);
    delay(10);

    setupPins();
    setupNeopixels();
    setupWifi();
    setupMQTT();
}

void loop()
{
    connectMQTT();
    MQTTClient.loop();
}
