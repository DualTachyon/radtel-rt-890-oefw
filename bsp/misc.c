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

void systick_clock_source_config(systick_clock_source_type source)
{
	if (source == SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV) {
		SysTick->CTRL |= SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV;
	} else {
		SysTick->CTRL &= ~(uint32_t)SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV;
	}
}

void nvic_irq_enable(IRQn_Type irqn, uint32_t preempt_priority, uint32_t sub_priority)
{
	uint32_t temp_priority = 0;

	/* encode priority */
	temp_priority = NVIC_EncodePriority(NVIC_GetPriorityGrouping(), preempt_priority, sub_priority);
	/* set priority */
	NVIC_SetPriority(irqn, temp_priority);
	/* enable irqn */
	NVIC_EnableIRQ(irqn);
}

void nvic_irq_disable(IRQn_Type irqn)
{
	NVIC_DisableIRQ(irqn);
}

