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

#include "driver/serial-flash.h"
#include "driver/st7735s.h"
#include "misc.h"
#include "ui/font.h"
#include "ui/gfx.h"

static uint8_t Bitmap[32];

static uint8_t LoadAndDraw(uint8_t X, uint8_t Y, uint32_t Offset)
{
	uint8_t i, j;
	uint16_t Mask;

	if (Offset < 0x0031A000) {
		SFLASH_Read(Bitmap, Offset, 32);
		Mask = 0x8000;
		for (i = 0; i < 16; i++) {
			ST7735S_SetPosition(X + i, Y - 16);
			for (j = 0; j < 32; j += 2) {
				const uint16_t Pixel = (Bitmap[30 - j] << 8) | Bitmap[31 - j];

				if (Pixel & Mask) {
					ST7735S_SendU16(gColorForeground);
				} else {
					ST7735S_SendU16(gColorBackground);
				}
			}
			Mask >>= 1;
		}

		return 16;
	} else {
		SFLASH_Read(Bitmap, Offset, 16);
		Mask = 0x0080;
		for (i = 0; i < 8; i++) {
			ST7735S_SetPosition(X + i, Y - 16);
			for (j = 0; j < 16; j++) {
				if (Bitmap[15 - j] & Mask) {
					ST7735S_SendU16(gColorForeground);
				} else {
					ST7735S_SendU16(gColorBackground);
				}
			}
			Mask >>= 1;
		}

		return 8;
	}
}

void FONT_Draw(uint8_t X, uint8_t Y, const uint32_t *pOffsets, uint32_t Count)
{
	uint8_t i, x;

	x = 0;
	for (i = 0; i < Count; i++) {
		x += LoadAndDraw(X + x, Y, SFLASH_FontOffsets[i]);
		if (X + x >= 160) {
			x = 0;
			if (Y < 32) {
				Y = 96;
			} else {
				Y -= 16;
			}
		}
	}
}

uint8_t FONT_GetOffsets(const char *String, uint8_t Size, bool bFlag)
{
	uint8_t i, j;

	for (i = 0, j = 0; i < Size; i++, j++) {
		uint16_t c = String[i];

		if ((c >> 4) == 0xF) {
			i++;
			c = ((c & 0xF) << 8) | String[i];
			if (!bFlag) {
				SFLASH_Offsets[j] = 0x002F8000 + (c * 34);
			} else {
				SFLASH_FontOffsets[j] = 0x002F8000 + (c * 34);
			}
		} else {
			c -= ' ';
			if (!bFlag) {
				SFLASH_Offsets[j] = 0x0031A000 + (c * 20);
			} else {
				SFLASH_FontOffsets[j] = 0x0031A000 + (c * 20);
			}
		}
	}

	return j;
}

