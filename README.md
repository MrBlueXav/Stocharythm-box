# Stocharythm Box §§ Bruitenkor ! 

## Another sound machine for STM32F4 Discovery kit
## Electronic free jazz ready !

![photo](zz_pictures/StocharythmBox_setup.jpg "photo")  

Funny and strange **drum machine** controlled by any PC USB AZERTY/QWERTY keyboard plugged into the board.
The STM32F4 Discovery kit acts as a USB host for the keyboard.  

Work In Progress !!  

8 voices at the moment :  
- 6 sample players which can access to 41 famous short samples (800kB)  
- 1 Synthetic Bass Drum  (DaisySP/ Emilie Gillet)  
- 1 white noise with ADSR  

FX : simple reverb based on Freeverb (MiniFreeverb.h)  

The sequencer is not step based. You define a loop duration and call for sound events which are randomly dispatched in the loop.  
These are so-called "stocharythms".  

Many debug messages through UART (Pin PA2).  

- - - - 
### Keyboard control :  
*One key controls* :  just hit the key  
- `+` and `-`   -> final volume (Output DAC)  
- `(` and `)` or arrows up and down -> source volume  
- `.`  -> clear loop  
- `space` -> play/pause sequencer
- `n` -> add 4 events  
- `t` : add 1 event played by instrument 0  
- `y` : add 1 event played by instrument 1  
- `u` : add 1 event played by instrument 2  
- `i` : add 1 event played by instrument 3  
- `o` : add 1 event played by instrument 4  
- `p` : add 1 event played by instrument 5  
- `h` : add 1 Synthetic Bass Drum event  
- `j` : add 1 white noise event  
- `k` : add 1 808 snare event  
- `z` : change the 6 samples  
- `s` : display sequencer status  
- `d` : display list of events in the loop  
- left and and right arrows : slow down or speed up  

*Multi key commands* :  press a sequence of keys and submit with [enter]  
...  

	

- - - - 
### Developer notes :
This is a configurable project with ioc and BSP files written in C and C++.  
It should be easily tailored for other powerful STM32 mcu (cortex M33, M4, M7, M55, ...)  

**Keyboard configuration** :  
- Keyboard layout is in ``azerty_hid_map.h`` (This one is for AZERTY french keyboards, sorry)  
- For mapping functions, in file `bruitenkor.cpp`, modify function : `void InterpretKey(uint8_t key, uint8_t keycode)`

There are several Git branches :
- **Stocharythm**    
- Bruitenkor  
- USBH_HID  
- Simple_demo : minimal synth platform  
- master  



<<< *By Xavier Halgand, Summer 2025* >>>

