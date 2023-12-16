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

#include "app/flashlight.h"
#ifdef ENABLE_FM_RADIO
	#include "app/fm.h"
#endif
#include "app/lock.h"
#include "app/radio.h"
#ifdef ENABLE_FM_RADIO
	#include "app/fm.h"
#endif
#include "app/menu.h"
#ifdef ENABLE_SPECTRUM
	#include "app/spectrum.h"
#endif
#include "driver/beep.h"
#include "driver/bk4819.h"
#include "driver/key.h"
#include "driver/pins.h"
#include "helper/dtmf.h"
#include "helper/helper.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/detector.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/alarm.h"
#include "task/idle.h"
#include "task/keyaction.h"
#include "task/keys.h"
#include "task/scanner.h"
#include "task/screen.h"
#include "task/sidekeys.h"
#include "task/vox.h"
#include "ui/dialog.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "ui/vfo.h"

void SetDefaultKeyShortcuts(uint8_t IncludeSideKeys) {
	if (IncludeSideKeys) {
		gSettings.Actions[0] = ACTION_FREQUENCY_DETECT;			//Side 1 long
		gSettings.Actions[1] = ACTION_MONITOR;					//Side 1 short
		gSettings.Actions[2] = ACTION_FLASHLIGHT;				//Side 2 long
		gSettings.Actions[3] = ACTION_ROGER_BEEP;				//Side 2 short
	}

	gExtendedSettings.KeyShortcut[0] = ACTION_FM_RADIO;			//0 key long
	gExtendedSettings.KeyShortcut[1] = ACTION_SCAN;				//1 key long
	gExtendedSettings.KeyShortcut[2] = ACTION_AM_FIX;			//2 key long
	gExtendedSettings.KeyShortcut[3] = ACTION_VOX;				//3 key long
	gExtendedSettings.KeyShortcut[4] = ACTION_TX_POWER;			//4 key long
	gExtendedSettings.KeyShortcut[5] = ACTION_SQ_LEVEL;			//5 key long
	gExtendedSettings.KeyShortcut[6] = ACTION_DUAL_STANDBY;		//6 key long
	gExtendedSettings.KeyShortcut[7] = ACTION_BACKLIGHT;		//7 key long
	gExtendedSettings.KeyShortcut[8] = ACTION_FREQ_STEP;		//8 key long
	gExtendedSettings.KeyShortcut[9] = ACTION_PRESET_CHANNEL;	//9 key long
	gExtendedSettings.KeyShortcut[10] = ACTION_TX_FREQ;			//* key long
	gExtendedSettings.KeyShortcut[11] = ACTION_LOCK;			//# key long
	gExtendedSettings.KeyShortcut[12] = ACTION_DTMF_DECODE;		//Menu key long
	gExtendedSettings.KeyShortcut[13] = ACTION_DUAL_DISPLAY;	//Exit key long

	SETTINGS_SaveGlobals();
}

