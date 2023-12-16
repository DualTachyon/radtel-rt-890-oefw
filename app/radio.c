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
#ifdef ENABLE_FM_RADIO
	#include "app/fm.h"
#endif
#include "app/radio.h"
#include "driver/beep.h"
#include "driver/bk4819.h"
#include "driver/delay.h"
#include "driver/key.h"
#include "driver/pins.h"
#include "driver/speaker.h"
#include "driver/uart.h"
#include "helper/dtmf.h"
#include "helper/helper.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/data.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/alarm.h"
#ifdef ENABLE_NOAA
	#include "task/noaa.h"
	#include "ui/noaa.h"
#endif
#include "task/ptt.h"
#include "task/am-fix.h"
#include "task/scanner.h"
#include "task/screen.h"
#include "ui/boot.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "ui/vfo.h"

uint8_t gCurrentVfo;
ChannelInfo_t *gMainVfo;
ChannelInfo_t gVfoState[3];
FrequencyInfo_t gVfoInfo[2];
uint16_t gCurrentRssi[2];

bool gNoaaMode;
uint16_t gCode;

static void EnableTxAmp(bool bEnable)
{
	if (!bEnable) {
		gpio_bits_reset(GPIOB, BOARD_GPIOB_TX_BIAS_LDO);
	} else {
		gpio_bits_set(GPIOB, BOARD_GPIOB_TX_BIAS_LDO);
		if (gUseUhfFilter) {
			gpio_bits_set(GPIOB, BOARD_GPIOB_TX_AMP_SEL);
		} else {
			gpio_bits_reset(GPIOB, BOARD_GPIOB_TX_AMP_SEL);
		}
	}
}

static void TuneCurrentVfo(void)
{
	if (gSettings.RepeaterMode == 2) {
		// Frequency reversal
		gVfoInfo[gCurrentVfo]  = gMainVfo->TX;
		gVfoInfo[!gCurrentVfo] = gVfoState[!gCurrentVfo].TX;
	} else if (gSettings.RepeaterMode == 1) {
		// Talk around
		gVfoInfo[gCurrentVfo].Frequency  = gMainVfo->RX.Frequency;
		gVfoInfo[gCurrentVfo].CodeType   = CODE_TYPE_OFF;
		gVfoInfo[gCurrentVfo].Code       = 0;

		gVfoInfo[!gCurrentVfo].Frequency = gVfoState[!gCurrentVfo].RX.Frequency;
		gVfoInfo[!gCurrentVfo].CodeType  = CODE_TYPE_OFF;
		gVfoInfo[!gCurrentVfo].Code      = 0;
	} else {
		// OFF
		gVfoInfo[gCurrentVfo]  = gMainVfo->RX;
		gVfoInfo[!gCurrentVfo] = gVfoState[!gCurrentVfo].RX;
	}

	if (!gScannerMode) {
		gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
	}
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);

	gRadioMode = RADIO_MODE_QUIET;
	EnableTxAmp(false);
	BK4819_SetFrequency(gVfoInfo[gCurrentVfo].Frequency);
	gCode = gVfoInfo[gCurrentVfo].Code;
	if (gMainVfo->bMuteEnabled) {
		CSS_SetCustomCode(gMainVfo->bIs24Bit, gMainVfo->Golay, gMainVfo->bIsNarrow);
	} else {
		CSS_SetStandardCode(gVfoInfo[gCurrentVfo].CodeType, gCode, gMainVfo->Encrypt, gMainVfo->bIsNarrow);
	}
	BK4819_SetSquelchGlitch(gMainVfo->bIsNarrow);
	BK4819_SetSquelchNoise(gMainVfo->bIsNarrow);
	BK4819_SetSquelchRSSI(gMainVfo->bIsNarrow);
	BK4819_EnableRX();
	BK4819_SetFilterBandwidth(gMainVfo->bIsNarrow);
	BK4819_EnableFilter(true);
}

