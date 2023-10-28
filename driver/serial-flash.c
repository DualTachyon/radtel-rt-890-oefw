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

#include "driver/pins.h"
#include "driver/serial-flash.h"
#include "radio/hardware.h"

static bool gSPI_Lock;

static uint8_t Transfer(uint8_t Output)
{
	uint8_t Input = 0U;
	uint8_t i;

	for (i = 0; i < 8; i++) {
		gpio_bits_reset(GPIOB, BOARD_GPIOB_SF_CLK);
		if (Output & 0x80U) {
			gpio_bits_set(GPIOB, BOARD_GPIOB_SF_MISO);
		} else {
			gpio_bits_reset(GPIOB, BOARD_GPIOB_SF_MISO);
		}
		Output <<= 1;
		gpio_bits_set(GPIOB, BOARD_GPIOB_SF_CLK);
		Input <<= 1;
		if (gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_SF_MOSI)) {
			Input |= 1U;
		}
	}

	gpio_bits_reset(GPIOB, BOARD_GPIOB_SF_CLK);

	return Input;
}

static void EnableWrite(void)
{
	gpio_bits_reset(GPIOB, BOARD_GPIOB_SF_CS);

	Transfer(0x06);

	gpio_bits_set(GPIOB, BOARD_GPIOB_SF_CS);
}

static uint8_t ReadStatus1(void)
{
	uint8_t Status;

	gpio_bits_reset(GPIOB, BOARD_GPIOB_SF_CS);

	Transfer(0x05);

	Status = Transfer(0xFF);

	gpio_bits_set(GPIOB, BOARD_GPIOB_SF_CS);

	return Status;
}

static void WaitBusy(void)
{
	while (ReadStatus1() & 1U) {
	}
}

void Write(const uint8_t *pBytes, uint32_t Address, uint16_t Size)
{
	uint16_t i;

	EnableWrite();

	gpio_bits_reset(GPIOB, BOARD_GPIOB_SF_CS);

	Transfer(0x02);

	Transfer((Address >> 16) & 0xFF);
	Transfer((Address >>  8) & 0xFF);
	Transfer((Address >>  0) & 0xFF);

	for (i = 0; i < Size; i++) {
		Transfer(pBytes[i]);
	}

	gpio_bits_set(GPIOB, BOARD_GPIOB_SF_CS);

	WaitBusy();
}

// Public

void SFLASH_Init(void)
{
	gpio_bits_set(GPIOB, BOARD_GPIOB_SF_CS);
	Transfer(0xFF);
}

void SFLASH_Read(void *pBuffer, uint32_t Address, uint16_t Size)
{
	uint8_t *pBytes = (uint8_t *)pBuffer;
	uint16_t i;

	if (!gSPI_Lock) {
		HARDWARE_EnableInterrupts(false);
	}

	gpio_bits_reset(GPIOB, BOARD_GPIOB_SF_CS);

	Transfer(0x03);
	Transfer((Address >> 16) & 0xFF);
	Transfer((Address >>  8) & 0xFF);
	Transfer((Address >>  0) & 0xFF);

	for (i = 0; i < Size; i++) {
		pBytes[i] = Transfer(0xFF);
	}

	gpio_bits_set(GPIOB, BOARD_GPIOB_SF_CS);

	if (!gSPI_Lock) {
		HARDWARE_EnableInterrupts(true);
	}
}

void SFLASH_Erase(uint32_t Page)
{
	Page <<= 12;

	EnableWrite();

	WaitBusy();

	gpio_bits_reset(GPIOB, BOARD_GPIOB_SF_CS);

	Transfer(0x20);

	Transfer((Page >> 16) & 0xFF);
	Transfer((Page >>  8) & 0xFF);
	Transfer((Page >>  0) & 0xFF);

	gpio_bits_set(GPIOB, BOARD_GPIOB_SF_CS);

	WaitBusy();
}

void SFLASH_Write(const void *pBuffer, uint32_t Address, uint16_t Size)
{
	const uint8_t *pBytes = (const uint8_t *)pBuffer;
	uint16_t Remaining;

	Remaining = 0x100 - (Address & 0xFF);
	if (Size <= Remaining) {
		Remaining = Size;
	}
	while (1) {
		Write(pBytes, Address, Remaining);
		if (Size == Remaining) {
			break;
		}
		pBytes += Remaining;
		Address += Remaining;
		Size -= Remaining;
		Remaining = 0x100;
		if (Size <= 0x100) {
			Remaining = Size;
		}
	}
}

void SFLASH_Update(const void *pBuffer, uint32_t Address, uint16_t Size)
{
	const uint8_t *pBytes = (const uint8_t *)pBuffer;
	uint8_t Buffer[4096];
	uint32_t Page;
	uint16_t Offset;
	uint16_t Remaining;
	uint16_t i;

	gSPI_Lock = true;

	HARDWARE_EnableInterrupts(false);

	Page = Address >> 12;
	Offset = Address & 0xFFF;
	Remaining = 0x1000 - Offset;
	if (Size <= 0x1000 - Offset) {
		Remaining = Size;
	}

	while (1) {
		SFLASH_Read(Buffer, Page << 12, 0x1000);
		for (i = 0; i < Remaining && Buffer[Offset + i] == 0xFF; i++) {
		}
		if (i < Remaining) {
			SFLASH_Erase(Page);
			for (i = 0; i < Remaining; i++) {
				Buffer[Offset + i] = pBytes[i];
			}
			SFLASH_Write(Buffer, Page << 12, 0x1000);
		} else {
			SFLASH_Write(pBytes, Address, Remaining);
		}
		if (Size == Remaining) {
			break;
		}
		Page++;
		Offset = 0;
		pBytes += Remaining;
		Address += Remaining;
		Size -= Remaining;
		Remaining = 0x1000;
		if (Size <= 0x1000) {
			Remaining = Size;
		}
	}

	HARDWARE_EnableInterrupts(true);

	gSPI_Lock = false;
}

