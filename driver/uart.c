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

#include <at32f421.h>
#include <stdbool.h>
#include "driver/uart.h"
#ifdef UART_DEBUG
	#include "external/printf/printf.h"
#endif

static void usart_reset_ex(usart_type *uart, uint32_t baudrate)
{
	crm_clocks_freq_type info;
	uint32_t high, low;

	uart->ctrl2_bit.stopbn = USART_STOP_1_BIT;
	uart->ctrl1_bit.ren = TRUE;
	uart->ctrl1_bit.ten = TRUE;
	uart->ctrl1_bit.dbn = USART_DATA_8BITS;
	uart->ctrl1_bit.psel = FALSE;
	uart->ctrl1_bit.pen = FALSE;
	uart->ctrl3_bit.rtsen = FALSE;
	uart->ctrl3_bit.ctsen = FALSE;

	crm_clocks_freq_get(&info);

	if (uart == USART1) {
		info.apb2_freq = info.apb1_freq;
	}

	baudrate = (uint32_t)((((uint64_t)info.apb2_freq * 1000U) / 16U) / baudrate);
	high = baudrate / 1000U;
	low = (baudrate - (1000U * high)) * 16;
	if ((low % 1000U) < 500U) {
		low /= 1000U;
	} else {
		low = (low / 1000U) + 1;
		if (low >= 16) {
			low = 0;
			high++;
		}
	}
	uart->baudr_bit.div = (high << 4) | low;
}

//

void UART_Init(uint32_t BaudRate)
{
	usart_reset_ex(USART1, BaudRate);
	PERIPH_REG((uint32_t)USART1, USART_RDBF_INT) |= PERIPH_REG_BIT(USART_RDBF_INT);
	USART1->ctrl1_bit.uen = TRUE;
}

void UART_SendByte(uint8_t Data)
{
	USART1->dt = Data;
	while (!(USART1->sts & USART_TDBE_FLAG)) {
	}
}

void UART_Send(const void *pBuffer, uint8_t Size)
{
	const uint8_t *pBytes = (const uint8_t *)pBuffer;
	uint8_t i;

	for (i = 0; i < Size; i++) {
		UART_SendByte(pBytes[i]);
	}
}

#ifdef UART_DEBUG
	void UART_printf(const char *str, ...)
	{
		char text[256];
		int  len;
		va_list va;
		va_start(va, str);
		len = vsnprintf(text, sizeof(text), str, va);
		va_end(va);
		UART_Send(text, len);
	}
#endif
