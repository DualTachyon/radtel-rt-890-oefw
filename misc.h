/* Copyright 2023 Dual Tachyon
 * https://github.com/DualTachyon
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 *     Unless required by applicable law or agreed to in writing, software
 *     distributed under the License is distributed on an "AS IS" BASIS,
 *     WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *     See the License for the specific language governing permissions and
 *     limitations under the License.
 */

#ifndef MISC_H
#define MISC_H

#include <stdbool.h>
#include <stdint.h>

#ifndef ARRAY_SIZE
	#define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

enum SCREEN_Mode_t {
	SCREEN_MAIN = 0,
	SCREEN_MENU,
	SCREEN_SETTING,
};

typedef enum SCREEN_Mode_t SCREEN_Mode_t;

enum IDLE_Mode_t {
	IDLE_MODE_OFF = 0,
	IDLE_MODE_DUAL_STANDBY,
#ifdef ENABLE_NOAA
	IDLE_MODE_NOAA,
#endif
	IDLE_MODE_SAVE,
};

typedef enum IDLE_Mode_t IDLE_Mode_t;

enum PTT_Lock_t {
	PTT_LOCK_VOX = 0x01U,
	PTT_LOCK_BATTERY = 0x02U,
	PTT_LOCK_INCOMING = 0x04U,
	PTT_LOCK_BUSY = 0x10U,
};

typedef enum PTT_Lock_t PTT_Lock_t;

enum {
	BUSY_LOCK_OFF = 0U,
	BUSY_LOCK_CARRIER,
	BUSY_LOCK_CSS,
	BUSY_LOCK_INVALID,
};

enum {
	RADIO_MODE_QUIET = 0U,
	RADIO_MODE_INCOMING,
	RADIO_MODE_RX,
	RADIO_MODE_TX,
};

extern SCREEN_Mode_t gScreenMode;
extern uint8_t gTxCodeType;
extern uint8_t gFskDataReceived;
extern bool gDataDisplay;
extern bool gDTMF_InputMode;
extern uint8_t gDTMF_WriteIndex;
extern bool gFrequencyDetectMode;
extern bool gEnableBlink;
extern bool gRedrawScreen;
extern bool gScannerMode;
extern bool gFlashlightMode;
extern bool gSaveMode;
extern bool gReceptionMode;
extern bool gReceivingAudio;
extern bool gMonitorMode;
extern bool gEnableLocalAlarm;
extern bool gSendTone;
extern bool gStartupSoundPlaying;
extern bool KEY_SideKeyLongPressed;
extern PTT_Lock_t gPttLock;
extern bool gSignalFound;
extern bool gBlinkGreen;
extern uint8_t gRadioMode;
extern IDLE_Mode_t gIdleMode;
extern uint8_t gRxLinkCounter;
extern uint8_t gNOAA_ChannelNow;
extern uint8_t gNOAA_ChannelNext;
extern uint8_t gTailToneCounter;
extern uint16_t gNoToneCounter;
extern bool gFrequencyReverse;
extern bool gManualScanDirection;
extern bool gForceScan;
extern uint8_t gSlot;
extern char gString[32];
extern char gBigString[40];

extern uint32_t SFLASH_Offsets[20];
extern uint32_t SFLASH_FontOffsets[32];
extern uint8_t gFlashBuffer[8192];


#endif
