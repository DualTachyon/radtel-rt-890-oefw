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

#include "app/css.h"
#include "app/menu.h"
#include "app/radio.h"
#include "app/t9.h"
#include "driver/audio.h"
#include "driver/beep.h"
#include "driver/key.h"
#include "driver/speaker.h"
#include "helper/dtmf.h"
#include "helper/helper.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/hardware.h"
#include "radio/settings.h"
#include "task/cursor.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "ui/menu.h"
#include "ui/version.h"

static const char Menu[48][16] = {
	"Startup Logo  01",
	"Voltage       02",
	"Ringtone      03",
	"Prompt Text   04",
	"Voice Prompt  05",
	"Key Beep      06",
	"Roger Beep    07",
	"Dual Display  08",
	"TX Priority   09",
	"Save Mode     10",
	"Freq Step     11",
	"SQ Level      12",
	"LED Timer     13",
	"Lock Timer    14",
	"TOT           15",
	"VOX Level     16",
	"VOX Delay     17",
	"NOAA Monitor  18",
	"FM Standby    19",
	"Tail Tone     20",
	"Scan DIR      21",
	"Personal ID   22",
	"Repeater Mode 23",
	"CTCSS/DCS     24",
	"RX CTCSS/DCS  25",
	"TX CTCSS/DCS  26",
	"TX Power      27",
	"Modulation    28",
	"Band Width    29",
	"Busy Lock     30",
	"Scrambler     31",
	"DCS Encrypt   32",
	"Mute Code     33",
	"CH Name       34",
	"Save CH       35",
	"Delete CH     36",
	"K1 Long       37",
	"K1 Short      38",
	"K2 Long       39",
	"K2 Short      40",
	"DTMF Delay    41",
	"DTMF Interval 42",
	"DTMF Mode     43",
	"DTMF Select   44",
	"DTMF Display  45",
	"Initialize    46",
	"Version       47",
	"Instruction   48",
};

static const uint8_t BitmapQR[] = {
	0x00, 0x7F, 0x7F, 0x60, 0x6F, 0x6F, 0x6F, 0x6F, 0x6F, 0x60, 0x60, 0x7F, 0x00, 0x00, 0x73, 0x0F,
	0x0F, 0x11, 0x11, 0x00, 0x7F, 0x7F, 0x60, 0x1E, 0x1E, 0x7E, 0x7F, 0x01, 0x6C, 0x6C, 0x11, 0x1F,
	0x1F, 0x61, 0x63, 0x63, 0x61, 0x6D, 0x0D, 0x6D, 0x6D, 0x71, 0x1C, 0x1C, 0x0C, 0x1E, 0x1E, 0x00,
	0x00, 0xF3, 0xF3, 0x33, 0xB3, 0xB3, 0xB1, 0xB1, 0xB3, 0x30, 0x30, 0xF2, 0x03, 0x03, 0x8E, 0x83,
	0x83, 0xB3, 0xB3, 0x00, 0xB3, 0xB3, 0x31, 0x0F, 0x0F, 0x71, 0xF3, 0xB2, 0x62, 0x62, 0xE3, 0xB0,
	0xB0, 0xFE, 0x83, 0x83, 0xB3, 0xB3, 0x82, 0xFE, 0xFE, 0xBF, 0x03, 0x03, 0x02, 0x60, 0x60, 0x00,
	0x00, 0xF0, 0xF0, 0xF1, 0xFF, 0xBF, 0x8C, 0x8C, 0xB6, 0x06, 0x06, 0x4D, 0xFE, 0xFE, 0x30, 0xFC,
	0xFC, 0xC0, 0xC0, 0x77, 0xB1, 0xB1, 0xCC, 0xBC, 0xBC, 0xFF, 0xFF, 0x3D, 0x31, 0x31, 0x8D, 0x77,
	0x77, 0x7F, 0x80, 0x80, 0xC0, 0xCE, 0x0E, 0x77, 0x77, 0xCC, 0xB7, 0xB7, 0x30, 0x47, 0x47, 0x00,
	0x00, 0x71, 0x71, 0xFE, 0xFE, 0xF8, 0x79, 0x79, 0x00, 0x46, 0x46, 0xB6, 0x48, 0x48, 0x06, 0x39,
	0x39, 0x0E, 0x0E, 0xBE, 0xB9, 0xB9, 0x41, 0x49, 0x49, 0xFF, 0xFF, 0x81, 0xF9, 0xF9, 0xC1, 0x89,
	0x89, 0xB8, 0x38, 0x38, 0x76, 0x77, 0x71, 0xCF, 0xCF, 0x48, 0xC7, 0xC7, 0x41, 0xF8, 0xF8, 0x00,
	0x00, 0xCF, 0xCF, 0x48, 0x49, 0x09, 0xC9, 0xC9, 0x49, 0x08, 0x08, 0x4F, 0x00, 0x00, 0x38, 0xC1,
	0xC1, 0x7E, 0x7E, 0x71, 0x8E, 0x8E, 0xB1, 0xF8, 0xF8, 0xF1, 0xFF, 0xBF, 0xC0, 0xC0, 0xF9, 0xB1,
	0xB1, 0x0F, 0x00, 0x00, 0x4F, 0xCF, 0x88, 0x89, 0x89, 0x09, 0x89, 0x89, 0x88, 0x4F, 0x4F, 0x00,
	0x00, 0xFE, 0xFE, 0x06, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0x06, 0x06, 0xFE, 0x00, 0x00, 0x06, 0x18,
	0x18, 0x38, 0x38, 0x06, 0xF6, 0xF6, 0xF0, 0x30, 0x30, 0x06, 0xDE, 0xD8, 0xDE, 0xDE, 0x1E, 0xD8,
	0xD8, 0xF8, 0x00, 0x00, 0xFE, 0xFE, 0x06, 0xF6, 0xF6, 0xF6, 0xF6, 0xF6, 0x06, 0xFE, 0xFE, 0x00,
};