static bool TuneTX(bool bUseMic)
{
	if (gSettings.RepeaterMode == 2) {
		gVfoInfo[gCurrentVfo] = gMainVfo->RX;
	} else if (gSettings.RepeaterMode == 1) {
		gVfoInfo[gCurrentVfo].Frequency = gMainVfo->RX.Frequency;
		gVfoInfo[gCurrentVfo].CodeType  = gMainVfo->TX.CodeType;
		gVfoInfo[gCurrentVfo].Code      = gMainVfo->TX.Code;
	} else {
		gVfoInfo[gCurrentVfo] = gMainVfo->TX;
	}

	gCode = gVfoInfo[gCurrentVfo].Code;
	BK4819_SetFrequency(gVfoInfo[gCurrentVfo].Frequency);
	if (gSettings.BandInfo[gCurrentFrequencyBand] == BAND_136MHz && gVfoInfo[gCurrentVfo].Frequency >= 13600000) {
		BK4819_EnableFilter(false);
		if (gMainVfo->bMuteEnabled) {
			CSS_SetCustomCode(gMainVfo->bIs24Bit, gMainVfo->Golay, gMainVfo->bIsNarrow);
			gTxCodeType = CODE_TYPE_DCS_N;
		} else {
			CSS_SetStandardCode(gVfoInfo[gCurrentVfo].CodeType, gVfoInfo[gCurrentVfo].Code, gMainVfo->Encrypt, gMainVfo->bIsNarrow);
			gTxCodeType = gMainVfo->TX.CodeType;
		}
		gRadioMode = RADIO_MODE_TX;
		BK4819_EnableRfTxDeviation();
		BK4819_SetMicSensitivityTuning();
		BK4819_EnableTX(bUseMic);
		BK4819_EnableScramble(gMainVfo->Scramble);
		if (gMainVfo->Scramble == 0) {
			BK4819_SetAFResponseCoefficients(true, true, gCalibration.TX_3000Hz_Coefficient);
		} else {
			BK4819_SetAFResponseCoefficients(true, true, 5);
		}
		EnableTxAmp(true);
		BK4819_SetupPowerAmplifier(gMainVfo->bIsLowPower ? gTxPowerLevelLow : gTxPowerLevelHigh);

		return true;
	} else {
		TuneCurrentVfo();

		return false;
	}
}

static void SpecialRxTxLoop(void)
{
	static bool bFlag;

	while (1) {
		while (1) {
			while (gSpecialTimer) {
			}
			gSpecialTimer = 30000;
			if (!bFlag) {
				break;
			}
			bFlag = false;
			RADIO_EndTX();
			RADIO_StartRX();
		}
		bFlag = true;
		RADIO_EndRX();
		RADIO_StartTX(true);
	}
}

static void TuneNOAA(void)
{
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);

	gRadioMode = RADIO_MODE_QUIET;
	EnableTxAmp(false);
	BK4819_SetFrequency(gMainVfo->RX.Frequency);
	if (!gNoaaMode) {
		BK4819_WriteRegister(0x51, 0x0000);
	} else {
		BK4819_WriteRegister(0x51, 0x9400 | gFrequencyBandInfo.CtcssTxGainWide);
		BK4819_WriteRegister(0x07, 0x152C);
	}
	BK4819_SetSquelchGlitch(false);
	BK4819_SetSquelchNoise(false);
	BK4819_SetSquelchRSSI(false);
	BK4819_EnableScramble(0);
	BK4819_EnableCompander(false);
	BK4819_EnableRX();
	BK4819_SetFilterBandwidth(false);
	BK4819_EnableFilter(true);
}

// Public

void RADIO_Init(void)
{
	if (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1)) {
		if (KEY_GetButton() == KEY_1) {
			UART_Init(19200);
			do {
				while (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1)) {
				}
			} while (KEY_GetButton() == KEY_1);
			KEY_Side1Counter = 0;
			KEY_KeyCounter = 0;
		}
	}

	SETTINGS_LoadCalibration();
	SETTINGS_LoadSettings();

	BK4819_Init();
	#ifdef ENABLE_AM_FIX
	AM_fix_init();
	#endif

	if (gSettings.DtmfState != DTMF_STATE_KILLED) {
		UI_DrawBoot();
	}

	CHANNELS_CheckFreeChannels();

	if (gSettings.WorkMode) {
		CHANNELS_LoadWorkMode();
	} else {
		CHANNELS_LoadVfoMode();
	}

	gCurrentVfo = gSettings.CurrentVfo;

	RADIO_Tune(gCurrentVfo);

	if (gSettings.DtmfState != DTMF_STATE_KILLED) {
		UI_DrawMain(false);
		BK4819_EnableVox(gSettings.Vox);
		if (!gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_KEY_PTT)) {
			if (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1)) {
				if (!gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_SIDE2)) {
					SpecialRxTxLoop();
				}
			}
		}
	}
}

