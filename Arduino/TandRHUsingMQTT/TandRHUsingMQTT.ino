/**************************************************
 * Program to get T/RH data from sensor package and 
 * broadcast in via MQTT from an ESP8266. 
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

// Sensor board library
#include "SparkFun_Si7021_Breakout_Library.h"

// Rename 'WifiSettingsTemplate.h' file to 'WifiSettings.h' 
//  and edit file with appropriate wifi settings
#include "WifiSettings.h"
//**************************************************
// Static location of my MQTT server
#define mqtt_server "192.168.1.25"

// Used to get and convert internet time to local time
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "us.pool.ntp.org"  
//**************************************************
// Global variables
static float RH;
static float T;

static unsigned long currentms;
static unsigned long lastSync;

static byte mac[6];
static char myName[] = "xxxx\0";

static bool pushButtonPressed = false;
static bool ledState = false;
static byte ledToggleState = 1;

//**************************************************
// Hardware parameters 
Weather sensor;
const byte ledPin = 13;
const byte pushButtonPin = 14;
//**************************************************
// Network declarations
WiFiClient espClient;
PubSubClient client(espClient);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

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
    Serial.print(" ");Serial.println(ampm[isPM(localTime)]);
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
      client.subscribe("ledToggle");
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
  else if (strncmp(topic,"ledToggle",9) == 0)
  {
    for (int i = 0; i < lPayload; i++) {
      char receivedChar = (char)thisPayload[i];
      Serial.print(receivedChar);
      if (receivedChar == '0')
        ledToggleState = 0;
      if (receivedChar == '1')
        ledToggleState = 1;
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

  // Establish local time
  currentms = millis();
  while (!establishLocalTime())
  {
    Serial.println("Attempting again in 5 seconds");
    delay(5000);
  }
  
  
  // Establish MQTT
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  // Establish SI7021
  sensor.begin();
}

//=============================================================
// Timing variables
unsigned long prevBlink_ms = 0;
const long blinkRate = 250;
unsigned long prevCheckButton_ms = 0;
const int checkButtonRate = 1000;
unsigned long prevBroadcast_ms = 0;
const long broadcastRate = 3000;
unsigned long lastPress_ms = 0;
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
  }

  // Get T/RH and send to MQTT server
  if (currentms - prevBroadcast_ms >= broadcastRate)
  {
    prevBroadcast_ms = currentms;
    handleBroadcast();
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

//**************************************************
// Get, format, and send data for MQTT
//  Character arrays used over Strings for stability
static char payloadVal[] = "xxx.xx\0";        
static char payloadTime[] = "xxxxxxxxxx\0";
static char payloadT[] = "xxxxxxxxxx xxx.xx\0";
static char payloadRH[] = "xxxxxxxxxx xxx.xx\0";
static int timeSinceLastSync;

void handleBroadcast() {
  // Get sensor data
  RH = sensor.getRH();
  T = sensor.getTemp();
  Serial.print(" T "); Serial.println(T);
  Serial.print("RH "); Serial.println(RH);

  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  // Convert local stored time to character string
  timeSinceLastSync = currentms/1000-lastSync;
  itoa(localTime+timeSinceLastSync,payloadTime,10);

  // Convert temperature value to string and write to 
  //  array. Insert converted time stamp into same array.
  //  Publish string to MQTT server
  dtostrf(T, 7, 2, payloadVal);
  for (int i = 0;i < 11;i++) {
    payloadT[i] = payloadTime[i];
  }
  for (int i = 0;i < 7;i++) {
    payloadT[i+10] = payloadVal[i];
  }
  client.publish("testSensors/T", payloadT, true);

  // Convert RH value to string and write to 
  //  array. Insert converted time stamp into same array
  //  Publish string to MQTT server
  dtostrf(RH, 7, 2, payloadVal);
  for (int i = 0;i < 11;i++) {
    payloadRH[i] = payloadTime[i];
  }
  for (int i = 0;i < 7;i++) {
    payloadRH[i+10] = payloadVal[i];
  }
  client.publish("testSensors/RH", payloadRH, true);
}

//=============================================================
