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
#include "app/lock.h"
#include "app/menu.h"
#include "app/radio.h"
#include "driver/audio.h"
#include "driver/beep.h"
#include "driver/bk1080.h"
#include "driver/bk4819.h"
#include "driver/key.h"
#include "driver/pins.h"
#include "dtmf.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/channels.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/alarm.h"
#include "task/keys.h"
#include "task/lock.h"
#include "task/noaa.h"
#include "task/scanner.h"
#include "task/screen.h"
#include "task/vox.h"
#include "ui/dialog.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/main.h"
#include "ui/vfo.h"
#include "unknown.h"

static void FM_AppendDigit(char Digit)
{
	if (gInputBoxWriteIndex == 0) {
		INPUTBOX_Pad(0, 10);
		if (Digit > 1) {
			gInputBox[gInputBoxWriteIndex] = 0;
			gInputBoxWriteIndex = 1;
		}
	}
	gInputBox[gInputBoxWriteIndex++] = Digit;
	gInputBoxWriteIndex %= 4U;
	UI_DrawDecimal(gInputBox);
	if (gInputBoxWriteIndex == 0) {
		FM_UpdateFrequency();
	}
}

static void CHANNEL_AppendDigit(char Digit)
{
	if (gInputBoxWriteIndex == 0) {
		INPUTBOX_Pad(0, '-');
	}
	gInputBox[gInputBoxWriteIndex++] = Digit;
	gInputBoxWriteIndex %= 3U;
	UI_DrawDigits(gInputBox, gSettings.CurrentVfo);
	if (gInputBoxWriteIndex == 0) {
		CHANNELS_UpdateChannel();
	}
}

static void VFO_AppendDigit(char Digit)
{
	if (gInputBoxWriteIndex == 0) {
		INPUTBOX_Pad(0, 10);
	}
	gInputBox[gInputBoxWriteIndex++] = Digit;
	gInputBoxWriteIndex %= 6U;
	UI_DrawFrequencyEx(gInputBox, gSettings.CurrentVfo, gFrequencyReverse);
	if (gInputBoxWriteIndex == 0) {
		CHANNELS_UpdateVFO();
	}
}

