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
#include "task/keyaction.h"
#include "task/sidekeys.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "ui/menu.h"
#include "ui/version.h"

static const char Menu[][14] = {
	"Startup Logo  ",
	"Voltage       ",
	"Ringtone      ",
	"Prompt Text   ",
	"Voice Prompt  ",
	"Key Beep      ",
	"Roger Beep    ",
	"Dual Display  ",
	"TX Priority   ",
	"Save Mode     ",
	"Freq Step     ",
	"SQ Level      ",
	"LED Timer     ",
	"Lock Timer    ",
	"TOT           ",
	"VOX Level     ",
	"VOX Delay     ",
	"NOAA Monitor  ",
	"FM Standby    ",
	"Tail Tone     ",
	"Scan DIR      ",
	"Personal ID   ",
	"Repeater Mode ",
	"Scan Resume   ",
	"Scan Blink    ",
	"CTCSS/DCS     ",
	"RX CTCSS/DCS  ",
	"TX CTCSS/DCS  ",
	"TX Power      ",
	"Modulation    ",
	"Band Width    ",
	"List To Scan  ",
	"Ch In List 1  ",
	"Ch In List 2  ",
	"Ch In List 3  ",
	"Ch In List 4  ",
	"Ch In List 5  ",
	"Ch In List 6  ",
	"Ch In List 7  ",
	"Ch In List 8  ",
	"Busy Lock     ",
	"Scrambler     ",
	"DCS Encrypt   ",
	"Mute Code     ",
	"CH Name       ",
	"Save CH       ",
	"Delete CH     ",
	"Side 1 Long   ",
	"Side 1 Short  ",
	"Side 2 Long   ",
	"Side 2 Short  ",
	"Key 0 Long    ",
	"Key 1 Long    ",
	"Key 2 Long    ",
	"Key 3 Long    ",
	"Key 4 Long    ",
	"Key 5 Long    ",
	"Key 6 Long    ",
	"Key 7 Long    ",
	"Key 8 Long    ",
	"Key 9 Long    ",
	"Key * Long    ",
	"Key # Long    ",
	"Key Menu Long ",
	"Key Exit Long ",
	"Reset Keys    ",
	"DTMF Delay    ",
	"DTMF Interval ",
	"DTMF Mode     ",
	"DTMF Select   ",
	"DTMF Display  ",
	"Dark Mode     ",
	"Initialize    ",
	"Version       ",
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
	.IsInscanList = 0xFF,
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
uint8_t gSettingsCount = sizeof(Menu) / sizeof(Menu[0]);

static void DrawMenu(uint8_t Index)
{
	UI_DrawString(24, 72, Menu[(Index + gSettingsCount - 1) % gSettingsCount], 14);
	Int2Ascii(((Index + gSettingsCount - 1) % gSettingsCount) + 1, 2);
	UI_DrawString(140, 72, gShortString, 2);
	UI_DrawString(24, 48, Menu[Index], 14);
	Int2Ascii(Index + 1, 2);
	UI_DrawString(140, 48, gShortString, 2);
	UI_DrawString(24, 24, Menu[(Index + 1) % gSettingsCount], 14);
	Int2Ascii(((Index + 1) % gSettingsCount) + 1, 2);
	UI_DrawString(140, 24, gShortString, 2);
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
	gColorForeground = COLOR_BLUE;
	UI_DrawString(24, 72, Menu[Index], 14);
	Int2Ascii((Index + 1), 2);
	UI_DrawString(140, 72, gShortString, 2);
	gColorForeground = COLOR_FOREGROUND;
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
	UI_DrawSettingArrow(0);
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
	UI_DrawString(132, 48, "   ", 3);
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
#ifdef ENABLE_NOAA
	case MENU_NOAA_MONITOR:
		gSettings.NoaaAlarm = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;
#endif
#ifdef ENABLE_FM_RADIO
	case MENU_FM_STANDBY:
		gSettings.FmStandby = gSettingIndex;
		SETTINGS_SaveGlobals();
		break;
#endif
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

	case MENU_SCAN_RESUME:
		gExtendedSettings.ScanResume = ((gSettingCurrentValue + gSettingIndex) % gSettingMaxValues) + 1;
		SETTINGS_SaveGlobals();
		break;

	case MENU_SCAN_BLINK:
		gExtendedSettings.ScanBlink = gSettingIndex;
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

	case MENU_LIST_TO_SCAN:
		gExtendedSettings.ScanAll = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues == 8;
		if (!gExtendedSettings.ScanAll) {
			gExtendedSettings.CurrentScanList = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		}
		SETTINGS_SaveGlobals();
		break;

	case MENU_SCANLIST_1:
	case MENU_SCANLIST_2:
	case MENU_SCANLIST_3:
	case MENU_SCANLIST_4:
	case MENU_SCANLIST_5:
	case MENU_SCANLIST_6:
	case MENU_SCANLIST_7:
	case MENU_SCANLIST_8:
		gVfoState[gSettings.CurrentVfo].IsInscanList =
				(gVfoState[gSettings.CurrentVfo].IsInscanList & ~(1 << (gMenuIndex - MENU_SCANLIST_1)))	// cleaning the bit corresponding to the scanlist
				| (gSettingIndex << (gMenuIndex - MENU_SCANLIST_1));									// set the bit to 1 if gSettingIndex = "On"
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
	case MENU_0_LONG:
	case MENU_1_LONG:
	case MENU_2_LONG:
	case MENU_3_LONG:
	case MENU_4_LONG:
	case MENU_5_LONG:
	case MENU_6_LONG:
	case MENU_7_LONG:
	case MENU_8_LONG:
	case MENU_9_LONG:
	case MENU_STAR_LONG:
	case MENU_HASH_LONG:
	case MENU_MENU_LONG:
	case MENU_EXIT_LONG:
		if (gMenuIndex >= MENU_0_LONG) {
			gExtendedSettings.KeyShortcut[gMenuIndex - MENU_0_LONG] = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		} else {
			gSettings.Actions[gMenuIndex - MENU_K1_LONG] = (gSettingCurrentValue + gSettingIndex) % gSettingMaxValues;
		}
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

	case MENU_DARK_MODE:
		gExtendedSettings.DarkMode = gSettingIndex;
		SETTINGS_SaveGlobals();
		UI_SetColors(gExtendedSettings.DarkMode);
		DrawStatusBar();
		MENU_Redraw(true);
		break;

	case MENU_INITIALIZE:
		if (gSettingIndex == 1) {
			SETTINGS_FactoryReset();
			HARDWARE_Reboot();
		}
		break;

	case MENU_KEYS_RESET:
		if (gSettingIndex == 1) {
			SetDefaultKeyShortcuts(true);
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
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawSettingRoger(gSettingCurrentValue);
		break;

	case MENU_DUAL_DISPLAY:
		gSettingIndex = gSettings.DualDisplay;
		UI_DrawToggle();
		break;

	case MENU_TX_PRIORITY:
		gSettingIndex = gSettings.TxPriority;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawTxPriority();
		break;

	case MENU_SAVE_MODE:
		gSettingIndex = gSettings.SaveMode;
		UI_DrawToggle();
		break;

	case MENU_FREQ_STEP:
		gSettingCurrentValue = gSettings.FrequencyStep;
		gSettingMaxValues = 15;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawFrequencyStep(gSettingCurrentValue);
		break;

	case MENU_SQ_LEVEL:
		gSettingCurrentValue = gSettings.Squelch;
		gSettingMaxValues = 10;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawLevel(gSettingCurrentValue);
		break;

	case MENU_LED_TIMER:
		gSettingCurrentValue = gSettings.DisplayTimer;
		gSettingMaxValues = 43;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawTimer(gSettingCurrentValue);
		break;

	case MENU_LOCK_TIMER:
		gSettingCurrentValue = gSettings.LockTimer;
		gSettingMaxValues = 43;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawTimer(gSettingCurrentValue);
		break;

	case MENU_TOT:
		gSettingCurrentValue = gSettings.TimeoutTimer;
		gSettingMaxValues = 43;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawTimer(gSettingCurrentValue);
		break;

	case MENU_VOX_LEVEL:
		gSettingCurrentValue = gSettings.VoxLevel;
		gSettingMaxValues = 10;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawLevel(gSettingCurrentValue);
		break;

	case MENU_VOX_DELAY:
		gSettingCurrentValue = gSettings.VoxDelay;
		gSettingMaxValues = 10;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawLevel(gSettingCurrentValue);
		break;
#ifdef ENABLE_NOAA
	case MENU_NOAA_MONITOR:
		gSettingIndex = gSettings.NoaaAlarm;
		UI_DrawToggle();
		break;
#endif
#ifdef ENABLE_FM_RADIO
	case MENU_FM_STANDBY:
		gSettingIndex = gSettings.FmStandby;
		UI_DrawToggle();
		break;
#endif
	case MENU_TAIL_TONE:
		gSettingIndex = gSettings.TailTone;
		UI_DrawToggle();
		break;

	case MENU_SCAN_DIR:
		gSettingIndex = gSettings.ScanDirection;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawScanDirection();
		break;

	case MENU_PERSONAL_ID:
		gSettingMaxValues = 1;
		T9State = 0;
		EditSize = 16;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
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
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawSettingRepeaterMode(gSettingCurrentValue);
		break;

	case MENU_SCAN_RESUME:
		gSettingCurrentValue = gExtendedSettings.ScanResume - 1;
		gSettingMaxValues = 3;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawSettingScanResume(gSettingCurrentValue);
		break;
	
	case MENU_SCAN_BLINK:
		gSettingIndex = gExtendedSettings.ScanBlink;
		UI_DrawToggle();
		break;

	case MENU_CTCSS_DCS:
	case MENU_RX_CTCSS_DCS:
		gSettingCode = gVfoState[gSettings.CurrentVfo].RX.Code;
		gSettingCodeType = gVfoState[gSettings.CurrentVfo].RX.CodeType;
		MENU_SetMaxCssValues();
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawSettingArrow(0);
		UI_DrawStringSwitchType();
		DrawCss(gSettingCodeType, gSettingCode);
		return;

	case MENU_TX_CTCSS_DCS:
		gSettingCode = gVfoState[gSettings.CurrentVfo].TX.Code;
		gSettingCodeType = gVfoState[gSettings.CurrentVfo].TX.CodeType;
		MENU_SetMaxCssValues();
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawSettingArrow(0);
		UI_DrawStringSwitchType();
		DrawCss(gSettingCodeType, gSettingCode);
		return;

	case MENU_TX_POWER:
		gSettingIndex = gVfoState[gSettings.CurrentVfo].bIsLowPower;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawSettingTxPower();
		break;

	case MENU_MODULATION:
		gSettingCurrentValue = gVfoState[gSettings.CurrentVfo].gModulationType;
		gSettingMaxValues = 4;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawSettingModulation(gSettingCurrentValue);
		break;

	case MENU_BAND_WIDTH:
		gSettingIndex = gVfoState[gSettings.CurrentVfo].bIsNarrow;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawSettingBandwidth();
		break;

	case MENU_LIST_TO_SCAN:
		gSettingCurrentValue = gExtendedSettings.ScanAll ? 8 : gExtendedSettings.CurrentScanList;
		gSettingMaxValues = 9;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawSettingScanlist(gSettingCurrentValue);
		break;

	case MENU_SCANLIST_1:
	case MENU_SCANLIST_2:
	case MENU_SCANLIST_3:
	case MENU_SCANLIST_4:
	case MENU_SCANLIST_5:
	case MENU_SCANLIST_6:
	case MENU_SCANLIST_7:
	case MENU_SCANLIST_8:
		gSettingIndex = ((gVfoState[gSettings.CurrentVfo].IsInscanList >> (gMenuIndex - MENU_SCANLIST_1)) & 1);	// pick the bit corresponding to the scanlist
		UI_DrawToggle();
		break;

	case MENU_BUSY_LOCK:
		gSettingCurrentValue = gVfoState[gSettings.CurrentVfo].BCL;
		gSettingMaxValues = 3;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawSettingBusyLock(gSettingCurrentValue);
		break;

	case MENU_SCRAMBLER:
		gSettingCurrentValue = gVfoState[gSettings.CurrentVfo].Scramble;
		gSettingMaxValues = 9;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawScrambler(gSettingCurrentValue);
		break;

	case MENU_DCS_ENCRYPT:
		gSettingCurrentValue = gVfoState[gSettings.CurrentVfo].Encrypt;
		gSettingMaxValues = 4;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
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
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawSettingArrow(0);
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawMute(gSettingCurrentValue, gSettingGolay, true);
		return;

	case MENU_CH_NAME:
		gSettingMaxValues = 1;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
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
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawChannelName(gSettingCurrentValue);
		break;

	case MENU_K1_LONG:
	case MENU_K1_SHORT:
	case MENU_K2_LONG:
	case MENU_K2_SHORT:
	case MENU_0_LONG:
	case MENU_1_LONG:
	case MENU_2_LONG:
	case MENU_3_LONG:
	case MENU_4_LONG:
	case MENU_5_LONG:
	case MENU_6_LONG:
	case MENU_7_LONG:
	case MENU_8_LONG:
	case MENU_9_LONG:
	case MENU_STAR_LONG:
	case MENU_HASH_LONG:
	case MENU_MENU_LONG:
	case MENU_EXIT_LONG:
		gSettingMaxValues = ACTIONS_COUNT;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		if (gMenuIndex >= MENU_0_LONG) {
			gSettingCurrentValue = gExtendedSettings.KeyShortcut[gMenuIndex - MENU_0_LONG] % ACTIONS_COUNT;
		} else {
			gSettingCurrentValue = gSettings.Actions[gMenuIndex - MENU_K1_LONG] % ACTIONS_COUNT;
		}
		UI_DrawActions(gSettingCurrentValue);
		break;

	case MENU_KEYS_RESET:
		UI_DrawToggle();
		break;

	case MENU_DTMF_DELAY:
		gSettingCurrentValue = gDTMF_Settings.Delay;
		gSettingMaxValues = 21;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawDtmfDelay(gSettingCurrentValue);
		break;

	case MENU_DTMF_INTERVAL:
		gSettingCurrentValue = gDTMF_Settings.Interval;
		gSettingMaxValues = 18;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawDtmfInterval(gSettingCurrentValue);
		break;

	case MENU_DTMF_MODE:
		gSettingCurrentValue = gDTMF_Settings.Mode;
		gSettingMaxValues = 4;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawDtmfMode(gSettingCurrentValue);
		break;

	case MENU_DTMF_SELECT:
		gSettingCurrentValue = gDTMF_Settings.Select;
		gSettingMaxValues = 16;
		DISPLAY_Fill(0, 159, 1, 55, COLOR_BACKGROUND);
		UI_DrawDtmfSelect(gSettingCurrentValue);
		break;

	case MENU_DTMF_DISPLAY:
		gSettingIndex = gDTMF_Settings.Display;
		UI_DrawToggle();
		break;

	case MENU_DARK_MODE:
		gSettingIndex = gExtendedSettings.DarkMode;
		UI_DrawToggle();
		break;

	case MENU_INITIALIZE:
		UI_DrawToggle();
		break;

	case MENU_VERSION:
		gSettingMaxValues = 0;
		UI_DrawVersion();
		break;
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
		DISPLAY_Fill(0, 159, 1, 81, COLOR_BACKGROUND);
	}
	gColorForeground = COLOR_FOREGROUND;
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
		UI_DrawMain(false);
		BEEP_Play(440, 4, 80);
		break;

	default:
		break;
	}
}

