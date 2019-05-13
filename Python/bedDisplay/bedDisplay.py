import paho.mqtt.client as mqtt  #import the client1
import threading
import os
import time
import csv

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk as gtk


broker="192.168.1.25"

######################################################################

class bedDisplay:
    def on_window1_destroy(self, object, data=None):
        print("Exit with window x")
        gtk.main_quit()

    def on_button_goodNight_clicked(self, button, data=None):
        print("Goodnight clicked")
        with open('./commandSets/cmds_goodNight.csv') as csv_file:
            csvReader = csv.reader(csv_file, delimiter=',')
            lineCount = 0
            for row in csvReader:
                client.publish(row[0],row[1])
                lineCount += 1
            print("[MQTT] Published "+str(lineCount)+" commands.")


    def on_button_goodMorning_clicked(self, button, data=None):
        print("Good morning clicked")
        with open('./commandSets/cmds_goodMorning.csv') as csv_file:
            csvReader = csv.reader(csv_file, delimiter=',')
            lineCount = 0
            for row in csvReader:
                client.publish(row[0],row[1])
                lineCount += 1
            print("[MQTT] Published "+str(lineCount)+" commands.")

    def on_button_more_clicked(self, button, data=None):
        print("More button clicked")
        print("Exit with 'more' button pressed. Update this method later!")
        gtk.main_quit()

    def on_button_investigate_clicked(self, button, data=None):
        print("Investigate button clicked")

    def __init__(self):
        self.gladefile = "bedDisplay.glade" # store the file name
        self.builder = gtk.Builder() # create an instance of the gtk.Builder
        self.builder.add_from_file(self.gladefile) # add the xml file to the Builder
        self.builder.connect_signals(self)
        self.window = self.builder.get_object("window1") # This gets the 'window1' object
        self.window.show() # this shows the 'window1' object

#---------------------------------------------------------------------
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        client.connected_flag=True #set flag
        print("connected OK")
        subscriptionList = updateSubscriptions()
        #print(subscriptionList)
        for x in subscriptionList:
            print("Subscribing to: " + x)
            client.subscribe(x)
        print("Finished subscribing")
    else:
        print("Connection failed")

def on_message(client, userdata, message):
    print(message.topic," ",message.payload.decode("utf-8"))
    #Glib.idle_add(udpateLabel)
    # glib.idle_add(idle)
    #self.result1.set_text(self.rpm)

# Read sensorList and update sensor subscriptions
def updateSubscriptions():
    subscriptionListFile = open("subscriptionList.txt","r")
    subscriptionList = subscriptionListFile.readlines()
    for i in range(len(subscriptionList)):
        subscriptionList[i] = subscriptionList[i].rstrip("\n\r")
    return subscriptionList
######################################################################

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


    main = bedDisplay() # create an instance of our class
    gtk.main() # run the darn thing

    for x in subscriptionList:
        print("Clearing: "+x)
        client.publish(x,"\0")

    client.loop_stop()    #Stop loop
    client.disconnect() # disconnect
