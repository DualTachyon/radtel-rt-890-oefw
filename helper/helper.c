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

#include "helper.h"

char gShortString[10];

void Int2Ascii(uint32_t Number, uint8_t Size)
{
	uint32_t Divider = 1;

	for (; Size != 0; Size--) {
		gShortString[Size - 1] = '0' + ((Number / Divider) - ((Number / Divider) / 10) * 10);
		Divider *= 10;
	}
}

uint16_t TIMER_Calculate(uint16_t Setting)
{
	if (Setting == 0) {
		return 0;
	}
	if (Setting != 1 && Setting != 2 && Setting != 3) {
		return (Setting - 2) * 15;
	}

	return Setting * 5;
}

