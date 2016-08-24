#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <EEPROM.h>

#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
    #include <avr/power.h>
#endif


/*
 * Connection configuration
 *
 */

#define WIFI_SSID                   ""
#define WIFI_PASSWORD               ""

#define MQTT_CLIENTID               "Vibelight Device 1.0 xxxxxxxxxxxxx"
#define MQTT_SERVER                 "mqtt.sicherheitskritisch.de"
#define MQTT_SERVER_TLS_FINGERPRINT "57 36 77 FE E4 3E A9 AE C2 3C 33 DC 60 82 56 18 18 4D 60 50"
#define MQTT_PORT                   8883
#define MQTT_USERNAME               "device_xxxxxxxxxxxxx"
#define MQTT_PASSWORD               ""

#define MQTT_CHANNEL                "/vibelight/api/1.0/"

// Try to connect N times and reset chip if limit is exceeded
// #define CONNECTION_RETRIES          3

#define PIN_STATUSLED               LED_BUILTIN

#define PIN_NEOPIXELS               D1      // GPIO5 = D1
#define NEOPIXELS_COUNT             60
#define NEOPIXELS_BRIGHTNESS        255     // [0..255]


#define EEPROM_ADDRESS_LIGHTSCENE   0
#define EEPROM_ADDRESS_COLOR1       1
#define EEPROM_ADDRESS_COLOR2       4

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
    const uint16_t neopixelCountCenter = neopixelCount / 2;

    for (uint16_t i = 0; i < neopixelCountCenter; i++)
    {
        neopixelStrip.setPixelColor(i, color1);
    }

    for (uint16_t i = neopixelCountCenter; i < neopixelCount; i++)
    {
        neopixelStrip.setPixelColor(i, color2);
    }

    neopixelStrip.show();
}

void neopixel_showGradientScene(const uint32_t color1, const uint32_t color2)
{
    const uint16_t neopixelCount = neopixelStrip.numPixels();

    // Split first color to R, B, G parts
    const uint8_t color1_r = (color1 >> 16) & 0xFF;
    const uint8_t color1_g = (color1 >>  8) & 0xFF;
    const uint8_t color1_b = (color1 >>  0) & 0xFF;

    // Split second color to R, B, G parts
    const uint8_t color2_r = (color2 >> 16) & 0xFF;
    const uint8_t color2_g = (color2 >>  8) & 0xFF;
    const uint8_t color2_b = (color2 >>  0) & 0xFF;

    for(uint16_t i = 0; i < neopixelCount; i++)
    {
        float percentage = _mapPixelCountToPercentage(i, neopixelCount);

        // Calculate the color of this iteration
        // see: https://stackoverflow.com/questions/27532/ and https://stackoverflow.com/questions/22218140/
        const uint8_t r = (color1_r * percentage) + (color2_r * (1 - percentage));
        const uint8_t g = (color1_g * percentage) + (color2_g * (1 - percentage));
        const uint8_t b = (color1_b * percentage) + (color2_b * (1 - percentage));

        const uint32_t currentColor = neopixelStrip.Color(r, g, b);
        neopixelStrip.setPixelColor(i, currentColor);
    }

    neopixelStrip.show();
}

float _mapPixelCountToPercentage(uint16_t i, float count)
{
    const float currentPixel = (float) i;
    const float neopixelCount = (float) count;

    const float min = 0.0f;
    const float max = 1.0f;

    return (currentPixel - 0.0f) * (max - min) / (neopixelCount - 0.0f) + min;
}

void showScene(const char lightScene, const uint32_t color1, const uint32_t color2)
{
    switch(lightScene)
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
            neopixel_showGradientScene(color1, color2);
        }
        break;
    }
}

void saveCurrentScene(const char lightScene, const uint32_t color1, const uint32_t color2)
{
    _setSceneEffectToEEPROM(lightScene);

    _setRGBColorToEEPROM(color1, EEPROM_ADDRESS_COLOR1);
    _setRGBColorToEEPROM(color2, EEPROM_ADDRESS_COLOR2);
}

