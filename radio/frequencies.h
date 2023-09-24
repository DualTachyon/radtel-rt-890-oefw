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

typedef struct __attribute__((packed)) {
	uint16_t FrequencyOffset;
	uint8_t MicSensitivityTuningWide;
	uint16_t TxDeviationWide;
	uint8_t TxGain1Tuning0Wide;
	uint8_t TxGain1Tuning0_L;
	uint16_t RX_DAC_GainWide;
	uint8_t MicSensitivityTuningNarrow;
	uint16_t TxDeviationNarrow;
	uint8_t TxGain1Tuning1Narrow;
	uint8_t TxGain1Tuning1_L;
	uint16_t RX_DAC_GainNarrow;
	uint8_t TxPowerLevelLow[16];
	uint8_t TxPowerLevelHigh[16];
	uint8_t SquelchNoiseThresholdWide[16];
	uint8_t SquelchRSSIThresholdWide[16];
	uint8_t SquelchNoiseThresholdNarrow[16];
	uint8_t SquelchRSSIThresholdNarrow[16];
} FrequencyBandInfo_t;

extern FrequencyBandInfo_t gFrequencyBandInfo;
extern bool gUseUhfFilter;
extern uint8_t gCurrentFrequencyBand;

extern uint8_t gTxPowerLevelLow;
extern uint8_t gTxPowerLevelHigh;
extern uint8_t gSquelchNoiseThresholdWide;
extern uint8_t gSquelchRSSIThresholdWide;
extern uint8_t gSquelchNoiseThresholdNarrow;
extern uint8_t gSquelchRSSIThresholdNarrow;

uint32_t FREQUENCY_GetStep(uint8_t StepSetting);
void FREQUENCY_SelectBand(uint32_t Frequency);

#endif

