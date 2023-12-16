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
#include "app/radio.h"
#include "driver/audio.h"
#include "driver/beep.h"
#include "driver/bk4819.h"
#include "driver/delay.h"
#include "driver/key.h"
#include "driver/pins.h"
#include "driver/speaker.h"
#include "helper/helper.h"
#include "misc.h"
#include "radio/detector.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/incoming.h"
#include "task/ptt.h"
#include "task/rssi.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/main.h"

//

static uint32_t RoundToNearest50(uint32_t Frequency)
{
	uint32_t Value;

	Value = (Frequency / 50U) * 50U;
	if ((Frequency % 50U) > 24U) {
		Value += 50U;
	}

	return Value;
}

static bool CheckScanResult(void)
{
	uint32_t Frequency;
	uint16_t Timeout;
	uint16_t Result;

	Timeout = 1000;
	while (Timeout) {
		DELAY_WaitMS(1);
		Result = BK4819_ReadRegister(0x0D);
		if ((Result & 0x8000U) == 0) {
			break;
		}
		Timeout--;
	}

	if (!Timeout) {
		return false;
	}

	Frequency = (Result & 0x07FF) << 16;
	Frequency |= BK4819_ReadRegister(0x0E);

	if (!gSettings.bUseVHF || Frequency <= 24000000) {
		if (!gSettings.bUseVHF && Frequency < 24000000) {
			Frequency *= 2U;
		}
	} else {
		Frequency /= 2U;
	}
	FREQUENCY_SelectBand(Frequency);
	Frequency = RoundToNearest50(32808U + (Frequency - gFrequencyBandInfo.FrequencyOffset));
	gVfoState[gSettings.CurrentVfo].RX.Frequency = Frequency;
	gVfoState[gSettings.CurrentVfo].TX.Frequency = Frequency;
	UI_DrawScanFrequency(Frequency);

	return true;
}

static void UpdateBand(bool bToggleBand)
{
	if (bToggleBand) {
		gSettings.bUseVHF ^= 1;
	}
	gUseUhfFilter = !gSettings.bUseVHF;
	BK4819_EnableFilter(true);
	if (bToggleBand) {
		SETTINGS_SaveGlobals();
	}
	UI_DrawBand();
	BEEP_Play(740, 2, 100);
}

static void CtdcScan(void)
{
	if (gSettings.WorkMode) {
		CHANNELS_LoadChannel(gSettings.VfoChNo[gSettings.CurrentVfo], gSettings.CurrentVfo);
	} else {
		CHANNELS_LoadChannel(gSettings.CurrentVfo ? 1000 : 999, gSettings.CurrentVfo);
	}
	RADIO_Tune(gSettings.CurrentVfo);
	UI_DrawCtdcScan();
	UI_DrawScanFrequency(gVfoState[gSettings.CurrentVfo].RX.Frequency);
	BEEP_Play(740, 2, 100);
}

static void StopDetect(void)
{
	gFrequencyDetectMode = false;
	SCREEN_TurnOn();

	if (gSettings.WorkMode) {
		CHANNELS_LoadChannel(gSettings.VfoChNo[!gSettings.CurrentVfo], !gSettings.CurrentVfo);
		CHANNELS_LoadChannel(gSettings.VfoChNo[gSettings.CurrentVfo], gSettings.CurrentVfo);
	} else {
		CHANNELS_LoadChannel(gSettings.CurrentVfo ? 999 : 1000, !gSettings.CurrentVfo);
		CHANNELS_LoadChannel(gSettings.CurrentVfo ? 1000 : 999, gSettings.CurrentVfo);
	}

	RADIO_Tune(gSettings.CurrentVfo);
	UI_DrawMain(true);
}

