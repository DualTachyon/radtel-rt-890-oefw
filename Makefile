TARGET = firmware

OBJS =
# Startup files
OBJS += startup/start.o
OBJS += startup/init.o
#OBJS += external/printf/printf.o

# BSP
OBJS += bsp/crm.o
OBJS += bsp/gpio.o
OBJS += bsp/misc.o
OBJS += bsp/tmr.o

# Drivers
OBJS += driver/audio.o
OBJS += driver/battery.o
OBJS += driver/beep.o
OBJS += driver/bk1080.o
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
OBJS += app/fm.o
OBJS += app/lock.o
OBJS += app/menu.o
OBJS += app/radio.o
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
OBJS += task/battery.o
OBJS += task/cursor.o
OBJS += task/encrypt.o
OBJS += task/fmscanner.o
OBJS += task/keys.o
OBJS += task/idle.o
OBJS += task/incoming.o
OBJS += task/lock.o
OBJS += task/noaa.o
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
OBJS += ui/noaa.o
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
CFLAGS = -Os -Wall -Werror -mcpu=cortex-m4 -fno-builtin -fshort-enums -fno-delete-null-pointer-checks -std=c11 -MMD
CFLAGS += -DAT32F421C8T7
CFLAGS += -DPRINTF_INCLUDE_CONFIG_H
CFLAGS += -DGIT_HASH=\"$(GIT_HASH)\"
LDFLAGS = -mcpu=cortex-m4 -nostartfiles -Wl,-T,firmware.ld

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

