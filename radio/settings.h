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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <stdint.h>

enum {
	DTMF_STATE_NORMAL = 0U,
	DTMF_STATE_STUNNED,
	DTMF_STATE_KILLED,
};

typedef struct {
	uint8_t _0x00;
	uint8_t _0x01;
	uint16_t BandSelectionThreshold;
	uint16_t AF_RX_Expander;
	uint16_t AF_TX_Compress;
	uint8_t TX_300Hz_Coefficient;
	uint8_t TX_3000Hz_Coefficient;
	uint8_t RX_300Hz_Coefficient;
	uint8_t RX_3000Hz_Coefficient;
	uint8_t _0x0C;
	uint8_t _0x0D;
	uint8_t _0x0E;
	uint8_t _0x0F;
	uint8_t BatteryCalibration[15];
	uint8_t _0x1F;
} Calibration_t;

typedef struct __attribute__((packed)) {
	// 0x00
	uint8_t unknown0:1;
	uint8_t KeyBeep:1;
	uint8_t VoicePrompt:1;
	uint8_t StartupRingTone:1;
	uint8_t WhatIsThis:1;
	uint8_t DisplayLabel:1;
	uint8_t DisplayVoltage:1;
	uint8_t DisplayLogo:1;
	// 0x01
	uint8_t FrequencyStep:4;
	uint8_t SaveMode:1;
	uint8_t RogerBeep:2;
	uint8_t TxPriority:1;
	// 0x02
	uint8_t DualDisplay:1;
	uint8_t NoaaAlarm:1;
	uint8_t RepeaterMode:2;
	uint8_t Squelch:4;
	// 0x03
	uint8_t DisplayTimer;
	uint8_t LockTimer;
	uint8_t TimeoutTimer;
	// 0x06
	uint8_t VoxDelay:4;
	uint8_t VoxLevel:4;
	// 0x07
	uint16_t ToneFrequency;
	uint16_t FmFrequency;
	// 0x0B
	uint8_t Unknown1:1;
	uint8_t WorkMode:1;
	uint8_t Unknown2:2;
	uint8_t ScanDirection:1;
	uint8_t StandbyArea:1;
	uint8_t DualStandby:1;
	uint8_t FmStandby:1;
	// 0x0C
	uint16_t VfoChNo[2];
	uint8_t Actions[4];
	// 0x14
	uint8_t bUseVHF:1;
	uint8_t bEnableDisplay:1;
	uint8_t Lock:1;
	uint8_t Vox:1;
	uint8_t TailTone:1;
	uint8_t CurrentVfo:1;
	uint8_t bFLock:1;
	uint8_t Unknown10:1;
	// 0x15
	uint8_t WelcomeX;
	uint8_t WelcomeY;
	uint16_t BorderColor;
	uint8_t _0x19;
	uint8_t _0x1A;
	uint8_t _0x1B;
	uint8_t _0x1C;
	uint8_t _0x1D;
	uint8_t _0x1E;
	uint8_t _0x1F;
	uint8_t BandInfo[8];
	uint8_t _0x28;
	uint8_t _0x29;
	uint8_t _0x2A;
	uint8_t _0x2B;
	uint8_t _0x2C;
	uint8_t _0x2D;
	uint8_t _0x2E;
	uint8_t _0x2F;
	uint8_t _0x30;
	uint8_t _0x31;
	uint8_t _0x32;
	uint8_t _0x33;
	uint8_t _0x34;
	uint8_t _0x35;
	uint8_t _0x36;
	uint8_t _0x37;
	uint8_t _0x38;
	uint8_t _0x39;
	uint8_t _0x3A;
	uint8_t _0x3B;
	uint8_t DtmfState;
	uint8_t _0x3D;
	uint8_t _0x3E;
	uint8_t _0x3F;
	uint16_t PresetChannels[4];
	uint8_t _0x48;
	uint8_t _0x49;
	uint8_t _0x4A;
	uint8_t _0x4B;
	uint8_t _0x4C;
	uint8_t _0x4D;
	uint8_t _0x4E;
	uint8_t _0x4F;
	uint8_t _0x50;
	uint8_t _0x51;
	uint8_t _0x52;
	uint8_t _0x53;
	uint8_t _0x54;
	uint8_t _0x55;
	uint8_t _0x56;
	uint8_t _0x57;
	uint8_t _0x58;
	uint8_t _0x59;
	uint8_t _0x5A;
	uint8_t _0x5B;
	uint8_t _0x5C;
	uint8_t _0x5D;
	uint8_t _0x5E;
	uint8_t _0x5F;
} gSettings_t;

// Extended settings added to the custom firmware
// stored outside of the original settings area to avoid breaking something
typedef struct __attribute__((packed)) {
	// 0x00
	uint8_t ScanResume: 2;	// Carrier=1, Time=2, No=3
	uint8_t AmFixEnabled: 1;
	uint8_t DarkMode: 1;
	uint8_t ScanBlink: 1;
	uint8_t CurrentScanList: 3;
	// 0x01 - 0x0E
	uint8_t KeyShortcut[14];
	// 0x0F
	uint8_t ScanAll: 1;
	uint8_t Undefined: 7;	// free for use
	// 0x10...
} gExtendedSettings_t;

extern Calibration_t gCalibration;
extern char gDeviceName[16];
extern gSettings_t gSettings;
extern char WelcomeString[32];
extern uint32_t gFrequencyStep;
extern gExtendedSettings_t gExtendedSettings;

void SETTINGS_BackupCalibration(void);
void SETTINGS_LoadCalibration(void);
void SETTINGS_LoadSettings(void);
void SETTINGS_SaveGlobals(void);
void SETTINGS_SaveState(void);
void SETTINGS_SaveDTMF(void);
void SETTINGS_FactoryReset(void);
void SETTINGS_SaveDeviceName(void);
void SETTINGS_BackupSettings(void);

#endif

