# Copyright 2023 Dual Tachyon
# https://github.com/DualTachyon
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

	.cpu cortex-m4
	.thumb

	.global StackVector
	.global HandlerReset
	.global Main

	.global BOARD_FLASH_Init
	.global BSS_Init

	.global SystickHandler
	.weak SystickHandler

	.global HandlerTMR1_BRK_OVF_TRG_HALL
	.weak HandlerTMR1_BRK_OVF_TRG_HALL

	.global HandlerUSART1
	.weak HandlerUSART1

	.global HandlerTMR6_GLOBAL
	.weak HandlerTMR6_GLOBAL

	.section .text.isr

StackVector:
	.long	0x20003FF0
Reset:
	.long	HandlerReset + 1
NMI:
	.long	HandlerNMI + 1
HardFault:
	.long	HandlerHardFault + 1
Reserved_10:
	.long	0
Reserved_14:
	.long	0
Reserved_18:
	.long	0
Reserved_1C:
	.long	0
Reserved_20:
	.long	0
Reserved_24:
	.long	0
Reserved_28:
	.long	0
SVCall:
	.long	HandlerSVCall + 1
Reserved_30:
	.long	0
Reserved_34:
	.long	0
PendSV:
	.long	HandlerPendSV + 1
SysTick:
	.long	SystickHandler + 1
WWDT:
	.long	HandlerWWDT + 1
PVM:
	.long	HandlerPVM + 1
ERTC:
	.long	HandlerERTC + 1
FLASH:
	.long	HandlerFLASH + 1
CRM:
	.long	HandlerCRM + 1
EXINT1_0:
	.long	HandlerEXINT1_0 + 1
EXINT3_2:
	.long	HandlerEXINT3_2 + 1
EXINT15_4:
	.long	HandlerEXINT15_4 + 1
Reserved_60:
	.long	0
DMA1_Channel1:
	.long	HandlerDMA1_Channel1 + 1
DMA1_Channel3_2:
	.long	HandlerDMA1_Channel3_2 + 1
DMA1_Channel5_4:
	.long	HandlerDMA1_Channel5_4 + 1
ADC1_CMP:
	.long	HandlerADC1_CMP + 1
TMR1_BRK_OVF_TRG_HALL:
	.long	HandlerTMR1_BRK_OVF_TRG_HALL + 1
TMR1_CH:
	.long	HandlerTMR1_CH + 1
Reserved_7C:
	.long	0
TMR3_GLOBAL:
	.long	HandlerTMR3_GLOBAL + 1
TMR6_GLOBAL:
	.long	HandlerTMR6_GLOBAL + 1
Reserved_88:
	.long	0
TMR14_GLOBAL:
	.long	HandlerTMR14_GLOBAL + 1
TMR15_GLOBAL:
	.long	HandlerTMR15_GLOBAL + 1
TMR16_GLOBAL:
	.long	HandlerTMR16_GLOBAL + 1
TMR17_GLOBAL:
	.long	HandlerTMR17_GLOBAL + 1
I2C1_EVT:
	.long	HandlerI2C1_EVT + 1
I2C2_EVT:
	.long	HandlerI2C2_EVT + 1
SPI1:
	.long	HandlerSPI1 + 1
SPI2:
	.long	HandlerSPI2 + 1
USART1:
	.long	HandlerUSART1 + 1
USART2:
	.long	HandlerUSART2 + 1
Reserved_B4:
	.long	0
Reserved_B8:
	.long	0
Reserved_BC:
	.long	0
I2C1_ERR:
	.long	HandlerI2C1_ERR + 1
Reserved_C4:
	.long	0
I2C2_ERR:
	.long	HandlerI2C2_ERR + 1

	.section .text

HandlerNMI:
	b	.

HandlerHardFault:
	b	.

HandlerSVCall:
	b	.

HandlerPendSV:
	b	.

SystickHandler:
	bx	lr

HandlerWWDT:
	b	.

HandlerPVM:
	b	.

HandlerERTC:
	b	.

HandlerFLASH:
	b	.

HandlerCRM:
	b	.

HandlerEXINT1_0:
	b	.

HandlerEXINT3_2:
	b	.

HandlerEXINT15_4:
	b	.

HandlerDMA1_Channel1:
	b	.

HandlerDMA1_Channel3_2:
	b	.

HandlerDMA1_Channel5_4:
	b	.

HandlerADC1_CMP:
	b	.

HandlerTMR1_BRK_OVF_TRG_HALL:
	b	.

HandlerTMR1_CH:
	b	.

HandlerTMR3_GLOBAL:
	b	.

HandlerTMR6_GLOBAL:
	b	.

HandlerTMR14_GLOBAL:
	b	.

HandlerTMR15_GLOBAL:
	b	.

HandlerTMR16_GLOBAL:
	b	.

HandlerTMR17_GLOBAL:
	b	.

HandlerI2C1_EVT:
	b	.

HandlerI2C2_EVT:
	b	.

HandlerSPI1:
	b	.

HandlerSPI2:
	b	.

HandlerUSART1:
	b	.

HandlerUSART2:
	b	.

HandlerI2C1_ERR:
	b	.

HandlerI2C2_ERR:
	b	.

HandlerReset:
	bl	DATA_Init
	bl	BSS_Init
	bl	Main
	b 	.

