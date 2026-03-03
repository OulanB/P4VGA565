## Hat for a WaveShare ESP32-P4-WIFI6-DEV-KIT board 
## VGA output at 565 depth max

### Hardware
- WaveShare ESP32-P4-WIFI6-DEV-KIT board (https://www.waveshare.com/esp32-p4-wifi6-dev-kit.htm?sku=32054)
- Hat from JLCPCB (see in EasyEDA_Pro (https://easyeda.com/) for eprj file)

### Bugs
When a display is connected to the vga, use of first usb-C port is impossible
use the second one.

I did not find the bug ... perhaps some erronous IO port definition.

### Capabilities
- 640x480 at 60 Hz (26.666 MHz pixel clock from 160 MHz default clock / 6)
- 800x600 at 56 Hz (36 MHz pixel clock from APPL clock programmed at 72 MHz / 2) 

Example of use : BasiliskII emulator ported to this board : https://sites.google.com/site/olivier2smet2/other-projects/basilisk-ii-esp32p4
(forked from https://github.com/amcchord/M5Tab-Macintosh)