static const ChannelInfo_t EmptyChannel = {
	.RX = { .Frequency = 0xFFFFFFFF, .Code = 0xFFF, .CodeType = 15, },
	.TX = { .Frequency = 0xFFFFFFFF, .Code = 0xFFF, .CodeType = 15, },
	.Golay = 0xFFFFFF,

	.Unknown0 = 15,
	.bIs24Bit = 1,
	.bMuteEnabled = 1,
	.Encrypt = 3,

	.Available = 1,
	.gModulationType = 2,
	.BCL = BUSY_LOCK_INVALID,
	.ScanAdd = 1,
	.bIsLowPower = 1,
	.bIsNarrow = 1,

	._0x11 = 0xFF,
	.Scramble = 0xFF,
	._0x13 = 0xFF,
	._0x14 = 0xFF,
	._0x15 = 0xFF,
	.Name = "          ",
};

static uint16_t gSettingMaxValues;
static uint16_t gSettingCurrentValue;
static uint8_t gSettingCodeType;
static uint16_t gSettingCode;
static uint8_t EditSize;

uint16_t gSettingGolay;

uint8_t gMenuIndex;
uint8_t gSettingIndex;

//

static void DrawMenu(uint8_t Index)
{
	UI_DrawString(24, 48, Menu[Index], 16);
	UI_DrawString(24, 24, Menu[(Index + 1) % gSettingsCount], 16);
}

static void DrawQR(void)
{
	gColorForeground = COLOR_BLACK;
	gColorBackground = COLOR_WHITE;
	UI_DrawBitmap(24, 4, 6, 48, BitmapQR);
	gColorForeground = COLOR_WHITE;
	gColorBackground = COLOR_BLACK;
}

static void EnableTextEditor(void)
{
	gCursorCountdown = 500;
	gCursorEnabled = true;
	gCursorBlink = true;
	gCursorPosition = 0;
	if (gMenuIndex == MENU_PERSONAL_ID) {
		gCursorPosition = 16;
	}
	if (gMenuIndex == MENU_CH_NAME) {
		gCursorPosition = 10;
	}
	UI_DrawCursor(gCursorPosition, true);
}

static void DrawSettingName(uint8_t Index)
{
	gColorForeground = COLOR_WHITE;
	UI_DrawString(24, 76, Menu[Index], 14);
}

static void DrawNewSetting(void)
{
	uint8_t Index;
	uint8_t i;

	Index = 0;
	for (i = 0; i < 2; i++) {
		Index = (Index * 10) + gInputBox[i];
	}
	gInputBoxWriteIndex = 0;
	INPUTBOX_Pad(0, 10);
	UI_DrawString(136, 76, "  ", 2);
	if (Index && Index <= gSettingsCount) {
		gMenuIndex = Index - 1;
		MENU_Redraw(false);
	}
}

static void MENU_Digit(uint8_t Digit)
{
	if (gInputBoxWriteIndex == 0) {
		INPUTBOX_Pad(0, '-');
	}
	gInputBox[gInputBoxWriteIndex++] = (char)Digit;
	gInputBoxWriteIndex %= 2U;
	UI_DrawMenuPosition(gInputBox);

	if (gInputBoxWriteIndex == 0) {
		DrawNewSetting();
	}
}

static void DrawCss(uint8_t CodeType, uint16_t Code)
{
	switch (CodeType) {
	case CODE_TYPE_CTCSS:
		Int2Ascii(Code, 4);
		gShortString[4] = gShortString[3];
		gShortString[3] = '.';
		UI_DrawString(24, 48, gShortString, 5);
		break;

	case CODE_TYPE_DCS_N:
	case CODE_TYPE_DCS_I:
		Int2Ascii(CSS_ConvertCode(Code), 3);
		gShortString[3] = gShortString[2];
		gShortString[2] = gShortString[1];
		gShortString[1] = gShortString[0];
		gShortString[0] = 'D';
		if (gSettingCodeType == CODE_TYPE_DCS_N) {
			gShortString[4] = 'N';
		} else {
			gShortString[4] = 'I';
		}
		UI_DrawString(24, 48, gShortString, 5);
		break;

	case CODE_TYPE_OFF:
		UI_DrawString(24, 48, "Off  ", 5);
		break;
	}
}

static void CSS_KeyHandler(KEY_t Key)
{
	if (Key == KEY_STAR) {
		gSettingCurrentValue = 0;
		if (gSettingCodeType == CODE_TYPE_OFF) {
			gSettingCodeType = CODE_TYPE_CTCSS;
			gSettingCode = 0x29E;
		} else if (gSettingCodeType == CODE_TYPE_DCS_I) {
			gSettingCodeType = CODE_TYPE_OFF;
			gSettingCode = 0xFFF;
		} else {
			gSettingCodeType++;
			gSettingCode = DCS_GetOption(0);
		}
		MENU_SetMaxCssValues();
		DrawCss(gSettingCodeType, gSettingCode);
	}
}

