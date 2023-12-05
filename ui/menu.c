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
#include "helper/helper.h"
#include "radio/channels.h"
#include "radio/settings.h"
#include "task/keyaction.h"
#include "task/sidekeys.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/menu.h"

void UI_DrawSettingOptionEx(const char *pString, uint8_t Length, uint8_t Index)
{
	UI_DrawString(24, 48 - (Index * 24), pString, Length);
}

void UI_DrawSettingOption(const char *pString, uint8_t Index)
{
	UI_DrawString(24, 48 - (Index * 24), pString, 16);
}

void UI_DrawSettingRoger(uint8_t Index)
{
	static const char Mode[4][7] = {
		"Off    ",
		"Roger 1",
		"Roger 2",
		"Send ID",
	};

	UI_DrawSettingOptionEx(Mode[Index], 7, 0);
	UI_DrawSettingOptionEx(Mode[(Index + 1) % 4], 7, 1);
}

void UI_DrawDtmfMode(uint8_t Index)
{
	static const char Mode[4][16] = {
		"Off             ",
		"TX Start        ",
		"TX End          ",
		"TX Start And End",
	};

	UI_DrawSettingOption(Mode[Index], 0);
	UI_DrawSettingOption(Mode[(Index + 1) % 4], 1);
}

void UI_DrawDtmfSelect(uint8_t Index)
{
	Int2Ascii(1 + Index, 2);
	UI_DrawSettingOptionEx(gShortString, 2, 0);

	Int2Ascii(1 + ((Index + 1) % 16), 2);
	UI_DrawSettingOptionEx(gShortString, 2, 1);
}

void UI_DrawToggle(void)
{
	static const char Toggle[2][3] = {
		"Off",
		"On ",
	};
	DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
	UI_DrawSettingOptionEx(Toggle[0], 3, 0);
	UI_DrawSettingOptionEx(Toggle[1], 3, 1);
}

void UI_DrawSettingArrow(uint8_t Selection)
{
	uint16_t Bitmap[6];
	uint8_t i;
	uint8_t j;

	Bitmap[0] = 0x1FFC;
	Bitmap[1] = 0x0FF8;
	Bitmap[2] = 0x07F0;
	Bitmap[3] = 0x03E0;
	Bitmap[4] = 0x01C0;
	Bitmap[5] = 0x0080;

	DISPLAY_Fill(8, 16,  8, 23, COLOR_BACKGROUND);
	DISPLAY_Fill(1, 16, 32, 47, COLOR_BACKGROUND);

	for (i = 0; i < 6; i++) {
		uint16_t Pixel = Bitmap[i];

		ST7735S_SetPosition(8 + i, 32 - (Selection * 24));

		for (j = 0; j < 16; j++) {
			if (Pixel & 0x8000U) {
				ST7735S_SendU16(COLOR_FOREGROUND);
			} else {
				ST7735S_SendU16(gColorBackground);
			}
			Pixel <<= 1;
		}
	}
}

void UI_DrawDtmfInterval(uint8_t Interval)
{
	gShortString[3] = 'm';
	gShortString[4] = 's';
	Int2Ascii((Interval + 3) * 10, 3);
	UI_DrawString(24, 48, gShortString, 5);

	Int2Ascii((3 + ((Interval + 1) % 18)) * 10, 3);
	UI_DrawString(24, 24, gShortString, 5);
}

void UI_DrawDtmfDelay(uint8_t Delay)
{
	gShortString[4] = 'm';
	gShortString[5] = 's';
	Int2Ascii(Delay * 100, 4);
	UI_DrawString(24, 48, gShortString, 6);

	Int2Ascii(((Delay + 1) % 21) * 100, 4);
	UI_DrawString(24, 24, gShortString, 6);
}

void UI_DrawActions(uint8_t Index)
{
	static const char Actions[][12] = {
		"None        ",
		"Monitor     ",
		"Freq Detect ",
		"Repeat Mode ",
		"Preset CH   ",
		"Local Alarm ",
		"Remote Alarm",
#ifdef ENABLE_NOAA
		"NOAA        ",
#else
		"[DISABLED]  ",
#endif
		"Send Tone   ",
		"Roger Beep  ",
		"FM Radio    ",
		"Freq Scanner",
		"Flashlight  ",
#ifdef ENABLE_AM_FIX
		"AM Fix      ",
#else
		"[DISABLED]  ",
#endif
		"VOX         ",
		"TX Power    ",
		"SQ Level    ",
		"Dual Standby",
		"Backlight   ",
		"Freq Step   ",
		"Key Beep    ",
		"Toggle SList",
		"DTMF Decode ",
		"Dual Display",
		"TX Frequency",
		"Lock        ",
#ifdef ENABLE_SPECTRUM
		"Spectrum    ",
#else
		"[DISABLED]  ",
#endif
		"Dark Mode   "
	};

	UI_DrawSettingOptionEx(Actions[Index], 12, 0);
	UI_DrawSettingOptionEx(Actions[(Index + 1) % ACTIONS_COUNT], 12, 1);
}

