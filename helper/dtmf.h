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

#ifndef DTMF_H
#define DTMF_H

#include <stdbool.h>
#include <stdint.h>

enum DTMF_Mode_t {
	DTMF_MODE_OFF = 0,
	DTMF_MODE_TX_START,
	DTMF_MODE_TX_END,
	DTMF_MODE_TX_START_END,
};

typedef enum DTMF_Mode_t DTMF_Mode_t;

typedef struct {
	DTMF_Mode_t Delay;
	uint8_t Interval;
	uint8_t Mode;
	uint8_t Select;
	bool Display;
	uint8_t EncodeGain;
	uint8_t DecodeThreshold;
} DTMF_Settings_t;

typedef struct {
	char String[14];
	uint8_t Padding;
	uint8_t Length;
} DTMF_String_t;

extern DTMF_String_t gDTMF_Input;
extern char gDTMF_String[14];
extern DTMF_Settings_t gDTMF_Settings;
extern DTMF_String_t gDTMF_Contacts[16];
extern DTMF_String_t gDTMF_Kill;
extern DTMF_String_t gDTMF_Stun;
extern DTMF_String_t gDTMF_Wake;
extern bool gDTMF_Playing;

void DTMF_FSK_InitReceive(uint8_t Unused);
char DTMF_GetCharacterFromKey(uint8_t Key);
void DTMF_PlayTone(uint8_t Key);
void DTMF_PlayContact(const DTMF_String_t *pContact);
void DTMF_ResetString(void);
void DTMF_ClearString(void);
bool DTMF_strcmp(const DTMF_String_t *pDtmf, const char *pString);
void DTMF_Disable(void);

#endif

