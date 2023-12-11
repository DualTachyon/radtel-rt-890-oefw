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

#ifndef RADIO_CHANNELS_H
#define RADIO_CHANNELS_H

#include <stdbool.h>
#include <stdint.h>

typedef struct __attribute__((packed)) {
	uint32_t Frequency;
	uint16_t Code:12;
	uint16_t CodeType:4;
} FrequencyInfo_t;

typedef struct __attribute__((packed)) {
	FrequencyInfo_t RX;
	FrequencyInfo_t TX;
	// 0x0C
	uint32_t Golay:24;
	uint32_t Unknown0:4;
	uint32_t bIs24Bit:1;
	uint32_t bMuteEnabled:1;
	uint32_t Encrypt:2;
	// 0x10
	uint8_t Available:1;
	uint8_t gModulationType:2;
	uint8_t BCL:2;
	uint8_t ScanAdd:1;
	uint8_t bIsLowPower:1;
	uint8_t bIsNarrow:1;
	// 0x11
	uint8_t _0x11;
	uint8_t Scramble;
	uint8_t IsInscanList;		// 8 lists: 1 bit per list
	uint8_t _0x14;
	uint8_t _0x15;
	char Name[10];
} ChannelInfo_t;

extern uint16_t gFreeChannelsCount;

bool CHANNELS_NextChannelMr(uint8_t Key, bool OnlyFromScanlist);
void CHANNELS_NextChannelVfo(uint8_t Key);
#ifdef ENABLE_NOAA
void CHANNELS_NextNOAA(uint8_t Key);
#endif
void CHANNELS_NextFM(uint8_t Key);

void CHANNELS_UpdateChannel(void);
void CHANNELS_UpdateVFO(void);
void CHANNELS_UpdateVFOFreq(uint32_t Frequency);

bool CHANNELS_LoadChannel(uint16_t ChNo, uint8_t Vfo);
void CHANNELS_CheckFreeChannels(void);
void CHANNELS_LoadVfoMode(void);
void CHANNELS_LoadWorkMode(void);
uint16_t CHANNELS_GetChannelUp(uint16_t Channel, uint8_t Vfo);
uint16_t CHANNELS_GetChannelDown(uint16_t Channel, uint8_t Vfo);
void CHANNELS_SaveChannel(uint16_t Channel, const ChannelInfo_t *pChannel);
#ifdef ENABLE_NOAA
void CHANNELS_SetNoaaChannel(uint8_t Channel);
#endif
void CHANNELS_SaveVfo(void);

#endif

