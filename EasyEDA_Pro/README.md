## Version 0 of the board

see schematic and io details of board

Be carefull, some io are used elswhere in the system 
(see ESP32-P4-WIFI6-DEV-KIT-details-inter.jpg and ESP32-P4-WIFI6-DEV-KIT-datasheet.pdf)

### bug with first usb C port
need to use the second one ... need to analyse schematics ...

### banding in the red channel
defect in hat ? not really ...

With my hat RED5, RED4 and RED3 pins are connected to GPIO powered by VDDO_4 (see board schematics)
so I need to power up LDO channel 4 at 3.3V ... BUT

This LDO is also used to power the SDMMC part of the board, but the voltage depends of the kind of SD card ...
it can be 1.8V for high speed card, or 3.3V for normal card.

the SDMMC should have priority !! so a needed change of the hat is :

- change RED3 to GPIO46, GREEN2 to GPIO47, BLUE3 to GPIO48
- change RED4 to GPIO3, RED5 to GPIO21

This change permit the use of LDO 4 at 1.8V with minor impact of the color displayed ... only least bit of color will be wrong 
when using 1.8V instead of 3.3V

this should be applied for a V0.1 of the hat ...