void RADIO_Tune(uint8_t Vfo)
{
	gMainVfo = &gVfoState[Vfo];
	if (Vfo != 2) {
		gNoaaMode = false;
		gCurrentVfo = Vfo;
		TuneCurrentVfo();
	} else {
		TuneNOAA();
	}
}

void RADIO_StartRX(void)
{
#ifdef ENABLE_FM_RADIO
	FM_Disable(FM_MODE_STANDBY);
#endif
	BK4819_StartAudio();
	if (!gFrequencyDetectMode) {
		DTMF_ClearString();
		DTMF_FSK_InitReceive(0);
		VOX_Timer = 0;
		Task_UpdateScreen();
		SCREEN_TurnOn();
		if (gScannerMode && gExtendedSettings.ScanResume == 2) {	// Time Operated
			SCANNER_Countdown = 5000;
		}
		if (gScreenMode == SCREEN_MAIN && !gDTMF_InputMode) {
			if (gSettings.DualDisplay == 0 && gSettings.CurrentVfo != gCurrentVfo) {
				const uint8_t Y = gCurrentVfo * 41;

				DISPLAY_Fill(1, 158, 1 + Y, 40 + Y, COLOR_BACKGROUND);
				DISPLAY_Fill(1, 158, 1 + ((!gCurrentVfo) * 41), 40 + ((!gCurrentVfo) * 41), COLOR_BACKGROUND);

				UI_DrawVoltage(!gCurrentVfo);
			}
			UI_DrawVfo(gCurrentVfo);
			UI_DrawMainBitmap(false, gSettings.CurrentVfo);
			UI_DrawRX(gCurrentVfo);
		}
		if (gMainVfo->BCL == BUSY_LOCK_CSS) {
			PTT_SetLock(PTT_LOCK_BUSY);
		}
		if (gSettings.TxPriority && gSettings.CurrentVfo != gCurrentVfo) {
			gSettings.CurrentVfo ^= 1;
			SETTINGS_SaveGlobals();
		}
	}
}

void RADIO_EndRX(void)
{
	gTailToneCounter = 0;
	BK4819_SetAF(BK4819_AF_MUTE);
	SPEAKER_TurnOff(SPEAKER_OWNER_RX);
	BK4819_EnableFFSK1200(false);
	BK4819_ResetFSK();
	DTMF_Disable();
	if (gScannerMode) {
		switch (gExtendedSettings.ScanResume) {
			case 1:		// Carrier Operated
				SCANNER_Countdown = 3000;
			case 2:		// Time Operated
				gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
				break;
			case 3:		// No resume
				SETTINGS_SaveState();
				break;
		}
	}
	TuneCurrentVfo();
	if (!gFrequencyDetectMode) {
		if (gScreenMode == SCREEN_MAIN && !gDTMF_InputMode) {
			if (!gFskDataReceived && !gDataDisplay) {
				UI_DrawSomething();
			} else {
				VOX_Timer = 5000;
				gRedrawScreen = true;
			}
		} else {
			if (gDTMF_InputMode) {
				if (gFskDataReceived || gDataDisplay) {
					DELAY_WaitMS(500);
				}
				UI_DrawDTMF();
			}
			DATA_WasDataReceived();
			gDataDisplay = false;
		}
		gRxLinkCounter = 0;
		gNoToneCounter = 0;
		gIdleTimer = 10000;
		PTT_ClearLock(PTT_LOCK_INCOMING);
		PTT_ClearLock(PTT_LOCK_BUSY);
#ifdef ENABLE_FM_RADIO
		if (!gRedrawScreen) {
			FM_Resume();
		}
#endif
		gIncomingTimer = 1;
	} else {
		gSignalFound = true;
		gDetectorTimer = 500;
	}
}

void RADIO_StartAudio(void)
{
	gReceivingAudio = true;
	SCREEN_TurnOn();
	BK4819_StartAudio();
}