void UI_DrawChannelName(uint16_t Channel)
{
	char String[12];
	ChannelInfo_t Info;

	String[0] = 'C';
	String[1] = 'H';
	String[2] = '-';
	String[3] = '0';
	String[4] = '0';
	String[5] = '1';
	String[6] = ' ';
	String[7] = ' ';
	String[8] = 'N';
	String[9] = 0;
	String[10] = 0;
	String[11] = 0;

	Int2Ascii(Channel + 1, 3);
	String[3] = gShortString[0];
	String[4] = gShortString[1];
	String[5] = gShortString[2];
	SFLASH_Read(&Info, 0x3C2000 + (Channel * sizeof(Info)), sizeof(Info));
	if (Info.Available) {
		String[8] = 'N';
	} else {
		String[8] = 'Y';
	}
	UI_DrawString(24, 48, String, 9);

	Int2Ascii(1 + (Channel + 1U) % 999U, 3);
	String[3] = gShortString[0];
	String[4] = gShortString[1];
	String[5] = gShortString[2];
	SFLASH_Read(&Info, 0x3C2000 + ((Channel + 1U) % 999U) * sizeof(Info), sizeof(Info));
	if (Info.Available) {
		String[8] = 'N';
	} else {
		String[8] = 'Y';
	}
	UI_DrawString(24, 24, String, 9);
}

void UI_DrawMute(uint8_t Index, uint16_t Golay, bool bEnabled)
{
	static const char Mode[3][6] = {
		"Off   ",
		"23bits",
		"24bits",
	};

	UI_DrawSettingOptionEx(Mode[Index], 6, 0);

	if (bEnabled) {
		Int2Ascii(Golay, 8);
		UI_DrawString(24, 24, gShortString, 8);
	}
}

void UI_DrawEncrypt(uint8_t Index)
{
	static const char Mode[4][9] = {
		"Off      ",
		"Encrypt 1",
		"Encrypt 2",
		"Encrypt 3",
	};

	UI_DrawSettingOptionEx(Mode[Index], 9, 0);
	UI_DrawSettingOptionEx(Mode[(Index + 1) % 4], 9, 1);
}

void UI_DrawScrambler(uint8_t Index)
{
	char String[2];

	String[0] = ' ';
	String[1] = '0' + Index;
	UI_DrawSettingOptionEx(String, 2, 0);
	String[1] = '0' + (Index + 1) % 9;
	UI_DrawSettingOptionEx(String, 2, 1);
}

void UI_DrawActivateBy(void)
{
	DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
	DISPLAY_DrawRectangle0(1, 20, 159, 1, gSettings.BorderColor);
	gColorForeground = COLOR_RED;
	UI_DrawString(20, 18, "Activate by [#]", 15);
	gColorForeground = COLOR_FOREGROUND;
}


void UI_DrawCursor(uint8_t X, bool bVisible)
{
	uint16_t Color;

	if (bVisible) {
		Color = COLOR_RED;
	} else {
		Color = gColorBackground;
	}
	DISPLAY_DrawRectangle1(4 + (X * 8), 32, 20, 1, Color);
}

void UI_DrawTxPriority(void)
{
	UI_DrawSettingOptionEx("Edit", 4, 0);
	UI_DrawSettingOptionEx("Busy", 4, 1);
}

void UI_DrawFrequencyStep(uint8_t Index)
{
	static const char Mode[15][5] = {
			"0.25K",
			"1.25K",
			"2.5K ",
			"5K   ",
			"6.25K",
			"10K  ",
			"12.5K",
			"20K  ",
			"25K  ",
			"50K  ",
			"100K ",
			"500K ",
			"1M   ",
			"5M   ",
			"0.01K",
	};

	UI_DrawSettingOptionEx(Mode[Index], 5, 0);
	UI_DrawSettingOptionEx(Mode[(Index + 1) % 15], 5, 1);
}

