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
#include "driver/bk4819.h"
#include "helper/helper.h"
#include "helper/dtmf.h"
#include "radio/settings.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/logo.h"
#include "ui/welcome.h"
#include "misc.h"

static void PlayStartupTone(void)
{
	if (gSettings.StartupRingTone) {
		BEEP_Enable();
	#ifdef MOTO_STARTUP_TONE
		BEEP_SetFrequency(260);
		SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);
		BK4819_SetAF(BK4819_AF_BEEP);
		DELAY_WaitMS(250);
		// the secret sauce - two tones at once.
		// this requires dtmf abuse.
		gDTMF_Playing = true;
		gStartupSoundPlaying = true;
		DTMF_PlayTone(0);
		DELAY_WaitMS(1);
		BEEP_SetFrequency(398);
		BEEP_SetTone2Frequency(788);
		DELAY_WaitMS(200);
		BEEP_SetFrequency(662);
		BEEP_SetTone2Frequency(998);
		DELAY_WaitMS(200);
		BEEP_SetFrequency(528);
		BEEP_SetTone2Frequency(1571);
		DELAY_WaitMS(200);
		DTMF_Disable();
		gDTMF_Playing = false;
		gStartupSoundPlaying = false;

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
		SPEAKER_TurnOff(SPEAKER_OWNER_SYSTEM);
	}
}

void UI_DrawBoot(void)
{
	SCREEN_TurnOn();
	if (gSettings.DisplayLogo) {
		UI_DrawLogo();
	}
	PlayStartupTone();
	if (gSettings.DisplayLabel || gSettings.DisplayVoltage) {
		DISPLAY_Fill(0, 159, 0, 96, COLOR_BACKGROUND);
	}
	if (gSettings.DisplayLabel) {
		UI_DrawWelcome();
	}
	if (gSettings.DisplayVoltage) {
		UI_DrawBootVoltage(24, 24);
	}
	if (gSettings.DisplayLabel || gSettings.DisplayVoltage) {
		DELAY_WaitMS(600);
	}

	DISPLAY_Fill(0, 159, 0, 96, COLOR_BACKGROUND);
}

