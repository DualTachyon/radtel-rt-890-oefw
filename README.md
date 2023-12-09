# Radtel RT-890 Custom Firmware

This project is an effort to improve the firmware of the Radtel RT-890 in terms of features and radio performance.

It is based on [DualTachyon's OEFW](https://github.com/DualTachyon/radtel-rt-890-oefw) which is reversed from the original Radtel 1.34 firmware.  
Thanks to him for making this possible!

## Disclaimer
This firmware is a work in progress and could be unstable; it could alter your radio and its data.  
Use at your own risk and remember to back up your SPI memory before installing any custom firmware.

## Features
- All stock features: [check user's manual](https://cdn.shopifycdn.net/s/files/1/0564/8855/8800/files/RT-890_user_manual.pdf?v=1670288968)
- RX frequency can be set from 10 to 1300 MHz (results may vary)
- SSB reception
- Light theme
- AM Fix (improvement in AM reception with strong signals, port of @OneOfEleven's excellent work on the Quansheng UV-K5)
- Full control over side key and main key shortcuts
- New configurable actions (FM Radio, Scanner, FLashlight)
- 0.01K step
- Displaying registers in single VFO mode
- Displaying dBM when receiving
- Reworked scanner
  - 8 Scan lists
  - Faster scanning
  - Resume mode: Time, Carrier, No
  - Change scan direction while scanning (up/down keys)
  - Force scan resume (up/down keys)
- Reworked main menu
- Ability to disable LED toggling when scanning
- And much more!

### Default Shortcut Keys (long press) - Configurable in main menu
```
1    => Start/stop scanning memory channels
2    => Toggle AM fix
3    => Toggle VOX
4    => Change TX power on current channel
5    => Change squelch level
6    => Toggle dual watch
7    => Toggle backlight
8    => Change frequency step
9    => Go to Preset Channel
0    => Launch broadcast FM mode
*    => Edit TX frequency on current channel
#    => Toggle key lock
Menu => DTMF decoder
Exit => Single/dual VFO display
```

### Scanning guide
Scan list management:
- This firmware has 8 scanlists.  
- The current channel can be added to any scanlist using the `Ch In List X` menus.  
- The scanlist to be used can be selected in the `List To Scan` menu.  
- To ignore scanlists and scan all channels, select `*` in the `List To Scan` menu.  
- To add/remove current channel to current scanlist, use the `Toggle SList` shortcut.

Scanning:
- To start scanning, press a key mapped to the `Freq scanner` shortcut (default: long press on key `1`).  
- When scanning is in progress, use the `Freq scanner` key to change the scan list, this action will move to the next non-empty scanlist, or switch to scan all mode if all subsequent lists are empty.  
- To change the direction of current scan, use the `up`/`down` keys.  
- To force the scan to resume when the scanner stops on a signal, use the `up`/`down` keys.  
- Press any key other than `Freq scanner` to stop scanning.  

### Spectrum Usage
Start spectrum by mapping a key (side key or keypad) to the Spectrum action using the main menu.  Spectrum will launch, centered on the frequency from the active VFO/Memory Channel.

Spectrum keys:
```
Up   => Normal: Increase frequency range by frequency +/- (number in middle of bottom row)
        Holding on a frequency: Move up to the next frequency
Down => Normal: Decrease frequency range by frequency +/- (number in middle of bottom row)
        Holding on a frequency: Move down to the previous frequency
1    => Change number of scan steps (16, 32, 64 or 128)
2    => Switch up to next preset
3    => Change modulation (AM, FM or SSB)
4    => Change step size (0.25k - 50k)
5    => Switch down to previous preset
6    => Inrease squelch level
7    => Hold on current frequency
8    => Toggle between bars and waterfall views
9    => Decrease squelch level
0    => Toggle filter (U = unfiltered, F = filtered)
*    => Change scan delay (0 - 10ms)
#    => Toggle bandwidth (W = wide, N = narrow)
Menu => Jump to VFO mode with current frequency and settings (to allow TX)
Exit => Exit spectrum
```

Spectrum display:
<p float="left">
<img src="/Images/SpectrumDisplay.png" height=300 />
</p>

## Update Instructions
### SPI memory backup
Use [RT-890-Flasher](https://github.com/DualTachyon/radtel-rt-890-flasher)

### SPI memory restore
Use [RT-890-SPI-restore-CLI](https://github.com/DualTachyon/radtel-rt-890-spi-restore-cli)

### Customizations
```
UART_DEBUG          => UART debug output
MOTO_STARTUP_TONE   => Moto XPS startup beeps
ENABLE_AM_FIX       => Experimental port of the great UV-K5 AM fix from OneOfEleven
ENABLE_LTO          => Link Time Optimization
ENABLE_NOAA         => NOAA weather channels (always re-set the sidekeys actions from menu after modifying the available actions)
```

### Build & Flash
See __Compiler__, __Building__ and __Flashing__ sections below.

## Pre-built firmware
You can find pre-built firmwares in the [Actions](https://github.com/OEFW-community/RT-890-custom-firmware/actions)

## Building from browser

To build the firmware without installing any software you can run a full featured IDE and compiler in your browser using GitHub Codespace.  
The preconfigured environment runs Linux Ubuntu 22.04 with gcc-arm-none-eabi 15:10.3-2021.07-4.

### Starting a new Codespace

You just need to clic on the green button `<> Code` -> `Codespace` -> `Create Codespace on ...`  
After the Codespace is initialized, you can open and edit any file, for example, modify the options in the makefile and build the firmware typing `make` in the terminal panel.

### Starting an existing Codespace

If you started a Codespace less than 7 days ago and have not manually deleted it, then you will be offered to restart it instead of creating a new one.  
The start-up will be much faster but you will need to update the code before compiling as it will contain the version of the code from the time of its creation.  
To do this, simply type the following commands in the console: `make clean`, then `git pull`, and finally `make`.

### A word about the free offer
The most observant users will have noticed this message: `Codespace usage for this repository is paid for by...`  
Github will never charge you without consent. If you reach the limit of the free offer, which is very unlikely, you will not be able to start your codespaces until the monthly limits reset, that's all.  
[More informations](https://docs.github.com/en/billing/managing-billing-for-github-codespaces/about-billing-for-github-codespaces)

## Telegram group
If you want to discuss this project, you can join the [Telegram group](https://t.me/RT890_OEFW).



---
_Original OEFW readme_

# Support

* If you like my work, you can support me through https://ko-fi.com/DualTachyon

# Open reimplementation of the Radtel RT-890 v1.34 firmware

This repository is a preservation project of the Radtel RT-890 v1.34 firmware.
It is dedicated to understanding how the radio works and help developers making their own customisations/fixes/etc.
It is by no means fully understood or has all variables/functions properly named, as this is best effort only.
As a result, this repository will not include any customisations or improvements over the original firmware.

# Compiler

arm-none-eabi GCC version 10.3.1 is recommended, which is the current version on Ubuntu 22.04.03 LTS.
Other versions may generate a flash file that is too big.
You can get an appropriate version from: https://developer.arm.com/downloads/-/gnu-rm

# Building

To build the firmware, you need to fetch the submodules and then run make:
```
git submodule update --init --recursive --depth=1
make
```

# Flashing

* Use the firmware.bin file with either [RT-890-Flasher](https://github.com/DualTachyon/radtel-rt-890-flasher) or [RT-890-Flasher-CLI](https://github.com/DualTachyon/radtel-rt-890-flasher-cli)

# License

Copyright 2023 Dual Tachyon
https://github.com/DualTachyon

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