void UI_DrawTimer(uint8_t Index)
{
	char String[4];
	uint16_t Timer;

	String[0] = ' ';
	String[1] = ' ';
	String[2] = ' ';
	String[3] = ' ';

	switch (Index) {
	case 0:
		UI_DrawSettingOptionEx(" Off", 4, 0);
		UI_DrawSettingOptionEx("   5", 4, 1);
		break;

	case 1:
		UI_DrawSettingOptionEx("   5", 4, 0);
		UI_DrawSettingOptionEx("  10", 4, 1);
		break;

	case 2:
		UI_DrawSettingOptionEx("  10", 4, 0);
		UI_DrawSettingOptionEx("  15", 4, 1);
		break;

	case 42:
		UI_DrawSettingOptionEx(" 600", 4, 0);
		UI_DrawSettingOptionEx(" Off", 4, 1);
		break;

	default:
		Timer = (Index - 2) * 15;
		if (Timer < 100) {
			Int2Ascii(Timer, 2);
			String[2] = gShortString[0];
			String[3] = gShortString[1];
			UI_DrawSettingOptionEx(String, 4, 0);
		} else {
			Int2Ascii(Timer, 3);
			String[1] = gShortString[0];
			String[2] = gShortString[1];
			String[3] = gShortString[2];
			UI_DrawSettingOptionEx(String, 4, 0);
		}
		Timer = (Index - 1) * 15;
		if (Timer < 100) {
			Int2Ascii(Timer, 2);
			String[2] = gShortString[0];
			String[3] = gShortString[1];
			UI_DrawSettingOptionEx(String, 4, 1);
		} else {
			Int2Ascii(Timer, 3);
			String[1] = gShortString[0];
			String[2] = gShortString[1];
			String[3] = gShortString[2];
			UI_DrawSettingOptionEx(String, 4, 1);
		}
		break;
	}
}

void UI_DrawLevel(uint8_t Index)
{
	char Digit;

	gColorForeground = COLOR_FOREGROUND;
	Digit = '0' + Index;
	UI_DrawSettingOptionEx(&Digit, 1, 0);
	Digit = '0' + (Index + 1) % 10;
	UI_DrawSettingOptionEx(&Digit, 1, 1);
}

void UI_DrawScanDirection(void)
{
	UI_DrawSettingOptionEx("Up  ", 4, 0);
	UI_DrawSettingOptionEx("Down", 4, 1);
}

void UI_DrawDeviceName(const char *pName)
{
	UI_DrawString(4, 48, pName, 16);
}

void UI_DrawSettingRepeaterMode(uint8_t Index)
{
	static const char Mode[3][13] = {
		"Off          ",
		"Talkaround   ",
		"Freq Reversal",
	};

	UI_DrawSettingOptionEx(Mode[Index], 13, 0);
	UI_DrawSettingOptionEx(Mode[(Index + 1) % 3], 13, 1);
}

void UI_DrawSettingTxPower(void)
{
	UI_DrawSettingOptionEx("High", 4, 0);
	UI_DrawSettingOptionEx("Low ", 4, 1);
}

void UI_DrawSettingModulation(uint8_t Index)
{
	static const char Mode[4][3] = {
		"FM ",
		"AM ",
		"LSB",
		"USB",
	};

	UI_DrawSettingOptionEx(Mode[Index], 3, 0);
	UI_DrawSettingOptionEx(Mode[(Index + 1) % 4], 3, 1);
}

void UI_DrawSettingBandwidth(void)
{
	UI_DrawSettingOptionEx("Wide  ", 6, 0);
	UI_DrawSettingOptionEx("Narrow", 6, 1);
}

void UI_DrawSettingBusyLock(uint8_t Index)
{
	static const char Mode[3][7] = {
		"Off    ",
		"Carrier",
		"CTC/DCS",
	};

	UI_DrawSettingOptionEx(Mode[Index], 7, 0);
	UI_DrawSettingOptionEx(Mode[(Index + 1) % 3], 7, 1);
}

void UI_DrawSettingScanResume(uint8_t Index)
{
	static const char Mode[3][16] = {
			"Carrier Operated",	// 1
			"Time Operated   ",	// 2
			"No Resume       ",	// 3
	};
	// Values start at 1 instead of 0 for this setting
	UI_DrawSettingOption(Mode[Index], 0);
	UI_DrawSettingOption(Mode[(Index + 1) % 3], 1);
}

void UI_DrawSettingScanlist(uint8_t Index)
{
	static const char Mode[9][1] = {
			"1",
			"2",
			"3",
			"4",
			"5",
			"6",
			"7",
			"8",
			"*",
	};

	UI_DrawSettingOptionEx(Mode[Index], 1, 0);
	UI_DrawSettingOptionEx(Mode[(Index + 1) % 9], 1, 1);
}
