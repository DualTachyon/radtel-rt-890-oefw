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

#include "app/uart.h"
#include "bsp/gpio.h"
#include "driver/pins.h"
#include "driver/serial-flash.h"
#include "driver/uart.h"
#include "radio/hardware.h"
#include "radio/settings.h"

static uint8_t Buffer[256];
static uint8_t BufferLength;
static uint8_t Region;
static bool bFlashing;
static uint8_t g_Unused;

uint16_t UART_Timer;
bool UART_IsRunning;

static uint8_t CalcSum(const uint8_t *pBytes, uint8_t Size)
{
	uint8_t Sum = 0;
	uint8_t i;

	for (i = 0; i < Size; i++) {
		Sum += *pBytes++;
	}

	return Sum;
}

static void FlashCmd(uint8_t Command, uint8_t Hi, uint8_t Lo)
{
	uint16_t Count = 0;
	uint16_t Page = 0;
	uint16_t Block;
	uint16_t i;

	Block = (Hi << 8) | Lo;
	if (Command == 0x52) {
		Buffer[0] = 0x52;
		Buffer[1] = Hi;
		Buffer[2] = Lo;
		SFLASH_Read(Buffer + 3, Block * 128, 128);
		Buffer[131] = CalcSum(Buffer, 0x83);
		UART_Send(Buffer, 132);
		return;
	}

	TMR1->ctrl1_bit.tmren = FALSE;
	// Why? Is this some left over from another radio?
	USART2->ctrl1_bit.uen = FALSE;

	switch (Command) {
	case 0x40:
		Page = 0x000;
		Count = 0x2D0;
		break;
	case 0x41:
		Page = 0x2D0;
		Count = 0x028;
		break;
	case 0x42:
		Page = 0x2F8;
		Count = 0x022;
		break;
	case 0x43:
		Page = 0x31A;
		Count = 0x002;
		break;
	case 0x47:
		Page = 0x3B5;
		Count = 0x00A;
		break;
	case 0x48:
		Page = 0x3BF;
		Count = 0x001;
		Region = 1;
		break;
	case 0x49:
		Page = 0x3C1;
		Count = 0x00A;
		Region = 2;
		break;
	case 0x4B:
		Page = 0x3D8;
		Count = 0x00A;
		break;
	case 0x4C:
		Page = 0x31C;
		Count = 0x99;
		break;
	}

	bFlashing = true;

	if (Block == 0) {
		for (i = 0; i < Count; i++) {
			SFLASH_Erase(Page + i);
		}
	}

	SFLASH_Write(Buffer + 3, (Page * 4096U) + (Block * 128U), 128U);
	UART_SendByte(0x06);
}

void HandlerUSART1(void)
{
	if (USART1->ctrl1_bit.rdbfien && USART1->sts & USART_RDBF_FLAG) {
		uint8_t Cmd;

		Buffer[BufferLength++] = USART1->dt;

		BufferLength %= 256;
		Cmd = Buffer[0];
		if (BufferLength == 1 && Cmd != 0x35 && !(Cmd >= 0x40 && Cmd <= 0x4C) && Cmd != 0x52) {
			UART_IsRunning = false;
			UART_Timer = 0;
			UART_SendByte(0xFF);
			BufferLength = 0;
		} else {
			if ((Cmd == 0x35 && BufferLength == 5) || (Cmd == 0x52 && BufferLength == 4) || (Cmd >= 0x40 && Cmd <= 0x4C && BufferLength == 132)) {
				if (CalcSum(Buffer, BufferLength - 1) == Buffer[BufferLength - 1]) {
					gpio_bits_flip(GPIOA, BOARD_GPIOA_LED_RED);
					UART_IsRunning = true;
					UART_Timer = 1000;
					if (Cmd == 0x35) {
						if (Buffer[3] == 16) {
							g_Unused = 0;
							UART_SendByte(0x06);
						} else if (Buffer[3] == 0xEE) {
							gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);
							if (bFlashing) {
								if (Region == 1) {
									SETTINGS_BackupCalibration();
								} else if (Region == 2) {
									SETTINGS_BackupSettings();
								}
								gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
								Region = 0;
								HARDWARE_Reboot();
							}
							UART_IsRunning = false;
							UART_Timer = 0;
						}
					} else {
						FlashCmd(Cmd, Buffer[1], Buffer[2]);
					}
				} else {
					gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);
					UART_SendByte(0xFF);
				}
				BufferLength = 0;
			} else if (Cmd == 0x32 && BufferLength == 5) {
				if (CalcSum(Buffer, 4) + 1 == Buffer[4]) {
					UART_IsRunning = true;
					UART_Timer = 1000;
					if (Buffer[3] != 0x16 && Buffer[3] == 0x10) {
						UART_SendByte(6);
					}
				} else {
					gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);
					UART_SendByte(0xFF);
					UART_IsRunning = false;
					UART_Timer = 0;
				}
				BufferLength = 0;
			}
		}
	}
}

