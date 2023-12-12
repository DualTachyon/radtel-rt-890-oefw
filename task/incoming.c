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

#ifdef ENABLE_FM_RADIO
	#include "app/fm.h"
#endif
#include "app/radio.h"
#include "driver/bk4819.h"
#include "driver/pins.h"
#include "misc.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/incoming.h"
#include "task/ptt.h"

void Task_CheckIncoming(void)
{
	if (
#ifdef ENABLE_FM_RADIO
			(gFM_Mode == FM_MODE_OFF || gSettings.FmStandby) &&
#endif
			gRadioMode != RADIO_MODE_TX && !gSaveMode && SCHEDULER_CheckTask(TASK_CHECK_INCOMING) && gIncomingTimer == 0) {
		bool bGotLink;

		SCHEDULER_ClearTask(TASK_CHECK_INCOMING);

		bGotLink = BK4819_CheckSquelchLink();
		if (!bGotLink || gRadioMode != RADIO_MODE_QUIET) {
			if (bGotLink || gRadioMode == RADIO_MODE_QUIET) {
				gRxLinkCounter = 0;
			} else if (gRxLinkCounter++ > 5) {
				gRxLinkCounter = 0;
				PTT_ClearLock(PTT_LOCK_INCOMING);
				gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);
				if (gRadioMode == RADIO_MODE_RX || gFrequencyDetectMode) {
					if (!gReceptionMode) {
						RADIO_EndRX();
					} else {
						RADIO_EndAudio();
					}
				}
				gRadioMode = RADIO_MODE_QUIET;
			}
		} else {
			if (gRxLinkCounter++ > 5) {
				gRxLinkCounter = 0;
				gSaveModeTimer = 300;
				if (gMainVfo->BCL == BUSY_LOCK_CARRIER && !gFrequencyDetectMode) {
					PTT_SetLock(PTT_LOCK_INCOMING);
				}
				gRadioMode = RADIO_MODE_INCOMING;
			}
		}
	}
}

