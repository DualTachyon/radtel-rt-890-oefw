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

#include <at32f421.h>
#include "app/fm.h"
#include "app/radio.h"
#include "ui/gfx.h"
#include "driver/audio.h"
#include "driver/bk4819.h"
#include "driver/delay.h"
#include "driver/pins.h"
#include "driver/speaker.h"
#include "driver/st7735s.h"
#include "dtmf.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/scheduler.h"
#include "radio/settings.h"
#include "task/screen.h"
#include "ui/helper.h"
#include "ui/vfo.h"
#include "unknown.h"

void SCREEN_TurnOn(void)
{
	if (gSettings.bEnableDisplay) {
		gEnableBlink = false;
		STANDBY_Counter = 0;
		gpio_bits_set(GPIOA, BOARD_GPIOA_LCD_RESX);
	}
}

void STANDBY_BlinkGreen(void)
{
	if (STANDBY_Counter > 5000) {
		STANDBY_Counter = 0;
		gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
		gBlinkGreen = 1;
		gGreenLedTimer = 0;
	}
	if (gBlinkGreen && gGreenLedTimer > 199) {
		if (!gScannerMode && gRadioMode != RADIO_MODE_RX) {
			gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
		}
		gBlinkGreen = false;
		gGreenLedTimer = 0;
	}
}

bool FUN_08006d64(void)
{
	if (gFskDataReceived) {
		gRedrawScreen = false;
		gFskDataReceived = false;
		return true;
	}

	return false;
}