static bool GetDcsCode(uint32_t Golay)
{
	uint8_t i;
	uint8_t j;

	for (i = 0; i < 23; i++) {
		for (j = 0; j < 106; j++) {
			uint16_t Code = DCS_GetOption(j);

			if (CSS_CalculateGolay(Code + 0x800) == Golay) {
				gVfoState[gSettings.CurrentVfo].RX.CodeType = CODE_TYPE_DCS_N;
				gVfoState[gSettings.CurrentVfo].RX.Code = Code;
				gVfoState[gSettings.CurrentVfo].TX.CodeType = CODE_TYPE_DCS_N;
				gVfoState[gSettings.CurrentVfo].TX.Code = Code;
				UI_DrawDcsCodeN(Code);

				return true;
			}
		}
		if (Golay & 0x400000U) {
			Golay = (Golay << 1) | 1;
		} else {
			Golay <<= 1;
		}
		Golay &= 0x7FFFFFU;
	}

	return false;
}

static void MuteCtcssScan(void)
{
	uint16_t Timeout = 1000;

	while (1) {
		uint32_t Code;

		if (Timeout == 0) {
			VFO_ClearMute();
			VFO_ClearCss();
			UI_DrawNone();
			break;
		}
		DELAY_WaitMS(1);
		Code = BK4819_ReadRegister(0x69);
		Timeout--;
		if ((Code & 0x8000U) == 0) {
			if (Code & 0x4000U) {
				gVfoState[gSettings.CurrentVfo].bIs24Bit = 1;
			} else {
				gVfoState[gSettings.CurrentVfo].bIs24Bit = 0;
			}

			// Double check the assembly, it didn't make sense!
			Code = (Code & 0xFFF) << 16;
			Code |= BK4819_ReadRegister(0x6A) & 0xFFF;
			gVfoState[gSettings.CurrentVfo].Golay = Code;

			if ((Code & 0XFFFFFF) != 0x555555 && (Code & 0xFFFFFF) != 0xAAAAAA) {
				if (Code != 0x800000 && (Code & 0xFFFFFF) != 0xFFFFFF && (Code & 0xFFFFFF) != 0x7FFFFF) {
					if (!gVfoState[gSettings.CurrentVfo].bIs24Bit) {
						Code &= 0x7FFFFF;
						gVfoState[gSettings.CurrentVfo].Golay = Code;
						if (GetDcsCode(Code)) {
							VFO_ClearMute();
							break;
						}
					}
					gVfoState[gSettings.CurrentVfo].bMuteEnabled = 1;
					UI_DrawMuteInfo(gVfoState[gSettings.CurrentVfo].bIs24Bit, gVfoState[gSettings.CurrentVfo].Golay);
				} else {
					VFO_ClearMute();
					VFO_ClearCss();
					UI_DrawNone();
				}
				break;
			}
		}
		VFO_ClearMute();
		Code = BK4819_ReadRegister(0x68);
		if ((Code & 0x8000U) == 0) {
			Code = (((Code & 0xFFFU) * 200U) / 412U) + 1U;
			if (Code > 500) {
				Code &= 0xFFFU;
				gVfoState[gSettings.CurrentVfo].RX.Code = Code;
				gVfoState[gSettings.CurrentVfo].TX.Code = Code;
				gVfoState[gSettings.CurrentVfo].RX.CodeType = CODE_TYPE_CTCSS;
				gVfoState[gSettings.CurrentVfo].TX.CodeType = CODE_TYPE_CTCSS;
				UI_DrawCtcssCode(Code);
				break;
			}
		}
	}
}

