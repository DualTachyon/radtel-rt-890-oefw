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

#include "misc.h"

SCREEN_Mode_t gScreenMode;
uint8_t gTxCodeType;
uint8_t gFskDataReceived;
bool gDataDisplay;
bool gDTMF_InputMode;
uint8_t gDTMF_WriteIndex;
bool gFrequencyDetectMode;
bool gEnableBlink;
bool gRedrawScreen;
bool gScannerMode;
bool gSaveMode;
bool gStartupSoundPlaying = false;
bool gReceptionMode;
bool gReceivingAudio;
bool gMonitorMode;
bool gFlashlightMode;
bool gEnableLocalAlarm;
bool gSendTone;
bool KEY_SideKeyLongPressed;
PTT_Lock_t gPttLock;
bool gSignalFound;
bool gBlinkGreen;
uint8_t gRadioMode;
IDLE_Mode_t gIdleMode;
uint8_t gRxLinkCounter;
uint8_t gNOAA_ChannelNow;
uint8_t gNOAA_ChannelNext;
uint8_t gTailToneCounter;
uint16_t gNoToneCounter;
bool gFrequencyReverse;
bool gManualScanDirection;
bool gForceScan;
uint8_t gSlot;
char gString[32];
char gBigString[40];

uint32_t SFLASH_Offsets[20];
uint32_t SFLASH_FontOffsets[32];
uint8_t gFlashBuffer[8192];

