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

#ifndef BOARD_DISPLAY_H
#define BOARD_DISPLAY_H

#include <stdint.h>

// RGB565
#define COLOR_RGB(r, g, b) ((r & 0x1F) | (((g) & 0x3F) << 5) | (((b) & 0x1F) << 11))

/*
enum {
	#ifndef LIGHT_THEME
	COLOR_BACKGROUND = COLOR_RGB( 0,  0,  0),
	#else
	COLOR_BACKGROUND = COLOR_RGB(31, 63, 31),
	#endif
	COLOR_RED   = COLOR_RGB(31,  0,  0),
	COLOR_GREEN = COLOR_RGB( 0, 63,  0),
	COLOR_BLUE  = COLOR_RGB( 0,  0, 31),
	COLOR_GREY  = COLOR_RGB(16, 32, 16),
	#ifndef LIGHT_THEME
	COLOR_FOREGROUND = COLOR_RGB(31, 63, 31),
	#else
	COLOR_FOREGROUND = COLOR_RGB( 0,  0,  0),
	#endif
};
*/

extern uint16_t COLOR_BACKGROUND;
extern uint16_t COLOR_FOREGROUND;
extern uint16_t COLOR_RED;
extern uint16_t COLOR_GREEN;
extern uint16_t COLOR_BLUE;
extern uint16_t COLOR_GREY;

extern uint16_t gColorForeground;
extern uint16_t gColorBackground;

void DISPLAY_FillColor(uint16_t Color);
void DISPLAY_Fill(uint8_t X0, uint8_t X1, uint8_t Y0, uint8_t Y1, uint16_t Color);
void DISPLAY_DrawRectangle0(uint8_t X, uint8_t Y, uint8_t W, uint8_t H, uint16_t Color);
void DISPLAY_DrawRectangle1(uint8_t X, uint8_t Y, uint8_t H, uint8_t W, uint16_t Color);
void UI_SetColors(uint8_t DarkMode);

#endif

