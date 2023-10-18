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
#include "driver/bk1080.h"
#include "driver/speaker.h"
#include "misc.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/fmscanner.h"
#include "ui/helper.h"

void Task_CheckScannerFM(void)
{
	if (gVfoMode < VFO_MODE_FM_SCROLL_UP || !SCHEDULER_CheckTask(TASK_FM_SCANNER)) {
		return;
	}

	SCHEDULER_ClearTask(TASK_FM_SCANNER);

	if (BK1080_CheckSignal()) {
		FM_Play();
		return;
	}

	if (gVfoMode == VFO_MODE_FM_SCROLL_UP) {
		gSettings.FmFrequency++;
		if (gSettings.FmFrequency > 1080) {
			gSettings.FmFrequency = 640;
		}
	} else {
		gSettings.FmFrequency--;
		if (gSettings.FmFrequency < 640) {
			gSettings.FmFrequency = 1080;
		}
	}

	BK1080_Tune(gSettings.FmFrequency);
	BK1080_SetVolume(0);
	SPEAKER_TurnOff(SPEAKER_OWNER_FM);
	UI_DrawFMFrequency(gSettings.FmFrequency);
}

