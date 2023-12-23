TARGET = firmware

UART_DEBUG			?= 0
MOTO_STARTUP_TONE		?= 1
ENABLE_AM_FIX			?= 1
ENABLE_NOAA			?= 1
ENABLE_SPECTRUM			?= 1
# Spectrum presets - 1.4 kB
ENABLE_SPECTRUM_PRESETS		?= 1
# FM radio = 2.6 kB
ENABLE_FM_RADIO			?= 1
# Space saving options
ENABLE_LTO			?= 0
ENABLE_OPTIMIZED		?= 1

OBJS =
# Startup files
OBJS += startup/start.o
OBJS += startup/init.o
ifeq ($(UART_DEBUG),1)
	OBJS += external/printf/printf.o
endif

# BSP
OBJS += bsp/crm.o
OBJS += bsp/gpio.o
OBJS += bsp/misc.o
OBJS += bsp/tmr.o

# Drivers
OBJS += driver/audio.o
OBJS += driver/battery.o
OBJS += driver/beep.o
ifeq ($(ENABLE_FM_RADIO), 1)
	OBJS += driver/bk1080.o
endif
OBJS += driver/bk4819.o
OBJS += driver/crm.o
OBJS += driver/delay.o
OBJS += driver/key.o
OBJS += driver/led.o
OBJS += driver/pwm.o
OBJS += driver/serial-flash.o
OBJS += driver/speaker.o
OBJS += driver/st7735s.o
OBJS += driver/uart.o

# "App" logic
OBJS += app/css.o
OBJS += app/flashlight.o
ifeq ($(ENABLE_FM_RADIO), 1)
	OBJS += app/fm.o
endif
OBJS += app/lock.o
OBJS += app/menu.o
OBJS += app/radio.o
ifeq ($(ENABLE_SPECTRUM), 1)
	OBJS += app/spectrum.o
endif
OBJS += app/t9.o
OBJS += app/uart.o

# Helper code
OBJS += helper/dtmf.o
OBJS += helper/helper.o
OBJS += helper/inputbox.o

# Misc data
OBJS += misc.o

# Radio management
OBJS += radio/channels.o
OBJS += radio/data.o
OBJS += radio/detector.o
OBJS += radio/frequencies.o
OBJS += radio/hardware.o
OBJS += radio/scheduler.o
OBJS += radio/settings.o

# Tasks
OBJS += task/alarm.o
ifeq ($(ENABLE_AM_FIX), 1)
        OBJS += task/am-fix.o
endif
OBJS += task/battery.o
OBJS += task/cursor.o
OBJS += task/encrypt.o
ifeq ($(ENABLE_FM_RADIO), 1)
	OBJS += task/fmscanner.o
endif
OBJS += task/keyaction.o
OBJS += task/keys.o
OBJS += task/idle.o
OBJS += task/incoming.o
OBJS += task/lock.o
ifeq ($(ENABLE_NOAA), 1)
OBJS += task/noaa.o
endif
OBJS += task/ptt.o
OBJS += task/rssi.o
OBJS += task/scanner.o
OBJS += task/screen.o
OBJS += task/sidekeys.o
OBJS += task/timeout.o
OBJS += task/voice.o
OBJS += task/vox.o

# User Interface
OBJS += ui/boot.o
OBJS += ui/dialog.o
OBJS += ui/font.o
OBJS += ui/gfx.o
OBJS += ui/helper.o
OBJS += ui/logo.o
OBJS += ui/main.o
OBJS += ui/menu.o
ifeq ($(ENABLE_NOAA), 1)
OBJS += ui/noaa.o
endif
OBJS += ui/version.o
OBJS += ui/vfo.o
OBJS += ui/welcome.o

# Main

OBJS += main.o

ifeq ($(OS),Windows_NT)
TOP := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
else
TOP := $(shell pwd)
endif

SDK := $(TOP)/external/SDK
LINKER_SCRIPT := $(SDK)/libraries/cmsis/cm4/device_support/startup/gcc/linker/AT32F421x8_FLASH.ld

AS = arm-none-eabi-as
CC = arm-none-eabi-gcc
LD = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
SIZE = arm-none-eabi-size

GIT_HASH_TMP := $(shell git rev-parse --short HEAD)

ifeq ($(GIT_HASH_TMP),)
GIT_HASH := "NOGIT"
else
GIT_HASH := $(GIT_HASH_TMP)
endif

ASFLAGS = -mcpu=cortex-m4
CFLAGS = -Os -Wall -Werror -mcpu=cortex-m4 -fno-builtin -fshort-enums -fno-delete-null-pointer-checks -std=c2x -MMD
CFLAGS += -DAT32F421C8T7
CFLAGS += -DPRINTF_INCLUDE_CONFIG_H
CFLAGS += -DGIT_HASH=\"$(GIT_HASH)\"
LDFLAGS = -mcpu=cortex-m4 -nostartfiles -Wl,-T,firmware.ld

ifeq ($(ENABLE_OPTIMIZED),1)
CFLAGS += --specs=nano.specs
LDFLAGS += --specs=nano.specs

CFLAGS += -ffunction-sections
LDFLAGS += -Wl,--gc-sections

CFLAGS += -finline-limit=0

CFLAGS += -fmerge-all-constants
endif
 
ifeq ($(DEBUG),1)
ASFLAGS += -g
CFLAGS += -g
LDFLAGS += -g
endif

INC =
INC += -I $(TOP)
INC += -I $(SDK)/libraries/cmsis/cm4/device_support
INC += -I $(SDK)/libraries/cmsis/cm4/core_support/
INC += -I $(SDK)/libraries/drivers/inc/

LIBS =

DEPS = $(OBJS:.o=.d)

ifeq ($(UART_DEBUG),1)
	CFLAGS += -DUART_DEBUG
endif
ifeq ($(MOTO_STARTUP_TONE),1)
	CFLAGS += -DMOTO_STARTUP_TONE
endif
ifeq ($(ENABLE_AM_FIX),1)
	CFLAGS += -DENABLE_AM_FIX
endif
ifeq ($(ENABLE_LTO),1)
	CFLAGS += -flto=auto
endif
ifeq ($(ENABLE_NOAA),1)
	CFLAGS += -DENABLE_NOAA
endif
ifeq ($(ENABLE_SPECTRUM), 1)
	CFLAGS += -DENABLE_SPECTRUM
endif
ifeq ($(ENABLE_SPECTRUM_PRESETS), 1)
	CFLAGS += -DENABLE_SPECTRUM_PRESETS
endif
ifeq ($(ENABLE_FM_RADIO), 1)
	CFLAGS += -DENABLE_FM_RADIO
endif

all: $(TARGET)
	$(OBJCOPY) -O binary $< $<.bin
	$(SIZE) $<

ctags:
	ctags -R -f .tags .

ui/version.o: .FORCE

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $^ -o $@ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

%.o: %.S
	$(AS) $(ASFLAGS) $< -o $@

.FORCE:

-include $(DEPS)

clean:
	rm -f $(TARGET).bin $(TARGET) $(OBJS) $(DEPS)
