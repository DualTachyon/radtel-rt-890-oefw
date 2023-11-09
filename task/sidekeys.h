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

#ifndef TASK_SIDEKEYS_H
#define TASK_SIDEKEYS_H

enum {
	ACTION_NONE = 0,
	ACTION_MONITOR,
	ACTION_FREQUENCY_DETECT,
	ACTION_REPEATER_MODE,
	ACTION_PRESET_CHANNEL,
	ACTION_LOCAL_ALARM,
	ACTION_REMOTE_ALARM,
#ifdef ENABLE_NOAA
	ACTION_NOAA_CHANNEL,
#endif
	ACTION_SEND_TONE,
	ACTION_ROGER_BEEP,
	ACTION_FM_RADIO,
	ACTION_SCAN,
	ACTIONS_COUNT,	// used to count the number of actions, keep this last
};

void Task_CheckSideKeys(void);

#endif

