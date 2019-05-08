import paho.mqtt.client as mqtt  #import the client1
import threading
import os
import time

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk as gtk


broker="192.168.1.25"


class bedSideDisplay:
    def on_window1_destroy(self, object, data=None):
        print("quit with window x")
        gtk.main_quit()

    def onLightToggle_click(self, button, data=None):
        print("Toggling light button")
        global lightOnOff
        if(lightOnOff == 0):
            print("Turning light on")
            client.publish("5EA9/setRelay","1")
            lightOnOff = 1
        else:
            print("Turning light off")
            client.publish("5EA9/setRelay","0")
            lightOnOff = 0

    def updateLabel_clicked(self, button, data=None):
        print("Update button pressed")
        self.label1 = self.builder.get_object("label1")
        self.label1.set_label("blah")
        print("Finished")

    def udpateLabel(self):
        print("Updating label begin")
        self.mqttResponse = self.builder.get_object("mqttResponse")
        self.mqttResponse.set_text("something")
        print("Updating label end")

    def closeButton_clicked(self,button,data=None):
        print("Quit via Close button")
        gtk.main_quit()

  # This is our init part where we connect the signals
    def __init__(self):
        self.gladefile = "gladePrototype.glade" # store the file name
        self.builder = gtk.Builder() # create an instance of the gtk.Builder
        self.builder.add_from_file(self.gladefile) # add the xml file to the Builder
        self.builder.connect_signals(self)
        self.window = self.builder.get_object("window1") # This gets the 'window1' object
        self.window.show() # this shows the 'window1' object

def on_connect(client, userdata, flags, rc):
    if rc == 0:
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
        print("Connection failed")

def on_message(client, userdata, message):
    print(message.topic," ",message.payload.decode("utf-8"))
    Glib.idle_add(udpateLabel)
    # glib.idle_add(idle)
    #self.result1.set_text(self.rpm)

# Read sensorList and update sensor subscriptions
def updateSubscriptions():
    subscriptionListFile = open("subscriptionList.txt","r")
    subscriptionList = subscriptionListFile.readlines()
    for i in range(len(subscriptionList)):
        subscriptionList[i] = subscriptionList[i].rstrip("\n\r")
    return subscriptionList

###############################################################################
lightOnOff = 0
Connected = False

if __name__ == "__main__":

    client = mqtt.Client("bedDisplay")             #create new instance
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

    main = bedSideDisplay() # create an instance of our class
    gtk.main() # run the darn thing

    client.loop_stop()    #Stop loop
    client.disconnect() # disconnect