static void MAIN_KeyHandler(KEY_t Key)
{
	uint8_t Vfo = 1;

	VOX_Timer = 0;
	Task_UpdateScreen();
	if (gScannerMode && Key != KEY_UP && Key != KEY_DOWN) {
		SETTINGS_SaveState();
		return;
	}

	switch (Key) {
	case KEY_0: case KEY_1: case KEY_2: case KEY_3:
	case KEY_4: case KEY_5: case KEY_6: case KEY_7:
	case KEY_8: case KEY_9:
		if (gVfoMode < VFO_MODE_FM_SCROLL_UP) {
			if (gVfoMode == VFO_MODE_FM) {
				FM_AppendDigit(Key);
			} else if (gSettings.WorkMode) {
				CHANNEL_AppendDigit(Key);
			} else {
				VFO_AppendDigit(Key);
			}
			AUDIO_PlayDigit(Key);
			BEEP_Play(740, 2, 100);
		}
		break;

	case KEY_MENU:
		if (gInputBoxWriteIndex == 0) {
			if (gVfoMode > VFO_MODE_1) {
				break;
			}
			gFrequencyReverse = false;
			MENU_Redraw(true);
		} else {
			INPUTBOX_Pad(gInputBoxWriteIndex, 0);
			if (gVfoMode == VFO_MODE_FM) {
				FM_UpdateFrequency();
				UI_DrawFMFrequency(gSettings.FmFrequency);
			} else if (gSettings.WorkMode) {
				CHANNELS_UpdateChannel();
			} else {
				CHANNELS_UpdateVFO();
			}
		}
		BEEP_Play(740, 3, 80);
		break;

	case KEY_UP:
	case KEY_DOWN:
		if (gInputBoxWriteIndex == 0) {
			if (gVfoMode < VFO_MODE_FM) {
				if (!gReceptionMode) {
					if (!gScannerMode) {
						RADIO_CancelMode();
						if (gSettings.WorkMode) {
							CHANNELS_NextChannelMr(Key);
							SETTINGS_SaveGlobals();
							AUDIO_PlayChannelNumber();
						} else {
							RADIO_Tune(gSettings.CurrentVfo);
							CHANNELS_NextChannelVfo(Key);
							CHANNELS_SaveChannel(gSettings.CurrentVfo ? 1000 : 999, &gVfoState[gSettings.CurrentVfo]);
							CHANNELS_LoadChannel(gSettings.CurrentVfo ? 1000 : 999, gSettings.CurrentVfo);
							RADIO_Tune(gSettings.CurrentVfo);
						}
					} else {
						// TODO: Logic is correct but doesn't make sense
						gSettings.StandbyArea = (Key + 1) & 1;
						SETTINGS_SaveGlobals();
					}
				} else {
					CHANNELS_NextNOAA(Key);
					NOAA_NextChannelCountdown = 3000;
				}
			} else if (gVfoMode == VFO_MODE_FM) {
				CHANNELS_NextFM(Key);
			} else {
				FM_Play();
			}
		}
		BEEP_Play(740, 2, 100);
		break;

	case KEY_EXIT:
		if (gInputBoxWriteIndex) {
			if (gVfoMode == VFO_MODE_FM) {
				RADIO_DrawFmMode();
			} else if (gSettings.WorkMode) {
				RADIO_DrawWorkMode();
			} else {
				RADIO_DrawFrequencyMode();
			}
			BEEP_Play(440, 4, 80);
			break;
		}
		if (gVfoMode < VFO_MODE_FM) {
			if (gFrequencyReverse) {
				gFrequencyReverse = false;
				UI_DrawVfo(gSettings.CurrentVfo);
				BEEP_Play(440, 4, 80);
				break;
			}
			RADIO_CancelMode();
			if (gSettings.DualDisplay == 0) {
				DISPLAY_Fill(1, 158, 1 + (gCurrentVfo * 41), 40 + (gCurrentVfo * 41), COLOR_BLACK);
				DISPLAY_Fill(1, 158, 1 + ((!gCurrentVfo) * 41), 40 + ((!gCurrentVfo) * 41), COLOR_BLACK);
				UI_DrawVoltage(gSettings.CurrentVfo);
			}
			UI_DrawMainBitmap(false, gSettings.CurrentVfo);
			gSettings.CurrentVfo ^= 1;
			gAudioOffsetIndex = gAudioOffsetLast;
			RADIO_Tune(gSettings.CurrentVfo);
			SETTINGS_SaveGlobals();
			gFrequencyReverse = false;
			UI_DrawVfo(gCurrentVfo);
			UI_DrawMainBitmap(true, gSettings.CurrentVfo);
			if (Vfo != 0) {
				BEEP_Play(740, 3, 80);
			} else {
				BEEP_Play(440, 4, 80);
			}
		}
		break;

	case KEY_STAR:
		break;

	case KEY_HASH:
		if (gVfoMode < VFO_MODE_FM) {
			gFrequencyReverse = false;
			RADIO_CancelMode();
			if (gFreeChannelsCount == 0) {
				UI_DrawDialogText(DIALOG_NO_CH_AVAILABLE, true);
				break;
			}
			gInputBoxWriteIndex = 0;
			gAudioOffsetIndex = gAudioOffsetLast;
			gSettings.WorkMode ^= 1;
			SETTINGS_SaveGlobals();
			if (gSettings.WorkMode) {
				CHANNELS_LoadWorkMode();
			} else {
				CHANNELS_LoadVfoMode();
			}
			RADIO_Tune(gSettings.CurrentVfo);
			if (gSettings.DualDisplay == 0) {
				UI_DrawVfo(gSettings.CurrentVfo);
			} else {
				UI_DrawVfo(0);
				UI_DrawVfo(1);
			}
			if (Vfo != 0) {
				BEEP_Play(740, 3, 80);
			} else {
				BEEP_Play(440, 4, 80);
			}
		}
		break;

	default:
		if (Vfo != 0) {
			BEEP_Play(740, 3, 80);
		} else {
			BEEP_Play(440, 4, 80);
		}
		break;
	}
}

static void HandlerShort(KEY_t Key)
{
	if (gSettings.Lock || gRadioMode == RADIO_MODE_TX || (gReceptionMode && Key != KEY_UP && Key != KEY_DOWN)) {
		return;
	}

	if (gEnableLocalAlarm) {
		ALARM_Stop();
		return;
	}
	switch (gScreenMode) {
	case SCREEN_MAIN:
		MAIN_KeyHandler(Key);
		break;
	case SCREEN_MENU:
		MENU_KeyHandler(Key);
		break;
	case SCREEN_SETTING:
		MENU_SettingKeyHandler(Key);
		break;
	}
}

