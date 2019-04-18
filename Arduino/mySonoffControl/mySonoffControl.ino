/**************************************************
 * Program to turn on and off AC device via Sonoff. 
 *************************************************/
#include <ESP8266WiFi.h>  // ESP8266 Wifi library
#include <PubSubClient.h> // MQTT library

#include "WifiSettings.h"
//**************************************************
// Static location of my MQTT server
#define mqtt_server "192.168.1.25"

//**************************************************
// Global variables

static unsigned long currentms;

static byte mac[6];
static char myName[] = "xxxx\0";

static bool pushButtonPressed = false;
static bool ledState = false;
static byte ledToggleState = 1;

//**************************************************
// Hardware parameters 
const byte ledPin = 13;
const byte pushButtonPin = 0;
const byte relayPin = 12;
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
// Called via pushbutton interrupt, using latch logic
//  mechanism to prevent switch bounce from interfering 
void capturePushButton() {
  pushButtonPressed = true;
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
      client.subscribe("toggleS");
      client.subscribe("reset");
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
void callback(char* topic, byte* thisPayload, unsigned int lPayload) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  // The reset doesn't put the ESP back in a working state if the 
  //  serial cable is still connected to the device. 
  if (strncmp(topic,"reset",5) == 0)
  {
    Serial.println("Reseting ESP in 3 seconds...");
    delay(3000);
    ESP.restart();
  }
  else if (strncmp(topic,"toggleS",7) == 0)
  {
    for (int i = 0; i < lPayload; i++) {
      char receivedChar = (char)thisPayload[i];
      Serial.print(receivedChar);
      if (receivedChar == '0')
        digitalWrite(relayPin,LOW);
      if (receivedChar == '1')
        digitalWrite(relayPin,HIGH);
    }
    Serial.println();
  }
  else
  {Serial.println("Unrecognized command via MQTT: ");}
  
}

//=============================================================
void setup() {
  // Establish arduino specifics
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin,OUTPUT);
  digitalWrite(relayPin,LOW);
  attachInterrupt(digitalPinToInterrupt(pushButtonPin), capturePushButton, CHANGE);

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

//=============================================================
// Timing variables
unsigned long prevBlink_ms = 0;
const long blinkRate = 250;
unsigned long prevCheckButton_ms = 0;
const int checkButtonRate = 1000;
unsigned long lastPress_ms = 0;
unsigned long prevSynchTime_ms = 0;

void loop() {
  // get current time tick
  currentms = millis();

  // Blink LED
  if (currentms - prevBlink_ms >= blinkRate)
  {
    prevBlink_ms = currentms;
    handleBlink();
  }


  // Check to see if push button is pressed, timing on this
  //  is such as to prevent switch bounce from triggering 
  //  multiple calls.
  if (currentms - prevCheckButton_ms >= checkButtonRate)
  {
    if (pushButtonPressed == true)
    {
      lastPress_ms = currentms;
    }
  }
}

//-------------------------------
// Function doesn't do anything right now, but if push button is
//  pressed this is where interrupt work goes
void handleSubscribe() {
  pushButtonPressed = false;
}

// Toggle the LED
void handleBlink() {
  if (ledToggleState == 1)
    ledState = !ledState;
  else
    ledState = false;
  digitalWrite(ledPin, ledState);
}


//=============================================================
