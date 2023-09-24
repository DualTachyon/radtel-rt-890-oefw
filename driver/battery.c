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
#include "driver/battery.h"

typedef struct {
	confirm_state sequence_mode;
	confirm_state repeat_mode;
	uint8_t ordinary_channel_length;
	adc_ordinary_trig_select_type trigger_select;
} adc_init_ex_type;

volatile uint16_t gBatteryAdcValue;
uint8_t gBatteryVoltage;

//

static void adc_para_init_ex(adc_init_ex_type *init)
{
	init->sequence_mode = FALSE;
	init->repeat_mode = FALSE;
	init->trigger_select = ADC12_ORDINARY_TRIG_TMR1CH1;
	init->ordinary_channel_length = 1;
}

static void adc_base_config_ex(adc_type *adc, const adc_init_ex_type *init)
{
	adc->ctrl1_bit.sqen = init->sequence_mode;
	adc->ctrl2_bit.rpen = init->repeat_mode;
	adc->ctrl2_bit.octesel_l = init->trigger_select;
	// TODO: Ignoring octesel_h until Artery replies to issue.
	adc->osq1_bit.oclen = init->ordinary_channel_length - 1;
}

static void adc_ordinary_conversion_trigger_enable(adc_type *adc, confirm_state new_state)
{
	adc->ctrl2_bit.octen = new_state;
}

static void adc_software_trigger_enable(adc_type *adc, confirm_state new_state)
{
	adc->ctrl2_bit.ocswtrg = new_state;
	adc->ctrl2_bit.octen = new_state;
}

//

void BATTERY_Init(void)
{
	dma_init_type init_dma;
	adc_init_ex_type init_adc;

	dma_reset(DMA1_CHANNEL1);
	dma_default_para_init(&init_dma);
	init_dma.peripheral_base_addr = (uint32_t)&ADC1->odt;
	init_dma.memory_base_addr = (uint32_t)&gBatteryAdcValue;
	init_dma.direction = DMA_DIR_PERIPHERAL_TO_MEMORY;
	init_dma.buffer_size = 1;
	init_dma.peripheral_inc_enable = FALSE;
	init_dma.memory_inc_enable = TRUE;
	init_dma.peripheral_data_width = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
	init_dma.memory_data_width = DMA_MEMORY_DATA_WIDTH_HALFWORD;
	init_dma.loop_mode_enable = TRUE;
	init_dma.priority = DMA_PRIORITY_HIGH;
	// TODO: ??? init_dma.field10_0x28 = 0;
	dma_init(DMA1_CHANNEL1, &init_dma);
	dma_channel_enable(DMA1_CHANNEL1, TRUE);

	adc_para_init_ex(&init_adc);
	init_adc.sequence_mode = TRUE;
	init_adc.repeat_mode = FALSE;
	init_adc.trigger_select = ADC12_ORDINARY_TRIG_SOFTWARE;
	init_adc.ordinary_channel_length = 1;
	adc_base_config_ex(ADC1, &init_adc);
	adc_ordinary_channel_set(ADC1, ADC_CHANNEL_11, 1, ADC_SAMPLETIME_28_5);
	adc_ordinary_conversion_trigger_enable(ADC1, TRUE);
	adc_dma_mode_enable(ADC1, TRUE);
	adc_enable(ADC1, TRUE);
}

uint8_t BATTERY_GetVoltage(void)
{
	adc_calibration_init(ADC1);
	while (adc_calibration_init_status_get(ADC1)) {
	}
	adc_calibration_start(ADC1);
	while (adc_calibration_status_get(ADC1)) {
	}
	adc_software_trigger_enable(ADC1, TRUE);
	return (gBatteryAdcValue * 4U) / 66U;
}

