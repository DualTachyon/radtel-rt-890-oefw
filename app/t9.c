#include <string.h>
#include "app/menu.h"
#include "app/radio.h"
#include "app/t9.h"
#include "driver/key.h"
#include "misc.h"
#include "radio/settings.h"
#include "task/cursor.h"
#include "ui/gfx.h"
#include "ui/helper.h"
#include "ui/menu.h"

static char T9Char;

uint8_t T9WritePos;
uint8_t T9State;

static void MoveCursor(bool bForward, uint16_t Amount)
{
	UI_DrawCursor(gCursorPosition, false);

	if (bForward) {
		gCursorPosition += Amount;
	} else {
		gCursorPosition -= Amount;
	}

	UI_DrawCursor(gCursorPosition, true);
}

static void Star(void)
{
	uint8_t Pos;
	uint8_t i;

	Pos = gCursorPosition;
	if (Pos != 0) {
		if (Pos < 2 || (gBigString[Pos - 2] >> 4) != 15) {
			Pos--;
			for (; Pos < 39; Pos++) {
				gBigString[Pos] = gBigString[Pos + 1];
			}
			MoveCursor(false, 1);
			T9WritePos--;
		} else {
			Pos -= 2;
			for (; Pos < 38; Pos++) {
				gBigString[Pos] = gBigString[Pos + 2];
			}
			MoveCursor(false, 2);
			T9WritePos -= 2;
		}
		for (i = T9WritePos; i < sizeof(gBigString); i++) {
			gBigString[i] = ' ';
		}
		if (gMenuIndex == MENU_PERSONAL_ID) {
			UI_DrawDeviceName(gBigString);
		}
		if (gMenuIndex == MENU_CH_NAME) {
			UI_DrawString(4, 48, gBigString, 10);
		}
	}
}

static void UpDown(uint8_t WPos, uint8_t Key)
{
	if (WPos != 0) {
		if (Key == KEY_UP) {
			const uint8_t Pos = gCursorPosition;

			if (Pos) {
				if (Pos == 1) {
					MoveCursor(false, 1);
				} else if ((gBigString[Pos - 2] >> 4) == 15) {
					MoveCursor(false, 2);
				} else {
					MoveCursor(false, 1);
				}
			}
		} else if (gCursorPosition != WPos) {
			if ((gBigString[gCursorPosition] >> 4) == 15) {
				MoveCursor(true, 2);
			} else {
				MoveCursor(true, 1);
			}
		}
	}
}

static void UpDownEx(char c)
{
	char String[16];
	uint8_t i;

	memset(String, ' ', sizeof(String));
	for (i = 0; i < 16; i += 2) {
		String[i] = c + (i / 2) + ' ';
	}
	DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
	gColorForeground = COLOR_RED;
	UI_DrawString(4, 18, String, 15);
	gColorForeground = COLOR_FOREGROUND;
}

static void InsertChar(uint8_t Limit, char c)
{
	uint8_t i;

	if (T9WritePos + 1 <= Limit) {
		for (i = 39; (gCursorPosition + 1) <= i; i--) {
			gBigString[i] = gBigString[i - 1];
		}
		gBigString[gCursorPosition] = c;
		MoveCursor(true, 1);
		T9WritePos++;
		if (gMenuIndex == MENU_PERSONAL_ID) {
			UI_DrawDeviceName(gBigString);
		}
		if (gMenuIndex == MENU_CH_NAME) {
			UI_DrawString(4, 48, gBigString, 10);
		}
	}
}

static void DrawT9Sequence(char c)
{
	char String[16];

	memset(String, ' ', sizeof(String));
	String[0] = c;
	String[2] = c + ' ';
	String[4] = c + 1;
	String[6] = c + 1 + ' ';
	String[8] = c + 2;
	String[10]= c + 2 + ' ';
	if (c == 'P' || c == 'W') {
		String[12] = c + 3;
		String[14] = c + 3 + ' ';
	}
	DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
	gColorForeground = COLOR_RED;
	UI_DrawString(4, 18, String, 15);
	gColorForeground = COLOR_FOREGROUND;
}

