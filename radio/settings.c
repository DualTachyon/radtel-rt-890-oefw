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
#include "driver/delay.h"
#include "driver/key.h"
#include "driver/pins.h"
#include "driver/serial-flash.h"
#include "helper/dtmf.h"
#include "misc.h"
#include "radio/hardware.h"
#include "radio/settings.h"
#include "task/keyaction.h"
#include "task/scanner.h"
#include "ui/gfx.h"
#include "ui/helper.h"

Calibration_t gCalibration;
char gDeviceName[16];
gSettings_t gSettings;
char WelcomeString[32];
uint32_t gFrequencyStep = 25;
gExtendedSettings_t gExtendedSettings;

static void RestoreCalibration(void)
{
	SFLASH_Read(gFlashBuffer, 0x3C0000, 0x1000);
	SFLASH_Update(gFlashBuffer, 0x3BF000, 0x1000);
}

void SETTINGS_BackupCalibration(void)
{
	SFLASH_Read(gFlashBuffer, 0x3BF000, 0x1000);
	SFLASH_Update(gFlashBuffer, 0x3C0000, 0x1000);
}

void SETTINGS_LoadCalibration(void)
{
	if (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1)) {
		if (KEY_GetButton() == KEY_EXIT) {
			gpio_bits_set(GPIOA, BOARD_GPIOA_LED_RED);
			gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
			RestoreCalibration();
			while (!gpio_input_data_bit_read(GPIOF, BOARD_GPIOF_KEY_SIDE1)) {
			}
			gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_RED);
			gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
			HARDWARE_Reboot();
		}
	}

	SFLASH_Read(&gCalibration, 0x3BF000, 0x20);
	if (gCalibration._0x00 != 0x9A) {
		gpio_bits_set(GPIOA, BOARD_GPIOA_LED_RED);
		gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
		while (1) {
		}
	}

	if (gCalibration._0x01 != 1) {
		if (gpio_input_data_bit_read(GPIOB, BOARD_GPIOB_KEY_PTT) || KEY_GetButton() != KEY_7) {
			return;
		}
	}

	gpio_bits_set(GPIOA, BOARD_GPIOA_LED_RED);
	gpio_bits_set(GPIOA, BOARD_GPIOA_LED_GREEN);
	gpio_bits_set(GPIOA, BOARD_GPIOA_LCD_RESX);

	while (1) {
		DISPLAY_Fill(0, 159, 0, 96, COLOR_RGB(0, 0, 0)); //Black
		DELAY_WaitMS(1000);
		DISPLAY_Fill(0, 159, 0, 96, COLOR_RGB(31, 63, 31)); //White
		DELAY_WaitMS(1000);
		DISPLAY_Fill(0, 159, 0, 96, COLOR_RGB(31, 0, 0)); //Red
		DELAY_WaitMS(1000);
		DISPLAY_Fill(0, 159, 0, 96, COLOR_RGB(0, 32, 0)); //Green
		DELAY_WaitMS(1000);
		DISPLAY_Fill(0, 159, 0, 96, COLOR_RGB(0, 0, 31)); //Blue
		DELAY_WaitMS(1000);
	}
}

void SETTINGS_LoadSettings(void)
{
	SFLASH_Read(WelcomeString, 0x3C1000, sizeof(WelcomeString));
	SFLASH_Read(gDeviceName, 0x3C1020, sizeof(gDeviceName));
	SFLASH_Read(&gSettings, 0x3C1030, sizeof(gSettings));
	SFLASH_Read(&gDTMF_Settings, 0x3C9D20, sizeof(gDTMF_Settings));
	SFLASH_Read(&gDTMF_Contacts, 0x3C9D30, sizeof(gDTMF_Contacts));
	SFLASH_Read(&gDTMF_Kill, 0x3C9E30, sizeof(gDTMF_Kill));
	SFLASH_Read(&gDTMF_Stun, 0x3C9E40, sizeof(gDTMF_Stun));
	SFLASH_Read(&gDTMF_Wake, 0x3C9E50, sizeof(gDTMF_Wake));
	// Extended Settings bits are all 1 at first read as the flash is full of 0xFF
	SFLASH_Read(&gExtendedSettings, 0x3D5000, sizeof(gExtendedSettings));

	if (gExtendedSettings.KeyShortcut[0] == 0xFF) {
		SetDefaultKeyShortcuts(false); //
	}

	gFrequencyStep = FREQUENCY_GetStep(gSettings.FrequencyStep);

	UI_SetColors(gExtendedSettings.DarkMode);

	gSettings.bEnableDisplay = 1;
	if (!gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_SIDE2)) {
		if (KEY_GetButton() == KEY_HASH) {
			gSettings.bFLock ^= 1;
			SETTINGS_SaveGlobals();
			do {
				while (!gpio_input_data_bit_read(GPIOA, BOARD_GPIOA_KEY_SIDE2)) {
				}
			} while (KEY_GetButton() == KEY_HASH);
			KEY_Side2Counter = 0;
			KEY_KeyCounter = 0;
		}
	}
}

void SETTINGS_SaveGlobals(void)
{
	SFLASH_Update(&gSettings, 0x3C1030, sizeof(gSettings));
	SFLASH_Update(&gExtendedSettings, 0x3D5000, sizeof(gExtendedSettings));
}

void SETTINGS_SaveState(void)
{
	gScannerMode = false;
	gpio_bits_reset(GPIOA, BOARD_GPIOA_LED_GREEN);
	SCANNER_Countdown = 0;
	if (gSettings.WorkMode) {
		SETTINGS_SaveGlobals();
	} else {
		CHANNELS_SaveChannel(gSettings.CurrentVfo ? 1000 : 999, &gVfoState[gSettings.CurrentVfo]);
	}
	UI_DrawScan();
}

void SETTINGS_SaveDTMF(void)
{
	SFLASH_Update(&gDTMF_Settings, 0x3C9D20, sizeof(gDTMF_Settings));
}

void SETTINGS_FactoryReset(void)
{
	uint8_t Lock;
	uint8_t i;

	Lock = gSettings.bFLock;
	for (i = 0; i < 10; i++) {
		SFLASH_Read(gFlashBuffer, 0x3CB000 + (i * 0x1000), 0x1000);
		SFLASH_Update(gFlashBuffer, 0x3C1000 + (i * 0x1000), 0x1000);
	}
	SFLASH_Read(&gSettings, 0x3C1030, sizeof(gSettings));
	gSettings.bFLock = Lock;
	SETTINGS_SaveGlobals();
}

void SETTINGS_SaveDeviceName(void)
{
	SFLASH_Update(gDeviceName, 0x3C1020, sizeof(gDeviceName));
}

void SETTINGS_BackupSettings(void)
{
	uint8_t i;

	for (i = 0; i < 10; i++) {
		SFLASH_Read(gFlashBuffer, 0x3C1000 + (i * 0x1000), 0x1000);
		SFLASH_Update(gFlashBuffer, 0x3CB000 + (i * 0x1000), 0x1000);
	}
}

