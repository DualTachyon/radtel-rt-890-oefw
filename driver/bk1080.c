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
#include "driver/bk1080.h"
#include "driver/delay.h"
#include "driver/pins.h"
#include "driver/speaker.h"
#include "misc.h"

static const uint16_t InitTable[] = {
	0x0008, 0x1080, 0x0201, 0x0000,
	0x40C0, 0x0A1F, 0x302E, 0x02FF,
	0x5B11, 0x0000, 0x411E, 0x0000,
	0xCE00, 0x0000, 0x0000, 0x1000,
	0x3197, 0x8000, 0x13FF, 0x9852,
	0x0000, 0x0000, 0x0008, 0x0000,
	0x51E1, 0x28DC, 0x2645, 0x00E4,
	0x1CD8, 0x3A50, 0xEAF0, 0x3000,
	0x0000, 0x0000,
};

static void SDA_SetOutput(void)
{
	gpio_init_type init;

	gpio_default_para_init_ex(&init);
	init.gpio_pins = BOARD_GPIOB_BK1080_SDA;
	init.gpio_mode = GPIO_MODE_OUTPUT;
	init.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	init.gpio_pull = GPIO_PULL_DOWN;
	gpio_init(GPIOB, &init);
}

static void SDA_SetInput(void)
{
	gpio_init_type init;

	gpio_default_para_init_ex(&init);
	init.gpio_pins = BOARD_GPIOB_BK1080_SDA;
	init.gpio_mode = GPIO_MODE_INPUT;
	init.gpio_pull = GPIO_PULL_DOWN;
	gpio_init(GPIOB, &init);
}

static void SendCommand(uint8_t Command, uint8_t Mode)
{
	uint16_t Value;
	uint8_t i;

	SDA_SetOutput();

	gpio_bits_set(GPIOB, BOARD_GPIOB_BK1080_SDA);
	gpio_bits_set(GPIOC, BOARD_GPIOC_BK1080_SCL);
	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK1080_SDA);
	gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SCL);

	Value = 0x80;
	for (i = 0; i < 8; i++) {
		if (Value & 0x80U) {
			gpio_bits_set(GPIOB, BOARD_GPIOB_BK1080_SDA);
		} else {
			gpio_bits_reset(GPIOB, BOARD_GPIOB_BK1080_SDA);
		}
		gpio_bits_set(GPIOC, BOARD_GPIOC_BK1080_SCL);
		gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SCL);
		Value = 0;
	}

	gpio_bits_set(GPIOC, BOARD_GPIOC_BK1080_SCL);
	gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SCL);
	Value = Command << 1;
	if (Mode != 0) {
		Value |= Mode;
	}
	for (i = 0; i < 8; i++) {
		if (Value & 0x80U) {
			gpio_bits_set(GPIOB, BOARD_GPIOB_BK1080_SDA);
		} else {
			gpio_bits_reset(GPIOB, BOARD_GPIOB_BK1080_SDA);
		}
		gpio_bits_set(GPIOC, BOARD_GPIOC_BK1080_SCL);
		gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SCL);
		Value <<= 1;
	}
}

static uint8_t RecvByte(void)
{
	uint8_t Value;
	uint8_t i;

	SDA_SetOutput();

	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK1080_SDA);
	DELAY_WaitUS(1);
	gpio_bits_set(GPIOC, BOARD_GPIOC_BK1080_SCL);
	DELAY_WaitUS(1);
	gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SCL);
	gpio_bits_set(GPIOB, BOARD_GPIOB_BK1080_SDA);
	DELAY_WaitUS(1);

	SDA_SetInput();

	Value = 0;
	for (i = 0; i < 8; i++) {
		Value <<= 1;
		gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SCL);
		DELAY_WaitUS(1);
		if (gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_BK1080_SDA)) {
			Value |= 1;
		}
		gpio_bits_set(GPIOC, BOARD_GPIOC_BK1080_SCL);
		DELAY_WaitUS(1);
	}

	SDA_SetOutput();

	gpio_bits_set(GPIOB, BOARD_GPIOB_BK1080_SDA);
	DELAY_WaitUS(1);
	gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SCL);
	DELAY_WaitUS(1);

	return Value;
}

static void SendByte(uint8_t Value)
{
	uint8_t i;

	SDA_SetOutput();

	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK1080_SDA);
	DELAY_WaitUS(1);
	gpio_bits_set(GPIOC, BOARD_GPIOC_BK1080_SCL);
	DELAY_WaitUS(1);
	gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SCL);

	for (i = 0; i < 8; i++) {
		if (Value & 0x80U) {
			gpio_bits_set(GPIOB, BOARD_GPIOB_BK1080_SDA);
		} else {
			gpio_bits_reset(GPIOB, BOARD_GPIOB_BK1080_SDA);
		}
		gpio_bits_set(GPIOC, BOARD_GPIOC_BK1080_SCL);
		Value <<= 1;
		gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SCL);
	}
}

static void StopI2C(void)
{
	gpio_bits_set(GPIOC, BOARD_GPIOC_BK1080_SCL);
	DELAY_WaitUS(1);
	gpio_bits_set(GPIOB, BOARD_GPIOB_BK1080_SDA);
}

static void RenameLater(void)
{
	SDA_SetOutput();
	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK1080_SDA);
	DELAY_WaitUS(1);
	gpio_bits_set(GPIOC, BOARD_GPIOC_BK1080_SCL);
	DELAY_WaitUS(1);
	gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SCL);
	DELAY_WaitUS(1);

	StopI2C();
}

//

void BK1080_Init(void)
{
	uint8_t Values[68];
	uint8_t i;

	gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SEN);
	DELAY_WaitMS(1000);
	// Could just use the table directly
	for (i = 0; i < 34; i++) {
		Values[(i * 2) + 0] = (InitTable[i] >> 8) & 0xFF;
		Values[(i * 2) + 1] = (InitTable[i] >> 0) & 0xFF;
	}
	BK1080_WriteRegisters(0x00, Values, sizeof(Values));
	DELAY_WaitMS(250);
	Values[(0x19 * 2) + 1] &= 0x7F;
	BK1080_WriteRegisters(0x19, Values + (0x19 * 2), 2);
	Values[(0x19 * 2) + 1] |= 0x80;
	BK1080_WriteRegisters(0x19, Values + (0x19 * 2), 2);
	DELAY_WaitMS(60);
}

void BK1080_WriteRegisters(uint8_t Index, const uint8_t *pValues, uint8_t Size)
{
	uint8_t i;

	SendCommand(Index, 0);

	for (i = 0; i < Size; i++) {
		SendByte(pValues[i]);
	}

	RenameLater();
}

void BK1080_ReadRegisters(uint8_t Index, uint8_t *pValues, uint8_t Size)
{
	uint8_t i;

	SendCommand(Index, 1);

	for (i = 0; i < Size; i++) {
		pValues[i] = RecvByte();
	}

	StopI2C();
}

