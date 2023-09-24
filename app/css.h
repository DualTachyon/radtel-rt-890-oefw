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

#ifndef APP_CSS_H
#define APP_CSS_H

#include <stdbool.h>
#include <stdint.h>

enum {
	CODE_TYPE_CTCSS = 0U,
	CODE_TYPE_DCS_N,
	CODE_TYPE_DCS_I,
	CODE_TYPE_OFF,
};

uint32_t CSS_CalculateGolay(uint32_t Code);
void CSS_SetCustomCode(bool bIs24Bit, uint16_t Code, bool bIsNarrow);
void CSS_SetStandardCode(uint8_t CodeType, uint16_t Code, uint8_t Encrypt, bool bNarrow);
uint16_t CSS_ConvertCode(uint16_t Code);
uint16_t CTCSS_GetOption(uint8_t Index);
uint16_t DCS_GetOption(uint8_t Index);

#endif

