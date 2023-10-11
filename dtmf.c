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
#include "driver/beep.h"
#include "driver/bk4819.h"
#include "driver/delay.h"
#include "driver/key.h"
#include "driver/speaker.h"
#include "dtmf.h"
#include "helper/inputbox.h"
#include "misc.h"

DTMF_String_t gDTMF_Input;
char gDTMF_String[14];
DTMF_Settings_t gDTMF_Settings;
DTMF_String_t gDTMF_Contacts[16];
DTMF_String_t gDTMF_Kill;
DTMF_String_t gDTMF_Stun;
DTMF_String_t gDTMF_Wake;
bool gDTMF_Playing;

static void Init(void)
{
	BK4819_WriteRegister(0x09, 0x006F);
	BK4819_WriteRegister(0x09, 0x106B);
	BK4819_WriteRegister(0x09, 0x2067);
	BK4819_WriteRegister(0x09, 0x3062);
	BK4819_WriteRegister(0x09, 0x4050);
	BK4819_WriteRegister(0x09, 0x5047);
	BK4819_WriteRegister(0x09, 0x603A);
	BK4819_WriteRegister(0x09, 0x702C);
	BK4819_WriteRegister(0x09, 0x8041);
	BK4819_WriteRegister(0x09, 0x9037);
	BK4819_WriteRegister(0x09, 0xA025);
	BK4819_WriteRegister(0x09, 0xB017);
	BK4819_WriteRegister(0x09, 0xC0E4);
	BK4819_WriteRegister(0x09, 0xD0CB);
	BK4819_WriteRegister(0x09, 0xE0B5);
	BK4819_WriteRegister(0x09, 0xF09F);
	BK4819_WriteRegister(0x21, 0x06D8);
	BK4819_WriteRegister(0x24, 0x807E | (gDTMF_Settings.DecodeThreshold << 7));
	BK4819_WriteRegister(0x70, ((0x80 + gDTMF_Settings.EncodeGain) << 8) | (0x80 + gDTMF_Settings.EncodeGain));
}

static void PlayDTMF(uint8_t Code)
{
	switch (Code) {
	case 0:
		BK4819_WriteRegister(0x71, 0x25F3);
		BK4819_WriteRegister(0x72, 0x35E1);
		break;
	case 1:
		BK4819_WriteRegister(0x71, 0x1C1C);
		BK4819_WriteRegister(0x72, 0x30C2);
		break;
	case 2:
		BK4819_WriteRegister(0x71, 0x1C1C);
		BK4819_WriteRegister(0x72, 0x35E1);
		break;
	case 3:
		BK4819_WriteRegister(0x71, 0x1C1C);
		BK4819_WriteRegister(0x72, 0x3B91);
		break;
	case 4:
		BK4819_WriteRegister(0x71, 0x1F0E);
		BK4819_WriteRegister(0x72, 0x30C2);
		break;
	case 5:
		BK4819_WriteRegister(0x71, 0x1F0E);
		BK4819_WriteRegister(0x72, 0x35E1);
		break;
	case 6:
		BK4819_WriteRegister(0x71, 0x1F0E);
		BK4819_WriteRegister(0x72, 0x3B91);
		break;
	case 7:
		BK4819_WriteRegister(0x71, 0x225C);
		BK4819_WriteRegister(0x72, 0x30C2);
		break;
	case 8:
		BK4819_WriteRegister(0x71, 0x225C);
		BK4819_WriteRegister(0x72, 0x35E1);
		break;
	case 9:
		BK4819_WriteRegister(0x71, 0x225C);
		BK4819_WriteRegister(0x72, 0x3B91);
		break;
	case 10:
		BK4819_WriteRegister(0x71, 0x1C1C);
		BK4819_WriteRegister(0x72, 0x41DC);
		break;
	case 11:
		BK4819_WriteRegister(0x71, 0x1F0E);
		BK4819_WriteRegister(0x72, 0x41DC);
		break;
	case 12:
		BK4819_WriteRegister(0x71, 0x225C);
		BK4819_WriteRegister(0x72, 0x41DC);
		break;
	case 13:
		BK4819_WriteRegister(0x71, 0x25F3);
		BK4819_WriteRegister(0x72, 0x41DC);
		break;
	case 14:
		BK4819_WriteRegister(0x71, 0x25F3);
		BK4819_WriteRegister(0x72, 0x30C2);
		break;
	case 15:
		BK4819_WriteRegister(0x71, 0x25F3);
		BK4819_WriteRegister(0x72, 0x3B91);
		break;
	}
	if (!gDTMF_Playing) {
		DELAY_WaitMS(60);
	}
}

