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

#include "app/flashlight.h"
#include "app/radio.h"
#ifdef ENABLE_FM_RADIO
	#include "app/fm.h"
#endif
#include "driver/beep.h"
#include "driver/key.h"
#include "driver/pins.h"
#include "helper/helper.h"
#include "misc.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/alarm.h"
#include "task/keyaction.h"
#include "task/sidekeys.h"

void Task_CheckSideKeys(void)
{
	uint8_t Action;

	if (!SCHEDULER_CheckTask(TASK_CHECK_SIDE_KEYS) || gSettings.DtmfState != DTMF_STATE_NORMAL) {
		return;
	}

	SCHEDULER_ClearTask(TASK_CHECK_SIDE_KEYS);

	// ??? Such a specific number
	gSlot = 6;

	if (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1) && KEY_Side1Counter > 1000) {
		KEY_SideKeyLongPressed = true;
		KEY_Side1Counter = 0;
		gSlot = 0;
	} else if (!gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_SIDE2) && KEY_Side2Counter > 1000) {
		KEY_SideKeyLongPressed = true;
		KEY_Side2Counter = 0;
		gSlot = 2;
	} else if (gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1) && gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_SIDE2)) {
		if (KEY_Side1Counter > 100) {
			gSlot = 1;
		} else if (KEY_Side2Counter > 100) {
			gSlot = 3;
		}
		KEY_SideKeyLongPressed = false;
		KEY_Side1Counter = 0;
		KEY_Side2Counter = 0;
	}

	if (gSlot >= 4) {
		return;
	}

	Action = gSettings.Actions[gSlot];
	if (gSettings.bEnableDisplay && gEnableBlink) {
		SCREEN_TurnOn();
		BEEP_Play(740, 2, 100);
		return;
	}

	if (Action == ACTION_NONE) {
		return;
	}

	SCREEN_TurnOn();

	KeypressAction(Action);
}

