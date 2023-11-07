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

#include "driver/pins.h"
#include "helper/helper.h"
#include "misc.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/timeout.h"

void Task_CheckDisplayTimeout(void)
{
	const uint16_t Timer = TIMER_Calculate(gSettings.DisplayTimer);

	if (!gSettings.bEnableDisplay) {
		STANDBY_BlinkGreen();
		return;
	}

	if (gSettings.DisplayTimer == 0 || gFlashlightMode) {
		STANDBY_Counter = 0;
	} else if (gEnableBlink) {
		STANDBY_BlinkGreen();
	} else if ((STANDBY_Counter / 1000) > Timer) {
		gEnableBlink = true;
		STANDBY_Counter = 0;
		gpio_bits_reset(GPIOA, BOARD_GPIOA_LCD_RESX);
	}
}