static void HandlerLong(KEY_t Key)
{
	bool bBeep740;

	if (gDTMF_InputMode && Key != KEY_MENU) {
		return;
	}

	if (gSettings.Lock && Key != KEY_HASH) {
		return;
	}

	if (gFrequencyDetectMode || gRadioMode == RADIO_MODE_TX) {
		return;
	}

	if (gScannerMode) {
		SETTINGS_SaveState();
		BEEP_Play(440, 4, 80);
		return;
	}

	if (gEnableLocalAlarm) {
		ALARM_Stop();
		return;
	}

	if (Key == KEY_7) {
		gSettings.bEnableDisplay ^= 1;
		if (gSettings.bEnableDisplay) {
			SCREEN_TurnOn();
			BEEP_Play(740, 3, 80);
		} else {
			gpio_bits_reset(GPIOA, BOARD_GPIOA_LCD_RESX);
			BEEP_Play(440, 4, 80);
		}
		SETTINGS_SaveGlobals();
		return;
	}

	if (gSettings.bEnableDisplay && gEnableBlink) {
		SCREEN_TurnOn();
		BEEP_Play(740, 2, 100);
		return;
	}
	bBeep740 = true;
	if (!gReceptionMode && (gVfoMode == VFO_MODE_MAIN || Key == KEY_0 || Key == KEY_HASH || Key == KEY_UP || Key == KEY_DOWN)) {
		SCREEN_TurnOn();
		if (gScreenMode == SCREEN_MAIN) {
			switch (Key) {
			case KEY_0:
				if (gVfoMode == VFO_MODE_MAIN) {
					RADIO_DisableSaveMode();
					if (gSettings.DualStandby) {
						RADIO_Tune(gSettings.CurrentVfo);
						gIdleMode = IDLE_MODE_DUAL_STANDBY;
					}
					FM_Play();
				} else {
					VFO_SetMode(VFO_MODE_MAIN);
				}
				break;

			case KEY_1:
				if (gSettings.bFLock) {
					return;
				}
				if (gVfoMode != VFO_MODE_MAIN) {
					return;
				}
				if (gFrequencyReverse) {
					return;
				}
				RADIO_CancelMode();
				gScannerMode = !gScannerMode;
				bBeep740 = gScannerMode;
				SCANNER_Countdown = 250;
				break;

			case KEY_2:
				gSettings.TxPriority ^= 1;
				SETTINGS_SaveGlobals();
				UI_DrawDialogText(DIALOG_TX_PRIORITY, gSettings.TxPriority);
				break;

			case KEY_3:
				RADIO_CancelMode();
				gSettings.Vox ^= 1;
				BK4819_EnableVox(gSettings.Vox);
				SETTINGS_SaveGlobals();
				if (VOX_IsTransmitting) {
					RADIO_EndTX();
					VOX_IsTransmitting = false;
				}
				UI_DrawStatusIcon(80, ICON_VOX, gSettings.Vox, COLOR_BLACK);
				UI_DrawDialogText(DIALOG_VOX, gSettings.Vox);
				break;

			case KEY_4:
				RADIO_CancelMode();
				gVfoState[gSettings.CurrentVfo].bIsLowPower ^= 1;
				UI_DrawTxPower(gVfoState[gSettings.CurrentVfo].bIsLowPower, gSettings.CurrentVfo);
				CHANNELS_SaveVfo();
				UI_DrawDialogText(DIALOG_TX_POWER, gVfoState[gSettings.CurrentVfo].bIsLowPower);
				break;

			case KEY_5:
				gMenuIndex = MENU_SQ_LEVEL;
				DISPLAY_Fill(0, 159, 1, 81, COLOR_BLACK);
				DISPLAY_DrawRectangle0(0, 56, 160, 1, gSettings.BorderColor);
				MENU_DrawSetting();
				break;

			case KEY_6:
				RADIO_CancelMode();
				gSettings.DualStandby ^= 1;
				RADIO_Tune(gSettings.CurrentVfo);
				SETTINGS_SaveGlobals();
				gIdleMode = IDLE_MODE_OFF;
				UI_DrawStatusIcon(56, ICON_DUAL_WATCH, gSettings.DualStandby, COLOR_BLACK);
				UI_DrawDialogText(DIALOG_DUAL_STANDBY, gSettings.DualStandby);
				break;

			case KEY_8:
				gMenuIndex = MENU_FREQ_STEP;
				DISPLAY_Fill(0, 159, 1, 81, COLOR_BLACK);
				DISPLAY_DrawRectangle0(0, 56, 160, 1, gSettings.BorderColor);
				MENU_DrawSetting();
				break;

			case KEY_9:
				gSettings.KeyBeep ^= 1;
				SETTINGS_SaveGlobals();
				UI_DrawDialogText(DIALOG_KEY_BEEP, gSettings.KeyBeep);
				break;

			case KEY_MENU:
				if (gVfoMode != VFO_MODE_MAIN) {
					return;
				}
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

			case KEY_UP:
			case KEY_DOWN:
				if (gInputBoxWriteIndex == 0) {
					if (gVfoMode == VFO_MODE_MAIN) {
						RADIO_CancelMode();
						if (gSettings.WorkMode) {
							do {
								CHANNELS_NextChannelMr(Key);
							} while (KEY_GetButton() != KEY_NONE);
							SETTINGS_SaveGlobals();
							AUDIO_PlayChannelNumber();
						} else {
							do {
								RADIO_Tune(gSettings.CurrentVfo);
								CHANNELS_NextChannelVfo(Key);
							} while (KEY_GetButton() != KEY_NONE);
							CHANNELS_SaveChannel(gSettings.CurrentVfo ? 1000 : 999, &gVfoState[gSettings.CurrentVfo]);
							CHANNELS_LoadChannel(gSettings.CurrentVfo ? 1000 : 999, gSettings.CurrentVfo);
						}
						RADIO_Tune(gSettings.CurrentVfo);
					} else {
						if (Key == KEY_UP) {
							gVfoMode = VFO_MODE_FM_SCROLL_UP;
							gSettings.FmFrequency += 2;
							if (gSettings.FmFrequency > 1080) {
								gSettings.FmFrequency = 640;
							}
						} else {
							gVfoMode = VFO_MODE_FM_SCROLL_DOWN;
							gSettings.FmFrequency -= 2;
							if (gSettings.FmFrequency < 640) {
								gSettings.FmFrequency = 1080;
							}
						}
						UI_DrawFMFrequency(gSettings.FmFrequency);
						BK1080_SetVolume(0);
					}
					bBeep740 = Key - KEY_UP;
				}
				break;

			case KEY_EXIT:
				if (gVfoMode != VFO_MODE_MAIN) {
					return;
				}
				gSettings.DualDisplay ^= 1;
				SETTINGS_SaveGlobals();
				VOX_Timer = 0;
				UI_DrawMain(true);
				break;

			case KEY_STAR:
				if (gSettings.WorkMode) {
					return;
				}
				gFrequencyReverse = !gFrequencyReverse;
				bBeep740 = gFrequencyReverse;
				UI_DrawVfo(gSettings.CurrentVfo);
				gInputBoxWriteIndex = 0;
				INPUTBOX_Pad(0, 10);
				break;

			case KEY_HASH:
				LOCK_Toggle();
				bBeep740 = gSettings.Lock;
				break;

			default:
				break;
			}
		} else {
			if (Key != KEY_UP && Key != KEY_DOWN) {
				return;
			}
			do {
				if (gScreenMode == SCREEN_MENU) {
					MENU_Next(Key);
				} else if (gScreenMode == SCREEN_SETTING) {
					MENU_ScrollSetting(Key);
				}
			} while (KEY_GetButton() != KEY_NONE);
			if (gScreenMode == SCREEN_MENU) {
				MENU_PlayAudio((gMenuIndex + gSettingIndex) % gSettingsCount);
			}
			bBeep740 = Key - KEY_UP;
		}
		if (bBeep740) {
			BEEP_Play(740, 3, 80);
		} else {
			BEEP_Play(440, 4, 80);
		}
	}
}

