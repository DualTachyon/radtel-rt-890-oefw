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

#ifndef FREQUENCIES_H
#define FREQUENCIES_H

#include <stdbool.h>
#include <stdint.h>

enum {
	BAND_136MHz = 0,
	BAND_400MHz = 1,
	BAND_64MHz  = 3,
	BAND_174MHz = 4,
	BAND_240MHz = 5,
	BAND_320MHz = 6,
	BAND_480MHz = 7,
};

typedef struct __attribute__((packed)) {
	uint16_t FrequencyOffset;
	uint8_t MicSensitivityTuningWide;
	uint16_t TxDeviationWide;
	uint8_t CtcssTxGainWide;
	uint8_t DcsTxGainWide;
	uint16_t RX_DAC_GainWide;
	uint8_t MicSensitivityTuningNarrow;
	uint16_t TxDeviationNarrow;
	uint8_t CtcssTxGainNarrow;
	uint8_t DcsTxGainNarrow;
	uint16_t RX_DAC_GainNarrow;
	uint8_t TxPowerLevelHigh[16];
	uint8_t TxPowerLevelLow[16];
	uint8_t SquelchNoiseWide[16];
	uint8_t SquelchRSSIWide[16];
	uint8_t SquelchNoiseNarrow[16];
	uint8_t SquelchRSSINarrow[16];
} FrequencyBandInfo_t;

extern FrequencyBandInfo_t gFrequencyBandInfo;
extern bool gUseUhfFilter;
extern uint8_t gCurrentFrequencyBand;

extern uint8_t gTxPowerLevelLow;
extern uint8_t gTxPowerLevelHigh;
extern uint8_t gSquelchNoiseWide;
extern uint8_t gSquelchRSSIWide;
extern uint8_t gSquelchNoiseNarrow;
extern uint8_t gSquelchRSSINarrow;

uint32_t FREQUENCY_GetStep(uint8_t StepSetting);
void FREQUENCY_SelectBand(uint32_t Frequency);

#endif

