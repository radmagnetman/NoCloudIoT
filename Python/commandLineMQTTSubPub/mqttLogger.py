# Working on a logger.
# cntrl-/ for block comment

import time
import paho.mqtt.client as paho
broker="192.168.1.25"

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
    if rc == 0:
        print("Connected to broker")
        global Connected                #Use global variable
        Connected = True                #Signal connection
    else:
        print("Connection failed")

Connected = False   #global variable for the state of the connection

#########################################################################
subscriptionList = updateSubscriptions()
print(subscriptionList)

client= paho.Client("myClient")
client.on_connect = on_connect
client.on_message = on_message


client.loop_start() #start loop to process received messages
while Connected != True:    #Wait for connection
    client.connect(broker)#connect
    time.sleep(0.1)


print("subscribing ")
for i in range(len(subscriptionList)):
    client.subscribe(subscriptionList[i]) #subscribe
time.sleep(2)
for i in range(100):
    time.sleep(.1)
#client.loop_forever()
client.loop_stop()
print("Finished")

# client.loop_forever()
# client.loop_stop() #stop loop
