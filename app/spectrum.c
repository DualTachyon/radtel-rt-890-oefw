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

#include "misc.h"
#include "app/spectrum.h"
#include "app/radio.h"
#include "driver/bk4819.h"
#include "driver/delay.h"
#include "driver/key.h"
#include "driver/pins.h"
#include "driver/speaker.h"
#include "helper/helper.h"
#include "helper/inputbox.h"
#include "radio/channels.h"
#include "radio/settings.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/main.h"

#ifdef UART_DEBUG
	#include "driver/uart.h"
	#include "external/printf/printf.h"
#endif

uint32_t CurrentFreq;
uint8_t CurrentFreqIndex;
uint32_t FreqCenter;
uint32_t FreqMin;
uint32_t FreqMax;
uint8_t CurrentModulation;
uint8_t CurrentFreqStepIndex;
uint32_t CurrentFreqStep;
uint32_t CurrentFreqChangeStep;
uint8_t CurrentStepCountIndex;
uint8_t CurrentStepCount;
uint16_t CurrentScanDelay;
uint16_t RssiValue[128] = {0};
uint16_t SquelchLevel;
uint8_t bExit;
uint8_t bRXMode;
uint8_t bResetSquelch;
uint8_t bRestartScan;
uint8_t bFilterEnabled;
uint8_t bNarrow;
uint16_t RssiLow;
uint16_t RssiHigh;
uint16_t BarScale;
uint8_t BarY;
uint8_t BarWidth;
uint16_t KeyHoldTimer = 0;
uint8_t bHold;
KEY_t Key;
KEY_t LastKey = KEY_NONE;
#ifdef ENABLE_SPECTRUM_PRESETS
FreqPreset CurrentBandInfo;
uint8_t CurrentBandIndex;
uint8_t bInBand = FALSE;
#endif

void ShiftShortStringRight(uint8_t Start, uint8_t End) {
	for (uint8_t i = End; i > Start; i--){
		gShortString[i] = gShortString[i-1];
	}
}

void DrawCurrentFreq(uint16_t Color) {

	gColorForeground = Color;
	Int2Ascii(CurrentFreq, 8);
	ShiftShortStringRight(2, 7);
	gShortString[3] = '.';
	UI_DrawString(30, 78, gShortString, 8);

	UI_DrawSmallString(98, 68, Mode[CurrentModulation], 2);
}

#ifdef ENABLE_SPECTRUM_PRESETS
void GetCurrentBand(void) {
	uint8_t PresetCount;

	PresetCount = sizeof(FreqPresets) / sizeof(FreqPresets[0]);

	for (uint8_t i = 0; i < PresetCount; i++) {
		if (FreqCenter >= FreqPresets[i].StartFreq && FreqCenter < FreqPresets[i].EndFreq) {
			bInBand = TRUE;
			CurrentBandInfo = FreqPresets[i];
			CurrentBandIndex = i;
		} else if (FreqCenter >= FreqPresets[i].EndFreq && FreqCenter >= FreqPresets[i+1].StartFreq) {
			bInBand = FALSE;
			CurrentBandIndex = i;
		}
	}
}
#endif

void DrawLabels(void) {

	gColorForeground = COLOR_FOREGROUND;

	Int2Ascii(FreqMin / 10, 7);
	ShiftShortStringRight(2, 7);
	gShortString[3] = '.';
	UI_DrawSmallString(2, 2, gShortString, 8);

	Int2Ascii(FreqMax / 10, 7);
	ShiftShortStringRight(2, 7);
	gShortString[3] = '.';
	UI_DrawSmallString(112, 2, gShortString, 8);

#ifdef ENABLE_SPECTRUM_PRESETS
	if (bInBand) {
		UI_DrawSmallString(2, 84, CurrentBandInfo.Name, 14);
	}
#endif

	gShortString[2] = ' ';
	Int2Ascii(CurrentStepCount, (CurrentStepCount < 100) ? 2 : 3);
	UI_DrawSmallString(2, 72, gShortString, 3);

	UI_DrawSmallString(2, 60, StepStrings[CurrentFreqStepIndex], 5);

	Int2Ascii(CurrentScanDelay, (CurrentScanDelay < 10) ? 1 : 2);
	if (CurrentScanDelay < 10) {
		gShortString[1] = gShortString[0];
		gShortString[0] = ' ';
	}
	UI_DrawSmallString(146, 72, gShortString, 2);

	UI_DrawSmallString(152, 60, (bFilterEnabled) ? "F" : "U", 1);

	UI_DrawSmallString(152, 48, (bNarrow) ? "N" : "W", 1);	

	UI_DrawSmallString(2, 14, (bHold) ? "H" : " ", 1);

	gColorForeground = COLOR_GREY;

	Int2Ascii(CurrentFreqChangeStep / 10, 5);
	ShiftShortStringRight(0, 5);
	gShortString[1] = '.';
	UI_DrawSmallString(64, 2, gShortString, 6);
}

