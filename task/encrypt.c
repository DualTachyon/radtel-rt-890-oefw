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
#include "misc.h"
#include "radio/scheduler.h"
#include "task/encrypt.h"

void Task_Encrypt(void)
{
	if (gRadioMode == RADIO_MODE_TX) {
		if (gMainVfo->Encrypt >= 2 && !gMainVfo->bMuteEnabled) {
			if (ENCRYPT_Timer < 200) {
				return;
			}
			gCode ^= 0x0083;
			CSS_SetStandardCode(gVfoInfo[gCurrentVfo].CodeType, gCode, gMainVfo->Encrypt, gMainVfo->bIsNarrow);
		}
	}

	ENCRYPT_Timer = 0;
}

