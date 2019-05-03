# GUI on a Pi TFT display

## Hardware

* Rasperry Pi B+

* [3.5" TFT Display](https://www.adafruit.com/product/2097)

## Getting started

[TFT installer](https://learn.adafruit.com/adafruit-pitft-3-dot-5-touch-screen-for-raspberry-pi/easy-install-2)

Above seems to bork GUI software. Temporary fix: `sudo mv /usr/share/X11/xorg.conf.d/99-fbturbo.conf ~` ([source](https://github.com/hypriot/x11-on-HypriotOS/issues/5)) before executing `startx`.

After GUI desktop is running on Pi run locally `sudo xhost +local:pi`. Then from SSH run `export DISPLAY=:0.0` before python script ([source](https://www.raspberrypi.org/forums/viewtopic.php?t=124021)). 

Unsure how much of this is persistent. 
