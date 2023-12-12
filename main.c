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
#include "app/radio.h"
#include "app/uart.h"
#include "driver/bk4819.h"
#include "driver/crm.h"
#include "driver/delay.h"
#include "driver/key.h"
#include "driver/uart.h"
#include "helper/helper.h"
#include "misc.h"
#include "radio/data.h"
#include "radio/hardware.h"
#include "radio/settings.h"
#include "task/am-fix.h"
#include "task/alarm.h"
#include "task/battery.h"
#include "task/cursor.h"
#include "task/encrypt.h"
#ifdef ENABLE_FM_RADIO
	#include "task/fmscanner.h"
#endif
#include "task/idle.h"
#include "task/incoming.h"
#include "task/keys.h"
#include "task/lock.h"
#ifdef ENABLE_NOAA
	#include "task/noaa.h"
#endif
#include "task/ptt.h"
#include "task/rssi.h"
#include "task/scanner.h"
#include "task/screen.h"
#include "task/sidekeys.h"
#include "task/timeout.h"
#include "task/voice.h"
#include "task/vox.h"

extern const uint8_t StackVector[];

void Main(void) __attribute__((noreturn));

void _putchar(char c)
{
	UART_SendByte((uint8_t)c);
}

void Main(void)
{

	CRM_GetCoreClock();
	SCB->VTOR = (uint32_t)StackVector;
	DELAY_Init();
	DELAY_WaitMS(200);
	HARDWARE_Init();
	RADIO_Init();

	if (gSettings.DtmfState == DTMF_STATE_KILLED) {
		DATA_ReceiverInit();
	}
	while (1) {
		do {
			while (!UART_IsRunning && gSettings.DtmfState != DTMF_STATE_KILLED) {
				Task_VoicePlayer();
				Task_CheckKeyPad();
				Task_CheckSideKeys();
				Task_UpdateScreen();
				Task_BlinkCursor();
				#ifdef ENABLE_AM_FIX
				Task_AM_fix();
				#endif
				Task_Scanner();
				Task_CheckPTT();
				Task_CheckIncoming();
				Task_CheckRSSI();
				Task_CheckDisplayTimeout();
				Task_Encrypt();
				Task_CheckLockScreen();
				Task_VoxUpdate();
				Task_Idle();
				Task_CheckBattery();
#ifdef ENABLE_FM_RADIO
				Task_CheckScannerFM();
#endif
#ifdef ENABLE_NOAA
				Task_CheckNOAA();
#endif
				Task_LocalAlarm();
			}
		} while (gSettings.DtmfState != DTMF_STATE_KILLED);
		if (BK4819_ReadRegister(0x0C) & 0x0001U) {
			DATA_ReceiverCheck();
		}
		DELAY_WaitMS(1);
		STANDBY_BlinkGreen();
	}
}

