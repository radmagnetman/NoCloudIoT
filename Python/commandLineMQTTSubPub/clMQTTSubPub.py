#!python3
import paho.mqtt.client as mqtt  #import the client1
import threading
import time

# Read sensorList and update sensor subscriptions
def updateSubscriptions():
    subscriptionListFile = open("subscriptionList.txt","r")
    subscriptionList = subscriptionListFile.readlines()
    for i in range(len(subscriptionList)):
        subscriptionList[i] = subscriptionList[i].rstrip("\n\r")
    return subscriptionList

#define callback
def on_message(client, userdata, message):
    print(message.topic," ",message.payload.decode("utf-8"))

def on_connect(client, userdata, flags, rc):
    if rc==0:
        client.connected_flag=True #set flag
        print("connected OK")
        subscriptionList = updateSubscriptions()
        #print(subscriptionList)
        for x in subscriptionList:
            print("Subscribing to: " + x)
            client.subscribe(x)
            #time.sleep(.5)
        print("Finished subscribing")

    else:
        print("Bad connection Returned code=",rc)

#def subscribe

#Connected = False
#mqtt.Client.connected_flag=False#create flag in class
broker="192.168.1.25"
client = mqtt.Client("python1")             #create new instance
client.connected_flag = False
client.on_message = on_message
client.on_connect=on_connect  #bind call back function
client.loop_start()
subscriptionList = updateSubscriptions()
while client.connected_flag != True:    #Wait for connection
    client.connect(broker)#connect
    time.sleep(0.1)
print("Finished connecting")
time.sleep(2)
print("")
print("Select option")
print("1. Return version")
print("2. Return relay state")
print("3. Toggle LED blink")
print("4. Set relay")
print("5. Quit")
text = '0'
while True:
    text = input("> ")
    if text[0] == '1':
        print("Requesting firmware version")
        client.publish("5EA9/firmwareVer","")
    elif text[0] == '2':
        print("Requesting relay state")
        client.publish("5EA9/returnRelayState","")
    elif text[0] == '3':
        print("Toggling LED blink")
        client.publish("5EA9/toggleLEDBlink","")
    elif text[0] == '4':
        relaySetTo = input("Set relay to (0/1): ")
        client.publish("5EA9/setRelay",relaySetTo)
    elif text[0] == '5':
        print("Exiting")
        break
    else:
        print("Invalid option")

#for i in range(1000):
#    time.sleep(.1)

client.loop_stop()    #Stop loop
client.disconnect() # disconnect
