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
#include "misc.h"
#include "radio/scheduler.h"
#include "task/screen.h"
#include "ui/main.h"
#include "unknown.h"

void Task_UpdateScreen(void)
{
	if (VOX_Timer == 0 && gRedrawScreen) {
		gRedrawScreen = false;
		if (!FUN_08006d64()) {
			if (gScreenMode == SCREEN_MAIN && !gReceptionMode) {
				UI_DrawMain(true);
			}
		} else if (gScreenMode == SCREEN_MAIN && !gReceptionMode) {
			if (gVfoMode == VFO_MODE_MAIN) {
				UI_DrawMain(true);
			} else if (gRadioMode != RADIO_MODE_RX) {
				FM_Resume();
			}
		}
	}
}