static void DrawEditChannel(void)
{
	uint16_t Channel;
	uint8_t i;

	Channel = 0;
	for (i = 0; i < 3; i++) {
		Channel = (Channel * 10) + gInputBox[i];
	}
	gInputBoxWriteIndex = 0;
	INPUTBOX_Pad(0, 10);
	UI_DrawString(128, 76, "   ", 3);
	if (Channel && Channel < 1000) {
		gSettingCurrentValue = Channel - 1;
		UI_DrawChannelName(gSettingCurrentValue);
	}
}

static void CHANNEL_KeyHandler(KEY_t Key)
{
	if (Key != KEY_STAR && Key != KEY_HASH) {
		if (gInputBoxWriteIndex == 0) {
			INPUTBOX_Pad(0, '-');
		}
		gInputBox[gInputBoxWriteIndex++] = (char)Key;
		gInputBoxWriteIndex %= 3;
		UI_DrawChannelNumber(gInputBox);
		if (gInputBoxWriteIndex == 0) {
			DrawEditChannel();
		}
	}
}

static void MUTE_KeyHandler(uint8_t Key)
{
	if (Key < 10) {
		if (gInputBoxWriteIndex == 0) {
			INPUTBOX_Pad(0, '-');
		}
		gInputBox[gInputBoxWriteIndex] = Key + 0x30;
		gInputBoxWriteIndex = (gInputBoxWriteIndex + 1) % 8;
		UI_DrawString(24, 24, gInputBox, 8);
		if (gInputBoxWriteIndex == 0) {
			UI_DrawGolay();
		}
	}
}

//

void MENU_SetMaxCssValues(void)
{
	switch (gSettingCodeType) {
	case CODE_TYPE_CTCSS:
		gSettingMaxValues = 50;
		break;

	case CODE_TYPE_DCS_N:
	case CODE_TYPE_DCS_I:
		gSettingMaxValues = 105;
		break;

	case CODE_TYPE_OFF:
		gSettingMaxValues = 0;
		break;
	}
}

