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

#ifndef BSP_TMR_H
#define BSP_TMR_H

#include <at32f421.h>
#include <stdbool.h>

enum {
	TMR_CC_CHANNEL_MAPPED_OUTPUT = 0x00,
};

typedef struct {
	uint32_t period;
	uint16_t division;
	uint8_t repetition;
	tmr_count_mode_type count_mode;
	tmr_clock_division_type clock_division;
} tmr_para_init_ex0_type;

typedef struct {
	bool ch1_enable;
	tmr_output_polarity_type ch1_polarity;
	bool ch1_idle_output_state;

	bool ch1_comp_enable;
	tmr_output_polarity_type ch1_comp_polarity;
	bool ch1_comp_idle_output_state;

	tmr_input_direction_mapped_type ch1_config;

	tmr_output_control_mode_type ch1_output_control_mode;
	uint16_t ch1_digital_filter;
} tmr_para_init_ex1_type;

void tmr_para_init_ex0(tmr_para_init_ex0_type *init);
void tmr_para_init_ex1(tmr_para_init_ex1_type *init);

void tmr_reset_ex0(tmr_type *tmr, const tmr_para_init_ex0_type *init);
void tmr_reset_ex1(tmr_type *tmr, const tmr_para_init_ex1_type *init);

void tmr_output_channel_switch_set_ex(tmr_type *tmr, confirm_state new_state);

#endif

