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

void tmr_para_init_ex0(tmr_para_init_ex0_type *init)
{
	init->period = 0xFFFFU;
	init->division = 0;
	init->clock_division = TMR_CLOCK_DIV1;
	init->count_mode = TMR_COUNT_UP;
	init->repetition = 0;
}

void tmr_para_init_ex1(tmr_para_init_ex1_type *init)
{
	init->ch1_config = 0;
	init->ch1_output_control_mode = 0;
	init->ch1_enable = false;
	init->ch1_comp_enable = false;
	init->ch1_digital_filter = 0;
	init->ch1_polarity = TMR_OUTPUT_ACTIVE_HIGH;
	init->ch1_comp_polarity = TMR_OUTPUT_ACTIVE_HIGH;
	init->ch1_idle_output_state = false;
	init->ch1_comp_idle_output_state = false;
}

void tmr_reset_ex0(tmr_type *tmr, const tmr_para_init_ex0_type *init)
{
	if (tmr == TMR1 || tmr == TMR14 || tmr == TMR15 || tmr == TMR16 || tmr == TMR17 || tmr == TMR3) {
		tmr->ctrl1_bit.cnt_dir = init->count_mode;
	}
	if (tmr != TMR6) {
		tmr->ctrl1_bit.clkdiv = init->clock_division;
	}

	tmr->pr = init->period;
	tmr->div = init->division;

	if (tmr == TMR1 || tmr == TMR16 || tmr == TMR17) {
		tmr->rpr = init->repetition;
	}

	tmr->swevt_bit.ovfswtr = 1;
}

void tmr_reset_ex1(tmr_type *tmr, const tmr_para_init_ex1_type *init)
{
	tmr->cctrl_bit.c1en = 0;

	if (tmr == TMR1 || tmr == TMR3 || tmr == TMR14 || tmr == TMR15 || tmr == TMR16 || tmr == TMR17) {
		tmr->cctrl_bit.c1cp   = init->ch1_comp_polarity;
		tmr->cctrl_bit.c1cen  = init->ch1_comp_enable;

		tmr->ctrl2_bit.c1ios  = init->ch1_idle_output_state;
		tmr->ctrl2_bit.c1cios = init->ch1_comp_idle_output_state;
	}

	tmr->cm1_output_bit.c1c = init->ch1_config;
	tmr->cm1_output_bit.c1octrl = init->ch1_output_control_mode;

	tmr->c1dt = init->ch1_digital_filter;

	tmr->cctrl_bit.c1p = init->ch1_polarity;
	tmr->cctrl_bit.c1en = init->ch1_enable;
}

void tmr_output_channel_switch_set_ex(tmr_type *tmr, confirm_state new_state)
{
	tmr->cm1_output_bit.c1osen = new_state;
}

