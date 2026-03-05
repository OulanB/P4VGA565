## Hat for a WaveShare ESP32-P4-WIFI6-DEV-KIT board 
## VGA output at 565 depth max

### Hardware
- WaveShare ESP32-P4-WIFI6-DEV-KIT board (https://www.waveshare.com/esp32-p4-wifi6-dev-kit.htm?sku=32054)
- Hat from JLCPCB (see in EasyEDA_Pro (https://easyeda.com/) for eprj file)

### Software
In rgb_test a .ino program to test display, and 3 pictures with gradients

### Bugs
#### Bug 1
When a display is connected to the vga, use of first usb-C port is impossible
use the second one.
I did not find the bug ... perhaps some erronous IO port definition.
#### Bug 2 Corrected
The gradient picture for the red channel is not correct, only 4 shades are displayed instead of 60+ as in blue and green pictures.
Perhaps an hardware bug in the hat? But sometimes the display is ok ... strange one 
##### Correction of bug
With my hat RED5, RED4 and RED3 pins are connected to GPIO powered by VDDO_4 (see board schematics)
so I need to power up LDO channel 4 at 3.3V ... BUT

This LDO is also used to power the SDMMC part of the board, but the voltage depends of the kind of SD card ...
it can be 1.8V for high speed card, or 3.3V for normal card.

the SDMMC should have priority !! so a needed change of the hat is :

- change RED3 to GPIO46, GREEN2 to GPIO47, BLUE3 to GPIO48
- change RED4 to GPIO3, RED5 to GPIO21

This change permit the use of LDO 4 at 1.8V with minor impact of the color displayed ... only least bit of color will be wrong 
when using 1.8V instead of 3.3V

#### Bug 3
framebuffer is not displayed correctly :
- in gradients picture, the top half present a defect in display all lines should starts as :
<img width="488" height="87" alt="start_with_hack" src="https://github.com/user-attachments/assets/1739f5db-ce4f-4537-94b7-c58a4a3f7795" />
but display
<img width="559" height="92" alt="start_without_hack" src="https://github.com/user-attachments/assets/80cbc823-b44e-4f47-83ef-c6094d1c7514" />

there is an offset of 8 pixels : the 8 first pixels displayed in each line are the last 8 of the previous line

The top half of the gradient compute the adress as it should be 
(see line 361 for the computing of the adress in the frame buffer as it should be ...)
but present an erronuous display

the bottom half use an offset of 8 pixels when computing the adress in the framebuffer
(see line 364 for the computing of the adress with this 'hack')
and is displayed correctly.

Certainly an hardware bug in the LCD RGB subsystem in the SOC.

### Capabilities
- 640x480 at 60 Hz (26.666 MHz pixel clock from 160 MHz default clock / 6)
- 800x600 at 56 Hz (36 MHz pixel clock from APPL clock programmed at 72 MHz / 2) 

Example of use : BasiliskII emulator ported to this board : https://sites.google.com/site/olivier2smet2/other-projects/basilisk-ii-esp32p4
(forked from https://github.com/amcchord/M5Tab-Macintosh)
