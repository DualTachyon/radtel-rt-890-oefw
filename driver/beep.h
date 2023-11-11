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

#ifndef DRIVER_BEEP_H
#define DRIVER_BEEP_H

#include <stdint.h>
#include <stdbool.h>

void BEEP_Interrupt(void);
void BEEP_Enable(void);
void BEEP_Disable(void);
void BEEP_Play(uint16_t Frequency, uint8_t Type, uint16_t Duration);
void BEEP_SetFrequency(uint16_t Frequency);
void BEEP_SetTone2Frequency(uint16_t Frequency);

#endif

