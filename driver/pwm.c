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

#include "bsp/tmr.h"
#include "driver/pwm.h"

void PWM_Init(void)
{
	tmr_para_init_ex0_type init;

	tmr_para_init_ex0(&init);
	init.period = 512;
	init.division = 3;
	init.clock_division = TMR_CLOCK_DIV1;
	init.count_mode = TMR_COUNT_UP;
	tmr_reset_ex0(TMR3, &init);
	tmr_output_channel_switch_set(TMR3, TMR_SELECT_CHANNEL_1, TRUE);
	tmr_counter_enable(TMR3, TRUE);
	PWM_Reset();
}

void PWM_Reset(void)
{
	PWM_Pulse(0);
	tmr_counter_enable(TMR3, FALSE);
}

void PWM_Pulse(uint16_t Data)
{
	tmr_para_init_ex1_type init;

	tmr_para_init_ex1(&init);
	init.ch1_output_control_mode = TMR_OUTPUT_CONTROL_PWM_MODE_A;
	init.ch1_polarity = TMR_POLARITY_ACTIVE_HIGH;
	init.ch1_enable = true;
	init.ch1_digital_filter = Data;
	tmr_reset_ex1(TMR3, &init);
	tmr_output_channel_buffer_enable(TMR3, TMR_SELECT_CHANNEL_1, true);
	tmr_counter_enable(TMR3, true);
	tmr_output_enable(TMR3, true);
}

