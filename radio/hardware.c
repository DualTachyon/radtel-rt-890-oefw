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

#include "app/radio.h"
#include "bsp/gpio.h"
#include "driver/battery.h"
#include "driver/crm.h"
#include "driver/delay.h"
#include "driver/led.h"
#include "driver/pins.h"
#include "driver/pwm.h"
#include "driver/serial-flash.h"
#include "driver/st7735s.h"
#include "driver/uart.h"
#include "radio/hardware.h"
#include "radio/scheduler.h"
#include "ui/gfx.h"

typedef struct {
	IRQn_Type Irq;
	uint8_t PreemptPriority;
	uint8_t SubPriority;
	bool bEnable;
} NVIC_Config_t;

static void AT32_EnableIRQ(NVIC_Config_t *pConfig)
{
	if (pConfig->bEnable) {
		nvic_irq_enable(pConfig->Irq, pConfig->PreemptPriority, pConfig->SubPriority);
	} else {
		nvic_irq_disable(pConfig->Irq);
	}
}

static void InitGPIO(void)
{
	gpio_init_type init;

	// GPIO A
	gpio_default_para_init_ex(&init);
	init.gpio_pins = BOARD_GPIOA_LCD_SCL | BOARD_GPIOA_SPEAKER | BOARD_GPIOA_LCD_SDA | BOARD_GPIOA_LCD_RESX | BOARD_GPIOA_KEY_COL3;
	init.gpio_drive_strength = GPIO_DRIVE_STRENGTH_MODERATE;
	init.gpio_mode = GPIO_MODE_OUTPUT;

	gpio_init(GPIOA, &init);

	gpio_bits_reset(GPIOA, BOARD_GPIOA_LCD_RESX);
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LCD_SDA);
	gpio_bits_reset(GPIOA, BOARD_GPIOA_SPEAKER);
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LCD_SCL);

	// GPIO B
	init.gpio_pins = 0
		// SPI Flash
		| BOARD_GPIOB_SF_CS
		| BOARD_GPIOB_SF_MISO
		| BOARD_GPIOB_SF_CLK
		// BK4819
		| BOARD_GPIOB_BK4819_SDA
		| BOARD_GPIOB_BK4819_CS
		| BOARD_GPIOB_BK4819_SCL
		//
		| BOARD_GPIOB_TX_BIAS_LDO | BOARD_GPIOB_TX_AMP_SEL | BOARD_GPIOB_KEY_COL2 | BOARD_GPIOB_KEY_COL1 | BOARD_GPIOB_KEY_COL0;

	gpio_init(GPIOB, &init);

	gpio_bits_reset(GPIOB, BOARD_GPIOB_TX_AMP_SEL);
	gpio_bits_reset(GPIOB, BOARD_GPIOB_TX_BIAS_LDO);
	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_SCL);
	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_CS);
	gpio_bits_reset(GPIOB, BOARD_GPIOB_BK4819_SDA);
	gpio_bits_reset(GPIOB, BOARD_GPIOB_SF_CLK);
	gpio_bits_reset(GPIOB, BOARD_GPIOB_SF_MISO);

	// GPIO C
	init.gpio_pins = BOARD_GPIOC_BK1080_SEN | BOARD_GPIOC_BK1080_SCL | BOARD_GPIOC_LCD_CS;

	gpio_init(GPIOC, &init);

	gpio_bits_reset(GPIOC, BOARD_GPIOC_LCD_CS);
	gpio_bits_reset(GPIOC, BOARD_GPIOC_BK1080_SCL);
	gpio_bits_set(GPIOC, BOARD_GPIOC_BK1080_SEN);

	// GPIO F
	init.gpio_pins = GPIO_PINS_0 | BOARD_GPIOF_LCD_DCX;

	gpio_init(GPIOF, &init);

	gpio_bits_reset(GPIOF, BOARD_GPIOF_LCD_DCX);
	gpio_bits_reset(GPIOF, GPIO_PINS_0);

	// GPIO A
	init.gpio_pins = BOARD_GPIOA_KEY_ROW3 | BOARD_GPIOA_KEY_ROW0 | BOARD_GPIOA_KEY_SIDE2;
	init.gpio_mode = GPIO_MODE_INPUT;
	init.gpio_pull = GPIO_PULL_UP;

	gpio_init(GPIOA, &init);

	init.gpio_pins = BOARD_GPIOA_SF_MOSI;
	init.gpio_pull = GPIO_PULL_NONE;

	gpio_init(GPIOA, &init);

	// GPIO B
	init.gpio_pins = BOARD_GPIOB_KEY_ROW1 | BOARD_GPIOB_KEY_ROW2;
	init.gpio_mode = GPIO_MODE_INPUT;
	init.gpio_pull = GPIO_PULL_UP;

	gpio_init(GPIOB, &init);

	// GPIO F
	init.gpio_pins = BOARD_GPIOF_KEY_SIDE1;

	gpio_init(GPIOF, &init);

	// GPIO B
	init.gpio_pins = BOARD_GPIOB_USART1_TX;
	init.gpio_mode = GPIO_MODE_MUX;
	init.gpio_out_type = GPIO_OUTPUT_PUSH_PULL;
	init.gpio_pull = GPIO_PULL_NONE;

	gpio_init(GPIOB, &init);

	init.gpio_pins = BOARD_GPIOB_KEY_PTT;
	init.gpio_mode = GPIO_MODE_MUX;
	init.gpio_pull = GPIO_PULL_UP;

	gpio_init(GPIOB, &init);

	// PB6 -> USART1_TX
	gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE6, GPIO_MUX_0);
	// PB7 -> USART1_RX
	gpio_pin_mux_config(GPIOB, GPIO_PINS_SOURCE7, GPIO_MUX_0);

	init.gpio_pins = BOARD_GPIOB_BATTERY;
	init.gpio_mode = GPIO_MODE_ANALOG;

	gpio_init(GPIOB, &init);

	// GPIO A
	init.gpio_pins = GPIO_PINS_2;
	init.gpio_mode = GPIO_MODE_ANALOG;

	gpio_init(GPIOA, &init);

	init.gpio_pins = BOARD_GPIOA_AF_OUT;
	init.gpio_mode = GPIO_MODE_MUX;

	gpio_init(GPIOA, &init);

	// PA6 -> TMR3_CH1
	gpio_pin_mux_config(GPIOA, GPIO_PINS_SOURCE6, GPIO_MUX_1);

	gpio_bits_set(GPIOA, BOARD_GPIOA_KEY_COL3);
	gpio_bits_set(GPIOB, BOARD_GPIOB_KEY_COL0);
	gpio_bits_set(GPIOB, BOARD_GPIOB_KEY_COL1);
	gpio_bits_set(GPIOB, BOARD_GPIOB_KEY_COL2);
}