void Task_CheckKeyPad(void)
{
	if (SCHEDULER_CheckTask(TASK_CHECK_KEY_PAD) && gSettings.DtmfState == DTMF_STATE_NORMAL) {
		KEY_t Key;

		SCHEDULER_ClearTask(TASK_CHECK_KEY_PAD);

		Key = KEY_GetButton();
		if (Key == KEY_NONE) {
			if (gDTMF_Playing) {
				gDTMF_Playing = false;
				DTMF_Disable();
				BEEP_Disable();
			}
			KEY_LongPressed = false;
			if (KEY_KeyCounter > 10) {
				KEY_KeyCounter = 0;
				if (gSettings.bEnableDisplay && gEnableBlink) {
					SCREEN_TurnOn();
					BEEP_Play(700, 2, 100);
				} else if (!gDTMF_InputMode) {
					HandlerShort(KEY_CurrentKey);
				} else {
					if (gDTMF_Input.Length < 14) {
						gDTMF_Input.String[gDTMF_Input.Length++] = DTMF_GetCharacterFromKey(KEY_CurrentKey);
						UI_DrawDTMF();
					}
					BEEP_Play(700, 2, 100);
				}
				SCREEN_TurnOn();
				gLockTimer = 0;
			}
			KEY_KeyCounter = 0;
			KEY_CurrentKey = KEY_NONE;
		} else {
			KEY_CurrentKey = Key;
			if (KEY_KeyCounter > 10 && gRadioMode == RADIO_MODE_TX && !gEnableLocalAlarm && !gDTMF_Playing) {
				gDTMF_Playing = true;
				DTMF_PlayTone(Key);
			} else if (KEY_KeyCounter > 1000) {
				KEY_LongPressed = true;
				gLockTimer = 0;
				HandlerLong(Key);
				KEY_KeyCounter = 0;
				gLockTimer = 0;
			}
		}
	}
}

