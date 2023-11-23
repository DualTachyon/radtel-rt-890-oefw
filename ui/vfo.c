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
#include "misc.h"
#include "radio/settings.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/vfo.h"

static void DrawBandwidth(bool bIsNarrow, uint8_t Vfo)
{
	const uint8_t Y = 43 - (Vfo * 41);

	gColorForeground = COLOR_FOREGROUND;
	if (bIsNarrow) {
		UI_DrawSmallString(150, Y, "N", 1);
	} else {
		UI_DrawSmallString(150, Y, "W", 1);
	}
}

void UI_DrawVfo(uint8_t Vfo)
{
	// TODO display enabled scan list, maybe using grid or dots
	UI_DrawName(Vfo, gVfoState[Vfo].Name);
	gColorForeground = COLOR_FOREGROUND;
	UI_DrawVfoFrame(Vfo);

	if (Vfo == gCurrentVfo) {
		if (gRadioMode == RADIO_MODE_RX) {
			UI_DrawRX(Vfo);
			UI_DrawExtra(2, gVfoState[Vfo].gModulationType, Vfo);
			gColorForeground = COLOR_BLUE;
		} else if (gRadioMode == RADIO_MODE_TX) {
			UI_DrawRX(Vfo);
			UI_DrawExtra(1, gVfoState[Vfo].gModulationType, Vfo);
			gColorForeground = COLOR_RED;
		} else {
			UI_DrawExtra(0, gVfoState[Vfo].gModulationType, Vfo);
			gColorForeground = COLOR_FOREGROUND;
		}
	} else {
		UI_DrawExtra(0, gVfoState[Vfo].gModulationType, Vfo);
		gColorForeground = COLOR_FOREGROUND;
	}

	if (gSettings.CurrentVfo == Vfo && gFrequencyReverse) {
		gColorForeground = COLOR_RED;
		UI_DrawFrequency(gVfoState[Vfo].TX.Frequency, Vfo, COLOR_RED);
		UI_DrawCss(gVfoState[Vfo].TX.CodeType, gVfoState[Vfo].TX.Code, gVfoState[Vfo].Encrypt, gVfoState[Vfo].bMuteEnabled, Vfo);
	} else {
		UI_DrawFrequency(gVfoInfo[Vfo].Frequency, Vfo, gColorForeground);
		UI_DrawCss(gVfoInfo[Vfo].CodeType, gVfoInfo[Vfo].Code, gVfoState[Vfo].Encrypt, gVfoState[Vfo].bMuteEnabled, Vfo);
	}

	UI_DrawTxPower(gVfoState[Vfo].bIsLowPower, Vfo);
	gColorForeground = COLOR_FOREGROUND;
	if (gSettings.WorkMode) {
		UI_DrawChannel(gSettings.VfoChNo[Vfo], Vfo);
	} else {
		UI_DrawChannel(Vfo ? 1000 : 999, Vfo);
	}

	DrawBandwidth(gVfoState[Vfo].bIsNarrow, Vfo);
}

