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

#include "driver/serial-flash.h"
#include "frequencies.h"

FrequencyBandInfo_t gFrequencyBandInfo;
bool gUseUhfFilter;

uint8_t gCurrentFrequencyBand = 0xFF;

uint8_t gTxPowerLevelLow = 0x28;
uint8_t gTxPowerLevelHigh = 0x14;
uint8_t gSquelchNoiseThresholdWide = 0x28;
uint8_t gSquelchNoiseThresholdNarrow = 0x28;
uint8_t gSquelchRSSIThresholdWide = 0x50;
uint8_t gSquelchRSSIThresholdNarrow = 0x55;

uint32_t FREQUENCY_GetStep(uint8_t StepSetting)
{
	switch(StepSetting) {
	case  0: return 25;
	case  1: return 125;
	case  2: return 250;
	case  3: return 500;
	case  4: return 625;
	case  5: return 1000;
	case  6: return 1250;
	case  7: return 2000;
	case  8: return 2500;
	case  9: return 5000;
	case 10: return 10000;
	case 11: return 50000;
	case 12: return 100000;
	case 13: return 500000;
	default: return 25;
	}
}

void FREQUENCY_SelectBand(uint32_t Frequency)
{
	uint8_t Band;
	uint8_t Level;

	if (Frequency >= 13600000  && Frequency <= 17400000) {
		Band = 0;
		Level = (Frequency - 13500000) / 500000;
		gUseUhfFilter = false;
	} else if (Frequency >= 40000000 && Frequency <= 48000000) {
		Band = 1;
		Level = (Frequency - 40000000) / 500000;
		gUseUhfFilter = true;
	} else if (Frequency >= 6400000 && Frequency <= 13600000) {
		Band = 3;
		Level = (Frequency - 6000000) / 500000;
		gUseUhfFilter = false;
	} else if (Frequency >= 17400000 && Frequency <= 24000000) {
		Band = 4;
		Level = (Frequency - 17000000) / 500000;
		gUseUhfFilter = false;
	} else if (Frequency >= 24000000 && Frequency <= 32000000) {
		Band = 5;
		Level = (Frequency - 24000000) / 500000;
		gUseUhfFilter = true;
	} else if (Frequency >= 32000000 && Frequency <= 40000000) {
		Band = 6;
		Level = (Frequency - 32000000) / 500000;
		gUseUhfFilter = true;
	} else if (Frequency >= 48000000 && Frequency <= 56000000) {
		Band = 7;
		Level = (Frequency - 48000000) / 500000;
		gUseUhfFilter = true;
	} else {
		return;
	}

	if (Band != gCurrentFrequencyBand) {
		gCurrentFrequencyBand = Band;
		SFLASH_Read(&gFrequencyBandInfo, 0x3BF020 + (Band * sizeof(gFrequencyBandInfo)), sizeof(gFrequencyBandInfo));
	}
	if (Level > 15) {
		Level = 15;
	}

	gSquelchNoiseThresholdWide = gFrequencyBandInfo.SquelchNoiseThresholdWide[Level];
	gSquelchRSSIThresholdWide = gFrequencyBandInfo.SquelchRSSIThresholdWide[Level];
	if (Band == 3) {
		gSquelchNoiseThresholdWide += 10;
		gSquelchRSSIThresholdWide -= 10;
	}
	gTxPowerLevelLow = gFrequencyBandInfo.TxPowerLevelLow[Level];
	gTxPowerLevelHigh = gFrequencyBandInfo.TxPowerLevelHigh[Level];
	gSquelchNoiseThresholdNarrow = gFrequencyBandInfo.SquelchNoiseThresholdNarrow[Level];
	gSquelchRSSIThresholdNarrow = gFrequencyBandInfo.SquelchRSSIThresholdNarrow[Level];
}

