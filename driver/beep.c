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
#include "driver/bk4819.h"
#include "driver/speaker.h"
#include "misc.h"
#include "radio/settings.h"

static uint8_t BeepType;
static uint8_t BeepRunCounter;
static uint16_t BeepDuration;

void BEEP_Interrupt(void)
{
	if (BeepDuration == 0) {
		switch (BeepType) {
		case 2:
			BeepRunCounter = 0;
			BeepType = 0;
			BEEP_Disable();
			break;

		case 3:
		case 4:
			BeepRunCounter++;
			if (BeepType == 3) {
				BEEP_SetFrequency(440);
			} else {
				BEEP_SetFrequency(740);
			}
			if (BeepRunCounter > 1) {
				BeepRunCounter = 0;
				BeepType = 0;
				BEEP_Disable();
			} else {
				BeepDuration = 80;
			}
			break;
		}
	} else {
		BeepDuration--;
	}
}

void BEEP_Enable(void)
{
	BK4819_SetAfGain(0xB32A);
	BK4819_EnableTone1(true);
}

void BEEP_Disable(void)
{
	SPEAKER_TurnOff(SPEAKER_OWNER_SYSTEM);
	BK4819_SetToneFrequency(false, 0);
	BK4819_EnableTone1(false);
}

void BEEP_Play(uint16_t Frequency, uint8_t Type, uint16_t Duration)
{
	if (gSettings.KeyBeep) {
		if (BeepType != 0) {
			BeepType = 0;
			BeepDuration = 0;
			BEEP_Disable();
		}
		BEEP_Enable();
		BEEP_SetFrequency(Frequency);
		BeepRunCounter = 0;
		BeepType = Type;
		BeepDuration = Duration;
		SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);
	}
}

void BEEP_SetFrequency(uint16_t Frequency)
{
	if (gSaveMode) {
		BK4819_EnableRX();
	}
	BK4819_SetToneFrequency(false, Frequency);
}

void BEEP_SetTone2Frequency(uint16_t Frequency)
{
	if (gSaveMode) {
		BK4819_EnableRX();
	}
	BK4819_SetToneFrequency(true, Frequency);
}
