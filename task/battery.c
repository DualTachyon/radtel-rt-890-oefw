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
#include "driver/battery.h"
#include "misc.h"
#include "radio/hardware.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/battery.h"
#include "task/ptt.h"
#include "ui/dialog.h"
#include "ui/helper.h"
#include "ui/main.h"

static uint8_t BatteryLevel;
static uint8_t ChargeTimer;

void Task_CheckBattery(void)
{
	if (gRadioMode == RADIO_MODE_TX || !SCHEDULER_CheckTask(TASK_CHECK_BATTERY) || gBatteryTimer) {
		return;
	}

	SCHEDULER_ClearTask(TASK_CHECK_BATTERY);

	gBatteryVoltage = BATTERY_GetVoltage();

	if (gRadioMode != RADIO_MODE_RX
			&& VOX_Timer == 0
			&& gFM_Mode == FM_MODE_OFF
			&& gScreenMode == SCREEN_MAIN
			&& !gDTMF_InputMode
			&& !gFlashlightMode) {
		UI_DrawVoltage(!gSettings.CurrentVfo);
	}
	UI_DrawBattery(!gSettings.RepeaterMode);

	if (BatteryLevel && ChargeTimer++ >= 30) {
		ChargeTimer = 0;
		if (gScreenMode == SCREEN_MAIN) {
			UI_DrawDialogText(DIALOG_PLEASE_CHARGE, true);
		}
	}
	if (BatteryLevel != 0) {
		if (BatteryLevel == 1) {
			if (gBatteryVoltage > gCalibration.BatteryCalibration[3]) {
				BatteryLevel = 0;
			} else if (gBatteryVoltage < gCalibration.BatteryCalibration[1]) {
				BatteryLevel = 2;
				PTT_SetLock(PTT_LOCK_BATTERY);
			}
		} else if (BatteryLevel == 2) {
			if (gCalibration.BatteryCalibration[2] < gBatteryVoltage) {
				BatteryLevel = 1;
				PTT_ClearLock(PTT_LOCK_BATTERY);
			} else if (gBatteryVoltage < gCalibration.BatteryCalibration[0]) {
				HARDWARE_Reboot();
			}
		}
	} else {
		if (gBatteryVoltage < gCalibration.BatteryCalibration[2]) {
			BatteryLevel = 1;
			ChargeTimer = 30;
		}
	}
}

