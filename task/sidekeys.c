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

#include "app/fm.h"
#include "app/radio.h"
#include "app/fm.h"
#include "driver/beep.h"
#include "driver/bk4819.h"
#include "driver/key.h"
#include "driver/pins.h"
#include "helper/helper.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/detector.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/alarm.h"
#include "task/idle.h"
#include "task/sidekeys.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "ui/vfo.h"

enum {
	ACTION_NONE = 0,
	ACTION_MONITOR,
	ACTION_FREQUENCY_DETECT,
	ACTION_REPEATER_MODE,
	ACTION_PRESET_CHANNEL,
	ACTION_LOCAL_ALARM,
	ACTION_REMOTE_ALARM,
	ACTION_NOAA_CHANNEL,
	ACTION_SEND_TONE,
	ACTION_ROGER_BEEP,
	ACTION_FM_RADIO,
};

//

void Task_CheckSideKeys(void)
{
	uint8_t Slot;
	uint8_t Action;

	if (!SCHEDULER_CheckTask(TASK_CHECK_SIDE_KEYS) || gSettings.DtmfState != DTMF_STATE_NORMAL) {
		return;
	}

	SCHEDULER_ClearTask(TASK_CHECK_SIDE_KEYS);

	if (gSettings.Lock || gFM_Mode != FM_MODE_OFF || gScreenMode != 0 || gDTMF_InputMode ||
			(!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1) && !gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_SIDE2)) ||
			!gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_KEY_PTT)
	   ) {
		KEY_Side1Counter = 0;
		KEY_Side2Counter = 0;
		return;
	}

	// ??? Such a specific number
	Slot = 6;

	if (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1) && KEY_Side1Counter > 1000) {
		KEY_SideKeyLongPressed = true;
		KEY_Side1Counter = 0;
		Slot = 0;
	} else if (!gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_SIDE2) && KEY_Side2Counter > 1000) {
		KEY_SideKeyLongPressed = true;
		KEY_Side2Counter = 0;
		Slot = 2;
	} else if (gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1) && gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_SIDE2)) {
		if (KEY_Side1Counter > 100) {
			Slot = 1;
		} else if (KEY_Side2Counter > 100) {
			Slot = 3;
		}
		KEY_SideKeyLongPressed = false;
		KEY_Side1Counter = 0;
		KEY_Side2Counter = 0;
	}

	if (Slot >= 4) {
		return;
	}

	Action = gSettings.Actions[Slot];
	if (gSettings.bEnableDisplay && gEnableBlink) {
		SCREEN_TurnOn();
		BEEP_Play(740, 2, 100);
		return;
	}

	if (Action == ACTION_NONE) {
		return;
	}

	SCREEN_TurnOn();

	if (gScannerMode) {
		SETTINGS_SaveState();
		BEEP_Play(440, 4, 80);
		return;
	}

	if (gMonitorMode) {
		gMonitorMode = false;
		RADIO_EndRX();
		return;
	}

	if (gEnableLocalAlarm) {
		ALARM_Stop();
		return;
	}

	if (!gReceptionMode || Action == ACTION_NOAA_CHANNEL) {
		if (gRadioMode == RADIO_MODE_QUIET) {
			IDLE_SelectMode();
		}
		switch (Action) {
		case ACTION_MONITOR:
			gMonitorMode = true;
			RADIO_Tune(gSettings.CurrentVfo);
			break;

		case ACTION_FREQUENCY_DETECT:
			if (!gSettings.bFLock) {
				gInputBoxWriteIndex = 0;
				RADIO_FrequencyDetect();
			}
			break;

		case ACTION_REPEATER_MODE:
			gSettings.RepeaterMode = (gSettings.RepeaterMode + 1) % 3;
			SETTINGS_SaveGlobals();
			RADIO_Tune(gSettings.CurrentVfo);
			UI_DrawRepeaterMode();
			if (gSettings.DualDisplay == 0) {
				UI_DrawVfo(gSettings.CurrentVfo);
			} else {
				UI_DrawVfo(0);
				UI_DrawVfo(1);
			}
			BEEP_Play(740, 2, 100);
			break;

		case ACTION_PRESET_CHANNEL:
			if (gSettings.WorkMode) {
				gInputBoxWriteIndex = 0;
				if (CHANNELS_LoadChannel(gSettings.PresetChannels[Slot], gSettings.CurrentVfo)) {
					CHANNELS_LoadChannel(gSettings.VfoChNo[gSettings.CurrentVfo], gSettings.CurrentVfo);
				} else {
					gSettings.VfoChNo[gSettings.CurrentVfo] = gSettings.PresetChannels[Slot];
					RADIO_Tune(gSettings.CurrentVfo);
					UI_DrawVfo(gSettings.CurrentVfo);
				}
				BEEP_Play(740, 2, 100);
			}
			break;

		case ACTION_LOCAL_ALARM:
			ALARM_Start();
			BK4819_SetAF(BK4819_AF_ALAM);
			break;

		case ACTION_REMOTE_ALARM:
			ALARM_Start();
			BK4819_SetAF(BK4819_AF_BEEP);
			RADIO_StartTX(false);
			break;

		case ACTION_NOAA_CHANNEL:
			if (gRadioMode != RADIO_MODE_TX) {
				gInputBoxWriteIndex = 0;
				gReceptionMode = !gReceptionMode;
				if (!gReceptionMode) {
					RADIO_NoaaRetune();
					BEEP_Play(440, 4, 80);
				} else {
					if (gRadioMode == RADIO_MODE_RX) {
						RADIO_EndRX();
					}
					RADIO_NoaaTune();
					BEEP_Play(740, 2, 100);
				}
			}
			break;

		case ACTION_SEND_TONE:
			gEnableLocalAlarm = true;
			gSendTone = true;
			RADIO_StartTX(false);
			BK4819_EnableTone1(true);
			BK4819_SetToneFrequency(gSettings.ToneFrequency);
			break;

		case ACTION_ROGER_BEEP:
			gSettings.RogerBeep = (gSettings.RogerBeep + 1) % 4;
			SETTINGS_SaveGlobals();
			BEEP_Play(740, 2, 100);
			UI_DrawRoger();
			break;

		case ACTION_FM_RADIO:
			// TODO FM radio
			RADIO_DisableSaveMode();
			if (gSettings.DualStandby) {
				RADIO_Tune(gSettings.CurrentVfo);
				gIdleMode = IDLE_MODE_DUAL_STANDBY;
			}
			FM_Play();
			break;
		}
	}
}

