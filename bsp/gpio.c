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

#include "bsp/gpio.h"

void gpio_bits_flip(gpio_type *gpio, uint16_t pins)
{
	if (gpio_output_data_bit_read(gpio, pins)) {
		gpio_bits_reset(gpio, pins);
	} else {
		gpio_bits_set(gpio, pins);
	}
}

// Original FW used a modified SDK that uses MODERATE instead of STRONGER
void gpio_default_para_init_ex(gpio_init_type *init)
{
	init->gpio_pins  = GPIO_PINS_ALL;
	init->gpio_mode = GPIO_MODE_INPUT;
	init->gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	init->gpio_pull = GPIO_PULL_NONE;
	init->gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
}

void gpio_bits_set(gpio_type *gpio_x, uint16_t pins)
{
	gpio_x->scr = pins;
}

void gpio_bits_reset(gpio_type *gpio_x, uint16_t pins)
{
	gpio_x->clr = pins;
}

flag_status gpio_input_data_bit_read(gpio_type *gpio_x, uint16_t pins)
{
	flag_status status = RESET;

	if (pins != (pins & gpio_x->idt)) {
		status = RESET;
	} else {
		status = SET;
	}

	return status;
}

void gpio_init(gpio_type *gpio_x, gpio_init_type *gpio_init_struct)
{
	uint16_t pinx_value, pin_index = 0;

	pinx_value = (uint16_t)gpio_init_struct->gpio_pins;

	while (pinx_value > 0) {
		if (pinx_value & 0x01) {
			gpio_x->cfgr  &= (uint32_t)~(0x03 << (pin_index * 2));
			gpio_x->cfgr  |= (uint32_t)(gpio_init_struct->gpio_mode << (pin_index * 2));

			gpio_x->omode &= (uint32_t)~(0x01 << (pin_index));
			gpio_x->omode |= (uint32_t)(gpio_init_struct->gpio_out_type << (pin_index));

			gpio_x->odrvr &= (uint32_t)~(0x03 << (pin_index * 2));
			gpio_x->odrvr |= (uint32_t)(gpio_init_struct->gpio_drive_strength << (pin_index * 2));

			gpio_x->pull  &= (uint32_t)~(0x03 << (pin_index * 2));
			gpio_x->pull  |= (uint32_t)(gpio_init_struct->gpio_pull << (pin_index * 2));
		}
		pinx_value >>= 1;
		pin_index++;
	}
}

void gpio_pin_mux_config(gpio_type *gpio_x, gpio_pins_source_type gpio_pin_source, gpio_mux_sel_type gpio_mux)
{
	uint32_t temp = 0x00;
	uint32_t temp_2 = 0x00;

	temp = ((uint32_t)(gpio_mux) << ((uint32_t)((uint32_t)gpio_pin_source & (uint32_t)0x07) * 4));
	if (gpio_pin_source >> 0x03) {
		gpio_x->muxh &= ~((uint32_t)0xF << ((uint32_t)((uint32_t)gpio_pin_source & (uint32_t)0x07) * 4));
		temp_2 = gpio_x->muxh | temp;
		gpio_x->muxh = temp_2;
	} else {
		gpio_x->muxl &= ~((uint32_t)0xF << ((uint32_t)((uint32_t)gpio_pin_source & (uint32_t)0x07) * 4));
		temp_2 = gpio_x->muxl | temp;
		gpio_x->muxl = temp_2;
	}
}

flag_status gpio_output_data_bit_read(gpio_type *gpio_x, uint16_t pins)
{
	flag_status status = RESET;

	if ((gpio_x->odt & pins) != RESET) {
		status = SET;
	} else {
		status = RESET;
	}

	return status;
}