void RADIO_EndAudio(void)
{
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
	gReceivingAudio = false;
	RADIO_Tune(2);
	SPEAKER_TurnOff(SPEAKER_OWNER_RX);
	gRxLinkCounter = 0;
	gNoToneCounter = 0;
	gIncomingTimer = 1;
#ifdef ENABLE_NOAA
	NOAA_NextChannelCountdown = 3000;
#endif
}

void RADIO_Sleep(void)
{
	EnableTxAmp(false);
	BK4819_EnableFilter(false);
	BK4819_WriteRegister(0x30, 0x0000);
	BK4819_WriteRegister(0x37, 0x1D00);
	gSaveMode = true;
}

static void PlayRogerBeep(uint8_t Mode)
{
	BEEP_Enable();

	if (Mode == 1) {
		BK4819_SetToneFrequency(false, 1000);
		SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);
		DELAY_WaitMS(25);
		BK4819_SetToneFrequency(false, 0);
		DELAY_WaitMS(25);
		BK4819_SetToneFrequency(false, 1000);
		DELAY_WaitMS(25);
		BK4819_SetToneFrequency(false, 0);
		DELAY_WaitMS(25);
		BK4819_SetToneFrequency(false, 1000);
		DELAY_WaitMS(25);
	} else {
		BK4819_SetToneFrequency(false, 590);
		SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM);
		DELAY_WaitMS(60);
		BK4819_SetToneFrequency(false, 660);
		DELAY_WaitMS(60);
		BK4819_SetToneFrequency(false, 730);
		DELAY_WaitMS(60);
	}

	BEEP_Disable();
}

void RADIO_Retune(void)
{
	if (gSettings.WorkMode) {
		CHANNELS_LoadChannel(gSettings.VfoChNo[gSettings.CurrentVfo], gSettings.CurrentVfo);
	} else {
		CHANNELS_LoadChannel(gSettings.CurrentVfo ? 1000 : 999, gSettings.CurrentVfo);
	}
	RADIO_Tune(gSettings.CurrentVfo);
}

#ifdef ENABLE_NOAA
void RADIO_NoaaRetune(void)
{
	gReceptionMode = false;
	gReceivingAudio = false;
	if (gRadioMode == RADIO_MODE_RX) {
		RADIO_EndAudio();
	}
	RADIO_Retune();
	RADIO_Tune(gSettings.CurrentVfo);
	UI_DrawMain(true);
}

void RADIO_NoaaTune(void)
{
	gReceptionMode = true;
	UI_DrawSky();
	UI_DrawNOAA(gNOAA_ChannelNow);
	CHANNELS_SetNoaaChannel(gNOAA_ChannelNow);
	RADIO_Tune(2);
	NOAA_NextChannelCountdown = 3000;
	gNoaaMode = false;
}
#endif

void VFO_ClearCss(void)
{
	gVfoState[gSettings.CurrentVfo].RX.CodeType = CODE_TYPE_OFF;
	gVfoState[gSettings.CurrentVfo].TX.CodeType = CODE_TYPE_OFF;
	gVfoState[gSettings.CurrentVfo].RX.Code = 0;
	gVfoState[gSettings.CurrentVfo].TX.Code = 0;
	gVfoState[gSettings.CurrentVfo].Encrypt = 0;
}

void VFO_ClearMute(void)
{
	gVfoState[gSettings.CurrentVfo].bIs24Bit = 0;
	gVfoState[gSettings.CurrentVfo].bMuteEnabled = 0;
	gVfoState[gSettings.CurrentVfo].Golay = 0;
}

void RADIO_SaveCurrentVfo(void)
{
	if (gSettings.WorkMode) {
		CHANNELS_SaveChannel(gSettings.VfoChNo[gSettings.CurrentVfo], &gVfoState[gSettings.CurrentVfo]);
	} else {
		CHANNELS_SaveChannel(gSettings.CurrentVfo ? 1000 : 999, &gVfoState[gSettings.CurrentVfo]);
	}
	RADIO_Tune(gSettings.CurrentVfo);
}

