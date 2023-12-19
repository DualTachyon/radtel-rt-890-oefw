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

#include <string.h>

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
	uint16_t init_0x9_values[] = {
		0x006F, 0x106B, 0x2067, 0x3062, 0x4050, 0x5047, 0x603A, 0x702C,
		0x8041, 0x9037, 0xA025, 0xB017, 0xC0E4, 0xD0CB, 0xE0B5, 0xF09F,
	};


	for (int i = 0; i < ARRAY_SIZE(init_0x9_values); i++) {
		BK4819_WriteRegister(0x09, init_0x9_values[i]);
	}

	BK4819_WriteRegister(0x24, 0x807E | (gDTMF_Settings.DecodeThreshold << 7));
	BK4819_WriteRegister(0x70, ((0x80 + gDTMF_Settings.EncodeGain) << 8) | (0x80 + gDTMF_Settings.EncodeGain));
}

static void PlayDTMF(uint8_t Code)
{
	struct  data{
		uint16_t reg_71;
		uint16_t reg_72;
	};

	struct data DTMF_tones_register_values [16]={
		{.reg_71 = 0x25F3,  .reg_72 = 0x35E1,},
		{.reg_71 = 0x1C1C, .reg_72 = 0x30C2,},
		{.reg_71 = 0x1C1C, .reg_72 = 0x35E1,},
		{.reg_71 = 0x1C1C, .reg_72 = 0x3B91,},
		{.reg_71 = 0x1F0E, .reg_72 = 0x30C2,},
		{.reg_71 = 0x1F0E, .reg_72 = 0x35E1,},
		{.reg_71 = 0x1F0E, .reg_72 = 0x3B91,},
		{.reg_71 = 0x225C, .reg_72 = 0x30C2,},
		{.reg_71 = 0x225C, .reg_72 = 0x35E1,},
		{.reg_71 = 0x225C, .reg_72 = 0x3B91,},
		{.reg_71 = 0x1C1C, .reg_72 = 0x41DC,},
		{.reg_71 = 0x1F0E, .reg_72 = 0x41DC,},
		{.reg_71 = 0x225C, .reg_72 = 0x41DC,},
		{.reg_71 = 0x25F3, .reg_72 = 0x41DC,},
		{.reg_71 = 0x25F3, .reg_72 = 0x30C2,},
		{.reg_71 = 0x25F3, .reg_72 = 0x3B91,},
	};


	if (Code < 16) {
		BK4819_WriteRegister(0x71, DTMF_tones_register_values[Code].reg_71);
		BK4819_WriteRegister(0x72, DTMF_tones_register_values[Code].reg_72);
	}

	if (!gDTMF_Playing) {
		DELAY_WaitMS(60);
	}
}

void DTMF_FSK_InitReceive(__attribute__((unused)) uint8_t Unused)
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
	BK4819_SetAfGain(0xB32A);

	Init();

	if(!gStartupSoundPlaying) {
		BK4819_EnableTone1(true);
	}

	SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);

	PlayDTMF(Code);

	if (!gDTMF_Playing) {
		DTMF_Disable();
	}
}

void DTMF_PlayContact(const DTMF_String_t *pContact)
{
	if (pContact->Length == 0 || pContact->Length > 14) {
		return;
	}

	for (uint8_t i = 0; i < pContact->Length; i++) {
		const char c = pContact->String[i];

		switch (c) {
		case '0' ... '9':
			DTMF_PlayTone(c - '0');
			break;
		case 'A' ... 'D':
			DTMF_PlayTone(10 + c - 'A');
			break;
		case '*':
			DTMF_PlayTone(14);
			break;
		case '#':
			DTMF_PlayTone(15);
			break;
		}
		DELAY_WaitMS((gDTMF_Settings.Interval + 3) * 10);
	}

	BEEP_Disable();
}

void DTMF_ResetString(void)
{
	memset(gDTMF_Input.String, '-', sizeof(gDTMF_Input.String));
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
	gDTMF_WriteIndex = 0;
	gDataDisplay = false;
	memset(gDTMF_String,' ' , sizeof(gDTMF_String));
}

bool DTMF_strcmp(const DTMF_String_t *pDtmf, const char *pString)
{
	for (uint8_t i = 0; i < pDtmf->Length; i++) {
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
