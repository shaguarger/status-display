#include <ArduinoJson.h>
#include <PubSubClient.h>

#include "FastLED.h"
#include "ESP8266WiFi.h"

// FASTLED
#define NUM_LEDS 256
#define DATA_PIN 2
CRGB leds[NUM_LEDS];
int memoryBrightness = 255;
const int NUM_LEDS_ROW = 16;
const int MAX_BRIGHTNESS_RANDOM_LEDS = 16;

// WIFI + MQTT
#define mqtt_server "your-mqtt-server"
#define mqtt_user "user"
#define mqtt_password "password"

const char* ssid = "ssid";
const char* password = "password";
const bool showRandomColor = true;
const int randomColorStartLine = 3;
const int randomColorEndLine = 13;

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

int randomNumberBetween(int low, int high) {
   return rand() % (high - low + 1) + low;
}

void showRandomColorIfWanted(){
  if(!showRandomColor){
    return;
  }

  int firstLedToSet = randomColorStartLine * NUM_LEDS_ROW;
  int lastLedToSet = (randomColorEndLine * NUM_LEDS_ROW) - 1;
  int randomLed = randomNumberBetween(firstLedToSet, lastLedToSet);
  leds[randomLed] = CRGB(
    rand() % MAX_BRIGHTNESS_RANDOM_LEDS,
    rand() % MAX_BRIGHTNESS_RANDOM_LEDS,
    rand() % MAX_BRIGHTNESS_RANDOM_LEDS);
  FastLED.show();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  showRandomColorIfWanted();

  // use lib 5.13.2 !
  StaticJsonBuffer<512> jsonBuffer;

  //Serial.print("parsing payload.... ");
  JsonObject& root = jsonBuffer.parseObject(payload);

  //Serial.print("payload parsed.... ");

  if(strcmp(topic, "status-monitor/setOnOff") == 0){
    const char* onOff = root["value"];
    if(strcmp("ON", onOff) == 0){
      //Serial.print("ON");
      FastLED.setBrightness(memoryBrightness);
    }else if(strcmp("OFF", onOff) == 0){
      //Serial.print("OFF");
      memoryBrightness = FastLED.getBrightness();
      FastLED.setBrightness(0);
    }
  }
  else if(strcmp(topic, "status-monitor/setPixel") == 0){
    uint8_t pixel          = root["pin"];
    uint8_t red          = root["red"];
    uint8_t green          = root["green"];
    uint8_t blue          = root["blue"];
    //Serial.print("pixel");
    //Serial.print(pixel);
    leds[pixel].setRGB(red, green, blue);
  } else{
      client.publish("yolo", "unknown command");
  }
  FastLED.show();
}

void setup() { 
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);

  Serial.begin(115200);

  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("status-monitor/started", "yolo");
      // ... and resubscribe
      client.subscribe("status-monitor/#");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void loop() { 
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}
