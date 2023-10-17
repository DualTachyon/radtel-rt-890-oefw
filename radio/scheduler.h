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

#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdbool.h>
#include <stdint.h>

enum {
	TASK_CHECK_PTT        = 0x0001U,
	TASK_CHECK_BATTERY    = 0x0002U,
	TASK_AM_FIX           = 0x0004U,
	TASK_SCANNER          = 0x0008U,
	TASK_1024_c           = 0x0010U,
	TASK_FM_SCANNER       = 0x0020U,
	TASK_CHECK_INCOMING   = 0x0040U,
	TASK_CHECK_RSSI       = 0x0080U,
	TASK_CHECK_KEY_PAD    = 0x0200U,
	TASK_CHECK_SIDE_KEYS  = 0x0400U,
	TASK_VOX              = 0x0800U,
};

extern uint32_t gPttTimeout;
extern uint16_t ENCRYPT_Timer;
extern uint32_t STANDBY_Counter;
extern uint32_t gTimeSinceBoot;
extern uint16_t gGreenLedTimer;

extern volatile uint16_t gSpecialTimer;
extern uint16_t VOX_Timer;
extern uint16_t gIncomingTimer;
extern uint16_t gBatteryTimer;
extern uint16_t gSaveModeTimer;
extern uint32_t gIdleTimer;
extern uint16_t gDetectorTimer;

void SCHEDULER_Init(void);
bool SCHEDULER_CheckTask(uint16_t Task);
void SCHEDULER_SetTask(uint16_t Task);
void SCHEDULER_ClearTask(uint16_t Task);

#endif

