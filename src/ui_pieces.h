/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
UI INDIVIDUAL PIECES
and corresponding functions
*/

#ifndef _PIECES
#define _PIECES

#include "files.h"
#include "log.h"
#include "ui.h"
#include "viewport.h"
#include "game.h"

#include <cmath>

//Menu control
//Root control of all parts of UI (should be root of the below ones)
extern control ui_menu;

//Settings menu pieces
extern control* ui_settingsMenu;

extern control* ui_set_fullscreenCheck;
extern control* ui_set_sfxCheck;
extern control* ui_set_debugModeCheck;
extern control* ui_set_upKey;
extern control* ui_set_leftKey;
extern control* ui_set_rightKey;
extern control* ui_set_language;

//Level pack selection menu pieces
extern control* ui_lpselMenu;
extern control* ui_lp_button;

//Level selection menu pieces
extern control* ui_levelSelMenu;
extern control* ui_lev_button;

//Game hud
extern control* ui_game;
extern control* ui_gam_restartButton;
extern control* ui_gam_pauseButton;
extern control* ui_gam_timer;
extern control* ui_gam_deaths;

//Pause menu
extern control* ui_pause;
extern control* ui_pau_back;
extern control* ui_pau_resume;

//Level completed screen
extern control* ui_levelClear;
extern control* ui_lc_star1, *ui_lc_star2, *ui_lc_star3;
extern control* ui_lc_nextLevel;
extern control* ui_lc_back;

//Draw flags
//To avoid interference with event handling, some UI drawing
//(e.g. drawing level selector) is done at the end of the main loop.
//To decide when to draw them, we use the below flags
extern bool mustDraw_levelSel;
extern levelPack* selectedPack;//Selected level pack



//Misc functions
string timeToString(int);
void dark(CScriptVar*, void*);


//Function to init user interface
//Loads interface from XML and gets all controls
void initUI();

//Function to handle achievement unlock
void showUnlockedAchievement(achievement*);

//Functions to handle settings menu
void drawSettingsMenu();
void applySettingsMenu();

//Functions to handle level pack selector
void drawLevelPackSelect();
void levelPackClick(control*, eventData*);

//Functions to draw level selector
void drawLevelSelect();

//Functions to handle hud
void updateHud();

//Functions to handle level cleared screen
void lcShow();
void lcHide();
void lcNextClick(control*, eventData*);
void lcBackClick(control*, eventData*);

//Function that draws needed UI pieces
void redrawUI();

#endif
