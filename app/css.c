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

#include "app/css.h"
#include "driver/bk4819.h"
#include "radio/frequencies.h"

static uint16_t CTCSS_Options[50] = {
	0x29E, 0x2B5, 0x2CF, 0x2E8,
	0x302, 0x31D, 0x339, 0x356,
	0x375, 0x393, 0x3B4, 0x3CE,
	0x3E8, 0x40B, 0x430, 0x455,
	0x47C, 0x4A4, 0x4CE, 0x4F9,
	0x526, 0x555, 0x585, 0x5B6,
	0x5EA, 0x61F, 0x63E, 0x656,
	0x677, 0x68F, 0x6B1, 0x6CA,
	0x6ED, 0x707, 0x72B, 0x746,
	0x76B, 0x788, 0x7AE, 0x7CB,
	0x7F3, 0x811, 0x83B, 0x885,
	0x8D1, 0x8F3, 0x920, 0x972,
	0x9C7, 0x9ED,
};

static uint16_t DCS_Options[105] = {
	0x13, 0x15, 0x16, 0x19,
	0x1A, 0x1E, 0x23, 0x27,
	0x29, 0x2B, 0x2C, 0x35,
	0x39, 0x3A, 0x3B, 0x3C,
	0x4C, 0x4D, 0x4E, 0x52,
	0x55, 0x59, 0x5A, 0x5C,
	0x63, 0x65, 0x6A, 0x6D,
	0x6E, 0x72, 0x75, 0x7A,
	0x7C, 0x85, 0x8A, 0x93,
	0x95, 0x96, 0xA3, 0xA4,
	0xA5, 0xA6, 0xA9, 0xAA,
	0xAD, 0xB1, 0xB3, 0xB5,
	0xB6, 0xB9, 0xBC, 0xC6,
	0xC9, 0xCD, 0xD5, 0xD9,
	0xDA, 0xE3, 0xE6, 0xE9,
	0xEE, 0xF4, 0xF5, 0xF9,
	0x09, 0x0A, 0x0B, 0x13,
	0x19, 0x1A, 0x25, 0x26,
	0x2A, 0x2C, 0x2D, 0x32,
	0x34, 0x35, 0x36, 0x43,
	0x46, 0x4E, 0x53, 0x56,
	0x5A, 0x66, 0x75, 0x86,
	0x8A, 0x94, 0x97, 0x99,
	0x9A, 0xA5, 0xAC, 0xB2,
	0xB4, 0xC3, 0xCA, 0xD3,
	0xD9, 0xDA, 0xDC, 0xE3,
	0xEC,
};

static uint32_t CalculateCode(uint32_t Code, bool bInverse)
{
	uint32_t Golay;

	if (bInverse) {
		Golay = (Code ^ 0x28) + 0xa00;
	} else {
		Golay = Code + 0x800;
	}
	Golay = CSS_CalculateGolay(Golay);
	if (bInverse) {
		Golay ^= 0x200;
	}

	return Golay & 0x7FFFFFU;
}

//

uint32_t CSS_CalculateGolay(uint32_t Code)
{
	uint32_t Golay;
	uint32_t Tmp;
	uint8_t i;

	Golay = 0;
	for (i = 0; i < 12; i++) {
		Golay = (Golay << 1) + (Code & 1);
		Code >>= 1;
	}
	Golay <<= 11;
	Tmp = Golay;
	for (i = 0; i < 12; i++) {
		if (Tmp >> (0x16 - i)) {
			Tmp ^= 0xAE3 << (11 - i);
		}
	}
	Tmp += Golay;
	Golay = 0;
	for (i = 0; i < 23; i++) {
		Golay = (Golay << 1) + (Tmp & 1);
		Tmp >>= 1;
	}

	return Golay;
}

void CSS_SetCustomCode(bool bIs24Bit, uint16_t Code, bool bIsNarrow)
{
	uint16_t Gain = 0x8000;

	if (bIsNarrow) {
		Gain |= gFrequencyBandInfo.DcsTxGainNarrow;
	} else {
		Gain |= gFrequencyBandInfo.DcsTxGainWide;
	}
	if (bIs24Bit) {
		Gain |= 0x0800;
	}
	BK4819_WriteRegister(0x51, Gain);
	BK4819_WriteRegister(0x07, 2775);
	BK4819_WriteRegister(0x08, 0x0000 | ((Code >>  0) & 0xFFFU));
	BK4819_WriteRegister(0x08, 0x8000 | ((Code >> 12) & 0xFFFU));
}

void CSS_SetStandardCode(uint8_t CodeType, uint16_t Code, uint8_t Encrypt, bool bNarrow)
{
	uint16_t Enable;
	uint32_t Golay;

	switch (CodeType) {
	case CODE_TYPE_CTCSS:
		if (bNarrow) {
			BK4819_WriteRegister(0x51, gFrequencyBandInfo.CtcssTxGainNarrow | 0x9000);
		} else {
			BK4819_WriteRegister(0x51, gFrequencyBandInfo.CtcssTxGainWide | 0x9000);
		}
		BK4819_WriteRegister(0x07, ((Code * 413) / 200) & 0x1FFF);
		break;

	case CODE_TYPE_OFF:
		BK4819_WriteRegister(0x51, 0x0000);
		break;

	case CODE_TYPE_DCS_N:
	case CODE_TYPE_DCS_I:
		if (Encrypt == 1 || Encrypt == 3) {
			Golay = CalculateCode(Code, true);
		} else {
			Golay = CalculateCode(Code, false);
		}
		if (CodeType == CODE_TYPE_DCS_I) {
			Enable = 0xA000;
		} else {
			Enable = 0x8000;
		}
		if (bNarrow) {
			BK4819_WriteRegister(0x51, Enable | gFrequencyBandInfo.DcsTxGainNarrow);
		} else {
			BK4819_WriteRegister(0x51, Enable | gFrequencyBandInfo.DcsTxGainWide);
		}
		BK4819_WriteRegister(0x07, 2775);
		BK4819_WriteRegister(0x08, 0x0000 | ((Golay >>  0) & 0xFFFU));
		BK4819_WriteRegister(0x08, 0x8000 | ((Golay >> 12) & 0xFFFU));
	}
}

uint16_t CSS_ConvertCode(uint16_t Code)
{
	return (Code & 7) + (((Code >> 6) & 7) * 10 + ((Code >> 3) & 7)) * 10;
}

uint16_t CTCSS_GetOption(uint8_t Index)
{
	return CTCSS_Options[Index];
}

uint16_t DCS_GetOption(uint8_t Index)
{
	uint16_t Option = DCS_Options[Index];

	if (Option > 63) {
		Option |= 0x100;
	}

	return Option;
}

