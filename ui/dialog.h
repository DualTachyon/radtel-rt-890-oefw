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

#ifndef UI_DIALOG_H
#define UI_DIALOG_H

#include <stdbool.h>
#include <stdint.h>

enum UI_DialogText_t {
	DIALOG_TX_PRIORITY = 1,
	DIALOG_VOX = 2,
	DIALOG_AM_FIX = 3,
	DIALOG_TX_POWER = 4,
	DIALOG_DUAL_STANDBY = 7,
	DIALOG_TOGGLE_SCANLIST = 8,
	DIALOG_KEY_BEEP = 9,
	DIALOG_PLEASE_CHARGE = 10,
	DIALOG_NO_CH_AVAILABLE = 14,
};

typedef enum UI_DialogText_t UI_DialogText_t;

void UI_DrawDialogText(UI_DialogText_t Text, bool bSet);

#endif