void T9_Editor(uint8_t Key, uint8_t Limit)
{
	uint8_t i;
	char c;

	if (Key == KEY_0 && T9State != 0) {
		InsertChar(Limit, ' ');
		return;
	}

	if (Key == KEY_STAR) {
		Star();
		return;
	}

	switch (T9State) {
	case 0:
		switch (Key) {
		case KEY_MENU:
			if (gMenuIndex == MENU_PERSONAL_ID) {
				for (i = 0; i < sizeof(gDeviceName); i++) {
					gDeviceName[i] = gBigString[i];
				}
				SETTINGS_SaveDeviceName();
			} else if (gMenuIndex == MENU_CH_NAME) {
				for (i = 0; i < 10; i++) {
					gVfoState[gSettings.CurrentVfo].Name[i] = gBigString[i];
				}
				RADIO_SaveCurrentVfo();
			}
			// Fallthrough
		case KEY_EXIT:
			T9State = 0;
			T9WritePos = 0;
			gCursorPosition = 0;
			MENU_Redraw(true);
			break;

		case KEY_UP:
		case KEY_DOWN:
			UpDown(T9WritePos, Key);
			break;

		case KEY_HASH:
			DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
			T9State = 1;
			break;

		default:
			InsertChar(Limit, '0' + Key);
			break;
		}
		break;

	case 1:
		switch (Key) {
		case KEY_1:
			T9Char = 0;
			T9State = 2;
			UpDownEx(0);
			return;

		case KEY_2: T9Char = 'A'; break;
		case KEY_3: T9Char = 'D'; break;
		case KEY_4: T9Char = 'G'; break;
		case KEY_5: T9Char = 'J'; break;
		case KEY_6: T9Char = 'M'; break;
		case KEY_7: T9Char = 'P'; break;
		case KEY_8: T9Char = 'T'; break;
		case KEY_9: T9Char = 'W'; break;

		case KEY_UP:
		case KEY_DOWN:
			UpDown(T9WritePos, Key);
			return;

		case KEY_HASH:
			T9State = 0;
			UI_DrawActivateBy();
			return;
		default:
			break;
		}

		T9State = 3;
		DrawT9Sequence(T9Char);
		break;

	case 2:
		switch (Key) {
		case KEY_1: case KEY_2: case KEY_3:
		case KEY_4: case KEY_5: case KEY_6:
		case KEY_7: case KEY_8: case KEY_MENU:
			c = T9Char;
			if (Key != KEY_MENU) {
				c += Key - 1;
			}
			InsertChar(Limit, c + ' ');
			DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
			T9State = 1;
			break;

		case KEY_UP:
			T9Char = (T9Char + 'W') - ((T9Char + 'W') / 'W') * 'W';
			UpDownEx(T9Char);
			break;

		case KEY_DOWN:
			T9Char = (T9Char + 0x08) - ((T9Char + 0x08) / 'W') * 'W';
			UpDownEx(T9Char);
			break;

		case KEY_EXIT:
		case KEY_HASH:
			T9State = 0;
			UI_DrawActivateBy();
			break;
		default:
			break;
		}
		break;

	case 3:
		switch (Key) {
		case KEY_1:
		case KEY_MENU:
			InsertChar(Limit, T9Char);
			T9State = 1;
			DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
			break;

		case KEY_2:
			InsertChar(Limit, T9Char + ' ');
			T9State = 1;
			DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
			break;

		case KEY_3:
			InsertChar(Limit, T9Char + 0x01);
			T9State = 1;
			DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
			break;
		case KEY_4:
			InsertChar(Limit, T9Char + '!');
			T9State = 1;
			DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
			break;
		case KEY_5:
			InsertChar(Limit, T9Char + 0x02);
			T9State = 1;
			DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
			break;

		case KEY_6:
			InsertChar(Limit, T9Char + '"');
			T9State = 1;
			DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
			break;

		case KEY_7:
			if (T9Char == 'P' || T9Char == 'W') {
				InsertChar(Limit, T9Char + 3);
				T9State = 1;
				DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
			}
			break;

		case KEY_8:
			if (T9Char == 'P' || T9Char == 'W') {
				InsertChar(Limit, T9Char + '#');
				T9State = 1;
				DISPLAY_Fill(1, 158, 1, 19, gColorBackground);
			}
			break;

		case KEY_EXIT:
		case KEY_HASH:
			T9State = 0;
			UI_DrawActivateBy();
			break;

		default:
			break;
		}
		break;
	}
}

