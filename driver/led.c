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
#include "driver/led.h"
#include "driver/pins.h"

void LED_Init(void)
{
	gpio_init_type init;
	gpio_default_para_init_ex(&init);

	init.gpio_pins = BOARD_GPIOA_LED_RED | BOARD_GPIOA_LED_GREEN;
	init.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	init.gpio_mode = GPIO_MODE_OUTPUT;
	init.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	init.gpio_pull = GPIO_PULL_NONE;

	gpio_init(GPIOA, &init);

	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);
}

