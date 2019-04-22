/**************************************************
 * Framework for MQTT
 *************************************************/
#include <ESP8266WiFi.h>  // ESP8266 Wifi library
#include <PubSubClient.h> // MQTT library

#include "WifiSettings.h"
//**************************************************
// MQTT settings
#define mqtt_server "192.168.1.25"
static char topic1[] = "topic1";
static int len_topic1 = 6;
static char topic2[] = "topic2";
static int len_topic2 = 6;

//**************************************************
// Global variables
static unsigned long currentms;

static byte mac[6];
static char myName[] = "xxxx\0";


//**************************************************
// Hardware parameters 
const byte ledPin = 13;
static bool ledState = false;
static bool ledToggleState = true;

//**************************************************
// Network declarations
WiFiClient espClient;
PubSubClient client(espClient);

//**************************************************
// Helper function to convert mac address bytes to chars
char convertHexToChar(byte in) {
  if (in < 10)
    return in+48;
   else
    return in+55;
}

//**************************************************
// MQTT reconnect function.
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Once connected subscribe to topics
      client.subscribe(topic1);
      client.subscribe(topic2);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//**************************************************
// Called when MQTT message arrives
// Parse commands in this function.
static char receivedChar;
void callback(char* topic, byte* thisPayload, unsigned int lPayload) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  // Handle each topic 
  if (strncmp(topic,topic1,len_topic1) == 0)
  {
    // Do something for topic1
  }
  else if (strncmp(topic,topic2,len_topic2) == 0)
  {
    for (int i = 0; i < lPayload; i++) {
      receivedChar = (char)thisPayload[0];
      Serial.print(receivedChar);
      if (receivedChar == '0')
      {
        // turn something off
      }
      if (receivedChar == '1')
      {
        // turn something on
      }
    }
  }
  else
  {Serial.println("Unrecognized command via MQTT: ");}
  Serial.println();
}

//=============================================================
void setup() {
  // Establish arduino specifics
  Serial.begin(9600);

  // Establish wifi credentials
  const char* wifiName = MyWifiName;
  const char* wifiPass = MyWifiPass;

  Serial.print("Connecting to ");
  Serial.println(wifiName);

  WiFi.begin(wifiName, wifiPass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[0], HEX); Serial.print(":");
  Serial.print(mac[1], HEX); Serial.print(":");
  Serial.print(mac[2], HEX); Serial.print(":");
  Serial.print(mac[3], HEX); Serial.print(":");
  Serial.print(mac[4], HEX); Serial.print(":");
  Serial.println(mac[5], HEX);

  myName[0] = convertHexToChar((mac[4] & B11110000) >> 4);
  myName[1] = convertHexToChar(mac[4] & B00001111);
  myName[2] = convertHexToChar((mac[5] & B11110000) >> 4);
  myName[3] = convertHexToChar(mac[5] & B00001111);
  Serial.print("My ID is ");Serial.println(myName);

  // Establish MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}

//**************************************************
unsigned long prevBlink_ms = 0;
const long blinkRate = 250;
unsigned long prevReconnect_ms = 0;
const long reconnectRate = 3000;

void loop() {
    currentms = millis();

  // Blink LED
  if (currentms - prevBlink_ms >= blinkRate)
  {
    prevBlink_ms = currentms;
    handleBlink();
  }

  // Every 3 seconds, check to see if connection to MQTT
  // is still active. If not, reconnect.
  // Send a simple MQTT message with the following
  //    Topic: '[macAddress]/sendData'
  //  Payload: 'Some text 10'
  // Here the last 4 characters replace [macAddress] and 
  //  the payload is static.
  if (currentms - prevReconnect_ms >= reconnectRate)
  {
    prevReconnect_ms = currentms;
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
    sendMQTT();
  }

}

//**************************************************
// Toggle the LED
void handleBlink() {
  if (ledToggleState == true)
    ledState = !ledState;
  else
    ledState = true;
  digitalWrite(ledPin, ledState);
}

//**************************************************
// Called when MQTT message arrives
// Parse commands in this function.
static char payloadToSend[] = "Some text xx\0";
static char topicToSend[] = "xxxx/sendData\0";
void sendMQTT()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  for (int i = 0;i < 4; i++)
     topicToSend[i] = myName[i];

  payloadToSend[10] = '1';
  payloadToSend[11] = '0';

  client.publish(topicToSend, payloadToSend, true);
}
