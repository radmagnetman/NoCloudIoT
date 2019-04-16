/****************************************************
 * This is my version to get internet time for an
 * ESP8266. The code was lifted and modified from
 * https://www.instructables.com/id/Simplest-ESP8266-Local-Time-Internet-Clock-With-OL/
 * 
 * Replace your wifi credentials 
 */
#include <ESP8266WiFi.h>
#include <WifiUDP.h>
#include <NTPClient.h>
#include <Time.h>
#include <TimeLib.h>
#include <Timezone.h>

// Define NTP properties
#define NTP_OFFSET   60 * 60      // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "us.pool.ntp.org"  // change this to whatever pool is closest (see ntp.org)

// Set up the NTP UDP client
WiFiUDP ntpUDP;
WiFiClient espClient;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);

void setup() {
  Serial.begin(9600);

  const char* wifiName = "yourWifiSSID";
  const char* wifiPass = "yourWifiPassPhrase";
  
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

}

const char * days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"} ;
const char * months[] = {"Jan", "Feb", "Mar", "Apr", "May", "June", "July", "Aug", "Sep", "Oct", "Nov", "Dec"} ;
const char * ampm[] = {"AM", "PM"} ;

void loop() {
  //Check WiFi connection status
  if (WiFi.status() == WL_CONNECTED) { 
    // update the NTP client and get the UNIX UTC timestamp 
    timeClient.update();
    unsigned long epochTime =  timeClient.getEpochTime();

    // convert received time stamp to time_t object
    time_t local, utc;
    utc = epochTime;
    
    // Then convert the UTC UNIX timestamp to local time
    TimeChangeRule usEDT = {"EDT", Second, Sun, Mar, 2, -300};  //UTC - 5 hours - change this as needed
    TimeChangeRule usEST = {"EST", First, Sun, Nov, 2, -360};   //UTC - 6 hours - change this as needed
    Timezone usEastern(usEDT, usEST);
    local = usEastern.toLocal(utc);
    Serial.print(" Local: ");Serial.println(local);
    Serial.print("  Days: ");Serial.print(weekday(local));Serial.print(" ");Serial.println(days[weekday(local)-1]);
    Serial.print("Months: ");Serial.print(month(local));Serial.print(" ");Serial.println(months[month(local)-1]);
    Serial.print("   Day: ");Serial.println(day(local));
    Serial.print("  Year: ");Serial.println(year(local));
    Serial.print("Hour12: ");Serial.println(hourFormat12(local));
    Serial.print("  Hour: ");Serial.println(hour(local));
    Serial.print("   Min: ");Serial.println(minute(local));
    Serial.print("   Sec: ");Serial.println(second(local));
    Serial.print(" AM/PM: ");Serial.println(ampm[isPM(local)]);
    
  }

  delay(2000);
}