void DTMF_FSK_InitReceive(uint8_t Unused)
{
	BK4819_WriteRegister(0x3F, 0x2800); // Enable Interrupts for DTMF/5TONE Found and FSK RX
	BK4819_WriteRegister(0x59, 0x4028); // Clear RX FIFO
	BK4819_WriteRegister(0x59, 0x3028); // Enable FSK Scramble, FSK RX, 2 byte preamble, 4 sync bytes
	if (gMainVfo->Scramble) {
		DTMF_Disable();
	} else {
		Init();
	}
}

char DTMF_GetCharacterFromKey(uint8_t Code)
{
	if (Code <= KEY_9) {
		return '0' + Code;
	}
	if (Code >= KEY_MENU && Code <= KEY_EXIT) {
		return '7' + Code;
	}
	if (Code == KEY_STAR) {
		return '*';
	}
	if (Code == KEY_HASH) {
		return '#';
	}

	return -1;
}

void DTMF_PlayTone(uint8_t Code)
{
	BK4819_SetAF_RX_and_DAC_Gain(0xB32A);
	Init();
	BK4819_EnableTone1(true);
	SPEAKER_TurnOn(2);
	PlayDTMF(Code);
	if (!gDTMF_Playing) {
		DTMF_Disable();
	}
}

void DTMF_PlayContact(const DTMF_String_t *pContact)
{
	uint8_t i;

	if (pContact->Length == 0 || pContact->Length > 14) {
		return;
	}

	for (i = 0; i < pContact->Length; i++) {
		const char c = pContact->String[i];

		switch (c) {
		case '0' ... '9':
			DTMF_PlayTone(c - '0');
			break;
		case '#':
			DTMF_PlayTone(15);
			break;
		case '*':
			DTMF_PlayTone(14);
			break;
		case 'A':
			DTMF_PlayTone(10);
			break;
		case 'B':
			DTMF_PlayTone(11);
			break;
		case 'C':
			DTMF_PlayTone(12);
			break;
		case 'D':
			DTMF_PlayTone(13);
			break;
		}
		DELAY_WaitMS((gDTMF_Settings.Interval + 3) * 10);
	}

	BEEP_Disable();
}

void DTMF_ResetString(void)
{
	uint8_t i;

	for (i = 0; i < 14; i++) {
		gDTMF_Input.String[i] = '-';
	}
	gDTMF_Input.Padding = 0xFF;
	gDTMF_Input.Length = 0;
}

void DTMF_Pad(uint8_t i, char c)
{
	for (; i < 8; i++) {
		gInputBox[i] = c;
	}
}

void DTMF_ClearString(void)
{
	uint8_t i;

	gDTMF_WriteIndex = 0;
	gDataDisplay = false;
	for (i = 0; i < 14; i++) {
		gDTMF_String[i] = ' ';
	}
}

bool DTMF_strcmp(const DTMF_String_t *pDtmf, const char *pString)
{
	uint8_t i;

	for (i = 0; i < pDtmf->Length; i++) {
		if (pDtmf->String[i] != pString[i]) {
			return false;
		}
	}

	return true;
}

void DTMF_Disable(void)
{
	BK4819_WriteRegister(0x24, 0x0000);
	BK4819_WriteRegister(0x70, 0x0000);
}

