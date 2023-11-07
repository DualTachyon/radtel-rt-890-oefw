/* Copyright 2023 Reppad
 * https://github.com/reppad
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
#include "misc.h"
#include "ui/gfx.h"
#include "ui/main.h"

void FLASHLIGHT_Toggle(void)
{
	gFlashlightMode = !gFlashlightMode;
	if (gFlashlightMode) {
		DISPLAY_FillColor(COLOR_RGB(31, 63, 31));
	} else {
		DISPLAY_FillColor(COLOR_BACKGROUND);
		UI_DrawMain(false);
	}
}
