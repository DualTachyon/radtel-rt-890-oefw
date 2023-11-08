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

#include "driver/delay.h"
#include "driver/serial-flash.h"
#include "driver/st7735s.h"
#include "misc.h"
#include "ui/gfx.h"
#include "ui/logo.h"

static void DrawImage(uint32_t Address)
{
	uint16_t i;
	uint8_t X = 0;
	uint8_t Y = 0;

	for (i = 0; i < 0x7800; i += 2) {
		uint16_t Color;

		if ((i & 0x1FFF) == 0) {
			SFLASH_Read(gFlashBuffer, Address + i, sizeof(gFlashBuffer));
		}
		Color = (gFlashBuffer[i & 0x1FFF] << 8) | gFlashBuffer[(i + 1) & 0x1FFF];
		if (Color != 0) {
			ST7735S_SetPixel(X, Y, Color);
		}
		X++;
		if (X == 160) {
			X = 0;
			Y++;
		}
	}
}

void UI_DrawLogo(void)
{
	DrawImage(0x3B5000);
	DELAY_WaitMS(750);
}