void MENU_Next(uint8_t Key)
{
	UI_DrawSettingArrow(0);
	if (Key == KEY_UP) {
		gMenuIndex = (gMenuIndex + gSettingsCount - 1) % gSettingsCount;
	} else {
		gMenuIndex = (gMenuIndex + 1) % gSettingsCount;
	}
	DrawMenu(gMenuIndex);
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
			if (gMenuIndex == MENU_PERSONAL_ID
					|| gMenuIndex == MENU_CH_NAME
					|| gMenuIndex == MENU_CTCSS_DCS
					|| gMenuIndex == MENU_RX_CTCSS_DCS
					|| gMenuIndex == MENU_TX_CTCSS_DCS
					|| gMenuIndex == MENU_SCAN_RESUME
					|| gMenuIndex == MENU_SAVE_CH
					|| gMenuIndex == MENU_DELETE_CH) {
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

	case MENU_SCAN_RESUME:
		UI_DrawSettingScanResume(gSettingCurrentValue);
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

	case MENU_LIST_TO_SCAN:
		UI_DrawSettingScanlist(gSettingCurrentValue);
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
	case MENU_0_LONG:
	case MENU_1_LONG:
	case MENU_2_LONG:
	case MENU_3_LONG:
	case MENU_4_LONG:
	case MENU_5_LONG:
	case MENU_6_LONG:
	case MENU_7_LONG:
	case MENU_8_LONG:
	case MENU_9_LONG:
	case MENU_STAR_LONG:
	case MENU_HASH_LONG:
	case MENU_MENU_LONG:
	case MENU_EXIT_LONG:
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
	uint8_t ID = 0;

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
#ifdef ENABLE_NOAA
	case MENU_NOAA_MONITOR:  ID = 0x1C; break;
#endif
#ifdef ENABLE_FM_RADIO
	case MENU_FM_STANDBY:    ID = 0x1D; break;
#endif
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
	}

	if (ID) {
		AUDIO_PlaySampleOptional(ID);
	}
}

