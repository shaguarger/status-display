#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "configs.h"

#include <FastLED.h>

WiFiClient espClient;
PubSubClient client(espClient);

#define NUM_LEDS 256
#define DATA_PIN 12
CRGB leds[NUM_LEDS];

int memoryBrightness = 255;
const int NUM_LEDS_ROW = 16;
const int MAX_BRIGHTNESS_RANDOM_LEDS = 16;

void doMqttStuff() {
    Serial.println("doMqttStuff");
    setup_mqtt();
    setup_fastled();

    while(true) {
        if (!client.connected()) {
            reconnect();
        }
        client.loop();
        delay(1);
    }
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        String clientId = "ESP32-";
        clientId += String(random(0xffff), HEX);
        if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
            Serial.println("connected");
            client.publish("status-monitor/started", "yolo");
            client.subscribe("status-monitor/#");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void setup_mqtt() {
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

void setup_fastled() {
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
    FastLED.setMaxPowerInMilliWatts(500);
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();

    // use lib 5.13.2 !
    StaticJsonBuffer<512> jsonBuffer;

    JsonObject& root = jsonBuffer.parseObject(payload);

    if (!root.success()) {
        Serial.println("Invalid JSON payload");
        return;
    }

    if (strcmp(topic, "status-monitor/setOnOff") == 0) {
        const char* onOff = root["value"];
        if (strcmp("ON", onOff) == 0) {
            FastLED.setBrightness(memoryBrightness);
        } else if (strcmp("OFF", onOff) == 0) {
            memoryBrightness = FastLED.getBrightness();
            FastLED.setBrightness(0);
        }
    } else if (strcmp(topic, "status-monitor/setPixel") == 0) {
        uint8_t pixel = root["pin"];
        uint8_t red = root["red"];
        uint8_t green = root["green"];
        uint8_t blue = root["blue"];

        leds[pixel].setRGB(red, green, blue);
    } else {
        client.publish("yolo", "unknown command");
    }
    FastLED.show();
}