void MENU_AcceptSetting(void)
{
	uint16_t Channel;
	uint8_t i;

	switch (gMenuIndex) {
	case MENU_STARTUP_LOGO:
		gSettings.DisplayLogo = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;

	case MENU_VOLTAGE:
		gSettings.DisplayVoltage = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;

	case MENU_RINGTONE:
		gSettings.StartupRingTone = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;

	case MENU_PROMPT_TEXT:
		gSettings.DisplayLabel = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;

	case MENU_VOICE_PROMPT:
		gSettings.VoicePrompt = gSettingIndex;
		if (!gSettings.VoicePrompt) {
			SPEAKER_TurnOff(SPEAKER_OWNER_VOICE);
		}
		SETTINGS_SaveGlobals();
		break;

	case MENU_KEY_BEEP:
		gSettings.KeyBeep = gSettingIndex;
		if (!gSettings.KeyBeep) {
			SPEAKER_TurnOff(SPEAKER_OWNER_SYSTEM);
		}
		SETTINGS_SaveGlobals();
		break;

	case MENU_ROGER_BEEP:
		gSettings.RogerBeep = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		UI_DrawRoger();
		SETTINGS_SaveGlobals();
		break;

	case MENU_DUAL_DISPLAY:
		gSettings.DualDisplay = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;

	case MENU_TX_PRIORITY:
		gSettings.TxPriority = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;

	case MENU_SAVE_MODE:
		gSettings.SaveMode = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;

	case MENU_FREQ_STEP:
		gSettings.FrequencyStep = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		gFrequencyStep = FREQUENCY_GetStep(gSettings.FrequencyStep);
		SETTINGS_SaveGlobals();
		break;

	case MENU_SQ_LEVEL:
		gSettings.Squelch = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		SETTINGS_SaveGlobals();
		break;

	case MENU_LED_TIMER:
		gSettings.DisplayTimer = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		SETTINGS_SaveGlobals();
		break;

	case MENU_LOCK_TIMER:
		gSettings.LockTimer = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		SETTINGS_SaveGlobals();
		break;

	case MENU_TOT:
		gSettings.TimeoutTimer = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		SETTINGS_SaveGlobals();
		break;

	case MENU_VOX_LEVEL:
		gSettings.VoxLevel = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		SETTINGS_SaveGlobals();
		break;

	case MENU_VOX_DELAY:
		gSettings.VoxDelay = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		SETTINGS_SaveGlobals();
		break;

	case MENU_NOAA_MONITOR:
		gSettings.NoaaAlarm = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;

	case MENU_FM_STANDBY:
		gSettings.FmStandby = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;

	case MENU_TAIL_TONE:
		gSettings.TailTone = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;

	case MENU_SCAN_DIR:
		gSettings.ScanDirection = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;

	case MENU_PERSONAL_ID:
		for (i = 0; i < 16; i++) {
			gDeviceName[i] = gBigString[i];
		}
		SETTINGS_SaveDeviceName();
		break;

	case MENU_REPEATER_MODE:
		gSettings.RepeaterMode = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		UI_DrawRepeaterMode();
		SETTINGS_SaveGlobals();
		break;

	case MENU_CTCSS_DCS:
		gVfoState[gSettings.CurrentVfo].TX.CodeType = gSettingCodeType;
		gVfoState[gSettings.CurrentVfo].TX.Code = gSettingCode;
		gVfoState[gSettings.CurrentVfo].RX.CodeType = gSettingCodeType;
		gVfoState[gSettings.CurrentVfo].RX.Code = gSettingCode;
		CHANNELS_SaveVfo();
		break;

	case MENU_RX_CTCSS_DCS:
		gVfoState[gSettings.CurrentVfo].RX.CodeType = gSettingCodeType;
		gVfoState[gSettings.CurrentVfo].RX.Code = gSettingCode;
		CHANNELS_SaveVfo();
		break;

	case MENU_TX_CTCSS_DCS:
		gVfoState[gSettings.CurrentVfo].TX.CodeType = gSettingCodeType;
		gVfoState[gSettings.CurrentVfo].TX.Code = gSettingCode;
		CHANNELS_SaveVfo();
		break;

	case MENU_TX_POWER:
		gVfoState[gSettings.CurrentVfo].bIsLowPower = gSettingIndex;
		CHANNELS_SaveVfo();
		break;

	case MENU_MODULATION:
		gVfoState[gSettings.CurrentVfo].gModulationType = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		CHANNELS_SaveVfo();
		break;

	case MENU_BAND_WIDTH:
		gVfoState[gSettings.CurrentVfo].bIsNarrow = gSettingIndex;
		CHANNELS_SaveVfo();
		break;

	case MENU_BUSY_LOCK:
		gVfoState[gSettings.CurrentVfo].BCL = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		CHANNELS_SaveVfo();
		break;

	case MENU_SCRAMBLER:
		BEEP_Disable();
		gVfoState[gSettings.CurrentVfo].Scramble = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		CHANNELS_SaveVfo();
		break;

	case MENU_DCS_ENCRYPT:
		gVfoState[gSettings.CurrentVfo].Encrypt = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		CHANNELS_SaveVfo();
		break;

	case MENU_MUTE_CODE:
		if (gInputBoxWriteIndex) {
			gSettingGolay = 0;
			for (i = 0; i < 8; i++) {
				gSettingGolay = (gSettingGolay * 10) + (gInputBox[i] - '0');
			}
			if (gSettingGolay < 0x1000000) {
				return;
			}
			// Illegal code
			gSettingGolay &= 0xFFFFFF;
			Int2Ascii(gSettingGolay, 8);
			UI_DrawString(24, 24, gShortString, 8);
			return;
		}
		if (gSettingCurrentValue == 0) {
			gVfoState[gSettings.CurrentVfo].bMuteEnabled = 0;
			gVfoState[gSettings.CurrentVfo].bIs24Bit = 0;
		} else if (gSettingCurrentValue == 1) {
			gVfoState[gSettings.CurrentVfo].bMuteEnabled = 1;
			// TODO: Feels like there should be a bIs24Bit assignment??
		} else {
			gVfoState[gSettings.CurrentVfo].bMuteEnabled = 1;
			gVfoState[gSettings.CurrentVfo].bIs24Bit = 1;
		}
		gVfoState[gSettings.CurrentVfo].Golay = gSettingGolay;
		CHANNELS_SaveVfo();
		break;

	case MENU_CH_NAME:
		for (i = 0; i < 10; i++) {
			gVfoState[gSettings.CurrentVfo].Name[i] = gBigString[i];
		}
		CHANNELS_SaveVfo();
		break;

	case MENU_SAVE_CH:
		CHANNELS_SaveChannel((gSettingCurrentValue + gSettingIndex) % gSettingMaxValues, &gVfoState[gSettings.CurrentVfo]);
		CHANNELS_CheckFreeChannels();
		// TODO: This "if" block doesn't exist in the original, but there's a bug where VFO A is cleared by the previous line
		// TODO: but somehow the original FW doesn't show a corrupted VFO like OEFW does. Still hunting this bug down.
		if (gSettings.WorkMode) {
			CHANNELS_LoadChannel(gSettings.VfoChNo[0], 0);
		} else {
			CHANNELS_LoadChannel(999, 0);
		}
		RADIO_Retune();
		break;

	case MENU_DELETE_CH:
		Channel = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		CHANNELS_SaveChannel(Channel, &EmptyChannel);
		CHANNELS_CheckFreeChannels();
		if (gSettings.WorkMode) {
			if (gSettings.VfoChNo[0] == Channel) {
				gSettings.VfoChNo[0] = CHANNELS_GetChannelUp(Channel, 0);
			}
			if (gSettings.VfoChNo[1] == Channel) {
				gSettings.VfoChNo[1] = CHANNELS_GetChannelUp(Channel, 1);
			}
			SETTINGS_SaveGlobals();
		}
		if (gSettings.WorkMode) {
			CHANNELS_LoadChannel(gSettings.VfoChNo[!gSettings.CurrentVfo], !gSettings.CurrentVfo);
		} else {
			CHANNELS_LoadChannel(gSettings.CurrentVfo ? 999 : 1000, !gSettings.CurrentVfo);
		}
		RADIO_Retune();
		break;

	case MENU_K1_LONG:
	case MENU_K1_SHORT:
	case MENU_K2_LONG:
	case MENU_K2_SHORT:
		gSettings.Actions[gMenuIndex - MENU_K1_LONG] = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		SETTINGS_SaveGlobals();
		break;

	case MENU_DTMF_DELAY:
		gDTMF_Settings.Delay = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		SETTINGS_SaveDTMF();
		break;

	case MENU_DTMF_INTERVAL:
		gDTMF_Settings.Interval = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		SETTINGS_SaveDTMF();
		break;

	case MENU_DTMF_MODE:
		gDTMF_Settings.Mode = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		SETTINGS_SaveDTMF();
		break;

	case MENU_DTMF_SELECT:
		gDTMF_Settings.Select = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		SETTINGS_SaveDTMF();
		break;

	case MENU_DTMF_DISPLAY:
		gDTMF_Settings.Display = gSettingIndex;
		SETTINGS_SaveDTMF();
		break;

	case MENU_INITIALIZE:
		if (gSettingIndex == 1) {
			SETTINGS_FactoryReset();
			HARDWARE_Reboot();
		}
		break;
	}

	UI_DrawSettingArrow(gSettingIndex);
}