//

void HARDWARE_Init(void)
{
	CRM_InitPeripherals();
	InitGPIO();
	SCHEDULER_Init();
	UART_Init(115200);
	BATTERY_Init();
	PWM_Init();
	HARDWARE_EnableInterrupts(true);

	while (gBatteryVoltage < 60) {
		gBatteryVoltage = BATTERY_GetVoltage();
		DELAY_WaitMS(200);
	}

	// TODO: LEDs will kill SWD, no thanks.
	LED_Init();

	SFLASH_Init();

	ST7735S_Init();
}

void HARDWARE_Reboot(void)
{
	DELAY_WaitMS(1000);
	DISPLAY_Fill(0, 159, 0, 96, COLOR_BACKGROUND);
	RADIO_Sleep();
	NVIC_SystemReset();
}

void HARDWARE_EnableInterrupts(bool bEnable)
{
	NVIC_Config_t Config;

	Config.bEnable = bEnable;

	Config.Irq = TMR1_BRK_OVF_TRG_HALL_IRQn;
	Config.PreemptPriority = 1;
	Config.SubPriority = 1;
	AT32_EnableIRQ(&Config);

	Config.Irq = USART1_IRQn;
	Config.PreemptPriority = 0;
	Config.SubPriority = 0;
	AT32_EnableIRQ(&Config);

	Config.Irq = TMR6_GLOBAL_IRQn;
	Config.PreemptPriority = 2;
	Config.SubPriority = 2;
	AT32_EnableIRQ(&Config);
}

