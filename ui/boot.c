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

#include "driver/beep.h"
#include "driver/delay.h"
#include "driver/speaker.h"
#include "helper/helper.h"
#include "radio/settings.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/logo.h"
#include "ui/welcome.h"

static void PlayStartupTone(void)
{
	if (gSettings.StartupRingTone) {
		BEEP_Enable();
	#ifdef MOTO_STARTUP_TONE
		BEEP_SetFrequency(784);
		SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);
		DELAY_WaitMS(250);
		BEEP_SetFrequency(660);
		DELAY_WaitMS(250);
		BEEP_SetFrequency(1046);
		DELAY_WaitMS(250);
		BEEP_Disable();
	#else
		BEEP_SetFrequency(1000);
		SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);
		DELAY_WaitMS(100);
		SPEAKER_TurnOff(SPEAKER_OWNER_SYSTEM);
		DELAY_WaitMS(25);
		BEEP_SetFrequency(1000);
		SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);
		DELAY_WaitMS(100);
	#endif
		BEEP_Disable();
	}
}

void UI_DrawBoot(void)
{
	SCREEN_TurnOn();
	if (gSettings.DisplayLogo) {
		UI_DrawLogo();
	}
	if (gSettings.DisplayLabel || gSettings.DisplayVoltage) {
		DISPLAY_Fill(0, 159, 0, 96, COLOR_BLACK);
	}
	if (gSettings.DisplayLabel) {
		UI_DrawWelcome();
	}
	if (gSettings.DisplayVoltage) {
		UI_DrawBootVoltage(24, 24);
	}
	if (gSettings.DisplayLabel || gSettings.DisplayVoltage) {
		DELAY_WaitMS(1000);
	}

	DISPLAY_Fill(0, 159, 0, 96, COLOR_BLACK);
	PlayStartupTone();
}