void MENU_DrawSetting(void)
{
	uint8_t i;

	gScreenMode = SCREEN_SETTING;
	gInputBoxWriteIndex = 0;
	DrawSettingName(gMenuIndex);
	gSettingIndex = 0;
	gSettingMaxValues = 2;

	switch (gMenuIndex) {
	case MENU_STARTUP_LOGO:
		gSettingIndex = gSettings.DisplayLogo;
		UI_DrawToggle();
		break;

	case MENU_VOLTAGE:
		gSettingIndex = gSettings.DisplayVoltage;
		UI_DrawToggle();
		break;

	case MENU_RINGTONE:
		gSettingIndex = gSettings.StartupRingTone;
		UI_DrawToggle();
		break;
	case MENU_PROMPT_TEXT:
		gSettingIndex = gSettings.DisplayLabel;
		UI_DrawToggle();
		break;

	case MENU_VOICE_PROMPT:
		gSettingIndex = gSettings.VoicePrompt;
		UI_DrawToggle();
		break;

	case MENU_KEY_BEEP:
		gSettingIndex = gSettings.KeyBeep;
		UI_DrawToggle();
		break;

	case MENU_ROGER_BEEP:
		gSettingCurrentValue = gSettings.RogerBeep;
		gSettingMaxValues = 4;
		UI_DrawSettingRoger(gSettingCurrentValue);
		break;

	case MENU_DUAL_DISPLAY:
		gSettingIndex = gSettings.DualDisplay;
		UI_DrawToggle();
		break;

	case MENU_TX_PRIORITY:
		gSettingIndex = gSettings.TxPriority;
		UI_DrawTxPriority();
		break;

	case MENU_SAVE_MODE:
		gSettingIndex = gSettings.SaveMode;
		UI_DrawToggle();
		break;

	case MENU_FREQ_STEP:
		gSettingCurrentValue = gSettings.FrequencyStep;
		gSettingMaxValues = 15;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawFrequencyStep(gSettingCurrentValue);
		break;

	case MENU_SQ_LEVEL:
		gSettingCurrentValue = gSettings.Squelch;
		gSettingMaxValues = 10;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawLevel(gSettingCurrentValue);
		break;

	case MENU_LED_TIMER:
		gSettingCurrentValue = gSettings.DisplayTimer;
		gSettingMaxValues = 43;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawTimer(gSettingCurrentValue);
		break;

	case MENU_LOCK_TIMER:
		gSettingCurrentValue = gSettings.LockTimer;
		gSettingMaxValues = 43;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawTimer(gSettingCurrentValue);
		break;

	case MENU_TOT:
		gSettingCurrentValue = gSettings.TimeoutTimer;
		gSettingMaxValues = 43;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawTimer(gSettingCurrentValue);
		break;

	case MENU_VOX_LEVEL:
		gSettingCurrentValue = gSettings.VoxLevel;
		gSettingMaxValues = 10;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawLevel(gSettingCurrentValue);
		break;

	case MENU_VOX_DELAY:
		gSettingCurrentValue = gSettings.VoxDelay;
		gSettingMaxValues = 10;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawLevel(gSettingCurrentValue);
		break;

	case MENU_NOAA_MONITOR:
		gSettingIndex = gSettings.NoaaAlarm;
		UI_DrawToggle();
		break;

	case MENU_FM_STANDBY:
		gSettingIndex = gSettings.FmStandby;
		UI_DrawToggle();
		break;

	case MENU_TAIL_TONE:
		gSettingIndex = gSettings.TailTone;
		UI_DrawToggle();
		break;

	case MENU_SCAN_DIR:
		gSettingIndex = gSettings.ScanDirection;
		UI_DrawScanDirection();
		break;

	case MENU_PERSONAL_ID:
		gSettingMaxValues = 1;
		T9State = 0;
		EditSize = 16;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawActivateBy();
		T9WritePos = 16;
		for (i = 0; i < EditSize; i++) {
			gBigString[i] = gDeviceName[i];
		}
		UI_DrawDeviceName(gBigString);
		EnableTextEditor();
		return;

	case MENU_REPEATER_MODE:
		gSettingCurrentValue = gSettings.RepeaterMode;
		gSettingMaxValues = 3;
		UI_DrawSettingRepeaterMode(gSettingCurrentValue);
		break;

	case MENU_CTCSS_DCS:
	case MENU_RX_CTCSS_DCS:
		gSettingCode = gVfoState[gSettings.CurrentVfo].RX.Code;
		gSettingCodeType = gVfoState[gSettings.CurrentVfo].RX.CodeType;
		MENU_SetMaxCssValues();
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawSettingArrow(0);
		UI_DrawStringSwitchType();
		DrawCss(gSettingCodeType, gSettingCode);
		return;

	case MENU_TX_CTCSS_DCS:
		gSettingCode = gVfoState[gSettings.CurrentVfo].TX.Code;
		gSettingCodeType = gVfoState[gSettings.CurrentVfo].TX.CodeType;
		MENU_SetMaxCssValues();
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawSettingArrow(0);
		UI_DrawStringSwitchType();
		DrawCss(gSettingCodeType, gSettingCode);
		return;

	case MENU_TX_POWER:
		gSettingIndex = gVfoState[gSettings.CurrentVfo].bIsLowPower;
		UI_DrawSettingTxPower();
		break;

	case MENU_MODULATION:
		gSettingIndex = gVfoState[gSettings.CurrentVfo].gModulationType;
		gSettingMaxValues = 4;
		UI_DrawSettingModulation(gSettingIndex);
		break;

	case MENU_BAND_WIDTH:
		gSettingIndex = gVfoState[gSettings.CurrentVfo].bIsNarrow;
		UI_DrawSettingBandwidth();
		break;

	case MENU_BUSY_LOCK:
		gSettingCurrentValue = gVfoState[gSettings.CurrentVfo].BCL;
		gSettingMaxValues = 3;
		UI_DrawSettingBusyLock(gSettingCurrentValue);
		break;

	case MENU_SCRAMBLER:
		gSettingCurrentValue = gVfoState[gSettings.CurrentVfo].Scramble;
		gSettingMaxValues = 9;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawScrambler(gSettingCurrentValue);
		break;

	case MENU_DCS_ENCRYPT:
		gSettingCurrentValue = gVfoState[gSettings.CurrentVfo].Encrypt;
		gSettingMaxValues = 4;
		UI_DrawEncrypt(gSettingCurrentValue);
		break;

	case MENU_MUTE_CODE:
		gSettingMaxValues = 3;
		if (gVfoState[gSettings.CurrentVfo].bMuteEnabled) {
			gSettingCurrentValue = 1 + gVfoState[gSettings.CurrentVfo].bIs24Bit;
		} else {
			gSettingCurrentValue = 0;
		}
		gSettingGolay = gVfoState[gSettings.CurrentVfo].Golay;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawSettingArrow(0);
		UI_DrawMute(gSettingCurrentValue, gSettingGolay, true);
		return;

	case MENU_CH_NAME:
		gSettingMaxValues = 1;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawActivateBy();
		T9State = 0;
		EditSize = 10;
		T9WritePos = 10;
		for (i = 0; i < 10; i++) {
			gBigString[i] = gVfoState[gSettings.CurrentVfo].Name[i];
		}
		UI_DrawString(4, 48, gBigString, 10);
		EnableTextEditor();
		return;

	case MENU_SAVE_CH:
	case MENU_DELETE_CH:
		gSettingMaxValues = 999;
		gSettingCurrentValue = gSettings.VfoChNo[gSettings.CurrentVfo];
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawChannelName(gSettingCurrentValue);
		break;

	case MENU_K1_LONG:
	case MENU_K1_SHORT:
	case MENU_K2_LONG:
	case MENU_K2_SHORT:
		gSettingCurrentValue = gSettings.Actions[gMenuIndex - MENU_K1_LONG] % 10U;
		gSettingMaxValues = 10;
		UI_DrawActions(gSettings.Actions[gMenuIndex - MENU_K1_LONG] % 10U);
		break;

	case MENU_DTMF_DELAY:
		gSettingCurrentValue = gDTMF_Settings.Delay;
		gSettingMaxValues = 21;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawDtmfDelay(gSettingCurrentValue);
		break;

	case MENU_DTMF_INTERVAL:
		gSettingCurrentValue = gDTMF_Settings.Interval;
		gSettingMaxValues = 18;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawDtmfInterval(gSettingCurrentValue);
		break;

	case MENU_DTMF_MODE:
		gSettingCurrentValue = gDTMF_Settings.Mode;
		gSettingMaxValues = 4;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawDtmfMode(gSettingCurrentValue);
		break;

	case MENU_DTMF_SELECT:
		gSettingCurrentValue = gDTMF_Settings.Select;
		gSettingMaxValues = 16;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		UI_DrawDtmfSelect(gSettingCurrentValue);
		break;

	case MENU_DTMF_DISPLAY:
		gSettingIndex = gDTMF_Settings.Display;
		UI_DrawToggle();
		break;

	case MENU_INITIALIZE:
		UI_DrawToggle();
		break;

	case MENU_VERSION:
		gSettingMaxValues = 0;
		UI_DrawVersion();
		break;

	case MENU_INSTRUCTION:
		gSettingMaxValues = 0;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BLACK);
		DrawQR();
		return;
	}

	UI_DrawSettingArrow(gSettingIndex);
}

