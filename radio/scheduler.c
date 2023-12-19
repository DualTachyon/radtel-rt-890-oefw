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

#include "app/uart.h"
#include "bsp/tmr.h"
#include "driver/audio.h"
#include "driver/beep.h"
#include "driver/key.h"
#include "misc.h"
#include "radio/scheduler.h"
#include "task/am-fix.h"
#include "task/alarm.h"
#include "task/cursor.h"
#include "task/lock.h"
#ifdef ENABLE_NOAA
	#include "task/noaa.h"
#endif
#include "task/scanner.h"
#include "task/vox.h"

static uint16_t SCHEDULER_Tasks;
static uint16_t SCHEDULER_Counter;

uint32_t gPttTimeout;
uint16_t ENCRYPT_Timer;
uint32_t STANDBY_Counter;
uint32_t gTimeSinceBoot;
uint16_t gGreenLedTimer;

volatile uint16_t gSpecialTimer;
uint16_t VOX_Timer;
uint16_t gIncomingTimer;
uint16_t gBatteryTimer;
uint16_t gSaveModeTimer;
uint32_t gIdleTimer;
uint16_t gDetectorTimer;

static void SetTask(uint16_t Task)
{
	SCHEDULER_Tasks |= Task;
}

bool SCHEDULER_CheckTask(uint16_t Task)
{
	return SCHEDULER_Tasks & Task;
}

void SCHEDULER_ClearTask(uint16_t Task)
{
	SCHEDULER_Tasks &= ~Task;
}

void SCHEDULER_Init(void)
{
	tmr_para_init_ex0_type init;

	tmr_para_init_ex0(&init);
	init.period = 999;
	init.division = 72;
	init.clock_division = TMR_CLOCK_DIV1;
	init.count_mode = TMR_COUNT_UP;
	tmr_reset_ex0(TMR1, &init);
	TMR1->ctrl1_bit.prben = TRUE;
	TMR1->iden |= TMR_OVF_INT;
	TMR1->ctrl1_bit.tmren = TRUE;
}

void HandlerTMR1_BRK_OVF_TRG_HALL(void)
{
	TMR1->ists = ~TMR_OVF_FLAG;

	KEY_ReadButtons();
	KEY_ReadSideKeys();
	BEEP_Interrupt();

	if (gSpecialTimer) {
		gSpecialTimer--;
	}
	if (gEnableLocalAlarm && !gSendTone) {
		gAlarmCounter++;
	}
	if (gAudioTimer) {
		gAudioTimer--;
	}
	if (VOX_Timer) {
		VOX_Timer--;
	}
	if (gCursorCountdown) {
		gCursorCountdown--;
	}
	#ifdef ENABLE_AM_FIX
	if (gAmFixCountdown) {
		gAmFixCountdown--;
	}
	#endif
	if (gIncomingTimer) {
		gIncomingTimer--;
	}
	if (gVoxRssiUpdateTimer) {
		gVoxRssiUpdateTimer--;
	}
	if (gBatteryTimer) {
		gBatteryTimer--;
	}
	#ifdef ENABLE_NOAA
	if (NOAA_NextChannelCountdown) {
		NOAA_NextChannelCountdown--;
	}
	#endif
	if (gSaveModeTimer) {
		gSaveModeTimer--;
	}
	if (gIdleTimer) {
		gIdleTimer--;
	}
	if (SCANNER_Countdown) {
		SCANNER_Countdown--;
	}
	if (gDetectorTimer) {
		gDetectorTimer--;
	}
	if (UART_Timer) {
		UART_Timer--;
	} else {
		if (UART_IsRunning) {
			UART_IsRunning = false;
		}
	}
	if (!VOX_IsTransmitting && gRadioMode == RADIO_MODE_TX) {
		gPttTimeout++;
	}
	gLockTimer++;
	SCHEDULER_Counter++;
	ENCRYPT_Timer++;
	STANDBY_Counter++;
	gTimeSinceBoot++;
	if (gBlinkGreen) {
		gGreenLedTimer++;
	}
	SetTask(TASK_CHECK_SIDE_KEYS | TASK_CHECK_KEY_PAD | TASK_CHECK_PTT | TASK_CHECK_INCOMING);
	if ((SCHEDULER_Counter & 1) == 0) {
	//	SetTask(TASK_CHECK_RSSI | TASK_CHECK_INCOMING);
		SetTask(TASK_CHECK_RSSI);
	}
	if ((SCHEDULER_Counter & 15) == 0) {
		// SetTask(TASK_VOX);
		SetTask(TASK_VOX | TASK_SCANNER);
	}
	//if ((SCHEDULER_Counter & 60) == 0) {
	//	SetTask(TASK_SCANNER);
	//}
	if ((SCHEDULER_Counter & 127) == 0) {
		//SetTask(TASK_FM_SCANNER | TASK_SCANNER);
		SetTask(TASK_FM_SCANNER);
	}
	if ((SCHEDULER_Counter & 0x3FF) == 0) {
		SetTask(TASK_1024_c | TASK_AM_FIX | TASK_CHECK_BATTERY);
		SCHEDULER_Counter = 0;
	}
}
