# GUI on a Pi TFT display

## NOTE: Code may be broken

The code in this section is, at this point, just me messing around with Glade and Python GUIs. A lot of it is broken.

Now that I know what I'm doing (sort of), going forward I will be copying various parts over to a here (link coming soon) with proper version control and documentation.

## Hardware

* Rasperry Pi B+

* [3.5" TFT Display](https://www.adafruit.com/product/2097)

## Getting started

Below may not apply any more. Fresh install then following the directions at the top of this [script](https://github.com/adafruit/Raspberry-Pi-Installer-Scripts/blob/master/adafruit-pitft.sh) made it work.

[TFT installer](https://learn.adafruit.com/adafruit-pitft-3-dot-5-touch-screen-for-raspberry-pi/easy-install-2)

Above seems to bork GUI software. Temporary fix: `sudo mv /usr/share/X11/xorg.conf.d/99-fbturbo.conf ~` ([source](https://github.com/hypriot/x11-on-HypriotOS/issues/5)) before executing `startx`.

After GUI desktop is running on Pi run locally `sudo xhost +local:pi`. Then from SSH run `export DISPLAY=:0.0` before python script ([source](https://www.raspberrypi.org/forums/viewtopic.php?t=124021)). Or insert line at end of `~/.bashrc`.

Unsure how much of this is persistent. 

## Backlight

Turn on and off:

`sudo sh -c 'echo "0" > /sys/class/backlight/soc\:backlight/brightness'`

`sudo sh -c 'echo "1" > /sys/class/backlight/soc\:backlight/brightness'`

([source](https://learn.adafruit.com/adafruit-pitft-28-inch-resistive-touchscreen-display-raspberry-pi/backlight-control) Couldn't make PWM work)