void MENU_Redraw(bool bClear)
{
	gCursorEnabled = false;
	gScreenMode = SCREEN_MENU;
	gSettingIndex = 0;
	gInputBoxWriteIndex = 0;
	if (bClear) {
		DISPLAY_Fill(0, 159, 1, 81, COLOR_BLACK);
		DISPLAY_DrawRectangle0(0, 56, 160, 1, gSettings.BorderColor);
	}
	gColorForeground = COLOR_WHITE;
	UI_DrawStringMenuSettings();
	UI_DrawSettingArrow(0);
	DrawMenu(gMenuIndex);
	MENU_PlayAudio(gMenuIndex);
}

void MENU_KeyHandler(uint8_t Key)
{
	switch (Key) {
	case KEY_0: case KEY_1: case KEY_2: case KEY_3:
	case KEY_4: case KEY_5: case KEY_6: case KEY_7:
	case KEY_8: case KEY_9:
		MENU_Digit(Key);
		BEEP_Play(740, 2, 100);
		break;

	case KEY_MENU:
		gMenuIndex = (gMenuIndex + gSettingIndex) % gSettingsCount;
		UI_DrawString(1, 112, "  ", 2);
		MENU_DrawSetting();
		BEEP_Play(740, 3, 80);
		break;

	case KEY_UP:
	case KEY_DOWN:
		UI_DrawString(1, 112, "  ", 2);
		MENU_Next(Key);
		MENU_PlayAudio((gMenuIndex + gSettingIndex) % gSettingsCount);
		BEEP_Play(740, 2, 100);
		break;

	case KEY_EXIT:
		gMenuIndex = (gMenuIndex + gSettingIndex) % gSettingsCount;
		gScreenMode = SCREEN_MAIN;
		gInputBoxWriteIndex = 0;
		if (gRadioMode != RADIO_MODE_RX) {
			RADIO_Tune(gSettings.CurrentVfo);
		}
		UI_DrawMain(true);
		BEEP_Play(440, 4, 80);
		break;

	default:
		break;
	}
}

