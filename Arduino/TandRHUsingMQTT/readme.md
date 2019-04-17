# Measuring temperature and RH with ESP8266 and Si7021 sensor module

See hardware section for wiring diagram. 

## Code outline 

* Sync local time (halt in setup if this fails)
* Subscribe to MQTT server
* Loop
  * Acquire T/RH from sensor
  * Format and send data via server
  * If time to sync, attempt internet time acquire

