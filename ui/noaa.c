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

#include "helper/helper.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/noaa.h"

static const uint32_t gNoaaFrequencyTable[11] = {
	16255000,
	16240000,
	16247500,
	16242500,
	16245000,
	16250000,
	16252500,
	16165000,
	16177500,
	16175000,
	16200000,
};

void UI_DrawNOAA(uint8_t Channel)
{
	Int2Ascii(gNoaaFrequencyTable[Channel], 8);
	gColorForeground = COLOR_BLUE;
	UI_DrawString( 8, 64, gShortString, 3);
	UI_DrawString(32, 64, ".", 1);
	UI_DrawString(40, 64, gShortString + 3, 5);
	UI_DrawString(80, 64, "MHz", 3);
	UI_DrawString( 8, 32, "CH-", 3);
	Int2Ascii(Channel + 1, 2);
	UI_DrawString(32, 32, gShortString, 2);
	gColorForeground = COLOR_FOREGROUND;
}

