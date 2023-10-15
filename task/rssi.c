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
#include "driver/bk4819.h"
#include "helper/helper.h"
#include "misc.h"
#include "radio/data.h"
#include "radio/scheduler.h"
#include "task/rssi.h"
#include "task/scanner.h"
#include "task/vox.h"
#include "ui/helper.h"
#include "ui/noaa.h"

enum {
	STATUS_NO_TONE = 0,
	STATUS_GOT_TONE,
	STATUS_TAIL_TONE,
};

static uint8_t GetToneStatus(uint8_t CodeType, bool bMuteEnabled)
{
	uint16_t Value;
	
	Value = BK4819_ReadRegister(0x0C);

	if (gNoaaMode) {
		// Checks CTC1
		return (Value & 0x0400U) ? STATUS_GOT_TONE : STATUS_NO_TONE;
	}

	// Check Interrupt Request
	if (Value & 0x0001U && gRadioMode == RADIO_MODE_RX) {
		DATA_ReceiverCheck();
		Value = BK4819_ReadRegister(0x0C);
	}

	if (gMonitorMode) {
		return STATUS_GOT_TONE;
	}

	// Check CTC2(55hz)
	if (Value & 0x0800U) {
		return STATUS_TAIL_TONE;
	}

	// Check CTC1
	if (Value & 0x0400U) {
		if (CodeType == CODE_TYPE_CTCSS) {
			return STATUS_GOT_TONE;
		}
		if (CodeType == CODE_TYPE_DCS_N || CodeType == CODE_TYPE_DCS_I || bMuteEnabled) {
			return STATUS_TAIL_TONE;
		}
	}

	// Check DCS N
	if (Value & 0x4000U && (CodeType == CODE_TYPE_DCS_N || bMuteEnabled)) {
		return STATUS_GOT_TONE;
	}

	// Check DCS I
	if (Value & 0x8000U && CodeType == CODE_TYPE_DCS_I) {
		return STATUS_GOT_TONE;
	}

	if (CodeType == CODE_TYPE_OFF && !bMuteEnabled) {
		return STATUS_GOT_TONE;
	}

	return STATUS_NO_TONE;
}

static void CheckRSSI(void)
{
	if (gVoxRssiUpdateTimer == 0 && !gDataDisplay && !gDTMF_InputMode && !gFrequencyDetectMode && !gReceptionMode && !gFskDataReceived && gScreenMode == SCREEN_MAIN) {
		uint16_t RSSI;

		gVoxRssiUpdateTimer = 100;
		RSSI = BK4819_ReadRegister(0x67) & 0x1FF;
		if (RSSI < 98U) {
			RSSI = 0U;
		} else {
			RSSI -= 97U;
		}
		RSSI = (RSSI * 9U) / 5U;
		if (RSSI > 100) {
			RSSI = 100;
		}
		UI_DrawBar(RSSI, gCurrentVfo);
	}
}

void Task_CheckRSSI(void)
{
	if (gRadioMode != RADIO_MODE_TX && gRadioMode != RADIO_MODE_QUIET && !gSaveMode && SCHEDULER_CheckTask(TASK_CHECK_RSSI)) {
		uint8_t Status;

		SCHEDULER_ClearTask(TASK_CHECK_RSSI);
		Status = GetToneStatus(gVfoInfo[gCurrentVfo].CodeType, gMainVfo->bMuteEnabled);
		if (gRadioMode != RADIO_MODE_INCOMING) {
			if (Status == STATUS_TAIL_TONE) {
				gTailToneCounter++;
			} else {
				gTailToneCounter = 0;
			}
			if (Status == STATUS_NO_TONE) {
				gNoToneCounter++;
			}
			if (gTailToneCounter <= 10 && gNoToneCounter <= 1000) {
				SCANNER_Countdown = 0;
				gNoToneCounter = 0;
				CheckRSSI();
			} else if (!gReceptionMode) {
				RADIO_EndRX();
			} else {
				RADIO_EndAudio();
			}
		} else if (!gNoaaMode) {
			if (gReceptionMode) {
				StartRX();
			} else if ((gVfoInfo[gCurrentVfo].CodeType == CODE_TYPE_OFF && !gMainVfo->bMuteEnabled) || gMainVfo->gModulationType > 0 || Status == STATUS_GOT_TONE) {
				FUN_08006b38();
			}
		} else if (Status == STATUS_GOT_TONE) {
			gReceptionMode = true;
			gNoaaMode = false;
			gNOAA_ChannelNow = gNOAA_ChannelNext;
			UI_DrawSky();
			UI_DrawNOAA(gNOAA_ChannelNow);
			gReceivingAudio = true;
			SCREEN_TurnOn();
			BK4819_StartAudio();
		}
	}
}

