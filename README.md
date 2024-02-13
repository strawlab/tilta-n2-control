# Tilta nucleus nano 2 motor control example

## Required hardware

* Tilta Nucleus Nano 2 motor (WLC-T05-MK))
* _"The Knob"_ - Tilta Nucleus Nano II Hand Wheel Controller (WLC-T05) (needed to configure the motor, once)
* nRF52840-Dongle dev board from Nordic Semiconductor

## How to

### 1. Set up the motor

Most importantly, we must set the motor to operate on IEEE 804.15.4 channel 12

1. turn on the Knob.
2. navigate to Menu -> Connect -> 2.4G -> Channels, activate CH1
3. apply power to the motor
4. with short presses, make the LED pink (that sets it to be a focus motor)
5. double-click motor's button (it enters pairing mode)
6. click Search on the Knob. The motor should appear in the list, tap it.

That's it, the motor should now be controlled by the Knob. (you may have to calibrate the motor range first)

7. Turn off the Knob.


### 2. Build and flash the firmware

1. install nRF Connect SDK for VSCode (follow [nRF Connect SDK introduction Excercise 1](https://academy.nordicsemi.com/courses/nrf-connect-sdk-fundamentals/lessons/lesson-1-nrf-connect-sdk-introduction/topic/exercise-1-1/))
2. open this project in VSCode
3. in VSCode, open nRF Connect on the activity bar, click "Create new build configuration". Select board `nrf52840dongle_nrf52840`, and `prj.conf` in Configuration field.  
3. build it.
The executable is usually placed into `<project>/build/zephyr/zephyr.hex`
4. flash it:
    1. open Nrf Connect Programmer tool (needs to be installed separately)
    2. insert the dongle and press the Reset button
    3. select the board in Programmer
    4. click Add File, browse to `zephyr.hex` we've just built
    5. click "write".

After the write is finished, the motor should start going back and forth by about a quarter of its range, in sync with the red led turning on and off on the dongle.

## Details

### Protocol

We have only reverse-engineered the position commands so far.

The communication is on IEEE 802.15.4 channel 12 ("CH1" in Knob's terms).

#### packets from the Knob:
```
vv vv vv vv vv vv vv vv vv    ieee802.15.4 packet header
                           vv vv vv vv  packet payload
61 88 c4 e7 01 ff ff 96 f0 44 05 00 b7
^^ ^^ packet type and flags (type=Data, Ack=1, PAN_compression=1, src and dst addr: short(16 bit))
      ^^  sequence number, ++ on each packet
         ^^ ^^  destination PAN addr.
               ^^ ^^  destination
                     ^^ ^^  source
                           ^  motor command type
                            ^ ^^ knob position (12-bit)
                                    ^^ internal checksum
                           ^^ ^^ ^^   (bytes covered by internal checksum)
```
__*command type:*__ 

4 = position, seeing lots of these when turning the Knob

Also seeing 8, C and 0 while idling, no idea what they mean.

__*internal checksum:*__ =(0u8 - byte0 - byte1 - byte2) 

We don't know yet if source and destination addresses matter, we've just copied what we've seen in the air. 

#### motor always responds ("Ack"):
```
02 00 c4 
      ^^ sequence number 
```

### Motion speed

Use the Knob to configure the motion speed (called "sensitivity" in Knob's menu) and torque.

We have noticed that the motor only engages the full speed if it senses that a human is turning the Knob, and would otherwise be very slow and gentle. When holding static position, adding some noise is enough to convince the motor to engage the full speed. That is being done in this example project.

## Hacking

If yours doesn't work, or if you want to implement some other features, you can use ieee802.15.4 sniffer from Nordic, which runs on the same dongle. This is what we have used for our reverse-engineering. 

https://infocenter.nordicsemi.com/index.jsp?topic=%2Fug_sniffer_802154%2FUG%2Fsniffer_802154%2Fintro_802154.html



