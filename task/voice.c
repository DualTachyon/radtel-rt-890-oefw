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

#include "driver/audio.h"
#include "driver/pwm.h"
#include "driver/speaker.h"
#include "misc.h"
#include "radio/settings.h"
#include "task/voice.h"

void Task_VoicePlayer(void)
{
	uint8_t Index;

	if (gSettings.VoicePrompt && !gAudioPlaying && (SPEAKER_State & SPEAKER_OWNER_SYSTEM) == 0) {
		Index = gAudioOffsetIndex;
		if (Index < gAudioOffsetLast) {
			if (SFLASH_Offsets[Index] < 0x188000) {
				gAudioTimer = 700;
			} else {
				gAudioTimer = 900;
			}
			gAudioOffsetIndex++;
			AUDIO_PlaySample(9375, SFLASH_Offsets[Index]);
		} else if (SPEAKER_State & SPEAKER_OWNER_VOICE) {
			SPEAKER_TurnOff(SPEAKER_OWNER_VOICE);
			PWM_Reset();
		}
	}
}

