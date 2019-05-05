import paho.mqtt.client as paho
import os
import gi
import time
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

broker="192.168.1.25"


def on_connect(client, userdata, flags, rc):
    if rc==0:
        client.connected_flag=True #set flag
        print("connected OK")
    else:
        print("Bad connection Returned code=",rc)

class ButtonWindow(Gtk.Window):

    def __init__(self):
        Gtk.Window.__init__(self, title="Button Demo")
        self.set_border_width(10)
        self.set_default_size(1920,1280)


        hbox = Gtk.Box(spacing=6)
        self.add(hbox)

        button = Gtk.Button.new_with_label("Toggle light")
        button.connect("clicked", self.on_toggleLight_clicked)
        hbox.pack_start(button, True, True, 0)

        button = Gtk.Button.new_with_label("Backlight off")
        button.connect("clicked", self.on_lightOff_clicked)
        hbox.pack_start(button, True, True, 0)

        button = Gtk.Button.new_with_mnemonic("_Close")
        button.connect("clicked", self.on_close_clicked)
        hbox.pack_start(button, True, True, 0)

    def on_toggleLight_clicked(self, button):
        # mosquitto_pub -h 192.168.1.25 -m "1" -t "5EA9/setRelay"
        global lightOnOff
        connectToMQTT()
        print(lightOnOff)
        if(lightOnOff == 0):
            print("Turning light on")
            client.publish("5EA9/setRelay","1")
            lightOnOff = 1
        else:
            print("Turning light off")
            client.publish("5EA9/setRelay","0")
            lightOnOff = 0

        client.loop_stop()


    def on_lightOff_clicked(self, button):
        print("\"Backlight off\" button was clicked")
        #myCmd = 'sudo sh -c \'echo \"1\" \> \/sys\/class\/backlight\/soc\\\:backlight\/brightness'
        myCmd = "sudo sh -c 'echo '1' > /sys/class/backlight/soc\\:backlight/brightness'"
        os.system(myCmd)

    def on_close_clicked(self, button):
        print("Closing application")
        Gtk.main_quit()

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to broker")
        global Connected                #Use global variable
        Connected = True                #Signal connection
    else:
        print("Connection failed")

def on_message(client, userdata, message):
    print(message.topic," ",message.payload.decode("utf-8"))

def connectToMQTT():
    client.loop_start() #start loop to process received messages
    while Connected != True:    #Wait for connection
        client.connect(broker)#connect
        time.sleep(0.1)

Connected = False   #global variable for the state of the connection
lightOnOff = 0

client= paho.Client("myClient")
client.on_connect = on_connect
client.on_message = on_message


client.loop_start() #start loop to process received messages
while Connected != True:    #Wait for connection
    client.connect(broker)#connect
    time.sleep(0.1)

client.loop_stop()

win = ButtonWindow()
win.connect("destroy", Gtk.main_quit)
win.show_all()
Gtk.main()