void showLastScene()
{
    const char lightScene = _getSceneEffectFromEEPROM();

    const uint32_t color1 = _getRGBColorFromEEPROM(EEPROM_ADDRESS_COLOR1);
    const uint32_t color2 = _getRGBColorFromEEPROM(EEPROM_ADDRESS_COLOR2);

    showScene(lightScene, color1, color2);
}

char _getSceneEffectFromEEPROM()
{
    const char lightScene = (char) EEPROM.read(EEPROM_ADDRESS_LIGHTSCENE);
    return lightScene;
}

void _setSceneEffectToEEPROM(const char lightScene)
{
    EEPROM.write(EEPROM_ADDRESS_LIGHTSCENE, (uint8_t) lightScene);
    EEPROM.commit();
}

uint32_t _getRGBColorFromEEPROM(const uint16_t startAddress)
{
    uint32_t color = 0x000000;

    const uint8_t r = EEPROM.read(startAddress);
    const uint8_t g = EEPROM.read(startAddress + 1);
    const uint8_t b = EEPROM.read(startAddress + 2);

    color = neopixelStrip.Color(r, g, b);
    return color;
}

void _setRGBColorToEEPROM(const uint32_t color, const uint16_t startAddress)
{
    // Split color to R, B, G parts
    const uint8_t r = (color >> 16) & 0xFF;
    const uint8_t g = (color >>  8) & 0xFF;
    const uint8_t b = (color >>  0) & 0xFF;

    EEPROM.write(startAddress, r);
    EEPROM.write(startAddress + 1, g);
    EEPROM.write(startAddress + 2, b);

    EEPROM.commit();
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

void setupEEPROM()
{
    Serial.println("Setup EEPROM...");

    EEPROM.begin(512);
}

void setupNeopixels()
{
    Serial.println("Setup Neopixels...");

    neopixelStrip.begin();
    neopixelStrip.setBrightness(NEOPIXELS_BRIGHTNESS);
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

void _MQTTRequestCallback(char* topic, byte* payload, unsigned int length)
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
        const char* payloadAsCharPointer = (char*) payload;

        const char lightScene = payloadAsCharPointer[0];
        const uint32_t color1 = _getRGBColorFromPayload(payloadAsCharPointer, 1);
        const uint32_t color2 = _getRGBColorFromPayload(payloadAsCharPointer, 1 + 6);

        Serial.printf("Light scene: %c\n", lightScene);
        Serial.printf("Color 1: %06X\n", color1);
        Serial.printf("Color 2: %06X\n", color2);

        showScene(lightScene, color1, color2);
        saveCurrentScene(lightScene, color1, color2);
    }
}

uint32_t _getRGBColorFromPayload(const char* payload, const uint8_t startPosition)
{
    uint32_t color = 0x000000;

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
        const uint32_t convertedRGBColor = strtol(rbgColorString, NULL, 16);

        // Verify that the given color values are in a valid range
        if ( convertedRGBColor >= 0x000000 && convertedRGBColor <= 0xFFFFFF )
        {
            color = convertedRGBColor;
        }
    }

    return color;
}

void setupMQTT()
{
    MQTTClient.setServer(MQTT_SERVER, MQTT_PORT);
    MQTTClient.setCallback(_MQTTRequestCallback);
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

        if (MQTTClient.connect(MQTT_CLIENTID, MQTT_USERNAME, MQTT_PASSWORD, MQTT_SERVER_TLS_FINGERPRINT) == true)
        {
            Serial.println("Connected.");

            // (Re)subscribe on topic
            MQTTClient.subscribe(MQTT_CHANNEL);

            // Enable onboard LED permanently
            digitalWrite(PIN_STATUSLED, LOW);
        }
        else
        {
            Serial.printf("Connection failed! Error code: %i\n", MQTTClient.state());

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
    delay(250);

    setupPins();
    setupEEPROM();

    setupNeopixels();
    showLastScene();

    setupWifi();
    setupMQTT();
}

void loop()
{
    connectMQTT();
    MQTTClient.loop();
}
