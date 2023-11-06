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
#include "driver/bk4819.h"
#include "driver/delay.h"
#include "helper/dtmf.h"
#include "helper/helper.h"
#include "misc.h"
#include "radio/data.h"
#include "radio/hardware.h"
#include "radio/settings.h"
#include "ui/gfx.h"
#include "ui/helper.h"

static char FSK[16];

static uint32_t GetU32(uint8_t Index)
{
	uint32_t Value;
	uint8_t i;

	Value = 0;
	for (i = 1; i < 5; i++) {
		Value = (Value << 8) + FSK[Index + i];
	}

	return Value;
}

static void DrawFSK(void)
{
	uint32_t Value0;
	uint32_t Value5;
	uint8_t i;

	Value0 = GetU32(0);
	Value5 = GetU32(5);
	UI_DrawDialog();
	gColorForeground = COLOR_RED;

	Int2Ascii(Value0, 8);
	for (i = 8; i > 3; i--) {
		gShortString[i] = gShortString[i - 1];
	}
	gShortString[3] = '.';
	gShortString[9] = FSK[5];
	UI_DrawString(10, 54, gShortString, 10);

	Int2Ascii(Value5, 7);
	for (i = 7; i > 2; i--) {
		gShortString[i] = gShortString[i - 1];
	}
	gShortString[2] = '.';
	gShortString[8] = FSK[10];
	UI_DrawString(10, 38, gShortString, 9);
}

static void DrawReceivedFSKID(void)
{
	if (gScreenMode == SCREEN_MAIN && !gReceptionMode) {
		UI_DrawDialog();
		gColorForeground = COLOR_RED;
		if (gCurrentVfo == 1) {
			UI_DrawString(10, 54, "Area B ID:", 10);
		} else {
			UI_DrawString(10, 54, "Area A ID:", 10);
		}
		UI_DrawString(10, 38, FSK, 16);
	}
}

void DATA_ReceiverInit(void)
{
	BK4819_EnableFFSK1200(true);
	DTMF_ClearString();
	DTMF_FSK_InitReceive(0);
}

bool DATA_ReceiverCheck(void)
{
	uint16_t Result;
	uint8_t i;

	Result = BK4819_ReadRegister(0x0B);
	BK4819_WriteRegister(2, 0);

	if (gSettings.DtmfState == DTMF_STATE_KILLED && (Result & 0x1000U) == 0) {
		return false;
	}

	if (Result & 0x0010U && gSettings.DtmfState != DTMF_STATE_KILLED) {
		for (i = 0; i < 8; i++) {
			const uint16_t Data = BK4819_ReadRegister(0x5F);
			FSK[(i * 2) + 0] = (Data >> 8) & 0xFFU;
			FSK[(i * 2) + 1] = (Data >> 0) & 0xFFU;
		}
		// BK4819_SetAF(BK4819_AF_MUTE);
		if (FSK[0] == 0xEE) {
			DrawFSK();
		} else {
			DrawReceivedFSKID();
		}
		gFskDataReceived = true;
		BK4819_WriteRegister(0x3F, 0x0000);
		BK4819_WriteRegister(0x59, 0x0028);
	} else if (Result & 0x1000U) {
		if (gDTMF_WriteIndex > 13) {
			gDTMF_WriteIndex = 13;
			// The original overflows!
			for (i = 0; i < 13; i++) {
				gDTMF_String[i] = gDTMF_String[i + 1];
			}
			gDTMF_String[i] = 0;
		}
		gDTMF_String[gDTMF_WriteIndex++] = DTMF_GetCharacterFromKey((Result >> 8) & 0xFU);

		if (gDTMF_Wake.Length == 0 || !DTMF_strcmp(&gDTMF_Wake, gDTMF_String)) {
			if (gSettings.DtmfState == DTMF_STATE_KILLED) {
				return false;
			}
			if (gDTMF_Kill.Length != 0 && DTMF_strcmp(&gDTMF_Kill, gDTMF_String)) {
				gSettings.DtmfState = DTMF_STATE_KILLED;
				gSettings.CurrentVfo = gCurrentVfo;
				SETTINGS_SaveGlobals();
				HARDWARE_Reboot();
			}
			if (gDTMF_Stun.Length != 0 && DTMF_strcmp(&gDTMF_Stun, gDTMF_String)) {
				gSettings.DtmfState = DTMF_STATE_STUNNED;
				SETTINGS_SaveGlobals();
				UI_DrawStatusIcon(4, ICON_LOCK, true, COLOR_RED);
			}
		} else if (gSettings.DtmfState == DTMF_STATE_STUNNED) {
			gSettings.DtmfState = DTMF_STATE_NORMAL;
			SETTINGS_SaveGlobals();
			UI_DrawStatusIcon(4, ICON_LOCK, gSettings.Lock, COLOR_FOREGROUND);
		} else if (gSettings.DtmfState == DTMF_STATE_KILLED) {
			gSettings.DtmfState = DTMF_STATE_NORMAL;
			SETTINGS_SaveGlobals();
			HARDWARE_Reboot();
		}
		if (gScreenMode == SCREEN_MAIN && !gFrequencyDetectMode) {
			UI_DrawDTMFString();
			if (gDTMF_Settings.Display) {
				gDataDisplay = true;
			}
		}

		return true;
	}

	return false;
}

void DATA_SendDeviceName(void)
{
	uint8_t i;

	BK4819_WriteRegister(0x3F, 0x8000);
	BK4819_WriteRegister(0x59, 0x8028);
	BK4819_WriteRegister(0x59, 0x0028);

	for (i = 0; i < 16; i += 2) {
		BK4819_WriteRegister(0x5F, (gDeviceName[i] << 8) | gDeviceName[i + 1]);
	}

	BK4819_WriteRegister(0x59, 0x2828);

	for (i = 0; i < 200; i++) {
		const uint16_t Value = BK4819_ReadRegister(0x0C);

		DELAY_WaitMS(5);

		if (Value & 1U) {
			break;
		}
	}

	BK4819_WriteRegister(0x02, 0x0000);
}

bool DATA_WasDataReceived(void)
{
	if (gFskDataReceived) {
		gRedrawScreen = false;
		gFskDataReceived = false;
		return true;
	}

	return false;
}

