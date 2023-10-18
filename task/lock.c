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

#include "app/lock.h"
#include "helper/helper.h"
#include "misc.h"
#include "radio/settings.h"
#include "task/lock.h"

uint32_t gLockTimer;

void Task_CheckLockScreen(void)
{
	uint16_t Timer;

	Timer = TIMER_Calculate(gSettings.LockTimer);
	if (gSettings.LockTimer == 0 || gSettings.Lock || gScreenMode != SCREEN_MAIN || gEnableLocalAlarm || gScannerMode || gDTMF_InputMode || gSettings.DtmfState != DTMF_STATE_NORMAL || gReceptionMode) {
		gLockTimer = 0;
	} else if ((gLockTimer / 1000) >= Timer) {
		LOCK_Toggle();
	}
}

