#include <WiFi.h>

#include "configs.h"

void setup_wifi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback_async(void * pvParameters) {
  Serial.println("callback_async");
  doMqttStuff();
}

void setup() {
  Serial.begin(115200);
  Serial.println("setup");

  setup_wifi();
    callback_async, /* Function to implement the task */
    "async",    /* Name of the task */
    16384,      /* Stack size in words */
    NULL,       /* Task input parameter */
    0,          /* Priority of the task */
    NULL,       /* Task handle. */
    1);         /* Core where the task should run */
}

void loop() {
  delay(500);
}
