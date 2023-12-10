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

#ifndef RADIO_SPECTRUM_H
#define RADIO_SPECTRUM_H

enum {
  STEPS_128,
  STEPS_64,
  STEPS_32,
  STEPS_16,
  STEPS_COUNT,
};

static const char StepStrings[][5] = {
	"0.25K",
	"1.25K",
	"2.5K ",
	"5K   ",
	"6.25K",
	"10K  ",
	"12.5K",
	"20K  ",
	"25K  ",
	"50K  ",
	"100K ",
	"500K ",
	"1M   ",
	"5M   "
};

static const char Mode[3][2] = {
	"FM",
	"AM",
	"SB",
};

#ifdef ENABLE_SPECTRUM_PRESETS
typedef struct FreqPreset {
  char Name[16];
  uint32_t StartFreq;
  uint32_t EndFreq;
  uint8_t StepCountIndex;
  uint8_t StepSizeIndex;
  uint8_t ModulationType; // 0=FM, 1=AM, 2=SSB
  uint8_t bNarrow; // 0 = Wide, 1 = Narrow
} FreqPreset;

static const FreqPreset FreqPresets[] = {
    {"16M BROADCAST", 1748000, 1790000, STEPS_128, 3, 1, 1},
    {"17M HAM BAND", 1806800, 1816800, STEPS_128, 1, 2, 1},
    {"15M BROADCAST", 1890000, 1902000, STEPS_128, 3, 1, 1},
    {"15M HAM BAND", 2100000, 2144990, STEPS_128, 1, 2, 1},
    {"13M BROADCAST", 2145000, 2185000, STEPS_128, 3, 1, 1},
    {"12M HAM BAND", 2489000, 2499000, STEPS_128, 1, 2, 1},
    {"11M BROADCAST", 2567000, 2610000, STEPS_128, 3, 1, 1},
    {"CB", 2697500, 2799990, STEPS_128, 3, 0, 1},
    {"10M HAM BAND", 2800000, 2970000, STEPS_128, 1, 2, 1},
    {"6M HAM BAND", 5000000, 5400000, STEPS_128, 1, 2, 1},
    {"AIR BAND VOICE", 11800000, 13500000, STEPS_128, 10, 1, 1},
    {"2M HAM BAND", 14400000, 14800000, STEPS_128, 8, 0, 0},
    {"RAILWAY", 15175000, 15599990, STEPS_128, 8, 0, 0},
    {"SEA", 15600000, 16327500, STEPS_128, 8, 0, 0},
    {"SATCOM", 24300000, 27000000, STEPS_128, 3, 0, 0},
    {"RIVER1", 30001250, 30051250, STEPS_64, 6, 0, 1},
    {"RIVER2", 33601250, 33651250, STEPS_64, 6, 0, 1},
    {"70CM HAM 1", 42000000, 43500000, STEPS_128, 10, 0, 0},
    {"70CM HAM 2", 43500000, 45000000, STEPS_128, 10, 0, 0},
    {"LPD", 43307500, 43477500, STEPS_128, 8, 0, 0},
    {"PMR", 44600625, 44620000, STEPS_32, 4, 0, 1},
    {"FRS-GMRS 462", 46256250, 46272500, STEPS_16, 6, 0, 1},
    {"FRS-GMRS 467", 46756250, 46771250, STEPS_16, 6, 0, 1},
    {"LORA WAN", 86400000, 86900000, STEPS_128, 10, 0, 0},
    {"GSM900 UP", 89000000, 91500000, STEPS_128, 10, 0, 0},
    {"GSM900 DOWN", 93500000, 96000000, STEPS_128, 10, 0, 0},
    {"23CM HAM BAND", 124000000, 130000000, STEPS_128, 8, 0, 0},
};
#endif

void APP_Spectrum(void);

#endif
