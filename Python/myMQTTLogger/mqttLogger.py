# Working on a logger.

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
    time.sleep(1)
    print("received message from {} = {}".format(message.topic.decode("utf-8"),message.payload.decode("utf-8")))
    # print('We are the {} who say "{}!"'.format('knights', 'Ni'))

#########################################################################
subscriptionList = updateSubscriptions()
print(subscriptionList)

client= paho.Client("client-001")
#create client object client1.on_publish = on_publish
#assign function to callback client1.connect(broker,port)
#establish connection client1.publish("house/bulb1","on")

######Bind function to callback
client.on_message=on_message
#####
print("connecting to broker ",broker)
client.connect(broker)#connect
client.loop_start() #start loop to process received messages
print("subscribing ")
for i in range(len(subscriptionList)):
    client.subscribe(subscriptionList[i]) #subscribe
time.sleep(2)
# print("publishing ")
#client.publish("house/bulb1","on")#publish
#time.sleep(4)
#client.disconnect() #disconnect
client.loop_forever() #stop loop
