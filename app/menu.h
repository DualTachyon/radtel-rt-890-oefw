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

#ifndef APP_MENU_H
#define APP_MENU_H

#include <stdbool.h>
#include <stdint.h>

enum {
	MENU_STARTUP_LOGO = 0,
	MENU_VOLTAGE,
	MENU_RINGTONE,
	MENU_PROMPT_TEXT,
	MENU_VOICE_PROMPT,
	MENU_KEY_BEEP,
	MENU_ROGER_BEEP,
	MENU_DUAL_DISPLAY,
	MENU_TX_PRIORITY,
	MENU_SAVE_MODE,
	MENU_FREQ_STEP,
	MENU_SQ_LEVEL,
	MENU_LED_TIMER,
	MENU_LOCK_TIMER,
	MENU_TOT,
	MENU_VOX_LEVEL,
	MENU_VOX_DELAY,
	MENU_NOAA_MONITOR,
	MENU_FM_STANDBY,
	MENU_TAIL_TONE,
	MENU_SCAN_DIR,
	MENU_PERSONAL_ID,
	MENU_REPEATER_MODE,
	MENU_SCAN_RESUME,
	MENU_SCAN_BLINK,
	MENU_CTCSS_DCS,
	MENU_RX_CTCSS_DCS,
	MENU_TX_CTCSS_DCS,
	MENU_TX_POWER,
	MENU_MODULATION,
	MENU_BAND_WIDTH,
	MENU_LIST_TO_SCAN,
	MENU_SCANLIST_1,
	MENU_SCANLIST_2,
	MENU_SCANLIST_3,
	MENU_SCANLIST_4,
	MENU_SCANLIST_5,
	MENU_SCANLIST_6,
	MENU_SCANLIST_7,
	MENU_SCANLIST_8,
	MENU_BUSY_LOCK,
	MENU_SCRAMBLER,
	MENU_DCS_ENCRYPT,
	MENU_MUTE_CODE,
	MENU_CH_NAME,
	MENU_SAVE_CH,
	MENU_DELETE_CH,
	MENU_K1_LONG,
	MENU_K1_SHORT,
	MENU_K2_LONG,
	MENU_K2_SHORT,
	MENU_0_LONG,
	MENU_1_LONG,
	MENU_2_LONG,
	MENU_3_LONG,
	MENU_4_LONG,
	MENU_5_LONG,
	MENU_6_LONG,
	MENU_7_LONG,
	MENU_8_LONG,
	MENU_9_LONG,
	MENU_STAR_LONG,
	MENU_HASH_LONG,
	MENU_MENU_LONG,
	MENU_EXIT_LONG,
	MENU_KEYS_RESET,
	MENU_DTMF_DELAY,
	MENU_DTMF_INTERVAL,
	MENU_DTMF_MODE,
	MENU_DTMF_SELECT,
	MENU_DTMF_DISPLAY,
	MENU_DARK_MODE,
	MENU_INITIALIZE,
	MENU_VERSION,
};

extern uint16_t gSettingGolay;
extern uint8_t gMenuIndex;
extern uint8_t gSettingIndex;
extern uint8_t gSettingsCount;

void MENU_SetMaxCssValues(void);
void MENU_AcceptSetting(void);
void MENU_DrawSetting(void);
void MENU_Redraw(bool bFlag);
void MENU_KeyHandler(uint8_t Key);
void MENU_Next(uint8_t Key);
void MENU_SettingKeyHandler(uint8_t Key);
void MENU_ScrollSetting(uint8_t Key);
void MENU_PlayAudio(uint8_t ID);

#endif