void SetFreqMinMax(void) {
	CurrentFreqChangeStep = CurrentFreqStep * (CurrentStepCount >> 1);
	FreqMin = FreqCenter - CurrentFreqChangeStep;
	FreqMax = FreqCenter + CurrentFreqChangeStep;
#ifdef ENABLE_SPECTRUM_PRESETS
	GetCurrentBand();
#endif
	FREQUENCY_SelectBand(FreqCenter);
	BK4819_EnableFilter(bFilterEnabled);
	RssiValue[CurrentFreqIndex] = 0; // Force a rescan
}

void SetStepCount(void) {
	CurrentStepCount = 128 >> CurrentStepCountIndex;
	BarWidth = 128 / CurrentStepCount;
}

void IncrementStepIndex(void) {
	CurrentStepCountIndex = (CurrentStepCountIndex + 1) % STEPS_COUNT;
	SetStepCount();
	SetFreqMinMax();
	DrawLabels();
}

void IncrementFreqStepIndex(void) {
	CurrentFreqStepIndex = (CurrentFreqStepIndex + 1) % 10;
	CurrentFreqStep = FREQUENCY_GetStep(CurrentFreqStepIndex);
	SetFreqMinMax();
	DrawLabels();
}

void IncrementScanDelay(void) {
	CurrentScanDelay = (CurrentScanDelay + 2) % 12;
	DrawLabels();
}

void ChangeCenterFreq(uint8_t Up) {
	if (Up) {
		FreqCenter += CurrentFreqChangeStep;
	} else {
		FreqCenter -= CurrentFreqChangeStep;
	}
	SetFreqMinMax();
	DrawLabels();
}

void ChangeHoldFreq(uint8_t Up) {
	if (Up) {
		CurrentFreqIndex = (CurrentFreqIndex + 1) % CurrentStepCount;
	} else {
		CurrentFreqIndex = (CurrentFreqIndex + CurrentStepCount -1) % CurrentStepCount;
	}
	CurrentFreq = FreqMin + (CurrentFreqIndex * CurrentFreqStep);
}

void ChangeSquelchLevel(uint8_t Up) {
	if (Up) {
		SquelchLevel += 2;
	} else {
		SquelchLevel -= 2;
	}
}

#ifdef ENABLE_SPECTRUM_PRESETS
void ChangeBandPreset(uint8_t Up) {
	uint8_t PresetCount;

	PresetCount = sizeof(FreqPresets) / sizeof(FreqPresets[0]);

	if (Up) {
		CurrentBandIndex = (CurrentBandIndex + 1) % PresetCount;
	} else {
		CurrentBandIndex = (CurrentBandIndex + PresetCount - 1) % PresetCount;
	}

	bInBand = TRUE;

	CurrentStepCountIndex = FreqPresets[CurrentBandIndex].StepCountIndex;
	SetStepCount();

	CurrentFreqStepIndex = FreqPresets[CurrentBandIndex].StepSizeIndex;
	CurrentFreqStep = FREQUENCY_GetStep(CurrentFreqStepIndex);
	FreqCenter = FreqPresets[CurrentBandIndex].StartFreq + (CurrentFreqStep * (CurrentStepCount >> 1));
	SetFreqMinMax();

	CurrentModulation = FreqPresets[CurrentBandIndex].ModulationType;

	bNarrow = FreqPresets[CurrentBandIndex].bNarrow;
	BK4819_WriteRegister(0x43, (bNarrow) ? 0x4048 : 0x3028);

	bRestartScan = TRUE;
	bResetSquelch = TRUE;

	DrawCurrentFreq((bRXMode) ? COLOR_GREEN : COLOR_BLUE);
	DrawLabels();
}
#endif

