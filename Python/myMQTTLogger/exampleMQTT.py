#!python3
import paho.mqtt.client as mqtt  #import the client1
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
    else:
        print("Bad connection Returned code=",rc)

mqtt.Client.connected_flag=False#create flag in class
broker="192.168.1.25"
client = mqtt.Client("python1")             #create new instance
client.on_connect=on_connect  #bind call back function
client.loop_start()
print("Connecting to broker ",broker)
client.connect(broker)      #connect to broker
while not client.connected_flag: #wait in loop
    print("In wait loop")
    time.sleep(1)
print("in Main Loop")
client.loop_stop()    #Stop loop
client.disconnect() # disconnect
