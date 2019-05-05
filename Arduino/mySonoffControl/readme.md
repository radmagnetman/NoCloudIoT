# Controlling Sonoff

See hardware section for wiring diagram. 

## Command set

Commands are sent to Sonoff via MQTT commands. For Mosquitto MQTT server, this could looke like: 

`mosquitto_pub -h 192.168.1.xx -m "1" -t "xxxx/setRelay"`

where the "xx" in the ip address is the location of the MQTT server and "xxxx" in the topic is the last 4 digits of the MAC address of the Sonoff.

* `reset` - Forces the ESP32 on the Sonoff to reset. Not 100% reliable
* `toggleLEDBlink` - Sent with an empty message to toggle the LED blink on and off
* `setRelay` - Sent with a message of "0" or "1" to turn the Sonoff realy off or on
* `returnVerNum` - Sent with empty message. Sonoff then broadcasts to "xxxx/firmwareVer" topic with message containing the firmware version
* `returnRelayState` - Returns a 0 or 1 on the topic "xxxx/relayState"

Subscribing to topic `xxxx/#` (where `xxxx` is the last 4 digits of the MAC address) will allow the MQTT server to pass along all messages pertaining to this particular Sonoff