void ToggleFilter(void) {
	bFilterEnabled ^= 1;
	BK4819_EnableFilter(bFilterEnabled);
	bResetSquelch = TRUE;
	bRestartScan = TRUE;
	DrawLabels();
}

void ToggleNarrowWide(void) {
	bNarrow ^= 1;
	BK4819_WriteRegister(0x43, (bNarrow) ? 0x4048 : 0x3028);
	DrawLabels();
}

void IncrementModulation(void) {
	CurrentModulation = (CurrentModulation + 1) % 3;
	DrawCurrentFreq((bRXMode) ? COLOR_GREEN : COLOR_BLUE);
}

uint16_t GetAdjustedLevel(uint16_t Level, uint16_t Low, uint16_t High, uint16_t Scale) {
	uint16_t Value = 0;

	//		Valid range 72-330, converted to 0-100, scaled to % based on Scale to fit on screen.
	if (Level < Low) {
		Level = Low;
	}

	Value = ((((((Level - Low) * 100) / (High - Low)) * 100) * Scale) / 10000);

	if (Value > Scale) {
		Value = Scale;
	} 

	return Value; 
}

void JumpToVFO(void) {
	gSettings.WorkMode = FALSE;

#ifdef UART_DEBUG
	Int2Ascii(gSettings.WorkMode, 1);
	UART_printf("gSettings.WorkMode: ");
	UART_printf(gShortString);
	UART_printf("     -----     ");
#endif

	SETTINGS_SaveGlobals();
	gVfoState[gSettings.CurrentVfo].bIsNarrow = bNarrow;
	// fixme: when previously in channel mode, the VFO name is overrided by the channel name
	CHANNELS_UpdateVFOFreq(CurrentFreq);

	bExit = TRUE;
}

void DrawSpectrum(uint16_t ActiveBarColor) {
	uint8_t BarLow;
	uint8_t BarHigh;
	uint16_t Power;
	uint16_t SquelchPower;
	uint8_t BarX;

	BarLow = RssiLow - 2;
	if ((RssiHigh - RssiLow) < 40) {
		BarHigh = RssiLow + 40;
	} else {
		BarHigh = RssiHigh + 5;
	}

	//Bars
	for (uint8_t i = 0; i < CurrentStepCount; i++) {
		BarX = 16 + (i * BarWidth);
		Power = GetAdjustedLevel(RssiValue[i], BarLow, BarHigh, BarScale);
		SquelchPower = GetAdjustedLevel(SquelchLevel, BarLow, BarHigh, BarScale);
		if (Power < SquelchPower) {
			DISPLAY_DrawRectangle1(BarX, BarY, Power, BarWidth, (i == CurrentFreqIndex) ? ActiveBarColor : COLOR_FOREGROUND);
			DISPLAY_DrawRectangle1(BarX, BarY + Power, SquelchPower - Power, BarWidth, COLOR_BACKGROUND);
			DISPLAY_DrawRectangle1(BarX, BarY + SquelchPower + 1, BarScale - SquelchPower, BarWidth, COLOR_BACKGROUND);
		} else { 
			DISPLAY_DrawRectangle1(BarX, BarY, SquelchPower, BarWidth, (i == CurrentFreqIndex) ? ActiveBarColor : COLOR_FOREGROUND);
			DISPLAY_DrawRectangle1(BarX, BarY + SquelchPower + 1, Power - SquelchPower, BarWidth, (i == CurrentFreqIndex) ? ActiveBarColor : COLOR_FOREGROUND);
			DISPLAY_DrawRectangle1(BarX, BarY + Power + 1, BarScale - Power, BarWidth, COLOR_BACKGROUND);
		} 
	}

	//Squelch Line
	Power = GetAdjustedLevel(SquelchLevel, BarLow, BarHigh, BarScale);
	DISPLAY_DrawRectangle1(16, BarY + Power, 1, 128, COLOR_RED);

	gColorForeground = ActiveBarColor;
	ConvertRssiToDbm(RssiValue[CurrentFreqIndex]);
	UI_DrawSmallString(118, 72, gShortString, 4);

	gColorForeground = COLOR_RED;
	ConvertRssiToDbm(SquelchLevel);
	UI_DrawSmallString(118, 60, gShortString, 4);
}

