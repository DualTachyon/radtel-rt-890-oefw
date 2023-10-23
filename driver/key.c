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

#include "driver/key.h"
#include "driver/pins.h"
#include "misc.h"

static uint16_t KeyPressed;
static uint8_t RowCounter;

KEY_t KEY_CurrentKey = KEY_NONE;
uint16_t KEY_KeyCounter;
uint16_t KEY_Side1Counter;
uint16_t KEY_Side2Counter;
bool KEY_LongPressed;

KEY_t KEY_GetButton(void)
{
	switch (KeyPressed) {
	case 0x0001: return KEY_MENU;
	case 0x0002: return KEY_1;
	case 0x0004: return KEY_4;
	case 0x0008: return KEY_7;
	case 0x0010: return KEY_UP;
	case 0x0020: return KEY_2;
	case 0x0040: return KEY_5;
	case 0x0080: return KEY_8;
	case 0x0100: return KEY_DOWN;
	case 0x0200: return KEY_3;
	case 0x0400: return KEY_6;
	case 0x0800: return KEY_9;
	case 0x1000: return KEY_EXIT;
	case 0x2000: return KEY_STAR;
	case 0x4000: return KEY_0;
	case 0x8000: return KEY_HASH;
	default:     return KEY_NONE;
	}
}

static void ReadRow(uint8_t Base)
{
	uint16_t Mask;

	Mask = 1U << ((Base * 4) + 0);
	if (!gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_ROW0)) {
		KeyPressed |= Mask;
	} else {
		KeyPressed &= ~Mask;
	}

	Mask = 1U << ((Base * 4) + 1);
	if (!gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_KEY_ROW1)) {
		KeyPressed |= Mask;
	} else {
		KeyPressed &= ~Mask;
	}

	Mask = 1U << ((Base * 4) + 2);
	if (!gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_KEY_ROW2)) {
		KeyPressed |= Mask;
	} else {
		KeyPressed &= ~Mask;
	}

	Mask = 1U << ((Base * 4) + 3);
	if (!gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_ROW3)) {
		KeyPressed |= Mask;
	} else {
		KeyPressed &= ~Mask;
	}
}

void KEY_ReadButtons(void)
{
	ReadRow(RowCounter);

	switch (RowCounter) {
	case 0:
		gpio_bits_reset(GPIOB, BOARD_GPIOB_KEY_COL0);
		gpio_bits_set(GPIOA, BOARD_GPIOA_KEY_COL3);
		RowCounter = 1;
		break;
	case 1:
		gpio_bits_reset(GPIOB, BOARD_GPIOB_KEY_COL1);
		gpio_bits_set(GPIOB, BOARD_GPIOB_KEY_COL0);
		RowCounter = 2;
		break;
	case 2:
		gpio_bits_reset(GPIOB, BOARD_GPIOB_KEY_COL2);
		gpio_bits_set(GPIOB, BOARD_GPIOB_KEY_COL1);
		RowCounter = 3;
		break;
	case 3:
		gpio_bits_reset(GPIOA, BOARD_GPIOA_KEY_COL3);
		gpio_bits_set(GPIOB, BOARD_GPIOB_KEY_COL2);
		RowCounter = 0;
		break;
	default:
		gpio_bits_set(GPIOA, BOARD_GPIOA_KEY_COL3);
		gpio_bits_set(GPIOB, BOARD_GPIOB_KEY_COL0);
		gpio_bits_set(GPIOB, BOARD_GPIOB_KEY_COL1);
		gpio_bits_set(GPIOB, BOARD_GPIOB_KEY_COL2);
		RowCounter = 0;
		break;
	}
}

void KEY_ReadSideKeys(void)
{
	if (KEY_CurrentKey != KEY_NONE && !KEY_LongPressed) {
		KEY_KeyCounter++;
	}
	if (!KEY_SideKeyLongPressed && gScreenMode == SCREEN_MAIN && !gFrequencyDetectMode) {
		if (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1)) {
			KEY_Side1Counter++;
		}
		if (!gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_SIDE2)) {
			KEY_Side2Counter++;
		}
	}
}

