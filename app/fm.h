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

#ifndef APP_FM_H
#define APP_FM_H

#include <stdbool.h>
#include <stdint.h>

enum FM_Mode_t {
	FM_MODE_OFF = 0,
	FM_MODE_STANDBY,
	FM_MODE_PLAY,
	FM_MODE_SCROLL_UP,
	FM_MODE_SCROLL_DOWN,
};

typedef enum FM_Mode_t FM_Mode_t;

extern FM_Mode_t gFM_Mode;

void FM_Play(void);
void FM_Resume(void);
void FM_UpdateFrequency(void);
void FM_Disable(bool bStandby);
void FM_Tune(uint16_t Frequency);
void FM_SetVolume(uint8_t Volume);
bool FM_CheckSignal(void);

#endif

