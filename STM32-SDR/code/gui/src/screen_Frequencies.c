/*
 * Code for frequency manager and selection screen
 *
 * STM32-SDR: A software defined HAM radio embedded system.
 * Copyright (C) 2013, STM32-SDR Group
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "screen_All.h"
#include "widgets.h"
#include <assert.h>
#include "FrequencyManager.h"

// Used in this file to refer to the correct screen (helps to keep code copy-paste friendly.
static GL_Page_TypeDef *s_pThisScreen = &g_screenFrequencies;

// Store reference to lables to change them.
GL_PageControls_TypeDef* lblStatus1;
GL_PageControls_TypeDef* lblStatus2;

#define ID_FREQBTN_START     200
#define FIRST_BUTTON_Y       30
#define SPACE_PER_BUTTON_Y   30
#define LEFT_COL_X           0
#define COLUMN_WIDTH         110
#define BUTTONS_PER_COLUMN   5


/**
 * Call-back prototypes
 */
static void freqButton_Click(GL_PageControls_TypeDef* pThis);
static void store_Click(GL_PageControls_TypeDef* pThis);
static void done_Click(GL_PageControls_TypeDef* pThis);
static void defaults_Click(GL_PageControls_TypeDef* pThis);


/**
 * Create the screen
 */
void ScreenFrequencies_Create(void)
{
	Create_PageObj(s_pThisScreen);

	/*
	 * Create the UI widgets
	 */
	GL_PageControls_TypeDef* lblTitle = Widget_NewLabel("Frequencies", LCD_COLOR_WHITE, LCD_COLOR_BLACK, 0, GL_FONTOPTION_12x12, 0);
	GL_PageControls_TypeDef* lblDirections1 = Widget_NewLabel("Tap a band", LCD_COLOR_YELLOW, LCD_COLOR_BLACK, 0, GL_FONTOPTION_8x12Bold, 0);
	GL_PageControls_TypeDef* lblDirections2 = Widget_NewLabel("to select.", LCD_COLOR_YELLOW, LCD_COLOR_BLACK, 0, GL_FONTOPTION_8x12Bold, 0);
	lblStatus1 = Widget_NewLabel("Press & turn frequency knob", LCD_COLOR_WHITE, LCD_COLOR_BLACK, 0, GL_FONTOPTION_8x12Bold, 0);
	lblStatus2 = Widget_NewLabel("to set frequency step size.", LCD_COLOR_WHITE, LCD_COLOR_BLACK, 0, GL_FONTOPTION_8x12Bold, 0);


	GL_PageControls_TypeDef* btnFreqBigButton = Widget_NewBigButtonFrequency();
	GL_PageControls_TypeDef* btnStore = NewButton(0, " Store  ", store_Click);
	GL_PageControls_TypeDef* btnDefaults = NewButton(0, "Defaults", defaults_Click);
	GL_PageControls_TypeDef* btnDone = NewButton(0, "  Done  ", done_Click);


	/*
	 * Place the widgets onto the screen
	 */
	// Title
	AddPageControlObj(  0,   0, lblTitle,        s_pThisScreen);
	AddPageControlObj(200,   0, lblDirections1,  s_pThisScreen);
	AddPageControlObj(200,  10, lblDirections2,  s_pThisScreen);

	// Populate the frequency buttons:
	for (int i = 0; i < FREQBAND_NUMBER_OF_BANDS; i++) {
		GL_PageControls_TypeDef* btnFreq = NewButton(ID_FREQBTN_START + i, FrequencyManager_GetBandName(i), freqButton_Click);

		int x = (i / BUTTONS_PER_COLUMN) * COLUMN_WIDTH;
		int y = (i % BUTTONS_PER_COLUMN) * SPACE_PER_BUTTON_Y + FIRST_BUTTON_Y;
		AddPageControlObj(x, y, btnFreq, s_pThisScreen);
	}

	// Button Row
	AddPageControlObj(0,   LCD_HEIGHT - 54, lblStatus1,  s_pThisScreen);
	AddPageControlObj(0,   LCD_HEIGHT - 42, lblStatus2,  s_pThisScreen);
	AddPageControlObj(80,   LCD_HEIGHT - 30, btnStore, s_pThisScreen);
	AddPageControlObj(160,  LCD_HEIGHT - 30, btnDefaults, s_pThisScreen);
	AddPageControlObj(0, LCD_HEIGHT - 30, btnDone, s_pThisScreen);
	AddPageControlObj(
			LCD_WIDTH - ((GL_Custom_TypeDef*)(btnFreqBigButton->objPTR))->GetWidth(btnFreqBigButton),
			LCD_HEIGHT - ((GL_Custom_TypeDef*)(btnFreqBigButton->objPTR))->GetHeight(btnFreqBigButton),
			btnFreqBigButton, s_pThisScreen);
}

static void displayNormalFeedback(void)
{
	Widget_ChangeLabelColour(lblStatus1, LCD_COLOR_WHITE);
	Widget_ChangeLabelColour(lblStatus2, LCD_COLOR_WHITE);
	Widget_ChangeLabelText(lblStatus1, "Press & turn frequency knob ");
	Widget_ChangeLabelText(lblStatus2, "to set frequency step size. ");
}
static void displayStoreFeedback(void)
{
	Widget_ChangeLabelColour(lblStatus1, LCD_COLOR_YELLOW);
	Widget_ChangeLabelColour(lblStatus2, LCD_COLOR_YELLOW);
	Widget_ChangeLabelText(lblStatus1, "Frequencies for each band   ");
	Widget_ChangeLabelText(lblStatus2, "now saved in EEPROM.        ");
}
static void displayDefaultsFeedback(void)
{
	Widget_ChangeLabelColour(lblStatus1, LCD_COLOR_YELLOW);
	Widget_ChangeLabelColour(lblStatus2, LCD_COLOR_YELLOW);
	Widget_ChangeLabelText(lblStatus1, "Frequencies for each band   ");
	Widget_ChangeLabelText(lblStatus2, "now reset to default.       ");
}

/**
 * Callbacks
 */
static void freqButton_Click(GL_PageControls_TypeDef* pThis)
{
	uint16_t userMode = pThis->ID - ID_FREQBTN_START;
	assert(userMode >= 0 && userMode <= FREQBAND_NUMBER_OF_BANDS);

	FrequencyManager_SetSelectedBand(userMode);

	// Reset message to clear any saved/defaults message.
	displayNormalFeedback();
}

static void store_Click(GL_PageControls_TypeDef* pThis)
{
	displayStoreFeedback();
	FrequencyManager_WriteBandsToEeprom();
}

static void defaults_Click(GL_PageControls_TypeDef* pThis)
{
	displayDefaultsFeedback();
	FrequencyManager_ResetBandsToDefault();
}

static void done_Click(GL_PageControls_TypeDef* pThis) {
	// Reset to defaults in preparation for next display.
	displayNormalFeedback();
	Screen_ShowMainScreen();
}
