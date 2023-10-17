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

#ifndef BOARD_COMMON_H
#define BOARD_COMMON_H

#include <at32f421.h>

enum {
	// GPIO A
	BOARD_GPIOA_LCD_SCL     = GPIO_PINS_0,
	BOARD_GPIOA_LCD_SDA     = GPIO_PINS_4,
	BOARD_GPIOA_LCD_RESX    = GPIO_PINS_5,

	BOARD_GPIOA_SPEAKER     = GPIO_PINS_1,
	BOARD_GPIOA_AF_OUT      = GPIO_PINS_6,

	BOARD_GPIOA_SF_MOSI     = GPIO_PINS_7,

	BOARD_GPIOA_KEY_COL3    = GPIO_PINS_8,
	BOARD_GPIOA_KEY_ROW3    = GPIO_PINS_9,
	BOARD_GPIOA_KEY_ROW0    = GPIO_PINS_10,
	BOARD_GPIOA_KEY_SIDE2   = GPIO_PINS_15,

	BOARD_GPIOA_LED_RED     = GPIO_PINS_13,
	BOARD_GPIOA_LED_GREEN   = GPIO_PINS_14,

	// GPIO B
	BOARD_GPIOB_SF_CS       = GPIO_PINS_0,
	BOARD_GPIOB_SF_MISO     = GPIO_PINS_3,
	BOARD_GPIOB_SF_CLK      = GPIO_PINS_4, // Shared with BK1080 SDA!

	BOARD_GPIOB_BK1080_SDA  = GPIO_PINS_4, // Shared with Serial Flash CLK!

	BOARD_GPIOB_USART1_TX   = GPIO_PINS_6,

	BOARD_GPIOB_BK4819_SDA  = GPIO_PINS_5,
	BOARD_GPIOB_BK4819_CS   = GPIO_PINS_8,
	BOARD_GPIOB_BK4819_SCL  = GPIO_PINS_9,

	BOARD_GPIOB_TX_BIAS_LDO = GPIO_PINS_10,
	BOARD_GPIOB_TX_AMP_SEL  = GPIO_PINS_11,

	BOARD_GPIOB_BATTERY     = GPIO_PINS_12,

	BOARD_GPIOB_KEY_ROW1    = GPIO_PINS_1,
	BOARD_GPIOB_KEY_ROW2    = GPIO_PINS_2,
	BOARD_GPIOB_KEY_PTT     = GPIO_PINS_7,
	BOARD_GPIOB_KEY_COL2    = GPIO_PINS_13,
	BOARD_GPIOB_KEY_COL1    = GPIO_PINS_14,
	BOARD_GPIOB_KEY_COL0    = GPIO_PINS_15,

	// GPIO C
	BOARD_GPIOC_BK1080_SEN  = GPIO_PINS_13,
	BOARD_GPIOC_BK1080_SCL  = GPIO_PINS_14,

	BOARD_GPIOC_LCD_CS      = GPIO_PINS_15,

	// GPIO F
	BOARD_GPIOF_LCD_DCX     = GPIO_PINS_1,

	BOARD_GPIOF_KEY_SIDE1   = GPIO_PINS_7,
};

#endif

