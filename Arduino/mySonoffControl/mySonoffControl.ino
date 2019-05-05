/**************************************************
 * Program to turn on and off AC device via Sonoff. 
 *************************************************/
#include <ESP8266WiFi.h>  // ESP8266 Wifi library
#include <PubSubClient.h> // MQTT library

// These libraries are used to synch the controller time with
// internet time
#include <WifiUDP.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>


#include "WifiSettings.h"
//**************************************************
// MQTT Settings
#define mqtt_server "192.168.1.25"
// Used to get and convert internet time to local time
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "us.pool.ntp.org"  

// MQTT Commands
// Each command is made up of a topic where the first 
//  4 characters are the last 4 digits of the MAC 
//  address (array is updated after MAC address is
//  determined) followed by the command.
//  LEN_ variables are the number of characters in 
//  each topic
static char CMD_reset[] = "xxxx/reset";
static int LEN_reset = 10;
static char CMD_toggleLEDBlink[] = "xxxx/toggleLEDBlink";
static int LEN_toggleLEDBlink = 19;
static char CMD_setRelay[] = "xxxx/setRelay";
static int LEN_setRelay = 13;
static char CMD_returnVerNum[] = "xxxx/returnVerNum";
static int LEN_returnVerNum = 17;
static char CMD_returnRelayState[] = "xxxx/returnRelayState";
static int LEN_returnRelayState = 21;

//**************************************************
// Global variables
static unsigned long currentms; 
static unsigned long lastSync;

static byte mac[6];             
static char myName[] = "xxxx\0";

static char myFirmwareVer[] = "v1.1\0";

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
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

//**************************************************
// Helper function to convert mac address bytes to chars
char convertHexToChar(byte in) {
  if (in < 10)
    return in+48;
   else
    return in+55;
}

//**************************************************
// Internet time
// See simpleGetTimeExample for better descriptions of what's going
//  on in this function.
static unsigned long epochTime;
static time_t localTime, utc;
static TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -300};  //UTC - 5 hours - change this as needed
static TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -360};   //UTC - 6 hours - change this as needed
Timezone usEastern(usEDT, usEST);
const char * ampm[] = {"AM", "PM"} ;
bool establishLocalTime() {
  if (WiFi.status() == WL_CONNECTED) { 
    Serial.println("Getting internet time");  
    timeClient.update();
    epochTime =  timeClient.getEpochTime();
    utc = epochTime;
    localTime = usEastern.toLocal(utc);
    Serial.print("Current time is ");Serial.print(hourFormat12(localTime));
    Serial.print(":");Serial.print(minute(localTime));
    Serial.print(":");Serial.print(second(localTime));
    Serial.print(" ");Serial.print(ampm[isPM(localTime)]);
    Serial.print(" (");Serial.print(localTime);Serial.println(")");
    lastSync = currentms/1000;
    return true;
  }
  else 
  {
    Serial.println("Getting internet time failed");
    return false;
  }
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
      client.subscribe(CMD_returnRelayState);
      //client.subscribe(CMD_aliveNotice);
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
static char stateTopic[] = "xxxx/relayState";
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
    broadcastRelayState();
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
  else if (strncmp(topic,CMD_returnRelayState,LEN_returnRelayState) == 0)
  {
    static char statePayload[] = "x\0";
    Serial.print("Relay state is ");
    if (relayState)
    {Serial.println("ON");statePayload[0] = '1';}
    else
    {Serial.println("OFF");statePayload[0] = '0';}
    for (int i = 0;i < 4;i++)
      stateTopic[i] = myName[i];
    client.publish(stateTopic, statePayload, true);
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
  Serial.begin(115200);
  Serial.println("Powering up...");
  pinMode(ledPin, OUTPUT);
  pinMode(relayPin,OUTPUT);
  digitalWrite(relayPin,LOW);
  attachInterrupt(digitalPinToInterrupt(pushButtonPin), capturePushButton, CHANGE);

  // Establish wifi credentials
  const char* wifiName = MyWifiName;
  const char* wifiPass = MyWifiPass;

  Serial.print("Connecting to ");
  Serial.println(wifiName);
  Serial.print("Using ");
  Serial.println(wifiPass);

  WiFi.begin(wifiName, wifiPass);


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    //Serial.println(WiFi.status());
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
    CMD_returnRelayState[i] = myName[i];
  }
  Serial.println(CMD_setRelay);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  currentms = millis();
  while(!broadcastAlive())
    {establishLocalTime();}
  
}

//=============================================================
// Timing variables
unsigned long prevBlink_ms = 0;
const long blinkRate = 250;
unsigned long prevCheckButton_ms = 0;
const int checkButtonRate = 1000;
unsigned long lastPress_ms = 0;
unsigned long prevreconnect_ms = 0;
const long reconnectRate = 5000;
unsigned long prevSynchTime_ms = 0;
const int synchTimeRate = 5*60*1000;

void loop() {
  // get current time tick
  currentms = millis();

  // Blink LED
  if (currentms - prevBlink_ms >= blinkRate)
  {
    prevBlink_ms = currentms;
    handleBlink();
  }

// Sync time
  if (currentms - prevSynchTime_ms >= synchTimeRate)
  {
    prevSynchTime_ms = currentms;
    establishLocalTime();
    while(!broadcastAlive())
      {establishLocalTime();}
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

// Return relay state
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

// Broadcast I'm alive message
static char alivePayload[] = "xxxxxxxxxx\0";
static char aliveTopic[] = "lastCheckin/xxxx\0";
static int timeSinceLastSync;
bool broadcastAlive()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Convert local stored time to character string
  timeSinceLastSync = currentms/1000-lastSync;
  itoa(localTime+timeSinceLastSync,alivePayload,10);

  for (int i = 0;i < 4;i++)
    {aliveTopic[i+12] = myName[i];}
  
  //for (int i = 0;i < 10;i++)
  //  {alivePayload[i] = myName[i];}

  Serial.print("  Topic: ");Serial.println(aliveTopic);
  Serial.print("Payload: ");Serial.println(alivePayload);

  // Occasionally the number returned from time sync is malformed and
  //  low. This is just an arbitrary cap to prevent transmission in such 
  //  cases.
  if(localTime+timeSinceLastSync < 1557051)
  {return false;}
  else
  {client.publish(aliveTopic, alivePayload, true);return true;}
}
//=============================================================
