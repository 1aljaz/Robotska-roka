#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <string>

Servo servos[3];

int pos[3]; // 0 - 14, 1 - 13, 2 - 12
const int servoPin[] = {14, 13, 12};

const char* ssid = "WiFi";
const char* pass = "kroki tuna <3";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

String message = "";
String sliderValues[] = {"0", "0", "0"}; // 0 - 14, 1 - 13, 2 - 12

JSONVar sliderValuesJSON;

String getSliderValues(){
  sliderValuesJSON["sliderValue14"] = String(sliderValues[0]);
  sliderValuesJSON["sliderValue13"] = String(sliderValues[1]);
  sliderValuesJSON["sliderValue12"] = String(sliderValues[2]);

  String jsonString = JSON.stringify(sliderValuesJSON);
  return jsonString;
}

void initSPIFFS() {
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount je failu");
  }
  else{
   Serial.println("SPIFFS Mount ni failu");
  }
}


void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  Serial.print("Konektam na wifi ..");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void notify(String sliderValues) {
  ws.textAll(sliderValues);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    message = (char*)data;
    
    if (message.indexOf("1s") >= 0 || message.indexOf("2s") >= 0 || message.indexOf("3s") >= 0) {
      int index = message[0] - '1';
      sliderValues[index] = message.substring(2);
    }
    
    if (strcmp((char*)data, "getValues") == 0) {
      notify(getSliderValues());
    }
    
    Serial.print(getSliderValues());
    notify(getSliderValues());
  }
}



void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    switch (type) {
        case WS_EVT_CONNECT:
            Serial.printf("Client se je skonektu iz: %s\n", client->remoteIP().toString().c_str());
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("Client se je dikonektu\n");
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}


void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup() { 
	Serial.begin(9600);
  initSPIFFS();
  initWiFi();

  initWebSocket();

  ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);

	for (int i=0;i<3;i++){
		servos[i].setPeriodHertz(50);
		servos[i].attach(servoPin[i], 500, 2400);
	}

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(SPIFFS, "/index.html", "text/html");
  });
  
  server.serveStatic("/", SPIFFS, "/");
  server.begin();

}

void loop(){
  servos[0].write(sliderValues[0].toInt());
  servos[1].write(sliderValues[1].toInt());
  servos[2].write(sliderValues[2].toInt());
  ws.cleanupClients();
}