void StopSpectrum(void) {

	SCREEN_TurnOn();

	if (gSettings.WorkMode) {
		CHANNELS_LoadChannel(gSettings.VfoChNo[!gSettings.CurrentVfo], !gSettings.CurrentVfo);
		CHANNELS_LoadChannel(gSettings.VfoChNo[gSettings.CurrentVfo], gSettings.CurrentVfo);
	} else {
		CHANNELS_LoadChannel(gSettings.CurrentVfo ? 999 : 1000, !gSettings.CurrentVfo);
		CHANNELS_LoadChannel(gSettings.CurrentVfo ? 1000 : 999, gSettings.CurrentVfo);
	}

	RADIO_Tune(gSettings.CurrentVfo);
	UI_DrawMain(false);
}

void CheckKeys(void) {
	Key = KEY_GetButton();
	if (Key == LastKey && Key != KEY_NONE) {
		if (bRXMode) {
			KeyHoldTimer += 10;
		} else {
			KeyHoldTimer++;
		}
	}
	if (Key != LastKey || KeyHoldTimer >= 50) {
		KeyHoldTimer = 0;
		switch (Key) {
			case KEY_NONE:
				break;
			case KEY_EXIT:
				bExit = TRUE;
				return;
			case KEY_MENU:
				JumpToVFO();
				return;
			case KEY_UP:
				if (!bHold) {
					ChangeCenterFreq(TRUE);
				} else {
					ChangeHoldFreq(TRUE);
				}
				break;
			case KEY_DOWN:
				if (!bHold) {
					ChangeCenterFreq(FALSE);
				} else {
					ChangeHoldFreq(FALSE);
				}
				break;
			case KEY_1:
				IncrementStepIndex();
				break;
			case KEY_2:
#ifdef ENABLE_SPECTRUM_PRESETS
				ChangeBandPreset(TRUE);
#endif
				break;
			case KEY_3:
				IncrementModulation();
				break;
			case KEY_4:
				IncrementFreqStepIndex();
				break;
			case KEY_5:
#ifdef ENABLE_SPECTRUM_PRESETS
				ChangeBandPreset(FALSE);
#endif
				break;
			case KEY_6:
				ChangeSquelchLevel(TRUE);
				break;
			case KEY_7:
				bHold ^= 1;
				DrawLabels();
				break;
			case KEY_8:
				break;
			case KEY_9:
				ChangeSquelchLevel(FALSE);
				break;
			case KEY_0:
				ToggleFilter();
				break;
			case KEY_HASH:
				ToggleNarrowWide();
				break;
			case KEY_STAR:
				IncrementScanDelay();
				break;
			default:
				break;
		}
		LastKey = Key;
	}
}

void Spectrum_StartAudio(void) {
	gReceivingAudio = true;

	gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
	gRadioMode = RADIO_MODE_RX;
	OpenAudio(bNarrow, CurrentModulation);
	if (CurrentModulation == 0) {
		BK4819_WriteRegister(0x4D, 0xA080);
		BK4819_WriteRegister(0x4E, 0x6F7C);
	}

	if (CurrentModulation > 0) {
		// AM, SSB
		BK4819_EnableScramble(false);
		BK4819_EnableCompander(false);
		// Set bit 4 of register 73 (Auto Frequency Control Disable)
		uint16_t reg_73 = BK4819_ReadRegister(0x73);
		BK4819_WriteRegister(0x73, reg_73 | 0x10U);
		if (CurrentModulation > 1) { // if SSB
			BK4819_WriteRegister(0x43, 0b0010000001011000); // Filter 6.25KHz
			BK4819_WriteRegister(0x37, 0b0001011000001111);
			BK4819_WriteRegister(0x3D, 0b0010101101000101);
			BK4819_WriteRegister(0x48, 0b0000001110101000);
		}
	} else {
		// FM
		BK4819_EnableScramble(false);
		BK4819_EnableCompander(true);
		uint16_t reg_73 = BK4819_ReadRegister(0x73);
		BK4819_WriteRegister(0x73, reg_73 & ~0x10U);
		BK4819_SetAFResponseCoefficients(false, true, gCalibration.RX_3000Hz_Coefficient);
	}
	SPEAKER_TurnOn(SPEAKER_OWNER_RX);
}

