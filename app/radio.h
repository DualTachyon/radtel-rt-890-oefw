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

#ifndef APP_RADIO_H
#define APP_RADIO_H

#include "radio/channels.h"
#include "radio/frequencies.h"

extern uint8_t gCurrentVfo;
extern ChannelInfo_t *gMainVfo;
extern ChannelInfo_t gVfoState[3];
extern FrequencyInfo_t gVfoInfo[2];
extern uint16_t gCurrentRssi[2];

extern bool gNoaaMode;
extern uint16_t gCode;

void RADIO_Init(void);
void RADIO_Tune(uint8_t Vfo);

void RADIO_StartRX(void);
void RADIO_EndRX(void);

void RADIO_StartAudio(void);
void RADIO_EndAudio(void);

void RADIO_Sleep(void);
void RADIO_Retune(void);
#ifdef ENABLE_NOAA
void RADIO_NoaaRetune(void);
void RADIO_NoaaTune(void);
#endif
void VFO_ClearCss(void);
void VFO_ClearMute(void);
void RADIO_SaveCurrentVfo(void);

void RADIO_StartTX(bool bFlag);
void RADIO_EndTX(void);

void RADIO_CancelMode(void);
void RADIO_DisableSaveMode(void);

void RADIO_DrawFmMode(void);
void RADIO_DrawWorkMode(void);
void RADIO_DrawFrequencyMode(void);

#endif

