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
#include "bsp/gpio.h"
#include "driver/key.h"
#include "driver/pins.h"
#include "misc.h"
#include "radio/channels.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/scanner.h"
#include "ui/helper.h"

uint16_t SCANNER_Countdown;

void Task_Scanner(void) {
	if ((gRadioMode < (gExtendedSettings.ScanResume == 2 ? RADIO_MODE_TX : RADIO_MODE_RX) 	// Allows Task_Scanner in RX mode if ScanResume is set to Time Operated
			&& gScannerMode
			&& SCANNER_Countdown == 0
			&& SCHEDULER_CheckTask(TASK_SCANNER)
			)
			|| gForceScan) {
		SCHEDULER_ClearTask(TASK_SCANNER);
		gForceScan = false;
		if (gRadioMode == RADIO_MODE_RX) {	// Scanner timeout
			RADIO_EndRX();
		}
		if (gSettings.WorkMode) {
			if(!CHANNELS_NextChannelMr(gManualScanDirection ? KEY_DOWN : KEY_UP, !gExtendedSettings.ScanAll)) {
				Next_ScanList();
			}
		} else {
			CHANNELS_NextChannelVfo(gManualScanDirection ? KEY_DOWN : KEY_UP);
			RADIO_Tune(gSettings.CurrentVfo);
		}
		SCANNER_Countdown = 15;
		if (gExtendedSettings.ScanBlink) {
			gpio_bits_flip(GPIOA, BOARD_GPIOA_LED_GREEN);
		}
	}
}

void Next_ScanList(void) {
	if (gExtendedSettings.ScanAll) {
		gExtendedSettings.ScanAll = 0;
	} else {
		gExtendedSettings.CurrentScanList = (gExtendedSettings.CurrentScanList + 1) % 8;
		if (gExtendedSettings.CurrentScanList == 0) {
			gExtendedSettings.ScanAll = 1;
		}
	}
	UI_DrawScan();
}

