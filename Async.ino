#include <WiFi.h>
#include "ESPAsyncWebServer.h"

//Pins we need + onboard blue LED in this case
#define RELAY 18
#define BLUE_LED 2

//Invert if you need a positive signal to turn on the relay module
#define relay_ON LOW
#define relay_OFF HIGH

//Wifi settings
const char* ssid = "Wifi Name";
const char* password = "Wifi Password";

//Other settings
const char* access_key = "123456";
const int relay_timer = 1000; //Time to stay on
const int max_reconnects = 3; //Max Wifi connect attempts

AsyncWebServer server(80);

void trigger() {
  digitalWrite(RELAY, relay_ON);
  delay(relay_timer);
  digitalWrite(RELAY, relay_OFF);
}

void setupPins() {
  pinMode(RELAY, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  digitalWrite(RELAY, relay_OFF);
  digitalWrite(BLUE_LED, LOW);
}

void connectingBlink() {
  digitalWrite(BLUE_LED, !digitalRead(BLUE_LED));
  delay(1000);
}

void checkWifi() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  digitalWrite(RELAY, relay_OFF); //Making sure we don't get stuck in the while loop with the relay on
  int attempts = 0;

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    if (attempts > max_reconnects) {
      ESP.restart();
    }

    connectingBlink();
    attempts++;
  }

  digitalWrite(BLUE_LED, HIGH); //Connected
}

void webServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    return request->send(200, "text/plain", "ok");
    });

  server.on("/actions", HTTP_GET, [](AsyncWebServerRequest* request) {
    try {
      if (request->params() == 0) {
        throw 400;
      }

      AsyncWebParameter* p = request->getParam(0);
      if (p->name() != "code" || p->value() != access_key) {
        throw 401;
      }

      trigger();
      return request->send(200, "text/plain", "Success");
    }
    catch (int code) {
      return request->send(code);
    }
    catch (...) {
      return request->send(500);
    }
    });

  server.begin();
}

void setup() {
  setupPins();
  checkWifi();
  webServer();
}

void loop() {
  checkWifi();
}