AnimatedGIFs for SmartMatrix/SDcard or NeoMatrix (Neopixel/WS2812B) from SDcard, SPIFFS, FatFS on Teensy 3.x, ESP8266, or ESP32
==================================================================================================================================

Video: https://www.youtube.com/watch?v=RJX2-j1PWFw

Demo on 24x32 NeoMatrix:
![125_20180713_neomatrix_animatedgifs](https://user-images.githubusercontent.com/1369412/42703527-b27a5450-8681-11e8-82a9-d074e473848c.jpg)
Demo on 64x64 SmartMatrix:
![144_20190409_RGBPanels_Diffusers](https://user-images.githubusercontent.com/1369412/55811945-df591280-5a9e-11e9-8546-520265b14e83.jpg)
![104_20190409_RGBPanels_Diffusers](https://user-images.githubusercontent.com/1369412/55813849-5cd25200-5aa2-11e9-8c6c-7e22ae2a5a18.jpg)

Configuration
-------------
Please see neomatrix_config.h to select whether you're using SDcard, FatFS, or SPIFFS, and
NeoMatrix instead of SmartMatrix (or any other display supported by
https://github.com/marcmerlin/Framebuffer_GFX ). FatFS is the default on ESP32 as it is
faster than SPIFFS. See http://marc.merlins.org/perso/arduino/post_2019-03-30_Using-FatFS-FFat-on-ESP32-Flash-With-Arduino.html )

animatedgif_config.h has a few variables specific to this library (while neomatrix_config.h
is a config file to allow lots of different code for your specific hardware and display).

For historical and compatibility reasons, this library was originally written for Smartmatrix
(without neomatrix or Framebuffer::GFX support). This is why animatedgif_config.h allows you
to use SmartMatrix natively without needing Frambuffer::GFX or SmartMatrix::GFX). I however
recommend you do use those as it will allow you to move your code to any other hardware backend
from FastLED, to SPI TFTs, to Raspberry Pi with huge RGBPanel displays)

AnimatedGIFs file browser
-------------------------
You either put your gifs inside a /gifs subdirectory on an sdcard or under /gifs in SPIFFS 

You can send the following commands over serial port (115200):
- n / p : go to next/previous gif
- \+ / \- : adjust gamma correction for NeoMatrix backend
- 1 to xx: jump to gif number xx (need newline after number)
= : display current GIF in loop for 1h instead of 10 seconds

config.h also has these compile time variables:
- #define FIRSTINDEX 0 : change which GIF is shown first
- #define DEBUGLINE 6: if defined will only decode first frame, stop, and dump to serial port the color values of the given line


SimpleGifAnimViewer
-------------------
This is a very shrunk version of AnimatedGIFs.  This subdirectory shows a near minimal amount of code necessary to display a GIF on a NeoMatrix (60 lines of code). You only need to change the hardcoded path to the GIF, and configure neomatrix_config.h to match your panel. With very minimal changes, you can read from sdcard instead of SPIFFS if you need more storage, or you are using a flash-less chip like Teensy.

SimpleGifAnimViewer2
-------------------
This version uses GifAnim_Impl.h as a library and shows how to animate a GIF in just 10 lines of code or so.  

NeoMatrix and SPIFFS support
----------------------------
I, Marc MERLIN <marc_soft@merlins.org>, took the excellent work
from Louis Beaudoin's GIF decoder for SmartMatrix and ported it to
https://github.com/marcmerlin/FastLED_NeoMatrix .

In the process, I also allowed it to work on SPIFFS (built in filesystem using
flash available on ESP8266 and ESP32) so that you can store your GIFS directly
on the flash and not need an sdcard at all.
Unfortunately the SPIFFS support on ESP32 and ESP8266 is not quite identical, so
the code needs to account for both APIs (ESP8266 is the odd one out, the ESP32
SPIFFS API is actually compatible with the sdcard API).

The data subdirectory allows the arduino API to automatically create a flash
filesystem and upload it to your chip, see:
- ESP8266: http://esp8266.github.io/Arduino/versions/2.3.0/doc/filesystem.html#uploading-files-to-file-system
- ESP32: https://github.com/me-no-dev/arduino-esp32fs-plugin

The gifs/ subdirectory contains animated gifs that don't long as good on
Neomatrix due to the lesser dynamic range of neopixels. Please look at
neomatrix_config.h and adjust these if needed:
- uint8_t matrix_brightness = 32;
- float matrix_gamma = 3.0;

That being said, some GIFs just don't look so good on Neomatrix while they will
look ok on SmartMatrix. Those in the gifs/ subdirectory and do not get uploaded
to ESP chips by default (although you could upload them if you actually connect
those chips to a SmartMatrix instead of a NeoMatrix).

You can get more animated GIFS here:
- https://imgur.com/a/NuFRD 
- https://imgur.com/a/HjMW0


FatFS/FFAT support on ESP32
---------------------------
Sadly when you add too many files to SPIFFS, it caused some inefficiencies in seeks that 
caused the GIF animations to hang at times. The fix was to switch to the new FatFS/FFAT 
support that was added recently to arduino-esp32.  
It now nicely animates up to 3MB of animated Gifs on ESP32 without any hangs.

https://github.com/marcmerlin/esp32_fatfsimage/blobmaster/README.md explains how to 
create the fatfs image and how to upload it.  
More details also on:
http://marc.merlins.org/perso/arduino/post_2019-03-30_Using-FatFS-FFat-on-ESP32-Flash-With-Arduino.html


Finding Animated GIFs
---------------------
look for "imagesize:64x64 michael jackson animated gif" on google, and click images tab.


Original SmartMatrix support from https://github.com/pixelmatix/AnimatedGIFs
----------------------------------------------------------------------------
This [SmartMatrix Library](http://docs.pixelmatix.com/SmartMatrix/index.html) example displays 32x32 GIF animations loaded from a SD Card connected to the Teensy 3.1 using the [SmartMatrix Shield](http://docs.pixelmatix.com/SmartMatrix/shieldref.html)

The GIFs can be up to 32 pixels in width and height.  This code has been tested with 32x32 pixel and 16x16 pixel GIFs, but is optimized for 32x32 pixel GIFs.

Wiring is on the default Teensy 3.1 SPI pins, and chip select can be on any GPIO (customize by defining SD_CS in the code)

Function     | Pin
-------------|----
DOUT         |  11
DIN          |  12
CLK          |  13
CS (default) |  15

This code first looks for .gif files in the /gifs/ directory (you can customize this with the GIF_DIRECTORY definition) then plays random GIFs in the directory, looping each GIF for DISPLAY_TIME_SECONDS

This example is meant to give you an idea of how to add GIF playback to your own sketch.  For a project that adds GIF playback with other features, take a look at [Light Appliance](https://github.com/CraigLindley/LightAppliance) and [Aurora](https://github.com/pixelmatix/aurora)

If you find any GIFs that won't play properly, please attach them to a new
Issue post in the GitHub repo [here](https://github.com/pixelmatix/AnimatedGIFs/issues)
