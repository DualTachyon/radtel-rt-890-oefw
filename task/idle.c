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
#include "driver/speaker.h"
#include "misc.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/idle.h"
#include "task/vox.h"

void Task_Idle(void)
{
	if (gRadioMode != RADIO_MODE_RX && gRadioMode != RADIO_MODE_TX && VOX_Counter == 0 && gRxLinkCounter == 0 && !gScannerMode && !gReceptionMode && !gMonitorMode && !gEnableLocalAlarm && gSaveModeTimer == 0 && SPEAKER_State == 0
#ifdef ENABLE_FM_RADIO
		&& gFM_Mode == FM_MODE_OFF
#endif
			) {
		switch (gIdleMode) {
		case IDLE_MODE_OFF:
#ifdef ENABLE_NOAA
			gNoaaMode = false;
#endif
			IDLE_SelectMode();
			break;

		case IDLE_MODE_DUAL_STANDBY:
#ifdef ENABLE_NOAA
			gNoaaMode = false;
#endif
			RADIO_Tune(!gSettings.CurrentVfo);
#ifdef ENABLE_NOAA
			if (gSettings.NoaaAlarm) {
				gIdleMode = IDLE_MODE_NOAA;
			} else
#endif
			if (gSettings.SaveMode) {
				gIdleMode = IDLE_MODE_SAVE;
			} else {
				gIdleMode = IDLE_MODE_OFF;
			}
			gSaveModeTimer = 150;
			break;

#ifdef ENABLE_NOAA
		case IDLE_MODE_NOAA:
			gNoaaMode = true;
			gNOAA_ChannelNext = (gNOAA_ChannelNext + 1) % 11;
			CHANNELS_SetNoaaChannel(gNOAA_ChannelNext);
			RADIO_Tune(2);
			if (gSettings.SaveMode) {
				gIdleMode = IDLE_MODE_SAVE;
			} else {
				gIdleMode = IDLE_MODE_OFF;
			}
			gSaveModeTimer = 150;
			break;
#endif

		case IDLE_MODE_SAVE:
#ifdef ENABLE_NOAA
			gNoaaMode = false;
#endif
			gIdleMode = IDLE_MODE_OFF;
			if (gIdleTimer == 0) {
				if (gTimeSinceBoot < 600000) {
					gSaveModeTimer = 160;
				} else if (gTimeSinceBoot >= 600000 && gTimeSinceBoot < 1200000) {
					gSaveModeTimer = 320;
				} else if (gTimeSinceBoot >= 1200000 && gTimeSinceBoot < 1800000) {
					gSaveModeTimer = 480;
				} else if (gTimeSinceBoot >= 1800000 && gTimeSinceBoot < 2400000) {
					gSaveModeTimer = 640;
				} else if (gTimeSinceBoot >= 2400000) {
					gSaveModeTimer = 750;
				}
				RADIO_Sleep();
			}
			break;
		}
	}
}

void IDLE_SelectMode(void)
{
	RADIO_DisableSaveMode();
#ifdef ENABLE_NOAA
	if (gSettings.DualStandby || gSettings.NoaaAlarm) {
#else
	if (gSettings.DualStandby) {
#endif
		RADIO_Tune(gSettings.CurrentVfo);
	}
	if (gSettings.DualStandby) {
		gIdleMode = IDLE_MODE_DUAL_STANDBY;
#ifdef ENABLE_NOAA
	} else if (gSettings.NoaaAlarm) {
		gIdleMode = IDLE_MODE_NOAA;
#endif
	} else if (gSettings.SaveMode) {
		gIdleMode = IDLE_MODE_SAVE;
	}
	gSaveModeTimer = 150;
}

