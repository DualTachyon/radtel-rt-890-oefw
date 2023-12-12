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

#include <string.h>
#include "app/css.h"
#ifdef ENABLE_FM_RADIO
	#include "app/fm.h"
#endif
#include "app/radio.h"
#include "driver/audio.h"
#include "driver/key.h"
#include "driver/serial-flash.h"
#include "helper/inputbox.h"
#include "misc.h"
#include "radio/channels.h"
#include "radio/settings.h"
#include "ui/helper.h"
#ifdef ENABLE_NOAA
	#include "ui/noaa.h"
#endif
#include "ui/vfo.h"

static const ChannelInfo_t VfoTemplate[2] = {
	{
		.RX = { .Frequency = 43002500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 43002500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "----------",
	},
	{

		.RX = { .Frequency = 43502500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 43502500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x00,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = "----------",
	},
};

#ifdef ENABLE_NOAA
static const ChannelInfo_t gNoaaDefaultChannels[11] = {
	{
		.RX = { .Frequency = 16255000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16255000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	},
	{
		.RX = { .Frequency = 16240000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16240000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	},
	{
		.RX = { .Frequency = 16247500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16247500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	},
	{
		.RX = { .Frequency = 16242500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16242500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	},
	{
		.RX = { .Frequency = 16245000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16245000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	},
	{
		.RX = { .Frequency = 16250000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16250000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	},
	{
		.RX = { .Frequency = 16252500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16252500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	},
	{
		.RX = { .Frequency = 16165000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16165000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	},
	{
		.RX = { .Frequency = 16177500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16177500, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	},
	{
		.RX = { .Frequency = 16175000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16175000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	},
	{
		.RX = { .Frequency = 16200000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.TX = { .Frequency = 16200000, .Code = 0x000, .CodeType = CODE_TYPE_OFF, },
		.Golay = 0x000000,

		.Unknown0 = 0,
		.bIs24Bit = 0,
		.bMuteEnabled = 0,
		.Encrypt = 0,

		.Available = 0,
		.gModulationType = 0,
		.BCL = BUSY_LOCK_OFF,
		.ScanAdd = 1,
		.bIsLowPower = 0,
		.bIsNarrow = 0,

		._0x11 = 0x11,
		.Scramble = 0x00,
		.IsInscanList = 0xFF,
		._0x14 = 0xFF,
		._0x15 = 0xFF,
		.Name = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF },
	},
};
#endif

uint16_t gFreeChannelsCount;

bool CHANNELS_NextChannelMr(uint8_t Key, bool OnlyFromScanlist) {
	uint16_t startChannel = gSettings.VfoChNo[gSettings.CurrentVfo];
	do {
		if (Key == KEY_UP) {
			gSettings.VfoChNo[gSettings.CurrentVfo] = CHANNELS_GetChannelUp(gSettings.VfoChNo[gSettings.CurrentVfo], gSettings.CurrentVfo);
		} else {
			gSettings.VfoChNo[gSettings.CurrentVfo] = CHANNELS_GetChannelDown(gSettings.VfoChNo[gSettings.CurrentVfo], gSettings.CurrentVfo);
		}
		if (gSettings.VfoChNo[gSettings.CurrentVfo] == startChannel)
			return false;	// empty list
	} while (OnlyFromScanlist && !((gVfoState[gSettings.CurrentVfo].IsInscanList >> gExtendedSettings.CurrentScanList) & 1));
	RADIO_Tune(gSettings.CurrentVfo);
	UI_DrawVfo(gSettings.CurrentVfo);
	return true;
}

void CHANNELS_NextChannelVfo(uint8_t Key)
{
	ChannelInfo_t *pInfo = &gVfoState[gSettings.CurrentVfo];

	if (!gFrequencyReverse) {
		if (Key == KEY_UP) {
			pInfo->RX.Frequency += gFrequencyStep;
			if (gSettings.bFLock) {
				if (pInfo->RX.Frequency > 13600000 && pInfo->RX.Frequency < 14400000) {
					pInfo->RX.Frequency = 14400000;
				}
				if (pInfo->RX.Frequency > 14600000 && pInfo->RX.Frequency < 43000000) {
					pInfo->RX.Frequency = 43000000;
				}
				if (pInfo->RX.Frequency > 44000000) {
					pInfo->RX.Frequency = 10800000;
				}
			} else if (pInfo->RX.Frequency > 130000000) {
				pInfo->RX.Frequency = 130000000;
			}
		} else {
			pInfo->RX.Frequency -= gFrequencyStep;
			if (gSettings.bFLock) {
				if (pInfo->RX.Frequency < 10800000) {
					pInfo->RX.Frequency = 44000000;
				}
				if (pInfo->RX.Frequency > 13600000 && pInfo->RX.Frequency < 14400000) {
					pInfo->RX.Frequency = 13600000;
				}
				if (pInfo->RX.Frequency > 14600000 && pInfo->RX.Frequency < 43000000) {
					pInfo->RX.Frequency = 14600000;
				}
			} else if (pInfo->RX.Frequency < 1000000) {
				pInfo->RX.Frequency = 1000000;
			}
		}
		pInfo->TX.Frequency = pInfo->RX.Frequency;
		gVfoInfo[gSettings.CurrentVfo].Frequency = pInfo->RX.Frequency;
		// if (pInfo->RX.Frequency < 13600000) {
		// 	pInfo->gModulationType = 1;
		// } else {
		// 	pInfo->gModulationType = 0;
		// }
	} else {
		if (Key == KEY_UP) {
			pInfo->TX.Frequency += gFrequencyStep;
			if (gSettings.bFLock) {
				if (pInfo->TX.Frequency > 13600000 && pInfo->TX.Frequency < 14400000) {
					pInfo->TX.Frequency = 14400000;
				}
				if (pInfo->TX.Frequency > 14600000 && pInfo->TX.Frequency < 43000000) {
					pInfo->TX.Frequency = 43000000;
				}
				if (pInfo->TX.Frequency > 44000000) {
					pInfo->TX.Frequency = 10800000;
				}
			} else if (pInfo->TX.Frequency > 130000000) {
				pInfo->TX.Frequency = 130000000;
			}
		} else {
			pInfo->TX.Frequency -= gFrequencyStep;
			if (gSettings.bFLock) {
				if (pInfo->TX.Frequency < 10800000) {
					pInfo->TX.Frequency = 44000000;
				}
				if (pInfo->TX.Frequency > 13600000 && pInfo->TX.Frequency < 14400000) {
					pInfo->TX.Frequency = 13600000;
				}
				if (pInfo->TX.Frequency > 14600000 && pInfo->TX.Frequency < 43000000) {
					pInfo->TX.Frequency = 14600000;
				}
			} else if (pInfo->TX.Frequency < 1000000) {
				pInfo->TX.Frequency = 1000000;
			}
		}
		gVfoInfo[gSettings.CurrentVfo].Frequency = pInfo->TX.Frequency;
	}

	UI_DrawVfo(gSettings.CurrentVfo);
}

#ifdef ENABLE_NOAA
void CHANNELS_NextNOAA(uint8_t Key)
{
	if (gRadioMode == RADIO_MODE_RX) {
		RADIO_EndAudio();
	}
	if (Key == KEY_UP) {
		gNOAA_ChannelNow = (gNOAA_ChannelNow + 1) % 11;
	} else {
		gNOAA_ChannelNow = (gNOAA_ChannelNow + 10) % 11;
	}
	UI_DrawNOAA(gNOAA_ChannelNow);
	CHANNELS_SetNoaaChannel(gNOAA_ChannelNow);
	RADIO_Tune(2);
}
#endif

void CHANNELS_NextFM(uint8_t Key)
{
	if (Key == KEY_UP) {
		gSettings.FmFrequency++;
		if (gSettings.FmFrequency > 1080) {
			gSettings.FmFrequency = 640;
		}
	} else {
		gSettings.FmFrequency--;
		if (gSettings.FmFrequency < 640) {
			gSettings.FmFrequency = 1080;
		}
	}
	SETTINGS_SaveGlobals();
	UI_DrawFMFrequency(gSettings.FmFrequency);
#ifdef ENABLE_FM_RADIO
	FM_Play();
#endif
}

void CHANNELS_UpdateChannel(void)
{
	uint16_t Channel = 0;
	uint8_t i;

	for (i = 0; i < 3; i++) {
		Channel = (Channel * 10) + gInputBox[i];
	}
	gInputBoxWriteIndex = 0;
	INPUTBOX_Pad(0, 10);
	if (Channel != 0) {
		Channel--;
		if (!CHANNELS_LoadChannel(Channel, gSettings.CurrentVfo)) {
			if (gSettings.VfoChNo[gSettings.CurrentVfo] != Channel) {
				gSettings.VfoChNo[gSettings.CurrentVfo] = Channel;
				SETTINGS_SaveGlobals();
				RADIO_Tune(gSettings.CurrentVfo);
				UI_DrawVfo(gSettings.CurrentVfo);
			}
			return;
		}
	}
	CHANNELS_LoadChannel(gSettings.VfoChNo[gSettings.CurrentVfo], gSettings.CurrentVfo);
	RADIO_Tune(gSettings.CurrentVfo);
	UI_DrawVfo(gSettings.CurrentVfo);
}

void CHANNELS_UpdateVFO(void)
{
	uint32_t Frequency = 0;
	uint8_t i;

	RADIO_CancelMode();
	for (i = 0; i < 7; i++) {
		Frequency = (Frequency * 10) + gInputBox[i];
	}
	Frequency *= 10;

	gInputBoxWriteIndex = 0;
	INPUTBOX_Pad(0, 10);

	CHANNELS_UpdateVFOFreq(Frequency);
}

void CHANNELS_UpdateVFOFreq(uint32_t Frequency)
{
	if (
			(!gSettings.bFLock && (Frequency >= 1000000  && Frequency <= 130000000)) ||
			(gSettings.bFLock && (
					(Frequency >= 14400000 && Frequency <= 14600000) ||
					(Frequency >= 43000000 && Frequency <= 44000000) ||
					(Frequency >= 10800000 && Frequency <= 13600000)))
			) {
		if (!gFrequencyReverse) {
			gVfoState[gSettings.CurrentVfo].RX.Frequency = Frequency;
			// if (Frequency < 13600000) {
			// 	gVfoState[gSettings.CurrentVfo].gModulationType = 1;
			// } else {
			// 	gVfoState[gSettings.CurrentVfo].gModulationType = 0;
			// }
		}
		gVfoState[gSettings.CurrentVfo].TX.Frequency = Frequency;
		CHANNELS_SaveChannel(gSettings.CurrentVfo ? 1000 : 999, &gVfoState[gSettings.CurrentVfo]);
		CHANNELS_LoadChannel(gSettings.CurrentVfo ? 1000 : 999, gSettings.CurrentVfo);
		RADIO_Tune(gSettings.CurrentVfo);
	}
	UI_DrawVfo(gSettings.CurrentVfo);
}

bool CHANNELS_LoadChannel(uint16_t ChNo, uint8_t Vfo)
{
	uint32_t Frequency;

	SFLASH_Read(&gVfoState[Vfo], 0x3C2000 + (ChNo * sizeof(ChannelInfo_t)), sizeof(ChannelInfo_t));
	if (gSettings.bFLock) {
		Frequency = gVfoState[Vfo].RX.Frequency;
		if (Frequency > 44000000) {
			return true;
		}
		if (Frequency > 14600000 && Frequency < 43000000) {
			return true;
		}
		if (Frequency > 13600000 && Frequency < 14400000) {
			return true;
		}
		if (Frequency < 10800000) {
			return true;
		}
		Frequency = gVfoState[Vfo].TX.Frequency;
		if (Frequency > 44000000) {
			return true;
		}
		if (Frequency > 14600000 && Frequency < 43000000) {
			return true;
		}
		if (Frequency > 13600000 && Frequency < 14400000) {
			return true;
		}
		if (Frequency < 10800000) {
			return true;
		}
	}

	return gVfoState[Vfo].Available;
}

void CHANNELS_CheckFreeChannels(void)
{
	uint16_t i;

	gFreeChannelsCount = 0;
	for (i = 0; i < 999; i++) {
		if (!CHANNELS_LoadChannel(i, 0)) {
			gFreeChannelsCount++;
		}
	}
	if (gFreeChannelsCount == 0) {
		gSettings.WorkMode = 0;
		SETTINGS_SaveGlobals();
		CHANNELS_LoadVfoMode();
	}
}

void CHANNELS_LoadVfoMode(void)
{
	ChannelInfo_t VfoState[2];

	memcpy(&VfoState[0], &VfoTemplate, sizeof(VfoState));

	while (CHANNELS_LoadChannel(999, 0)) {
		SFLASH_Update(&VfoState[0], 0x3C9CE0, sizeof(VfoState[0]));
	}

	while (CHANNELS_LoadChannel(1000, 1)) {
		SFLASH_Update(&VfoState[1], 0x3C9D00, sizeof(VfoState[1]));
	}

	if (gSettings.CurrentVfo) {
		CHANNELS_LoadChannel(1000, 1);
	} else {
		CHANNELS_LoadChannel(999, 0);
	}
	if (gScreenMode == SCREEN_MAIN && gSettings.DtmfState != DTMF_STATE_KILLED) {
		AUDIO_PlaySampleOptional(0);
	}
}

void CHANNELS_LoadWorkMode(void)
{
	if (CHANNELS_LoadChannel(gSettings.VfoChNo[0], 0)) {
		gSettings.VfoChNo[0] = CHANNELS_GetChannelUp(gSettings.VfoChNo[0], 0);
		SETTINGS_SaveGlobals();
	}
	if (CHANNELS_LoadChannel(gSettings.VfoChNo[1], 1)) {
		gSettings.VfoChNo[1] = CHANNELS_GetChannelUp(gSettings.VfoChNo[1], 1);
		SETTINGS_SaveGlobals();
	}
	CHANNELS_LoadChannel(gSettings.VfoChNo[gSettings.CurrentVfo], gSettings.CurrentVfo);
	if (gScreenMode == SCREEN_MAIN && gSettings.DtmfState != DTMF_STATE_KILLED) {
		AUDIO_PlayChannelNumber();
	}
}

uint16_t CHANNELS_GetChannelUp(uint16_t Channel, uint8_t Vfo)
{
	Channel++;
	while (1) {
		Channel %= 999;
		if (!CHANNELS_LoadChannel(Channel, Vfo)) {
			break;
		}
		Channel++;
	}

	return Channel;
}

uint16_t CHANNELS_GetChannelDown(uint16_t Channel, uint8_t Vfo)
{
	Channel += 998;
	while (1) {
		Channel %= 999;
		if (!CHANNELS_LoadChannel(Channel, Vfo)) {
			break;
		}
		Channel += 998;
	}

	return Channel;
}

void CHANNELS_SaveChannel(uint16_t Channel, const ChannelInfo_t *pChannel)
{
	SFLASH_Update(pChannel, 0x3C2000 + (Channel * sizeof(*pChannel)), sizeof(*pChannel));
}

#ifdef ENABLE_NOAA
void CHANNELS_SetNoaaChannel(uint8_t Channel)
{
	gVfoState[2] = gNoaaDefaultChannels[Channel];
}
#endif

void CHANNELS_SaveVfo(void)
{
	if (gSettings.WorkMode) {
		CHANNELS_SaveChannel(gSettings.VfoChNo[gSettings.CurrentVfo], &gVfoState[gSettings.CurrentVfo]);
	} else {
		CHANNELS_SaveChannel(gSettings.CurrentVfo ? 1000 : 999, &gVfoState[gSettings.CurrentVfo]);
	}

	RADIO_Tune(gSettings.CurrentVfo);
}