void RunRX(void) {
	bRXMode = TRUE;
	Spectrum_StartAudio();

	while(RssiValue[CurrentFreqIndex] > SquelchLevel) {
		RssiValue[CurrentFreqIndex] = BK4819_GetRSSI();
		CheckKeys();
		if (bExit){
			RADIO_EndAudio();
			return;
		}
		DrawCurrentFreq(COLOR_GREEN);
		DrawSpectrum(COLOR_GREEN);
		DELAY_WaitMS(5);
	}

	RADIO_EndAudio();
	bRXMode = FALSE;
}

void Spectrum_Loop(void) {
	uint32_t FreqToCheck;
	CurrentFreqIndex = 0;
	CurrentFreq = FreqMin;
	bResetSquelch = TRUE;
	bRestartScan = FALSE;

	UI_DrawStatusIcon(139, ICON_BATTERY, true, COLOR_FOREGROUND);
	UI_DrawBattery(false);

	DrawLabels();

	while (1) {
		FreqToCheck = FreqMin;
		bRestartScan = TRUE;

		for (uint8_t i = 0; i < CurrentStepCount; i++) {

			if (bRestartScan) {
				bRestartScan = FALSE;
				RssiLow = 330;
				RssiHigh = 72;
				i = 0;
			}

			BK4819_set_rf_frequency(FreqToCheck, TRUE);

			DELAY_WaitMS(CurrentScanDelay);

			RssiValue[i] = BK4819_GetRSSI();

			if (RssiValue[i] < RssiLow) {
				RssiLow = RssiValue[i];
			} else if (RssiValue[i] > RssiHigh) {
				RssiHigh = RssiValue[i];
			}

			if (RssiValue[i] > RssiValue[CurrentFreqIndex] && !bHold) {
				CurrentFreqIndex = i;
				CurrentFreq = FreqToCheck;
			}

			FreqToCheck += CurrentFreqStep;

			CheckKeys();
			if (bExit){
				return;
			}
		}

		if (bResetSquelch) {
			bResetSquelch = FALSE;
			SquelchLevel = RssiHigh + 5;
		}

		if (RssiValue[CurrentFreqIndex] > SquelchLevel) {
			BK4819_set_rf_frequency(CurrentFreq, TRUE);
			DELAY_WaitMS(CurrentScanDelay);
			RunRX();
		}

		DrawCurrentFreq(COLOR_BLUE);
		DrawSpectrum(COLOR_BLUE);
	}
}

void APP_Spectrum(void) {
	RADIO_EndAudio();  // Just in case audio is open when spectrum starts

	bExit = FALSE;
	bRXMode = FALSE;

	FreqCenter = gVfoState[gSettings.CurrentVfo].RX.Frequency;
	bNarrow = gVfoState[gSettings.CurrentVfo].bIsNarrow;
	CurrentModulation = gVfoState[gSettings.CurrentVfo].gModulationType;
	CurrentFreqStepIndex = gSettings.FrequencyStep;
	CurrentFreqStep = FREQUENCY_GetStep(CurrentFreqStepIndex);
	//Defaults
	CurrentStepCountIndex = STEPS_64;
	CurrentScanDelay = 4;
	bFilterEnabled = TRUE;
	SquelchLevel = 0;
	BarScale = 40;
	BarY = 15;
	bHold = 0;

	SetStepCount();
	SetFreqMinMax(); 

	for (int i = 0; i < 8; i++) {
		gShortString[i] = ' ';
	}
	
	DISPLAY_Fill(0, 159, 1, 96, COLOR_BACKGROUND);
	Spectrum_Loop();

	StopSpectrum();
}
