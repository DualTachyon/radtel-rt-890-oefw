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

#include "app/fm.h"
#include "app/radio.h"
#include "driver/bk1080.h"
#include "driver/pins.h"
#include "driver/speaker.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/settings.h"
#include "ui/helper.h"
#include "ui/main.h"

static bool bLowerBand;

FM_Mode_t gFM_Mode;

// Private

static uint16_t AdjustFrequency(uint32_t Frequency)
{
	int16_t Base;

	if (bLowerBand) {
		Base = 640;
	} else {
		Base = 760;
	}

	// TODO: Original has some dodgy signed division by 1
	return (uint16_t)(Frequency - Base);
}

static void DisableFM(void)
{
	FM_SetVolume(0);
	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK1080_SDA);
	gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SCL);
	gpio_bits_set(GPIOC, BOARD_GPIOC_BK1080_SEN);
	SPEAKER_TurnOff(SPEAKER_OWNER_FM);
}

// Public

void FM_Play(void)
{
	if (gScreenMode == SCREEN_MAIN && gFM_Mode < FM_MODE_PLAY) {
		UI_DrawFM();
		UI_DrawFMFrequency(gSettings.FmFrequency);
		BK1080_Init();
	} else {
		gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SEN);
	}

	if (gFM_Mode > FM_MODE_PLAY) {
		SETTINGS_SaveGlobals();
	}

	FM_Tune(gSettings.FmFrequency);
	FM_SetVolume(15);
	gFM_Mode = FM_MODE_PLAY;
	SPEAKER_TurnOn(SPEAKER_OWNER_FM);
}

void FM_Resume(void)
{
	if (gFM_Mode != FM_MODE_OFF) {
		FM_Play();
	}
}

void FM_UpdateFrequency(void)
{
	uint16_t Frequency = 0;
	uint8_t i;

	for (i = 0; i < 4; i++) {
		Frequency = (Frequency * 10) + gInputBox[i];
	}
	if (Frequency >= 640 && Frequency <= 1080) {
		gSettings.FmFrequency = Frequency;
	} else {
		UI_DrawFMFrequency(gSettings.FmFrequency);
	}
	SETTINGS_SaveGlobals();
	FM_Play();
}

void FM_Disable(bool bStandby)
{
	if (gFM_Mode != FM_MODE_OFF) {
		DisableFM();
		if (gFM_Mode > FM_MODE_PLAY) {
			SETTINGS_SaveGlobals();
		}
		gFM_Mode = bStandby ? FM_MODE_STANDBY : FM_MODE_OFF;
		if (gRadioMode != RADIO_MODE_RX && gRadioMode != RADIO_MODE_TX) {
			RADIO_Tune(gSettings.CurrentVfo);
		}
		UI_DrawMain(true);
	}
}

void FM_Tune(uint16_t Frequency)
{
	uint8_t Values[4];

	if (Frequency > 760) {
		Values[0] = 0x0A;
		Values[1] = 0x5F;
		bLowerBand = false;
	} else {
		Values[0] = 0x0A;
		Values[1] = 0xDF;
		bLowerBand = true;
	}

	BK1080_WriteRegisters(0x05, Values, 2);
	BK1080_ReadRegisters(0x02, Values, 4);

	Frequency = AdjustFrequency(Frequency);
	Values[0] &= 0xFE; // Clear Seek bit
	Values[2] = 0;
	Values[3] = (Frequency >> 0) & 0xFF;
	BK1080_WriteRegisters(0x02, Values, 4);
	Values[2] = 0x80 | ((Frequency >> 8) & 0xFF);
	Values[3] = (Frequency >> 0) & 0xFF;
	BK1080_WriteRegisters(0x02, Values, 4);

	if (gFM_Mode < FM_MODE_SCROLL_UP) {
		if (SPEAKER_State == 0) {
			gpio_bits_set(GPIOA, BOARD_GPIOA_SPEAKER);
		}
		SPEAKER_TurnOn(SPEAKER_OWNER_FM);
	}
}

void FM_SetVolume(uint8_t Volume)
{
	uint8_t Values[2];

	Values[0] = 0x0A;
	Values[1] = Volume | 0x10;
	BK1080_WriteRegisters(0x05, Values, 2);
}

bool FM_CheckSignal(void)
{
	uint8_t Values[8];
	uint16_t Deviation;

	BK1080_ReadRegisters(0x07, Values, 8);

	Deviation = (Values[0] << 4) | ((Values[1] >> 4) & 0xF);

	// AFC railed
	if (Values[6] & 0x10U) {
		return false;
	}

	// SNR < 2
	if ((Values[1] & 0xF) < 2) {
		return false;
	}

	if (Deviation >= 280 && Deviation < 3816) {
		return false;
	}

	return true;
}

