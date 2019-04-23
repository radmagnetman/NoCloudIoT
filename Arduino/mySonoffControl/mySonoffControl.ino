/**************************************************
 * Program to turn on and off AC device via Sonoff. 
 *************************************************/
#include <ESP8266WiFi.h>  // ESP8266 Wifi library
#include <PubSubClient.h> // MQTT library

#include "WifiSettings.h"
//**************************************************
// MQTT Settings
#define mqtt_server "192.168.1.25"

static char CMD_reset[] = "xxxx/reset";
static int LEN_reset = 10;
static char CMD_toggleLEDBlink[] = "xxxx/toggleLEDBlink";
static int LEN_toggleLEDBlink = 19;
static char CMD_setRelay[] = "xxxx/setRelay";
static int LEN_setRelay = 13;
static char CMD_returnVerNum[] = "xxxx/returnVerNum";
static int LEN_returnVerNum = 17;
//**************************************************
// Global variables
static unsigned long currentms;

static byte mac[6];
static char myName[] = "xxxx\0";

static char myFirmwareVer[] = "v1.0\0";

static bool pushButtonPressed = false;
static bool ledState = false;
static bool ledToggleState = true;
static bool relayState = false;
static bool mqttConnectStatus = false;

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
// MQTT reconnect function.
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(myName)) {
      Serial.println("connected");
      // Once connected subscribe to topics
      client.subscribe(CMD_reset);
      client.subscribe(CMD_toggleLEDBlink);
      client.subscribe(CMD_setRelay);
      client.subscribe(CMD_returnVerNum);
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
static char firmwareTopic[] = "xxxx/firmwareVer\0";
void callback(char* topic, byte* thisPayload, unsigned int lPayload) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  // The reset doesn't put the ESP back in a working state if the 
  //  serial cable is still connected to the device. 
  if (strncmp(topic,CMD_reset,LEN_reset) == 0)
  {
    Serial.println("Reseting in 3 seconds...");
    delay(3000);
    ESP.restart();
  }
  else if (strncmp(topic,CMD_setRelay,LEN_setRelay) == 0)
  {
    for (int i = 0; i < lPayload; i++) {
      receivedChar = (char)thisPayload[0];
      Serial.print(receivedChar);
      if (receivedChar == '0')
      {
        digitalWrite(relayPin,LOW);
        relayState = false;
      }
      if (receivedChar == '1')
      {
        digitalWrite(relayPin,HIGH);
        relayState = true;
      }
    }
  }
  else if (strncmp(topic,CMD_toggleLEDBlink,LEN_toggleLEDBlink) == 0)
  {
    receivedChar = (char)thisPayload[0];
    ledToggleState = !ledToggleState;
  }
  else if (strncmp(topic,CMD_returnVerNum,LEN_returnVerNum) == 0)
  {
    Serial.print("My firmware version number is ");
    Serial.println(myFirmwareVer);
    for (int i = 0;i < 4;i++)
      firmwareTopic[i] = myName[i];
    Serial.println(firmwareTopic);
    client.publish(firmwareTopic, myFirmwareVer, true);
  }
  else
  {Serial.println("Unrecognized command via MQTT: ");}
  Serial.println();
}
//**************************************************
// Called via pushbutton interrupt, using latch logic
//  mechanism to prevent switch bounce from interfering 
void capturePushButton() {
  pushButtonPressed = true;
  detachInterrupt(digitalPinToInterrupt(pushButtonPin));
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
  // Copy over myName to command set.
  for (int i = 0;i < 4;i++)
  {
    CMD_reset[i] = myName[i];
    CMD_toggleLEDBlink[i] = myName[i];
    CMD_setRelay[i] = myName[i];
    CMD_returnVerNum[i] = myName[i];
  }
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
unsigned long prevreconnect_ms = 0;
const long reconnectRate = 5000;


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
    prevCheckButton_ms = currentms;
    if (pushButtonPressed == true)
    {
      handleButtonPress();
    }
  }

  // Reconnect to MQTT server every 5 seconds
  if (currentms - prevreconnect_ms >= reconnectRate)
  {
    prevreconnect_ms = currentms;
    if (!client.connected()) {
      reconnect();
    }
    client.loop();
  }
}

//-------------------------------
// 
void handleButtonPress() {
  if (relayState)
  {
    digitalWrite(relayPin,LOW);
    Serial.println("[Button] Relay OFF");
  }
  else
  {
    digitalWrite(relayPin,HIGH);
    Serial.println("[Button] Relay ON");
  }
  
  relayState = !relayState;
  attachInterrupt(digitalPinToInterrupt(pushButtonPin), capturePushButton, CHANGE);
  pushButtonPressed = false;
  broadcastRelayState();  
}

// Toggle the LED
void handleBlink() {
  if (ledToggleState == true)
    ledState = !ledState;
  else
    ledState = true;
  digitalWrite(ledPin, ledState);
}

static char relayPayload[] = "x\0";
static char relayTopic[] = "xxxx/relayState\0";
void broadcastRelayState()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  if (relayState)
    relayPayload[0] = '1';
  else
    relayPayload[0] = '0';

  for (int i = 0;i < 4;i++)
    {relayTopic[i] = myName[i];}
  
  client.publish(relayTopic, relayPayload, true);
}


//=============================================================
