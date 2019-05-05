import os
import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk

class ButtonWindow(Gtk.Window):

    def __init__(self):
        Gtk.Window.__init__(self, title="Button Demo")
        self.set_border_width(10)
        self.set_default_size(480*1.4,320*1.4)


        hbox = Gtk.Box(spacing=6)
        self.add(hbox)

        button = Gtk.Button.new_with_label("Backlight on")
        button.connect("clicked", self.on_lightOn_clicked)
        hbox.pack_start(button, True, True, 0)

        button = Gtk.Button.new_with_label("Backlight off")
        button.connect("clicked", self.on_lightOff_clicked)
        hbox.pack_start(button, True, True, 0)

        button = Gtk.Button.new_with_mnemonic("_Close")
        button.connect("clicked", self.on_close_clicked)
        hbox.pack_start(button, True, True, 0)

    def on_lightOn_clicked(self, button):
        print("\"Backlight on\" button was clicked")
        myCmd = "sudo sh -c 'echo '1' > /sys/class/backlight/soc\\:backlight/brightness'"
        os.system(myCmd)
    def on_lightOff_clicked(self, button):
        print("\"Backlight off\" button was clicked")
        myCmd = "sudo sh -c 'echo '0' > /sys/class/backlight/soc\\:backlight/brightness'"
        os.system(myCmd)

    def on_close_clicked(self, button):
        print("Closing application")
        Gtk.main_quit()

win = ButtonWindow()
win.connect("destroy", Gtk.main_quit)
win.show_all()
Gtk.main()
