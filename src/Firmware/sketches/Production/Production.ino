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

#define PIN_NEOPIXELS           5       // GPIO5 = D1

#define NEOPIXELS_COUNT         4


/*
 * Important note:
 * On the ESP8266 the output state 'LOW' means enabled, the state 'HIGH' disabled!
 *
 */

WiFiClientSecure secureWifiClient = WiFiClientSecure();
PubSubClient MQTTClient = PubSubClient(secureWifiClient);
Adafruit_NeoPixel neopixelStrip = Adafruit_NeoPixel(NEOPIXELS_COUNT, PIN_NEOPIXELS, NEO_GRB + NEO_KHZ800);







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


// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<neopixelStrip.numPixels(); i++) {
    neopixelStrip.setPixelColor(i, c);
    neopixelStrip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<neopixelStrip.numPixels(); i++) {
      neopixelStrip.setPixelColor(i, Wheel((i+j) & 255));
    }
    neopixelStrip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return neopixelStrip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return neopixelStrip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return neopixelStrip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}






void _MQTTCallback(char* topic, byte* payload, unsigned int length)
{
    // Nullpointer check
    if (!topic || !payload)
    {
        return ;
    }

    Serial.print("Message arrived on channel: ");
    Serial.println(topic);




    char* payloadAsCharPointer =  (char*) payload;


    char sceneEffectType = payloadAsCharPointer[0];

    char neopixelColorString1[7]; // length = 7
    strncpy(neopixelColorString1, payloadAsCharPointer + 1, 6);
    neopixelColorString1[6] = '\0';

    char neopixelColorString2[7]; // length = 7
    strncpy(neopixelColorString2, payloadAsCharPointer + 1 + 6, 6);
    neopixelColorString2[6] = '\0';


    //if (strcmp(payloadAsCharPointer, "hallo") == 0)
    Serial.println(sceneEffectType);
    Serial.println(neopixelColorString1);
    Serial.println(neopixelColorString2);

    uint32_t neopixelColor1 = strtol(neopixelColorString1, NULL, 16);
    uint32_t neopixelColor2 = strtol(neopixelColorString2, NULL, 16);


    switch(sceneEffectType)
    {
        case '0':
        {
            neopixel_off();
        }
        break;

        case '1':
        {
            neopixel_showSingleColorScene(neopixelColor1);
        }
        break;

        case '2':
        {
            neopixel_showMixedColorScene(neopixelColor1, neopixelColor2);
        }
        break;
    }
}










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
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);

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
    Serial.println("IP address: ");
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

    while (!MQTTClient.connected())
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
            Serial.print("Connection failed! Error code: ");
            Serial.println(MQTTClient.state());

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
