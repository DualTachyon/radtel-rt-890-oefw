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
#include "driver/beep.h"
#include "driver/pins.h"
#include "helper/helper.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/alarm.h"
#include "task/cursor.h"
#include "task/ptt.h"
#include "task/vox.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "unknown.h"

static uint8_t gPttCounter;

bool gPttPressed;

void Task_CheckPTT(void)
{
	const uint16_t Timer = TIMER_Calculate(gSettings.TimeoutTimer);

	if (!SCHEDULER_CheckTask(TASK_CHECK_PTT)) {
		return;
	}

	SCHEDULER_ClearTask(TASK_CHECK_PTT);
	if (!gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_KEY_PTT)) {
		if (gSettings.DtmfState == DTMF_STATE_NORMAL && !gPttPressed) {
			if (gPttCounter++ < 100) {
				return;
			}
			SCREEN_TurnOn();
			if (gVfoMode == VFO_MODE_MAIN) {
				if (!gScannerMode) {
					if (!gReceptionMode) {
						if (!gEnableLocalAlarm) {
							if (gInputBoxWriteIndex == 0) {
								if (gScreenMode != SCREEN_MAIN) {
									gScreenMode = SCREEN_MAIN;
									gCursorEnabled = false;
									if (gRadioMode != RADIO_MODE_RX) {
										RADIO_Tune(gSettings.CurrentVfo);
									}
									UI_DrawMain(true);
									gPttPressed = true;
								}
							} else {
								if (gSettings.WorkMode) {
									RADIO_DrawWorkMode();
								} else {
									RADIO_DrawFrequencyMode();
								}
								gPttPressed = true;
							}
						} else {
							ALARM_Stop();
							gPttPressed = true;
						}
					} else {
						RADIO_NoaaRetune();
						gPttPressed = true;
					}
				} else {
					SETTINGS_SaveState();
					gPttPressed = true;
				}
			} else {
				VFO_SetMode(VFO_MODE_MAIN);
				gPttPressed = true;
			}
			if (gPttPressed) {
				BEEP_Play(440, 4, 80);
				return;
			} else if (gRadioMode == RADIO_MODE_TX) {
				UI_DrawVox();
				if (Timer && (gPttTimeout / 1000) >= Timer) {
					PTT_SetLock(PTT_LOCK_VOX);
					RADIO_EndTX();
				}
			} else if (gPttLock == 0) {
				RADIO_StartTX(true);
			}
		}
	} else {
		gPttCounter = 0;
		gPttPressed = false;
		gPttTimeout = 0;
		PTT_ClearLock(PTT_LOCK_VOX);
		if (gRadioMode == RADIO_MODE_TX && !VOX_IsTransmitting && !gEnableLocalAlarm) {
			RADIO_EndTX();
		}
	}
}

void PTT_SetLock(uint8_t Flags)
{
	gPttLock |= Flags;
}

void PTT_ClearLock(uint8_t Flags)
{
	gPttLock &= ~Flags;
}