void MENU_Next(uint8_t Key)
{
	if (Key == KEY_UP) {
		if (gSettingIndex) {
			gSettingIndex = 0;
			UI_DrawSettingArrow(0);
		} else {
			gMenuIndex = (gMenuIndex + gSettingsCount - 1) % gSettingsCount;
			DrawMenu(gMenuIndex);
		}
	} else {
		if (gSettingIndex) {
			gMenuIndex = (gMenuIndex + 1) % gSettingsCount;
			DrawMenu(gMenuIndex);
		} else {
			gSettingIndex = 1;
			UI_DrawSettingArrow(1);
		}
	}
}

void MENU_SettingKeyHandler(uint8_t Key)
{
	switch (Key) {
	case KEY_MENU:
	case KEY_EXIT:
		if (T9State == 0) {
			if (Key == KEY_MENU) {
				MENU_AcceptSetting();
			}
			if (gMenuIndex == MENU_PERSONAL_ID || gMenuIndex == MENU_CH_NAME || gMenuIndex == MENU_INSTRUCTION || gMenuIndex == MENU_CTCSS_DCS || gMenuIndex == MENU_RX_CTCSS_DCS || gMenuIndex == MENU_TX_CTCSS_DCS) {
				MENU_Redraw(true);
			} else {
				MENU_Redraw(false);
			}
			if (Key == KEY_MENU) {
				BEEP_Play(740, 3, 80);
			} else {
				BEEP_Play(440, 4, 80);
			}
		}
		break;

	case KEY_UP:
	case KEY_DOWN:
		MENU_ScrollSetting(Key);
		BEEP_Play(740, 2, 100);
		break;

	default:
		switch (gMenuIndex) {
		case MENU_PERSONAL_ID:
		case MENU_CH_NAME:
			T9_Editor(Key, EditSize);
			break;

		case MENU_CTCSS_DCS:
		case MENU_RX_CTCSS_DCS:
		case MENU_TX_CTCSS_DCS:
			CSS_KeyHandler(Key);
			break;

		case MENU_MUTE_CODE:
			MUTE_KeyHandler(Key);
			break;

		case MENU_SAVE_CH:
		case MENU_DELETE_CH:
			CHANNEL_KeyHandler(Key);
			break;
		}
		BEEP_Play(740, 2, 100);
		break;
	}
}

void MENU_ScrollSetting(uint8_t Key)
{
	if (!gSettingMaxValues) {
		return;
	}

	if (gSettingMaxValues == 2) {
		gSettingIndex = !gSettingIndex;
		UI_DrawSettingArrow(gSettingIndex);
		return;
	}

	if (gSettingMaxValues == 1) {
		T9_Editor(Key, EditSize);
		return;
	}

	if (gMenuIndex == MENU_CTCSS_DCS || gMenuIndex == MENU_RX_CTCSS_DCS || gMenuIndex == MENU_TX_CTCSS_DCS) {
		if (gSettingCodeType != CODE_TYPE_OFF) {
			if (Key == KEY_UP) {
				gSettingCurrentValue = (gSettingCurrentValue + gSettingMaxValues - 1) % gSettingMaxValues;
			} else {
				gSettingCurrentValue = (gSettingCurrentValue + 1) % gSettingMaxValues;
			}
			if (gSettingCodeType == CODE_TYPE_CTCSS) {
				gSettingCode = CTCSS_GetOption(gSettingCurrentValue);
			} else {
				gSettingCode = DCS_GetOption(gSettingCurrentValue);
			}
			DrawCss(gSettingCodeType, gSettingCode);
		}
		return;
	}

	if (Key == KEY_UP && (gSettingIndex == 0 || gMenuIndex == MENU_MUTE_CODE)) {
		gSettingCurrentValue = (gSettingCurrentValue + gSettingMaxValues - 1) % gSettingMaxValues;
	} else {
		if (Key != KEY_DOWN || ((gSettingIndex != 1 && gMenuIndex != MENU_MUTE_CODE))) {
			gSettingIndex = !gSettingIndex;
			UI_DrawSettingArrow(gSettingIndex);
			return;
		}
		gSettingCurrentValue = (gSettingCurrentValue + 1) % gSettingMaxValues;
	}

	switch (gMenuIndex) {
	case MENU_ROGER_BEEP:
		UI_DrawSettingRoger(gSettingCurrentValue);
		break;

	case MENU_FREQ_STEP:
		UI_DrawFrequencyStep(gSettingCurrentValue);
		break;

	case MENU_SQ_LEVEL:
	case MENU_VOX_LEVEL:
	case MENU_VOX_DELAY:
		UI_DrawLevel(gSettingCurrentValue);
		break;

	case MENU_LED_TIMER:
	case MENU_LOCK_TIMER:
	case MENU_TOT:
		UI_DrawTimer(gSettingCurrentValue);
		break;

	case MENU_REPEATER_MODE:
		UI_DrawSettingRepeaterMode(gSettingCurrentValue);
		break;

	case MENU_TX_POWER:
		UI_DrawSettingTxPower();
		break;

	case MENU_MODULATION:
		UI_DrawSettingModulation(gSettingCurrentValue);
		break;

	case MENU_BAND_WIDTH:
		UI_DrawSettingBandwidth();
		break;

	case MENU_BUSY_LOCK:
		UI_DrawSettingBusyLock(gSettingCurrentValue);
		break;

	case MENU_SCRAMBLER:
		UI_DrawScrambler(gSettingCurrentValue);
		break;

	case MENU_DCS_ENCRYPT:
		UI_DrawEncrypt(gSettingCurrentValue);
		break;

	case MENU_MUTE_CODE:
		UI_DrawMute(gSettingCurrentValue, gSettingGolay, true);
		break;

	case MENU_SAVE_CH:
	case MENU_DELETE_CH:
		UI_DrawChannelName(gSettingCurrentValue);
		break;

	case MENU_K1_LONG:
	case MENU_K1_SHORT:
	case MENU_K2_LONG:
	case MENU_K2_SHORT:
		UI_DrawActions(gSettingCurrentValue);
		break;

	case MENU_DTMF_DELAY:
		UI_DrawDtmfDelay(gSettingCurrentValue);
		break;

	case MENU_DTMF_INTERVAL:
		UI_DrawDtmfInterval(gSettingCurrentValue);
		break;

	case MENU_DTMF_MODE:
		UI_DrawDtmfMode(gSettingCurrentValue);
		break;

	case MENU_DTMF_SELECT:
		UI_DrawDtmfSelect(gSettingCurrentValue);
		break;
	}
}

