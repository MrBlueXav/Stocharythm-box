# Stocharythm Box §§ Bruitenkor ! 
- - - 
## Another sound machine for STM32F4 Discovery kit
## Electronic free jazz ready !
- - -

Funny and strange drum machine controlled by any PC USB AZERTY/QWERTY keyboard plugged into the board.
The STM32F4 Discovery kit acts as a USB host for the keyboard.  

Work In Progress !!  

8 voices at the moment :  
- 6 sample players which can access to 41 famous samples (800kB)  
- 1 Synthetic Bass Drum  (DaisySP/ Emilie Gillet)  
- 1 white noise  
FX : simple reverb  

Debug messages through UART

The sequencer is not step based. You define a loop duration and call for sound events which are randomly dispatched in the loop.  
These are so-called "stocharythms".
- - - - 

One key controls :  
- + and -   : end volume  
- arrows up and down : source volume  
- .  : clear loop  
- n : add 4 events  
- t : add 1 event played by instrument 0  
- y : add 1 event played by instrument 1  
- u : add 1 event played by instrument 2  
- i : add 1 event played by instrument 3  
- o : add 1 event played by instrument 4  
- p : add 1 event played by instrument 5  
- h : add 1 Synthetic Bass Drum event  
- j : add 1 white noise event  
- k : add 1 808 snare event  
- z : change the 6 samples  
- s : display sequencer status
- d : display list of events in the loop  
- left and and right arrows : slow down or speed up  

Multi key commands :  
...  

	

- - - - 
Configurable project with ioc and BSP files.  
There are several Git branches :  
- *Stocharythm*    
- Bruitenkor  
- USBH_HID  
- Simple_demo  
- master  

C++ compatible.

<<<By Xavier Halgand, Summer 2025>>>

