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
	adc_init_ex_type init_adc;

	DMA1_CHANNEL1->ctrl_bit.chen = FALSE;
	DMA1_CHANNEL1->ctrl = 0;

	DMA1->clr |= 0xF;

	DMA1_CHANNEL1->ctrl &= 0xBFEF;
	DMA1_CHANNEL1->ctrl |= DMA_DIR_PERIPHERAL_TO_MEMORY;

	DMA1_CHANNEL1->ctrl_bit.chpl =  DMA_PRIORITY_HIGH;
	DMA1_CHANNEL1->ctrl_bit.mwidth = DMA_MEMORY_DATA_WIDTH_HALFWORD;
	DMA1_CHANNEL1->ctrl_bit.pwidth = DMA_PERIPHERAL_DATA_WIDTH_HALFWORD;
	DMA1_CHANNEL1->ctrl_bit.mincm = TRUE;
	DMA1_CHANNEL1->ctrl_bit.pincm = FALSE;
	DMA1_CHANNEL1->ctrl_bit.lm = TRUE;
	DMA1_CHANNEL1->dtcnt = 1;
	DMA1_CHANNEL1->paddr = (uint32_t)&ADC1->odt;
	DMA1_CHANNEL1->maddr = (uint32_t)&gBatteryAdcValue;

	DMA1_CHANNEL1->ctrl_bit.chen = TRUE;

	adc_para_init_ex(&init_adc);
	init_adc.sequence_mode = TRUE;
	init_adc.repeat_mode = FALSE;
	init_adc.trigger_select = ADC12_ORDINARY_TRIG_SOFTWARE;
	init_adc.ordinary_channel_length = 1;
	adc_base_config_ex(ADC1, &init_adc);

	ADC1->spt1 = (ADC1->spt1 & ~(7U << 3)) | (ADC_SAMPLETIME_28_5 << 3);
	ADC1->osq3 = (ADC1->osq3 & ~0x1FU) | ADC_CHANNEL_11;

	adc_ordinary_conversion_trigger_enable(ADC1, TRUE);

	ADC1->ctrl2_bit.ocdmaen = TRUE;
	ADC1->ctrl2_bit.adcen = TRUE;
}

uint8_t BATTERY_GetVoltage(void)
{
	ADC1->ctrl2_bit.adcalinit = TRUE;

	while (ADC1->ctrl2_bit.adcalinit) {
	}

	ADC1->ctrl2_bit.adcal = TRUE;

	while (ADC1->ctrl2_bit.adcal) {
	}

	adc_software_trigger_enable(ADC1, TRUE);

	return (gBatteryAdcValue * 4U) / 66U;
}