void MENU_PlayAudio(uint8_t MenuID)
{
	uint8_t ID;

	switch (MenuID) {
	case MENU_STARTUP_LOGO:  ID = 0x09; break;
	case MENU_VOLTAGE:       ID = 0x0A; break;
	case MENU_RINGTONE:      ID = 0x0B; break;
	case MENU_PROMPT_TEXT:   ID = 0x0C; break;
	case MENU_VOICE_PROMPT:  ID = 0x0E; break;
	case MENU_KEY_BEEP:      ID = 0x0F; break;
	case MENU_ROGER_BEEP:    ID = 0x10; break;
	case MENU_DUAL_DISPLAY:  ID = 0x1E; break;
	case MENU_TX_PRIORITY:   ID = 0x13; break;
	case MENU_SAVE_MODE:     ID = 0x14; break;
	case MENU_FREQ_STEP:     ID = 0x15; break;
	case MENU_SQ_LEVEL:      ID = 0x16; break;
	case MENU_LED_TIMER:     ID = 0x17; break;
	case MENU_LOCK_TIMER:    ID = 0x18; break;
	case MENU_TOT:           ID = 0x19; break;
	case MENU_VOX_LEVEL:     ID = 0x1A; break;
	case MENU_VOX_DELAY:     ID = 0x1B; break;
	case MENU_NOAA_MONITOR:  ID = 0x1C; break;
	case MENU_FM_STANDBY:    ID = 0x1D; break;
	case MENU_TAIL_TONE:     ID = 0x1F; break;
	case MENU_SCAN_DIR:      ID = 0x20; break;
	case MENU_PERSONAL_ID:   ID = 0x21; break;
	case MENU_REPEATER_MODE: ID = 0x40; break;
	case MENU_CTCSS_DCS:     ID = 0x25; break;
	case MENU_RX_CTCSS_DCS:  ID = 0x26; break;
	case MENU_TX_CTCSS_DCS:  ID = 0x27; break;
	case MENU_TX_POWER:      ID = 0x28; break;
	case MENU_BAND_WIDTH:    ID = 0x29; break;
	case MENU_BUSY_LOCK:     ID = 0x2A; break;
	case MENU_SCRAMBLER:     ID = 0x2B; break;
	case MENU_DCS_ENCRYPT:   ID = 0x2C; break;
	case MENU_MUTE_CODE:     ID = 0x2D; break;
	case MENU_CH_NAME:       ID = 0x2F; break;
	case MENU_SAVE_CH:       ID = 0x30; break;
	case MENU_DELETE_CH:     ID = 0x31; break;
	case MENU_K1_LONG:       ID = 0x33; break;
	case MENU_K1_SHORT:      ID = 0x34; break;
	case MENU_K2_LONG:       ID = 0x35; break;
	case MENU_K2_SHORT:      ID = 0x36; break;
	case MENU_INITIALIZE:    ID = 0x23; break;
	case MENU_VERSION:       ID = 0x3B; break;
	case MENU_INSTRUCTION:   ID = 0x3A; break;
	}

	if (MenuID != MENU_DTMF_DELAY && MenuID != MENU_DTMF_INTERVAL && MenuID != MENU_DTMF_MODE && MenuID != MENU_DTMF_SELECT && MenuID != MENU_DTMF_DISPLAY) {
		AUDIO_PlaySampleOptional(ID);
	}
}