void RADIO_StartTX(bool bUseMic)
{
	if (gRadioMode == RADIO_MODE_RX) {
		RADIO_EndRX();
	}
	RADIO_Tune(gSettings.CurrentVfo);
	RADIO_DisableSaveMode();
	if (!TuneTX(bUseMic)) {
		if (gEnableLocalAlarm) {
			ALARM_Stop();
			return;
		}
	} else {
		gpio_bits_set(GPIOA, BOARD_GPIOA_LED_RED);
		VOX_Timer = 0;
		if (gDTMF_InputMode) {
			UI_DrawMain(true);
		}
		Task_UpdateScreen();
		UI_DrawMainBitmap(false, gSettings.CurrentVfo);
		UI_DrawVfo(gSettings.CurrentVfo);
		if (gSettings.RogerBeep == 3) {
			// Play a tone for 300ms to make sure squelch opens on remote end before FSK ID
			BEEP_Enable();
			BK4819_SetToneFrequency(false, 610);
			SPEAKER_TurnOn(SPEAKER_OWNER_SYSTEM); // The local user should also hear it, we want to make sure we don't cut their voice
			DELAY_WaitMS(150);
			BEEP_Disable();
			BK4819_EnableFFSK1200(true);
			DATA_SendDeviceName();
			BK4819_EnableFFSK1200(false);
			BK4819_ResetFSK();
		}
		if (gDTMF_Settings.Mode == DTMF_MODE_TX_START || gDTMF_Settings.Mode == DTMF_MODE_TX_START_END || gDTMF_InputMode) {
			DELAY_WaitMS(gDTMF_Settings.Delay * 100);
			if (!gDTMF_InputMode) {
				DTMF_PlayContact(&gDTMF_Contacts[gDTMF_Settings.Select]);
			} else {
				DTMF_PlayContact(&gDTMF_Input);
				DTMF_ResetString();
				gDTMF_InputMode = false;
			}
		}
	}
}

void RADIO_EndTX(void)
{
	if (gDTMF_Settings.Mode == DTMF_MODE_TX_END || gDTMF_Settings.Mode == DTMF_MODE_TX_START_END) {
		DTMF_PlayContact(&gDTMF_Contacts[gDTMF_Settings.Select]);
	}
	// if (gSettings.RogerBeep == 3) {
	// 	BK4819_EnableFFSK1200(true);
	// 	DATA_SendDeviceName();
	// 	BK4819_EnableFFSK1200(false);
	// 	BK4819_ResetFSK();
	// } else
	if (gSettings.RogerBeep && gSettings.RogerBeep != 3) {
		PlayRogerBeep(gSettings.RogerBeep);
	}
	BK4819_GenTail(gMainVfo->bIsNarrow);
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);
	BK4819_SetupPowerAmplifier(0);
	TuneCurrentVfo();
	UI_DrawSomething();
	gBatteryTimer = 3000;
	gIdleTimer = 10000;
}

void RADIO_CancelMode(void)
{
	if (gRadioMode == RADIO_MODE_INCOMING) {
		gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
		gRadioMode = RADIO_MODE_QUIET;
	}
	if (gRadioMode == RADIO_MODE_QUIET) {
		return;
	}
	if (gRadioMode == RADIO_MODE_TX) {
		RADIO_EndTX();
	} else if (gRadioMode == RADIO_MODE_RX) {
		gMonitorMode = false;
		RADIO_EndRX();
	}
	VOX_Timer = 0;
	Task_UpdateScreen();
}

void RADIO_DisableSaveMode(void)
{
	if (gSaveMode) {
		BK4819_EnableRX();
		BK4819_EnableFilter(true);
		gSaveMode = false;
		DELAY_WaitMS(10);
	}
}

void RADIO_DrawFmMode(void)
{
	gInputBoxWriteIndex = 0;
	INPUTBOX_Pad(0, 10);
	UI_DrawFMFrequency(gSettings.FmFrequency);
}

void RADIO_DrawWorkMode(void)
{
	gInputBoxWriteIndex = 0;
	INPUTBOX_Pad(0, '-');
	UI_DrawVfo(gSettings.CurrentVfo);
}

void RADIO_DrawFrequencyMode(void)
{
	RADIO_CancelMode();
	INPUTBOX_Pad(0, 10);
	gInputBoxWriteIndex = 0;
	if (gFrequencyReverse) {
		UI_DrawFrequency(gVfoState[gSettings.CurrentVfo].TX.Frequency, gSettings.CurrentVfo, COLOR_RED);
	} else {
		UI_DrawFrequency(gVfoState[gSettings.CurrentVfo].RX.Frequency, gSettings.CurrentVfo, COLOR_FOREGROUND);
	}
}

