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

#ifndef DRIVER_SPEAKER_H
#define DRIVER_SPEAKER_H

#include <stdint.h>

enum {
	SPEAKER_OWNER_VOICE = 0x01U,
	SPEAKER_OWNER_SYSTEM = 0x02U,
	SPEAKER_OWNER_FM = 0x04U,
	SPEAKER_OWNER_RX = 0x08U,
};

extern uint8_t SPEAKER_State;

void SPEAKER_TurnOn(uint8_t Owner);
void SPEAKER_TurnOff(uint8_t Owner);

#endif

