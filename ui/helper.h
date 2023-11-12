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

#ifndef UI_HELPER_H
#define UI_HELPER_H

#include <stdbool.h>
#include <stdint.h>

enum UI_Icon_t {
	ICON_LOCK       = 0x0008U,
	ICON_BELL       = 0x080AU,
	ICON_DUAL_WATCH = 0x1F11U,
	ICON_BATTERY    = 0x3013U,
	ICON_VOX        = 0x4318U,
	ICON_TR         = 0x660FU,
	ICON_RR         = 0x750FU,
};

typedef enum UI_Icon_t UI_Icon_t;

void UI_DrawString(uint8_t X, uint8_t Y, const char *String, uint8_t Size);
void UI_DrawSmallCharacter(uint8_t X, uint8_t Y, char Digit);
void UI_DrawDigits(const char *pDigits, uint8_t Vfo);
void UI_DrawSmallString(uint8_t X, uint8_t Y, const char *String, uint8_t Size);
void UI_DrawStatusIcon(uint8_t X, UI_Icon_t Icon, bool bDraw, uint16_t Color);
void UI_DrawRoger(void);
void UI_DrawVoltage(uint8_t Vfo);
void UI_DrawVfoFrame(uint8_t Y);
void UI_DrawName(uint8_t Vfo, const char *pName);
void UI_DrawExtra(uint8_t Mode, uint8_t gModulationType, uint8_t Vfo);
void UI_DrawFrequency(uint32_t Frequency, uint8_t Vfo, uint16_t Color);
void UI_DrawBigDigit(uint8_t X, uint8_t Y, uint8_t Digit);
void UI_DrawCss(uint8_t CodeType, uint16_t Code, uint8_t Encrypt, bool bMute, uint8_t Vfo);
void UI_DrawRxDBM(uint16_t RXdBM, bool isNeg, uint16_t len, uint8_t Vfo, bool Clear);
void UI_DrawTxPower(bool bIsLow, uint8_t Vfo);
void UI_DrawChannel(uint16_t Channel, uint8_t Vfo);
void UI_DrawRX(uint8_t Vfo);
void UI_DrawDTMF(void);
void UI_DrawFMFrequency(uint16_t Frequency);
void UI_DrawFM(void);
void UI_DrawBitmap(uint8_t X,uint8_t Y, uint8_t H, uint8_t W, const uint8_t *pBitmap);
void UI_DrawFrame(uint8_t X0, uint8_t X1, uint8_t Y0, uint8_t Y1, uint8_t Thickness, uint16_t Color);
void UI_DrawDialog(void);
void UI_DrawBar(uint8_t Level, uint8_t Vfo);
void UI_DrawSomething(void);
void UI_DrawMainBitmap(bool bOverride, uint8_t Vfo);
void UI_DrawSky(void);
void UI_DrawFrequencyEx(const char *String, uint8_t Vfo, bool bFlag);
void UI_DrawBootVoltage(uint8_t X, uint8_t Y);
void UI_DrawDecimal(const char *pInput);
void UI_DrawMenuPosition(const char *pString);
void UI_DrawStringSwitchType(void);
void UI_DrawRadar(void);
void UI_DrawGolay(void);
void UI_DrawChannelNumber(const char *pString);
void UI_DrawBand(void);
void UI_DrawScanFrequency(uint32_t Frequency);
void UI_DrawCtdcScan(void);
void UI_DrawCtcssCode(uint16_t Code);
void UI_DrawDcsCodeN(uint16_t Code);
void UI_DrawDTMFString(void);
void UI_DrawMuteInfo(bool bIs24Bit, uint32_t Golay);
void UI_DrawNone(void);

#endif

