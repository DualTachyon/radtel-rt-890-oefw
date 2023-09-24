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

#ifndef DRIVER_BK4819_H
#define DRIVER_BK4819_H

#include <stdbool.h>
#include <stdint.h>

enum BK4819_AF_Type_t {
	BK4819_AF_MUTE = 0U,
	BK4819_AF_OPEN = 1U,
	BK4819_AF_ALAM = 2U,
	BK4819_AF_BEEP = 3U,
	BK4819_AF_CTCO = 6U,
	BK4819_AF_AM   = 7U,
	BK4819_AF_FSKO = 8U,
};

typedef enum BK4819_AF_Type_t BK4819_AF_Type_t;

uint16_t BK4819_ReadRegister(uint8_t Reg);
void BK4819_WriteRegister(uint8_t Reg, uint16_t Data);

void BK4819_Init(void);
void BK4819_SetAFResponseCoefficients(bool bTx, bool bLowPass, uint8_t Index);
void BK4819_RX_TurnOn(void);
void BK4819_SetAF(BK4819_AF_Type_t Type);
void BK4819_SetFrequency(uint32_t Frequency);
void BK4819_SetSquelchGlitchThreshold(bool bIsNarrow);
void BK4819_SetSquelchNoiseThreshold(bool bIsNarrow);
void BK4819_SetSquelchRSSIThreshold(bool bIsNarrow);
void BK4819_SetFilterBandwidth(bool bIsNarrow);
void BK4819_UpdateGpioOut(bool bEnable);
void BK4819_EnableScramble(bool bIsNarrow);
void BK4819_EnableAF_ExpanderCompress(bool bIsNarrow);
void BK4819_EnableVox(bool bEnable);
void BK4819_SetToneFrequency(uint16_t Tone);
void BK4819_SetupFSK(bool bEnable);
void BK4819_ResetFSK(void);
void BK4819_StartAudio(void);
void BK4819_SetAF_RX_and_DAC_Gain(uint16_t Gain);
void BK4819_InitDTMF(void);
bool BK4819_CheckSquelchLink(void);
void BK4819_EnableTone1(bool bEnable);
void BK4819_GenTail(bool bIsNarrow);
void BK4819_SetupPowerAmplifier(uint8_t Bias);
void BK4819_EnableRfTxDeviation(void);
void BK4819_SetMicSensitivityTuning(void);
void BK4819_TxOn_BeepWithATwist(bool bFlag);
void BK4819_StartFrequencyScan(void);
void BK4819_StopFrequencyScan(void);
void BK4819_EnableAutoCssBW(void);

#endif

