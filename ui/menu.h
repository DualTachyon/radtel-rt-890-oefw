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

#ifndef UI_MENU_H
#define UI_MENU_H

#include <stdbool.h>
#include <stdint.h>

void UI_DrawSettingOptionEx(const char *pString, uint8_t Length, uint8_t Index);
void UI_DrawSettingOption(const char *pString, uint8_t Index);
void UI_DrawSettingRoger(uint8_t Index);
void UI_DrawDtmfMode(uint8_t Index);
void UI_DrawDtmfSelect(uint8_t Index);
void UI_DrawToggle(void);
void UI_DrawSettingArrow(uint8_t Selection);
void UI_DrawDtmfInterval(uint8_t Interval);
void UI_DrawDtmfDelay(uint8_t Delay);
void UI_DrawActions(uint8_t Index);
void UI_DrawChannelName(uint16_t Channel);
void UI_DrawMute(uint8_t Index, uint16_t Golay, bool bEnabled);
void UI_DrawEncrypt(uint8_t Index);
void UI_DrawScrambler(uint8_t Index);
void UI_DrawActivateBy(void);
void UI_DrawCursor(uint8_t X, bool bVisible);
void UI_DrawTxPriority(void);
void UI_DrawFrequencyStep(uint8_t Index);
void UI_DrawTimer(uint8_t Index);
void UI_DrawLevel(uint8_t Index);
void UI_DrawScanDirection(void);
void UI_DrawDeviceName(const char *pName);
void UI_DrawSettingRepeaterMode(uint8_t Index);
void UI_DrawSettingTxPower(void);
void UI_DrawSettingModulation(uint8_t Index);
void UI_DrawSettingBandwidth(void);
void UI_DrawSettingBusyLock(uint8_t Index);
void UI_DrawSettingScanlist(uint8_t Index);
void UI_DrawSettingScanResume(uint8_t Index);

#endif

