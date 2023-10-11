
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
#include "driver/speaker.h"

uint8_t SPEAKER_State;

void SPEAKER_TurnOn(uint8_t Owner)
{
	if (SPEAKER_State == 0) {
		gpio_bits_set(GPIOA, BOARD_GPIOA_SPEAKER);
	}
	SPEAKER_State |= Owner | 0x80;
}

void SPEAKER_TurnOff(uint8_t Owner)
{
	SPEAKER_State &= ~Owner;
	if (SPEAKER_State == 0x80) {
		SPEAKER_State = 0;
		gpio_bits_reset(GPIOA, BOARD_GPIOA_SPEAKER);
	}
}

