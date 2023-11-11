# Radtel RT-890 Custom Firmware

This project is an effort to improve the firmware of the Radtel RT-890 in terms of features and radio performance.

It is based on [DualTachyon's OEFW](https://github.com/DualTachyon/radtel-rt-890-oefw) which is reversed from the original Radtel 1.34 firmware.  
Thanks to him for making this possible!

## Disclaimer
This firmware is a work in progress and could be unstable; it could alter your radio and its data.  
Use at your own risk and remember to back up your SPI memory before installing any custom firmware.

## Features
- All stock features (check user's manual)
- SSB reception
- Light theme
- AM Fix (improvement in AM reception with strong signals, port of @OneOfEleven's excellent work on the Quansheng UV-K5)
- And much more!

## Instructions
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

### Shortcut Keys (long press)
```
1    => Start/stop scanning memory channels
2    => Toggle AM fix
3    => Toggle VOX
4    => Change TX power on current channel
5    => Change squelch level
6    => Toggle dual watch
7    => Toggle backlight
8    => Change frequency step
9    => Toggle scan skip for current channel
0    => Launch broadcast FM mode
*    => Edit TX frequency on current channel
#    => Toggle key lock
Menu => DTMF decoder
Exit => Single/dual VFO display
```

### Build & Flash
See __Compiler__, __Building__ and __Flashing__ sections below.

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

