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

#include "misc.h"
#include "radio/scheduler.h"
#include "ui/dialog.h"
#include "ui/gfx.h"
#include "ui/helper.h"

void UI_DrawDialogText(UI_DialogText_t Text, bool bSet)
{
	UI_DrawDialog();

	gColorForeground = COLOR_RED;

	switch (Text) {
	case DIALOG_AM_FIX:
		if (bSet) {
			UI_DrawString(10, 48, "AM Fix: On", 10);
		} else {
			UI_DrawString(10, 48, "AM Fix: Off", 11);
		}
		break;
	case DIALOG_TX_PRIORITY:
		if (bSet) {
			UI_DrawString(10, 48, "TX PRI: Busy", 12);
		} else {
			UI_DrawString(10, 48, "TX PRI: Edit", 12);
		}
		break;

	case DIALOG_VOX:
		if (bSet) {
			UI_DrawString(10, 48, "VOX: On ", 8);
		} else {
			UI_DrawString(10, 48, "VOX: Off", 8);
		}
		break;

	case DIALOG_TX_POWER:
		if (bSet) {
			UI_DrawString(10, 48, "Power: Low ", 11);
		} else {
			UI_DrawString(10, 48, "Power: High", 11);
		}
		break;

	case DIALOG_DUAL_STANDBY:
		if (bSet) {
			UI_DrawString(10, 48, "Dual Standby: On ", 17);
		} else {
			UI_DrawString(10, 48, "Dual Standby: Off", 17);
		}
		break;
	
	case DIALOG_KEY_BEEP:
		if (bSet) {
			UI_DrawString(10, 48, "Key Beep: On ", 13);
		} else {
			UI_DrawString(10, 48, "Key Beep: Off", 13);
		}
		break;

	case DIALOG_TOGGLE_SCANLIST:
		if (bSet) {
			UI_DrawString(10, 48, "SList: Ch Added", 15);
		} else {
			UI_DrawString(10, 48, "SList: Ch Removed", 17);
		}
		break;

	case DIALOG_PLEASE_CHARGE:
		UI_DrawString(10, 48, "Please Charge!", 14);
		break;

	case DIALOG_NO_CH_AVAILABLE:
		UI_DrawString(10, 48, "No CH Available", 15);
		break;
	}

	gRedrawScreen = true;
	VOX_Timer = 1200;
}

