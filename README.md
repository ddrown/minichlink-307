# minichlink-307

This was originally from the minichlink from https://github.com/cnlohr/ch32v003fun

A free, open mechanism to use the CH-LinkE $4 programming dongle for the CH32V307.

If on Linux, be sure to type make sure to install the `99-WCH-LinkE.rules` build rule to `/etc/udev/rules.d/`

On Windows, if you need to you can install the WinUSB driver over the WCH interface 1.

The exe here is about 12kB and contains everything except for the libusb driver.  In Linux you need `libusb-1.0-dev`.

## Usage

```
Usage: minichlink [args]
 single-letter args may be combined, i.e. -3r
 multi-part args cannot.
 -3 Enable 3.3V
 -5 Enable 5V
 -t Disable 3.3V
 -f Disable 5V
 -u Clear all code flash - by power off (also can unbrick)
 -b Reboot out of Halt
 -e Resume from halt
 -a Place into Halt
 -D Configure NRST as GPIO
 -d Configure NRST as NRST
 -s [debug register] [value]
 -g [debug register]
 -w [binary image to write] [address, decimal or 0x, try0x08000000]
 -r [output binary image] [memory address, decimal or 0x, try 0x08000000] [size, decimal or 0x, try 16384]
   Note: for memory addresses, you can use 'flash' 'launcher' 'bootloader' 'option' 'ram' and say "ram+0x10" for instance
   For filename, you can use - for raw or + for hex.
 -T is a terminal. This MUST be the last argument.
 -K <KB Ram> - RAM/flash split, Valid arguments are 32/64/128/192
```
 
## Build info

All platforms use the Makefile to build

Windows: this requires mingw-w64-x86-64-dev gcc-mingw-w64-x86-64 and libusb-1.0

Linux: this requires libusb & libudev, Ubuntu: libusb-1.0-0-dev libudev-dev, Fedora: libusb1-devel systemd-devel

MacOS: libusb also required, unknown install details
