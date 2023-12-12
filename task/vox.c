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

#ifdef ENABLE_FM_RADIO
	#include "app/fm.h"
#endif
#include "app/radio.h"
#include "driver/bk4819.h"
#include "helper/helper.h"
#include "misc.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/vox.h"
#include "ui/helper.h"

static const uint16_t gVoxTable[10] = {
	0x0000,
	0x0002,
	0x0004,
	0x0006,
	0x000A,
	0x003C,
	0x006E,
	0x00A0,
	0x00D2,
	0x0104,
};

uint16_t gVoxRssiUpdateTimer;
uint16_t VOX_Counter;
bool VOX_IsTransmitting;

static bool CheckStatus(void)
{
	uint16_t Value;

	Value = BK4819_ReadRegister(0x64);
	if (Value >= gVoxTable[gSettings.VoxLevel] + 150) {
		return true;
	}
	if (Value <= gVoxTable[gSettings.VoxLevel] + 145) {
		return false;
	}

	return VOX_IsTransmitting;
}

// Public

void VOX_Update(void)
{
	if (gVoxRssiUpdateTimer == 0) {
		uint16_t Vox;

		gVoxRssiUpdateTimer = 100;
		Vox = BK4819_ReadRegister(0x64);
		if (Vox > 5000) {
			Vox = 5000;
		}
		UI_DrawBar(Vox / 50, gSettings.CurrentVfo);
	}
}

void Task_VoxUpdate(void)
{
	if (gSettings.Vox && gPttLock == 0 && !gSaveMode && gScreenMode == SCREEN_MAIN && VOX_Timer == 0) {
		if (SCHEDULER_CheckTask(TASK_VOX)
#ifdef ENABLE_FM_RADIO
			&& gFM_Mode == FM_MODE_OFF
#endif
			&& !gDTMF_InputMode) {
			bool bFlag;

			SCHEDULER_ClearTask(TASK_VOX);

			bFlag = CheckStatus();
			if (!bFlag || gRadioMode != RADIO_MODE_QUIET) {
				if (!bFlag && gRadioMode == RADIO_MODE_TX) {
					uint16_t Delay;

					Delay = VOX_Counter++ / 64;
					if (Delay >= gSettings.VoxDelay) {
						VOX_IsTransmitting = false;
						VOX_Counter = 0;
						RADIO_EndTX();
					}
					VOX_Update();
				} else {
					if (VOX_IsTransmitting) {
						VOX_Update();
					}
					VOX_Counter = 0;
				}
			} else {
				if (VOX_Counter++ > 4) {
					VOX_IsTransmitting = true;
					SCREEN_TurnOn();
					RADIO_StartTX(true);
				}
			}
		}
	}
}

