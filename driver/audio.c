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

#include "bsp/tmr.h"
#include "driver/audio.h"
#include "driver/pwm.h"
#include "driver/serial-flash.h"
#include "driver/speaker.h"
#include "misc.h"
#include "radio/settings.h"

static bool bAudioSpeakerEnable;
static bool bPauseTimer;

static uint8_t g_Unused;

static uint32_t AudioEndPosition;
static uint32_t AudioFlashOffset;
static uint16_t SamplePreviousByte;
static uint16_t SampleCurrentByte;
static uint32_t SampleReadPosition;

uint16_t gAudioTimer;
bool gAudioPlaying;
uint8_t gAudioOffsetLast;
uint8_t gAudioOffsetIndex;

//

static void TimerStart(uint16_t SampleRate)
{
	tmr_para_init_ex0_type init;

	tmr_para_init_ex0(&init);
	init.period = 4000000 / SampleRate;
	init.division = 18;
	init.clock_division = TMR_CLOCK_DIV1;
	init.count_mode = TMR_COUNT_UP;
	tmr_reset_ex0(TMR6, &init);
	tmr_output_channel_switch_set_ex(TMR6, true);
	TMR6->iden |= TMR_OVF_INT;
	TMR6->ctrl1_bit.tmren = TRUE;
}

static uint32_t GetDigitAddress(uint8_t Digit)
{
	return 0x118000U + (Digit * 0x4000U);
}

static void PlayNumber(uint16_t Channel)
{
	if (gSettings.VoicePrompt && gSettings.WorkMode) {
		Channel++;
		if (Channel <= 20) {
			SFLASH_Offsets[0] = GetDigitAddress(Channel);
			gAudioOffsetLast = 1;
		} else if (Channel <= 99) {
			SFLASH_Offsets[0] = GetDigitAddress((Channel / 10) + 18);
			Channel %= 10;
			if (Channel == 0) {
				gAudioOffsetLast = 1;
			} else {
				SFLASH_Offsets[1] = GetDigitAddress(Channel);
				gAudioOffsetLast = 2;
			}
		} else {
			SFLASH_Offsets[0] = GetDigitAddress((Channel / 100) + 27);
			Channel %= 100;
			if (Channel == 0) {
				gAudioOffsetLast = 1;
			} else if (Channel <= 20) {
				SFLASH_Offsets[1] = GetDigitAddress(Channel);
				gAudioOffsetLast = 2;
			} else {
				SFLASH_Offsets[1] = GetDigitAddress((Channel / 10) + 18);
				Channel %= 10;
				if (Channel == 0) {
					gAudioOffsetLast = 2;
				} else {
					SFLASH_Offsets[2] = GetDigitAddress(Channel);
					gAudioOffsetLast = 3;
				}
			}
		}
		gAudioOffsetIndex = 0;
		AudioEndPosition = 0x4000;
	}
}

static void PlaySample(void)
{
	if (SPEAKER_State & SPEAKER_OWNER_SYSTEM) {
		return;
	}

	if (AudioEndPosition <= SampleReadPosition || gAudioTimer == 0) {
		gAudioPlaying = false;
		TMR6->ctrl1_bit.tmren = FALSE;
		return;
	}

	SampleCurrentByte = gFlashBuffer[SampleReadPosition];
	if (bAudioSpeakerEnable == false || gFlashBuffer[SampleReadPosition] != 0x80) {
		if (bAudioSpeakerEnable) {
			SPEAKER_TurnOn(SPEAKER_OWNER_VOICE);
		}
		bAudioSpeakerEnable = false;
		if (SampleCurrentByte == 0) {
			SampleReadPosition = AudioEndPosition;
			return;
		}
		if (SamplePreviousByte != SampleCurrentByte) {
			SamplePreviousByte = SampleCurrentByte;
			PWM_Pulse((gFlashBuffer[SampleReadPosition] * 165) / 50);
		}
		SampleReadPosition++;
		if ((SampleReadPosition & 0x1fff) == 0) {
			SampleReadPosition = 0;
			AudioFlashOffset += 0x2000;
			AudioEndPosition -= 0x2000;
			SFLASH_Read(gFlashBuffer, AudioFlashOffset, 0x2000);
		}
	} else {
		SampleReadPosition++;
	}
}

// Interrupt handler

void HandlerTMR6_GLOBAL(void)
{
	TMR6->ists = ~TMR_OVF_FLAG;
	if (gAudioPlaying) {
		PlaySample();
	}
}

//

void AUDIO_PlaySample(uint16_t SampleRate, uint32_t Offset)
{
	if (bPauseTimer) {
		TMR6->ctrl1_bit.tmren = FALSE;
	}
	gAudioPlaying = true;
	bAudioSpeakerEnable = true;
	AudioFlashOffset = Offset;
	SFLASH_Read(gFlashBuffer, Offset, 0x2000);
	AudioEndPosition = 0x4000;
	g_Unused = 0;
	SampleReadPosition = 0;
	SampleCurrentByte = 0;
	SamplePreviousByte = 0;
	TimerStart(SampleRate);
}

void AUDIO_PlaySampleOptional(uint8_t ID)
{
	if (gSettings.VoicePrompt) {
		AudioEndPosition = 0x4000;
		bPauseTimer = true;
		gAudioTimer = 3000;
		AUDIO_PlaySample(9375, ID << 14);
	}
}

void AUDIO_PlayChannelNumber(void)
{
	PlayNumber(gSettings.VfoChNo[gSettings.CurrentVfo]);
	gAudioTimer = 350;
}

void AUDIO_PlayDigit(uint8_t Digit)
{
	SFLASH_Offsets[0] = GetDigitAddress(Digit);
	gAudioOffsetLast = 1;
	gAudioOffsetIndex = 0;
	AudioEndPosition = 0x4000;
}

