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

#include <at32f421.h>
#include "app/fm.h"
#include "driver/bk1080.h"
#include "driver/pins.h"
#include "driver/speaker.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/settings.h"
#include "ui/helper.h"

void FM_Play(void)
{
	if (gScreenMode == SCREEN_MAIN && gVfoMode < VFO_MODE_FM) {
		UI_DrawFM();
		UI_DrawFMFrequency(gSettings.FmFrequency);
		BK1080_Init();
	} else {
		gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SEN);
	}

	if (gVfoMode > VFO_MODE_FM) {
		SETTINGS_SaveGlobals();
	}

	BK1080_Tune(gSettings.FmFrequency);
	BK1080_SetVolume(15);
	gVfoMode = VFO_MODE_FM;
	SPEAKER_TurnOn(SPEAKER_OWNER_FM);
}

void FM_Resume(void)
{
	if (gVfoMode != VFO_MODE_MAIN) {
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