void KeypressAction(uint8_t Action) {
	if (gSettings.Lock && Action != ACTION_LOCK) {
		return;
	}

	if (gDTMF_InputMode && Action != ACTION_DTMF_DECODE) {
		return;
	}

	if (gFlashlightMode) {
		FLASHLIGHT_Toggle();
		return;
	}

	if (gFrequencyDetectMode || gRadioMode == RADIO_MODE_TX) {
		return;
	}

	if (gScannerMode) {
		if (Action == ACTION_SCAN && gSettings.WorkMode) {
			Next_ScanList();
		} else {
			SETTINGS_SaveState();
			BEEP_Play(440, 4, 80);
		}
		return;
	}

	if (gEnableLocalAlarm) {
		ALARM_Stop();
		return;
	}

	if (gMonitorMode) {
		gMonitorMode = false;
		RADIO_EndRX();
		return;
	}

#ifdef ENABLE_NOAA
	if (!gReceptionMode || Action == ACTION_NOAA_CHANNEL) {
#else
	if (!gReceptionMode) {
#endif
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
					KEY_LongPressed = false;
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

			// This action acts like previous for when used from sidekeys and like "1 call" when used from keyboard
			case ACTION_PRESET_CHANNEL:
				if (gSettings.WorkMode) {
					gInputBoxWriteIndex = 0;
					uint8_t Slot = (gSlot < 4 ? gSlot : 0);
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

#ifdef ENABLE_NOAA
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
#endif

			case ACTION_SEND_TONE:
				gEnableLocalAlarm = true;
				gSendTone = true;
				RADIO_StartTX(false);
				BK4819_EnableTone1(true);
				BK4819_SetToneFrequency(false, gSettings.ToneFrequency);
				break;

			case ACTION_ROGER_BEEP:
				gSettings.RogerBeep = (gSettings.RogerBeep + 1) % 4;
				SETTINGS_SaveGlobals();
				BEEP_Play(740, 2, 100);
				UI_DrawRoger();
				break;

			case ACTION_SCAN:
				RADIO_CancelMode();
				gManualScanDirection = gSettings.ScanDirection;
				gScannerMode ^= 1;
				bBeep740 = gScannerMode;
				SCANNER_Countdown = 15;
				UI_DrawScan();
				break;

			case ACTION_FLASHLIGHT:
				if (!gFlashlightMode) {
					FLASHLIGHT_Toggle();
				}
				break;

#ifdef ENABLE_FM_RADIO
			case ACTION_FM_RADIO:
				if (gFM_Mode == FM_MODE_OFF) {
					RADIO_DisableSaveMode();
					if (gSettings.DualStandby) {
						RADIO_Tune(gSettings.CurrentVfo);
						gIdleMode = IDLE_MODE_DUAL_STANDBY;
					}
					FM_Play();
				} else {
					FM_Disable(FM_MODE_OFF);
				}
				break;
#endif

			case ACTION_AM_FIX:
#ifdef ENABLE_AM_FIX
				BK4819_RestoreGainSettings();
				gExtendedSettings.AmFixEnabled = !gExtendedSettings.AmFixEnabled;
				SETTINGS_SaveGlobals();
				UI_DrawDialogText(DIALOG_AM_FIX, gExtendedSettings.AmFixEnabled);
#endif
				break;

			case ACTION_VOX:
				RADIO_CancelMode();
				gSettings.Vox ^= 1;
				BK4819_EnableVox(gSettings.Vox);
				SETTINGS_SaveGlobals();
				if (VOX_IsTransmitting) {
					RADIO_EndTX();
					VOX_IsTransmitting = false;
				}
				UI_DrawStatusIcon(80, ICON_VOX, gSettings.Vox, COLOR_FOREGROUND);
				UI_DrawDialogText(DIALOG_VOX, gSettings.Vox);
				break;

			case ACTION_TX_POWER:
				RADIO_CancelMode();
				gVfoState[gSettings.CurrentVfo].bIsLowPower ^= 1;
				UI_DrawTxPower(gVfoState[gSettings.CurrentVfo].bIsLowPower, gSettings.CurrentVfo);
				CHANNELS_SaveVfo();
				UI_DrawDialogText(DIALOG_TX_POWER, gVfoState[gSettings.CurrentVfo].bIsLowPower);
				break;

			case ACTION_SQ_LEVEL:
				gMenuIndex = MENU_SQ_LEVEL;
				DISPLAY_Fill(0, 159, 1, 81, COLOR_BACKGROUND);
				MENU_DrawSetting();
				break;

			case ACTION_DUAL_STANDBY:
				RADIO_CancelMode();
				gSettings.DualStandby ^= 1;
				RADIO_Tune(gSettings.CurrentVfo);
				SETTINGS_SaveGlobals();
				gIdleMode = IDLE_MODE_OFF;
				UI_DrawStatusIcon(56, ICON_DUAL_WATCH, gSettings.DualStandby, COLOR_FOREGROUND);
				UI_DrawDialogText(DIALOG_DUAL_STANDBY, gSettings.DualStandby);
				break;

			case ACTION_BACKLIGHT:
				gSettings.bEnableDisplay ^= 1;
				if (gSettings.bEnableDisplay) {
					SCREEN_TurnOn();
					BEEP_Play(740, 3, 80);
				} else {
					gpio_bits_reset(GPIOA, BOARD_GPIOA_LCD_RESX);
					BEEP_Play(440, 4, 80);
				}
				SETTINGS_SaveGlobals();
				break;

			case ACTION_FREQ_STEP:
				gMenuIndex = MENU_FREQ_STEP;
				DISPLAY_Fill(0, 159, 1, 81, COLOR_BACKGROUND);
				MENU_DrawSetting();
				break;

			case ACTION_TOGGLE_SCANLIST:
				if (gSettings.WorkMode && !gExtendedSettings.ScanAll) {
					gVfoState[gSettings.CurrentVfo].IsInscanList ^= (1 << gExtendedSettings.CurrentScanList);
					CHANNELS_SaveVfo();
					UI_DrawDialogText(DIALOG_TOGGLE_SCANLIST, ((gVfoState[gSettings.CurrentVfo].IsInscanList >> gExtendedSettings.CurrentScanList) & 1));
				}
				break;

			case ACTION_DTMF_DECODE:
#ifdef ENABLE_FM_RADIO
				if (gFM_Mode != FM_MODE_OFF) {
					return;
				}
#endif
				if (!gDTMF_InputMode) {
					if (gRadioMode == RADIO_MODE_RX) {
						return;
					}
					if (VOX_Timer) {
						VOX_Timer = 0;
						Task_UpdateScreen();
					}
					DTMF_ResetString();
					gDTMF_InputMode = true;
					UI_DrawDialog();
					UI_DrawDTMF();
				} else {
					DTMF_ResetString();
					gDTMF_InputMode = false;
					UI_DrawMain(true);
					bBeep740 = false;
				}
				break;

			case ACTION_DUAL_DISPLAY:
#ifdef ENABLE_FM_RADIO
				if (gFM_Mode != FM_MODE_OFF) {
					return;
				}
#endif
				gSettings.DualDisplay ^= 1;
				SETTINGS_SaveGlobals();
				VOX_Timer = 0;
				UI_DrawMain(true);
				break;

			case ACTION_TX_FREQ:
				gFrequencyReverse = !gFrequencyReverse;
				bBeep740 = gFrequencyReverse;
				UI_DrawVfo(gSettings.CurrentVfo);
				gInputBoxWriteIndex = 0;
				INPUTBOX_Pad(0, 10);
				break;

			case ACTION_LOCK:
				LOCK_Toggle();
				bBeep740 = gSettings.Lock;
				break;

			case ACTION_BEEP:
				gSettings.KeyBeep ^= 1;
				SETTINGS_SaveGlobals();
				UI_DrawDialogText(DIALOG_KEY_BEEP, gSettings.KeyBeep);
				break;

            case ACTION_DARK_MODE:
                gExtendedSettings.DarkMode ^= 1;
		        SETTINGS_SaveGlobals();
		        UI_SetColors(gExtendedSettings.DarkMode);
                UI_DrawMain(FALSE);
                break;
			
#ifdef ENABLE_SPECTRUM
			case ACTION_SPECTRUM:
				gInputBoxWriteIndex = 0;
				APP_Spectrum();
#endif

		}
	}
}