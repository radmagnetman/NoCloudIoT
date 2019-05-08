import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk as gtk

class thisTest:
    def on_window1_destroy(self, object, data=None):
        print("quit with window x")
        gtk.main_quit()

    def aButton_clicked(self,button,data=None):
        print("Button pressed")
        self.testLabel = self.builder.get_object("testLabel")
        self.testLabel.set_text("blah")

    def __init__(self):
        self.gladefile = "test1.glade" # store the file name
        self.builder = gtk.Builder() # create an instance of the gtk.Builder
        self.builder.add_from_file(self.gladefile) # add the xml file to the Builder
        self.builder.connect_signals(self)
        self.window = self.builder.get_object("window1") # This gets the 'window1' object
        self.window.show() # this shows the 'window1' object

if __name__ == "__main__":
    main = thisTest()
    gtk.main()
