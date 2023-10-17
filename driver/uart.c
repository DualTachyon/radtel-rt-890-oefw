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

typedef struct {
	uint32_t baudrate;
	usart_data_bit_num_type data_bit;
	usart_stop_bit_num_type stop_bit;
	usart_parity_selection_type parity;
	usart_hardware_flow_control_type hw_flow_control;
	bool enable_rx;
	bool enable_tx;
} usart_init_type;

static void usart_para_init(usart_init_type *init)
{
	init->baudrate = 9600;
	init->parity = USART_PARITY_NONE;
	init->stop_bit = USART_STOP_1_BIT;
	init->data_bit = USART_DATA_8BITS;
	init->enable_rx = true;
	init->enable_tx = true;
	init->hw_flow_control = USART_HARDWARE_FLOW_NONE;
}

static void usart_reset_ex(usart_type *uart, const usart_init_type *init)
{
	crm_clocks_freq_type info;
	uint32_t baudrate;
	uint32_t high, low;

	uart->ctrl2_bit.stopbn = init->stop_bit;
	uart->ctrl1_bit.ren = init->enable_rx;
	uart->ctrl1_bit.ten = init->enable_tx;
	uart->ctrl1_bit.dbn = init->data_bit;
	usart_parity_selection_config(uart, init->parity);
	usart_hardware_flow_control_set(uart, init->hw_flow_control);

	crm_clocks_freq_get(&info);

	if (uart == USART1) {
		info.apb2_freq = info.apb1_freq;
	}

	baudrate = (uint32_t)((((uint64_t)info.apb2_freq * 1000U) / 16U) / init->baudrate);
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
	usart_init_type init;

	usart_para_init(&init);
	init.parity = USART_PARITY_NONE;
	init.data_bit = USART_DATA_8BITS;
	init.stop_bit = USART_STOP_1_BIT;
	init.hw_flow_control = USART_HARDWARE_FLOW_NONE;
	init.baudrate = BaudRate;
	init.enable_rx = true;
	init.enable_tx = true;
	usart_reset_ex(USART1, &init);
	usart_interrupt_enable(USART1, USART_RDBF_INT, true);
	usart_enable(USART1, TRUE);
}

void UART_SendByte(uint8_t Data)
{
	usart_data_transmit(USART1, Data);
	while (!usart_flag_get(USART1, USART_TDBE_FLAG)) {
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

