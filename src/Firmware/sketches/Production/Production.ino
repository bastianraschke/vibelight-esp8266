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
#define NEOPIXELS_COUNT         60


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

void neopixel_showMixedColorScene(const uint32_t beginColor, const uint32_t endColor)
{
    const uint16_t middleOfLEDStripe = neopixelStrip.numPixels() / 2;

    for (uint16_t i = 0; i < middleOfLEDStripe; i++)
    {
        neopixelStrip.setPixelColor(i, beginColor);
    }

    for (uint16_t i = middleOfLEDStripe; i < neopixelStrip.numPixels(); i++)
    {
        neopixelStrip.setPixelColor(i, endColor);
    }

    neopixelStrip.show();
}

void neopixel_showRainbowScene(const uint32_t beginColor, const uint32_t endColor)
{
    // for(i = 0; i < neopixelStrip.numPixels(); i++)
    // {
    //     uint8_t r = (beginColor * p) + (endColor * (1 - p))

    //     uint32_t stepColor = neopixelStrip.Color(r, g, b);

    //     neopixelStrip.setPixelColor(i, stepColor);
    // }

    // neopixelStrip.show();
}


void _MQTTCallback(char* topic, byte* payload, unsigned int length)
{
    if (!topic || !payload)
    {
        Serial.println("Invalid argument (nullpointer) given!");
        return ;
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
        uint32_t lightSceneColor1 = _getRGBColorFromPayload(payloadAsCharPointer, 1);
        uint32_t lightSceneColor2 = _getRGBColorFromPayload(payloadAsCharPointer, 1 + 6);

        Serial.printf("Scene effect: %c\n", lightSceneEffect);
        Serial.printf("Color 1: %06X\n", lightSceneColor1);
        Serial.printf("Color 2: %06X\n", lightSceneColor2);

        _showScene(lightSceneEffect, lightSceneColor1, lightSceneColor2);
    }
}

uint32_t _getRGBColorFromPayload(char* payload, uint8_t startPosition)
{
    uint32_t rgbColor = 0;

    if (!payload)
    {
        Serial.println("Invalid argument (nullpointer) given!");
    }
    else
    {
        Serial.println(sizeof(payload));

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

void _showScene(char lightSceneEffect, uint32_t lightSceneColor1, uint32_t lightSceneColor2)
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
            neopixel_showSingleColorScene(lightSceneColor1);
        }
        break;

        case '2':
        {
            neopixel_showMixedColorScene(lightSceneColor1, lightSceneColor2);
        }
        break;

        case '3':
        {
            neopixel_showRainbowScene();
        }
        break;

        // Add more effects if desired:
        // ...
    }
}

/*
 * Important note:
 * On the ESP8266 the output state 'LOW' means enabled, the state 'HIGH' disabled!
 *
 */

void blinkStatusLED(const int times)
{
    for (int i = 0; i < times; i++)
    {
        // Enable
        digitalWrite(PIN_STATUSLED, LOW);
        delay(100);

        // Disable
        digitalWrite(PIN_STATUSLED, HIGH);
        delay(100);
    }
}

void setupPins()
{
    pinMode(PIN_STATUSLED, OUTPUT);

    neopixelStrip.begin();

    // Initialize all pixels to 'off'
    neopixelStrip.show();
}

void setupNeopixels()
{
    neopixelStrip.begin();

    const uint32_t defaultColor = neopixelStrip.Color(240, 0, 255);
    neopixel_showSingleColorScene(defaultColor);
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

    while ( ! MQTTClient.connected() )
    {
        Serial.print("Attempting MQTT connection... ");

        if (MQTTClient.connect(MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD))
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
