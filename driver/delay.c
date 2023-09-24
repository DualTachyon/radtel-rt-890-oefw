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

#include <at32f421.h>
#include "driver/crm.h"
#include "driver/delay.h"

static uint32_t gCyclesPerNanoSec;
static uint32_t gCyclesPerMicroSec;

void DELAY_Init(void)
{
	systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
	gCyclesPerNanoSec = gSystemCoreClock / 1000000;
	gCyclesPerMicroSec = (gSystemCoreClock / 1000000) * 1000;
}

void DELAY_WaitNS(uint32_t Delay)
{
	uint32_t Control;

	SysTick->LOAD = gCyclesPerNanoSec * Delay;
	SysTick->VAL = 0;
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

	do {
		Control = SysTick->CTRL;
	} while (Control & SysTick_CTRL_ENABLE_Msk && (Control & SysTick_CTRL_COUNTFLAG_Msk) == 0);

	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	SysTick->VAL = 0;
}

void DELAY_WaitUS(uint16_t Delay)
{
	uint32_t Control;

	while (1) {
		if (Delay == 0) {
			return;
		}
		if (Delay > 50) {
			SysTick->LOAD = gCyclesPerMicroSec * 50;
			Delay = Delay - 50;
		} else {
			SysTick->LOAD = gCyclesPerMicroSec * Delay;
			Delay = 0;
		}
		SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

		do {
			Control = SysTick->CTRL;
		} while (Control & SysTick_CTRL_ENABLE_Msk && (Control & SysTick_CTRL_COUNTFLAG_Msk) == 0);

		SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
		SysTick->VAL = 0;
	}
}

void DELAY_WaitMS(uint16_t Delay)
{
	uint16_t i;

	for (i = 0; i < (Delay / 500); i++) {
		DELAY_WaitUS(500);
		DELAY_WaitNS(13000);
	}
	DELAY_WaitUS(Delay - (500 * (Delay / 500)));
}

