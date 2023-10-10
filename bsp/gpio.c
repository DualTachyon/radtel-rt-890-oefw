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

