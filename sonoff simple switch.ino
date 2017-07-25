#include <ESP8266WiFi.h>
#include <PubSubClient.h> // https://github.com/knolleary/pubsubclient/releases/tag/v2.3

const char* ssid = "Roco";
const char* password = "xxxx";
char* startCommand = "{\"command\" : \"start\"}";
char* stopCommand = "{\"command\" : \"stop\"}";
String currentSwitchStatus = "{\"status\" : \"off\"}";

//HW https://www.itead.cc/smart-home/sonoff-wifi-wireless-switch.html

const uint8_t     BUTTON_PIN = 0;
const uint8_t     RELAY_PIN  = 12;
const uint8_t     LED_PIN    = 13;

//HW

#define ORG "1d9l1c"
#define DEVICE_TYPE "ESP8266"
#define DEVICE_ID "test1"
#define TOKEN "12345678"

char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
char cmdtopic[] = "iot-2/cmd/test/fmt/json";
char statustopic[] = "iot-2/evt/status/fmt/json";
char authMethod[] = "use-token-auth";
char token[] = TOKEN;
char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;
char message_buff[100];
int counter = 0;

WiFiClient wifiClient;
PubSubClient client;

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
      currentSwitchStatus = "{\"status\" : \"on\"}";
    }
    else if (msgString.substring(14,18) == "stop"){
      //client.publish(statustopic, stopCommand);
      digitalWrite(LED_PIN, HIGH);
      digitalWrite(RELAY_PIN, LOW);
      currentSwitchStatus = "{\"status\" : \"off\"}";
    }
    
        
}

void setup() {
  pinMode(LED_PIN,    OUTPUT);
  pinMode(RELAY_PIN,  OUTPUT);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(LED_PIN, HIGH);
  client = PubSubClient(server, 1883, callback, wifiClient);
  wifiConnect();
  mqttConnect();
}

void loop() {
  if (!client.loop()) {
    mqttConnect();
  }

   publishCurrentStatus();
   delay(1000);
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



