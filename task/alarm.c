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

#include "app/radio.h"
#include "bsp/gpio.h"
#include "driver/bk4819.h"
#include "driver/pins.h"
#include "driver/speaker.h"
#include "misc.h"
#include "task/alarm.h"

uint16_t gAlarmFrequency = 800;
uint8_t gAlarmCounter;
bool gAlarmSiren;

void Task_LocalAlarm(void)
{
	if (gEnableLocalAlarm && !gSendTone && gAlarmCounter > 15) {
		gAlarmCounter = 0;

		if (!gAlarmSiren) {
			gAlarmFrequency += 80;
		} else {
			gAlarmFrequency -= 80;
		}

		if (gAlarmFrequency < 850 || gAlarmFrequency > 1450) {
			gAlarmSiren = !gAlarmSiren;
			gpio_bits_flip(GPIOA, BOARD_GPIOA_LED_RED);
		}
		BK4819_SetToneFrequency(false, gAlarmFrequency);
	}
}

void ALARM_Start(void)
{
	gEnableLocalAlarm = true;
	gAlarmSiren = false;
	gAlarmFrequency = 800;
	gAlarmCounter = 0;
	BK4819_SetAfGain(0xB325);
	BK4819_EnableTone1(true);
	BK4819_SetToneFrequency(false, gAlarmFrequency);
	SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);
}

void ALARM_Stop(void)
{
	gEnableLocalAlarm = false;
	gSendTone = false;
	BK4819_SetToneFrequency(false, 0);
	BK4819_EnableTone1(false);
	SPEAKER_TurnOff(SPEAKER_OWNER_SYSTEM);
	if (gRadioMode == RADIO_MODE_TX) {
		RADIO_EndTX();
	}
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);
}