static void DETECTOR_Loop(void)
{
	bool bCtdcScan;
	bool bScan;
	KEY_t Key;

	bCtdcScan = false;
	bScan = false;

	while (1) {
		DISPLAY_Fill(80, 159, 8, 40, COLOR_BACKGROUND);
		gRxLinkCounter = 0;
		do {
			if (gRxLinkCounter == 0 && !bCtdcScan) {
				BK4819_StartFrequencyScan();
				bScan = CheckScanResult();
				BK4819_StopFrequencyScan();
				if (bScan) {
					RADIO_Tune(gSettings.CurrentVfo);
				}
			}
			DELAY_WaitMS(5);
			if (gRxLinkCounter++ > 20) {
				gRxLinkCounter = 0;
			}
			Key = KEY_GetButton();
			if (Key == KEY_NONE) {
				if (KEY_KeyCounter > 10) {
					KEY_KeyCounter = 0;
					if (KEY_CurrentKey == KEY_HASH && !bCtdcScan) {
						KEY_CurrentKey = KEY_NONE;
						UpdateBand(true);
					} else if (KEY_CurrentKey == KEY_STAR) {
						KEY_CurrentKey = KEY_NONE;
						if (!bCtdcScan) {
							CtdcScan();
							bCtdcScan = true;
						} else {
							UpdateBand(false);
							bCtdcScan = false;
						}
					}
				}
				KEY_KeyCounter = 0;
				KEY_CurrentKey = KEY_NONE;
				Key = KEY_CurrentKey;
			}
			KEY_CurrentKey = Key;
		} while ((!bScan || !BK4819_CheckSquelchLink()) && gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_KEY_PTT));

		if (!gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_KEY_PTT)) {
			gPttPressed = true;
			KEY_SideKeyLongPressed = false;
			KEY_KeyCounter = 0;
			StopDetect();
			BEEP_Play(440, 4, 80);
			return;
		}
		BK4819_DisableAutoCssBW();
		MuteCtcssScan();
		KEY_CurrentKey = KEY_NONE;
		RADIO_Tune(gSettings.CurrentVfo);
		gSignalFound = false;
		Key = KEY_CurrentKey;
		while (1) {
			KEY_CurrentKey = Key;
			if (!gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_KEY_PTT)) {
				gPttPressed = true;
				KEY_SideKeyLongPressed = false;
				KEY_KeyCounter = 0;
				StopDetect();
				BEEP_Play(440, 4, 80);
				return;
			}
			Key = KEY_GetButton();
			Task_CheckIncoming();
			Task_CheckRSSI();
			if (bCtdcScan && gSignalFound && gRadioMode != RADIO_MODE_QUIET && gDetectorTimer == 0) {
				CtdcScan();
				break;
			}
			if (Key == KEY_NONE) {
				if (KEY_KeyCounter > 50) {
					KEY_KeyCounter = 0;
					if (KEY_CurrentKey == KEY_MENU) {
						KEY_CurrentKey = KEY_NONE;
						RADIO_EndRX();
						gSettings.WorkMode = 0;
						SETTINGS_SaveGlobals();
						RADIO_SaveCurrentVfo();
						KEY_SideKeyLongPressed = false;
						KEY_KeyCounter = 0;
						StopDetect();
						BEEP_Play(740, 3, 80);
						return;
					}
					if (KEY_CurrentKey == KEY_EXIT) {
						KEY_CurrentKey = KEY_NONE;
						RADIO_EndRX();
						BEEP_Play(740, 2, 100);
						break;
					}
					if (KEY_CurrentKey == KEY_HASH && !bCtdcScan) {
						KEY_CurrentKey = KEY_NONE;
						RADIO_EndRX();
						UpdateBand(true);
						break;
					}
					if (KEY_CurrentKey == KEY_STAR) {
						KEY_CurrentKey = KEY_NONE;
						RADIO_EndRX();
						if (!bCtdcScan) {
							CtdcScan();
							bCtdcScan = true;
						} else {
							UpdateBand(false);
							bCtdcScan = false;
						}
						break;
					}
				}
				KEY_KeyCounter = 0;
				KEY_CurrentKey = KEY_NONE;
				Key = KEY_CurrentKey;
			}
		}
	}
}

//

void RADIO_FrequencyDetect(void)
{
	SPEAKER_State = 0;
	gpio_bits_reset(GPIOA, BOARD_GPIOA_SPEAKER);
	gAudioOffsetIndex = gAudioOffsetLast;
	gFrequencyDetectMode = true;
	gVfoState[gSettings.CurrentVfo].RX.Frequency = 10000000;
	UI_DrawRadar();
	gUseUhfFilter = !gSettings.bUseVHF;
	UI_DrawBand();
	VFO_ClearCss();
	VFO_ClearMute();
	BK4819_EnableFilter(true);
	DETECTOR_Loop();
}

