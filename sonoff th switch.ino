#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3
#include "DHT.h"
#include <Ticker.h>

const char* ssid = "Roco";
const char* password = "xxxxx";
char* startCommand = "{\"command\" : \"start\"}";
char* stopCommand = "{\"command\" : \"stop\"}";
String currentSwitchStatus = "off";

//HW https://www.itead.cc/smart-home/sonoff-th.html

const uint8_t     BUTTON_PIN = 0;
const uint8_t     RELAY_PIN  = 12;
const uint8_t     LED_PIN    = 13;
#define DHTPIN 14
#define DHTTYPE DHT21
//HW

#define ORG "1d9l1c"
#define DEVICE_TYPE "ESP8266"
#define DEVICE_ID "test2"
#define TOKEN "12345678"
#define BUTTON          0    

Ticker btn_timer;

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char cmdtopic[] = "iot-2/cmd/test/fmt/json";
char statustopic[] = "iot-2/evt/status/fmt/json";
char sensortopic[] = "iot-2/evt/sensor/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;
char message_buff[100];
int counter = 0;

WiFiClient wifiClient;
PubSubClient client;
DHT dht(DHTPIN, DHTTYPE);

void callback(char* cmdtopic, byte* payload, unsigned int payloadLength) {
    int i = 0;
    for (i = 0; i < payloadLength; i++) {
       message_buff[i] = payload[i];
    }

    message_buff[i] = '\0';   
    String msgString = String(message_buff);
    
    if(msgString.substring(14,19) == "start" ){
      //client.publish(statustopic, startCommand);
      digitalWrite(LED_PIN, LOW);
      digitalWrite(RELAY_PIN, HIGH);
      currentSwitchStatus = "on";
    }
    else if (msgString.substring(14,18) == "stop"){
      //client.publish(statustopic, stopCommand);
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(RELAY_PIN, LOW);
      currentSwitchStatus = "off";
    }
}

void setup() {
  pinMode(LED_PIN,    OUTPUT);
  pinMode(RELAY_PIN,  OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(LED_PIN, HIGH);
  btn_timer.attach(0.05, button);
  client = PubSubClient(server, 1883, callback, wifiClient);
  dht.begin();
  wifiConnect();
  mqttConnect();
}

void loop() {
  if (!client.loop()) {
    mqttConnect();
  }
   publishData();
   delay(2000);
}

void publishCurrentStatus(){ 
   client.publish(statustopic, currentSwitchStatus.c_str());
}

void wifiConnect() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void mqttConnect() {
  if (!client.connected()) {
    while (!client.connect(clientId, authMethod, token)) {
      delay(500);
    }
    initManagedDevice();
  }
}

void initManagedDevice() {
  client.subscribe(cmdtopic);
}

void publishData(){
  
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    return;
  }
  
  String pl = "{\"d\":{\"Name\":\"test2\"";
  pl += ",\"relaystatus\":\"";
  pl += currentSwitchStatus;
  pl += "\",\"temperature\":\"";
  pl += t;
  pl += "\",\"humidity\":\"";
  pl += h;
  pl += "\"}}";

  client.publish(sensortopic, pl.c_str());
}

void button() {
  if (!digitalRead(BUTTON)) {
    count++;
  } 
  else {
      if(digitalRead(RELAY_PIN) == HIGH)  {
	  digitalWrite(LED_PIN, HIGH);
        digitalWrite(RELAY_PIN, LOW);
      } else {
        digitalWrite(LED_PIN, LOW);
        digitalWrite(RELAY_PIN, HIGH);
      }
    } 
    else if (count >40){
      Serial.println("\n\nSonoff Rebooting . . . . . . . . Please Wait"); 
      
    } 
    count=0;
  }
}


