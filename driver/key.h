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

#ifndef DRIVER_KEY_H
#define DRIVER_KEY_H

#include <stdbool.h>
#include <stdint.h>

enum KEY_t {
	KEY_0    = 0U,
	KEY_1    = 1U,
	KEY_2    = 2U,
	KEY_3    = 3U,
	KEY_4    = 4U,
	KEY_5    = 5U,
	KEY_6    = 6U,
	KEY_7    = 7U,
	KEY_8    = 8U,
	KEY_9    = 9U,
	KEY_MENU = 10U,
	KEY_UP   = 11U,
	KEY_DOWN = 12U,
	KEY_EXIT = 13U,
	KEY_STAR = 14U,
	KEY_HASH = 15U,
	KEY_NONE = 16U,
};

typedef enum KEY_t KEY_t;

extern KEY_t KEY_CurrentKey;
extern uint16_t KEY_KeyCounter;
extern uint16_t KEY_Side1Counter;
extern uint16_t KEY_Side2Counter;
extern bool KEY_LongPressed;

KEY_t KEY_GetButton(void);
void KEY_ReadButtons(void);
void KEY_ReadSideKeys(void);

#endif

