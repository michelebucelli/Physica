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

#include "ui_pieces.h"

//Main menu
//Contains all menu pieces (level selector, settings...)
control ui_menu;

control* ui_settingsMenu = NULL;

control* ui_set_fullscreenCheck = NULL;
control* ui_set_sfxCheck = NULL;
control* ui_set_debugModeCheck = NULL;
control* ui_set_upKey = NULL;
control* ui_set_leftKey = NULL;
control* ui_set_rightKey = NULL;

control* ui_lpselMenu = NULL;
control* ui_lp_button = NULL;

control* ui_levelSelMenu = NULL;
control* ui_lev_button = NULL;

control* ui_game = NULL;

control* ui_gam_pauseButton = NULL;
control* ui_gam_restartButton = NULL;
control* ui_gam_timer = NULL;
control* ui_gam_deaths = NULL;

control* ui_pause = NULL;
control* ui_pau_back = NULL;
control* ui_pau_resume = NULL;

control* ui_levelClear = NULL;
control* ui_lc_star1 = NULL, *ui_lc_star2 = NULL, *ui_lc_star3 = NULL;
control* ui_lc_deaths = NULL;
control* ui_lc_time = NULL;
control* ui_lc_nextLevel = NULL;
control* ui_lc_back = NULL;

control* ui_achievements = NULL;
control* ui_ach_single = NULL;

//Draw flags
bool mustDraw_levelPackSel = false;

bool mustDraw_levelSel = false;
levelPack* selectedPack = NULL;

//Function to darken the screen (used in pre-printing functions)
void dark(CScriptVar* c, void* userdata){
	SDL_Rect vp; VPRECT(&vp);
	boxColor(mainRenderer, 0, 0, vp.w, vp.h, 0x7F000000);
}

//UI initialization function
void initUI(){
	sc_easingsSetupScript = "importScript(\"(#easings)\");";
	SDL_SetRenderDrawBlendMode(mainRenderer, SDL_BLENDMODE_BLEND);
	
	ui_menu.load_file(FILE_MENU);
	
	ui_settingsMenu = ui_menu.getChild("settings");
	LOG_CHECK(ui_settingsMenu, "settings control not found");
	
	ui_set_fullscreenCheck = ui_settingsMenu->getChild("fullscreen");
	LOG_CHECK(ui_set_fullscreenCheck, "settings/fullscreen control not found");
	
	ui_set_sfxCheck = ui_settingsMenu->getChild("sfx");
	LOG_CHECK(ui_set_sfxCheck, "settings/sfx control not found");
	
	ui_set_debugModeCheck = ui_settingsMenu->getChild("debug");
	LOG_CHECK(ui_set_debugModeCheck, "settings/debug control not found");
	
	ui_set_upKey = ui_settingsMenu->getChild("upKey");
	LOG_CHECK(ui_set_upKey, "settings/upKey control not found");
	
	ui_set_leftKey = ui_settingsMenu->getChild("leftKey");
	LOG_CHECK(ui_set_leftKey, "settings/leftKey control not found");
	
	ui_set_rightKey = ui_settingsMenu->getChild("rightKey");
	LOG_CHECK(ui_set_rightKey, "settings/rightKey control not found");
	
	ui_lpselMenu = ui_menu.getChild("levelPackSelect");
	LOG_CHECK(ui_lpselMenu, "levelPackSelect control not found");
	
	ui_lp_button = ui_lpselMenu->getChild("button");
	LOG_CHECK(ui_lp_button, "levelPackSelect/button control not found");
	
	ui_lpselMenu->children.remove(ui_lp_button);
	ui_lp_button->events.push_back( event ("onMouseDown", levelPackClick) );
	
	ui_levelSelMenu = ui_menu.getChild("levelSelect");
	LOG_CHECK(ui_levelSelMenu, "levelSelect control not found");
	
	ui_lev_button = ui_levelSelMenu->getChild("button");
	LOG_CHECK(ui_lev_button, "levelSelect/button control not found");
	
	ui_levelSelMenu->children.remove(ui_lev_button);
	
	ui_game = ui_menu.getChild("game");
	LOG_CHECK(ui_game, "game control not found");
	
	ui_gam_timer = ui_game->getChild("timer");
	LOG_CHECK(ui_gam_timer, "game/timer control not found");
	
	ui_gam_deaths = ui_game->getChild("deaths");
	LOG_CHECK(ui_gam_deaths, "game/deaths control not found");
	
	ui_levelClear = ui_menu.getChild("successScreen");
	LOG_CHECK(ui_levelClear, "successScreen control not found");
	
	ui_lc_nextLevel = ui_levelClear->getChild("next");
	LOG_CHECK(ui_lc_nextLevel, "successScreen/next control not found");
	
	ui_lc_back = ui_levelClear->getChild("back");
	LOG_CHECK(ui_lc_back, "successScreen/back control not found");
	
	ui_lc_time = ui_levelClear->getChild("timer");
	LOG_CHECK(ui_lc_time, "successScreen/timer control not found");
	
	ui_lc_deaths = ui_levelClear->getChild("deaths");
	LOG_CHECK(ui_lc_deaths, "successScreen/deaths control not found");
	
	ui_lc_nextLevel->events.push_back( event("onMouseUp", lcNextClick) );
	
	ui_achievements = ui_menu.getChild("achievements");
	LOG_CHECK(ui_achievements, "achievements control not found");
	
	ui_ach_single = ui_achievements->getChild("achievement");
	LOG_CHECK(ui_ach_single, "achievements/achievement control not found");
	
	ui_achievements->children.remove(ui_ach_single);
	
	ui_achievements->events.push_back ( event("onDraw", achShow, -6) );
	
	drawSettingsMenu();
	drawLevelPackSelect();
}

//Function to handle achievement unlock
void showUnlockedAchievement ( achievement* a ) {
	eventData d;
	
	d.type = 2;
	d.data.custom.var = new CScriptVar;
	CScriptVar* v = new CScriptVar;
	a->icon.toJSVar(v);
	
	v->ref();//WHY??? Without it, segfault
	
	d.data.custom.var->addChild("icon", v);
	d.data.custom.var->addChild("title")->var->setString(a->name);
	d.data.custom.var->addChild("info")->var->setString(a->info);
	
	ui_menu.triggerEvent("onAchievementUnlocked", &d);
}

//Function to draw settings menu
void drawSettingsMenu(){
	(*ui_set_upKey)["storedKey"]->setInt(playerControls.up);
	(*ui_set_leftKey)["storedKey"]->setInt(playerControls.left);
	(*ui_set_rightKey)["storedKey"]->setInt(playerControls.right);
	
	ui_set_upKey->draw(); ui_set_leftKey->draw(); ui_set_rightKey->draw();
	
	ui_set_fullscreenCheck->cStatus = fullscreen ? cs_active : cs_normal;
}

//Function to apply settings changes
void applySettingsMenu(){
	if (fullscreen != (ui_set_fullscreenCheck->cStatus == cs_active)){
		fullscreen = ui_set_fullscreenCheck->cStatus == cs_active;
		SET_FULLSCREEN(fullscreen);
		
		SDL_SetRenderDrawBlendMode(mainRenderer, SDL_BLENDMODE_BLEND);
	}
	
	playerControls.up = (SDL_Scancode) (*ui_set_upKey)["storedKey"]->getInt();
	playerControls.left = (SDL_Scancode) (*ui_set_leftKey)["storedKey"]->getInt();
	playerControls.right = (SDL_Scancode) (*ui_set_rightKey)["storedKey"]->getInt();
}

//Function to draw level pack selector
void drawLevelPackSelect(){
	ui_lpselMenu->clear();
	
	double lpCount = levelPacks.size();
	int bpr = (*ui_lpselMenu)["buttonsPerRow"]->getInt();//Buttons per row (defined in the xml file)
	
	if (bpr <= 0) bpr = 4;
	
	int rows = ceil(lpCount / bpr);
	list<levelPack*>::iterator l = levelPacks.begin();
	
	int xOffset = -(bpr * ui_lp_button->area.w + (bpr - 1) * 6) / 2;
	int yOffset = -(rows * ui_lp_button->area.h + (rows - 1) * 6) / 2 + (rows % 2 == 1 ? ui_lp_button->area.h / 2 : 0);
	
	for (int i = 0; i < rows; i++){
		int rowY = yOffset + i * (ui_lp_button->area.h + 6);
		int buttonsLeft = lpCount - i * bpr;
		
		if (i == rows - 1) xOffset = -(buttonsLeft * ui_lp_button->area.w + (buttonsLeft - 1) * 6) / 2;
		
		for (int j = 0; j < bpr && l != levelPacks.end(); j++, l++){
			control *btn = ui_lp_button->copy();

			btn->id = (*l)->id;
			
			btn->area.xRef = 1;
			btn->area.yRef = 1;
			
			btn->area.x = xOffset + j * (ui_lp_button->area.w + 6) + btn->area.w / 2;
			btn->area.y = rowY;
			
			btn->getContent("icon")->data.img = renderableCopy(&(*l)->icon);
			btn->getContent("label")->data.text = new string ((*l)->name);
			
			(*btn)["packId"]->setString((*l)->id);//Pack identifier is stored within the script variable "packId" inside the button
			
			ui_lpselMenu->addChild(btn);
		}
	}
	
	ui_menu.linkScriptVar();
}

//Function to handle click on level pack selection button
void levelPackClick(control* caller, eventData* data){
	levelPack* pack = NULL;
	string packId = (*caller)["packId"]->getString();
	
	LOG("Opening pack " << packId);
	
	for (list<levelPack*>::iterator i = levelPacks.begin(); i != levelPacks.end(); i++)
		if ((*i)->id == packId) pack = *i;
		
	if (pack){
		pack->loadLevels();
		selectedPack = pack;
		mustDraw_levelSel = true;
		
		currentGame.currentLevelPack = pack;
	}
	
	else LOG_ERR("failed opening pack " << packId);
	
}

//Function to draw level selector
void drawLevelSelect(){	
	ui_levelSelMenu->clear();
	
	double levCount = selectedPack->levels.size();
	
	if (levCount <= 0) return;
	
	int bpr = (*ui_levelSelMenu)["buttonsPerRow"]->getInt();//Buttons per row (defined in the xml file)
	
	if (bpr <= 0) bpr = 4;
	
	int rows = ceil(levCount / bpr);
	int l = 0;
	
	int xOffset = -(bpr * ui_lev_button->area.w + (bpr - 1) * 6) / 2;
	int yOffset = -(rows * ui_lev_button->area.h + (rows - 1) * 6) / 2 + (rows % 2 == 1 ? ui_lev_button->area.h / 2 : 0);
	
	static int a = 0;
	
	for (int i = 0; i < rows; i++){
		int rowY = yOffset + i * (ui_lev_button->area.h + 6);
		int buttonsLeft = levCount - i * bpr;
		
		if (i == rows - 1) xOffset = -(buttonsLeft * ui_lev_button->area.w + (buttonsLeft - 1) * 6) / 2;
		
		for (int j = 0; j < bpr && l < levCount; j++, l++){
			control *btn = ui_lev_button->copy();
			btn->clearScriptVar();

			btn->id = selectedPack->levels[l]->id;
			
			btn->area.xRef = 1;
			btn->area.yRef = 1;
			
			btn->area.x = xOffset + j * (ui_lev_button->area.w + 6) + btn->area.w / 2;
			btn->area.y = rowY;
			
			btn->getContent("label")->data.text = new string (toString(l + 1));
			
			int rating = progress.getRating ( selectedPack->id + "." + selectedPack->levels[l]->id );
			
			if ( rating > 0 ) btn->getContent("star1")->data.img->fromJSVar ( (*btn) [ "starOn" ] );
			else btn->getContent("star1")->data.img->fromJSVar ( (*btn) [ "starOff" ] );
			
			if ( rating > 1 ) btn->getContent("star2")->data.img->fromJSVar ( (*btn) [ "starOn" ] );
			else btn->getContent("star2")->data.img->fromJSVar ( (*btn) [ "starOff" ] );
			
			if ( rating > 2 ) btn->getContent("star3")->data.img->fromJSVar ( (*btn) [ "starOn" ] );
			else btn->getContent("star3")->data.img->fromJSVar ( (*btn) [ "starOff" ] );
			
			(*btn)["levelIndex"]->setInt(l);//Level index is stored within the script variable "levelIndex" inside the button
			
			ui_levelSelMenu->addChild(btn);
			
			btn->genScriptVar();
		}
	}
	
	mustDraw_levelSel = false;
}

//Function to convert time to time string
string timeToString(int time){
	int minutes = floor(time / 60000);
	int seconds = int(floor(time / 1000)) % 60;
	int cents = int(floor(time / 10)) % 100;
	
	return 	(minutes < 10 ? "0" : "") + toString(minutes) + ":" +
			(seconds < 10 ? "0" : "") + toString(seconds) + ":" +
			(cents < 10 ? "0" : "") + toString(cents);
}

//Function to update the hud
void updateHud(){
	controlContent* timerText = ui_gam_timer->getContent("label");
	controlContent* deathsText = ui_gam_deaths->getContent("label");
	
	if (timerText){
		if (timerText->type == "text") *timerText->data.text = timeToString(currentGame.time);
		else timerText->data.text = new string(timeToString(currentGame.time));
	}
	
	if (deathsText){
		if (deathsText->type == "text") *deathsText->data.text = (currentGame.deaths < 10 ? "00" : (currentGame.deaths < 100 ? "0" : "")) + toString(currentGame.deaths);
		else deathsText->data.text = new string((currentGame.deaths < 10 ? "00" : (currentGame.deaths < 100 ? "0" : "")) + toString(currentGame.deaths));
	}
}

//Function to handle click on next button on level cleared screen
void lcNextClick(control* caller, eventData* data){
	bool b = currentGame.next();
	
	lcHide();
	
	if ( !b ) {
		ui_game->triggerEvent ( "onCompleted", NULL );
	}
}

//Function to handle back click on level cleared screen
void lcBackClick(control* caller, eventData* data){
}

//Function to show level cleared screen
void lcShow(){
	LOG("Level cleared. Time: " << timeToString(currentGame.time) << " - Deaths: " << currentGame.deaths << " - Rating: " << currentGame.rating());
	
	controlContent* timerText = ui_lc_time->getContent("label");
	controlContent* deathsText = ui_lc_deaths->getContent("label");
	
	string oldtext = *timerText->data.text;
	
	if (timerText){
		if (timerText->type == "text") *timerText->data.text = timeToString(currentGame.time);
		else timerText->data.text = new string(timeToString(currentGame.time));
	}
	
	if (deathsText){
		if (deathsText->type == "text") *deathsText->data.text = (currentGame.deaths < 10 ? "00" : (currentGame.deaths < 100 ? "0" : "")) + toString(currentGame.deaths);
		else deathsText->data.text = new string((currentGame.deaths < 10 ? "00" : (currentGame.deaths < 100 ? "0" : "")) + toString(currentGame.deaths));
	}
	
	ui_menu.clearScriptVar();
	ui_menu.genScriptVar();
	ui_menu.linkScriptVar();
	
	eventData d;
	d.type = 2;
	d.data.custom.var = new CScriptVar();
	d.data.custom.var->addChild("rating", new CScriptVar( currentGame.rating() ) );
	
	ui_game->triggerEvent("onSuccess", &d);
}

//Function to hide level cleared screen
void lcHide(){
	ui_game->triggerEvent("onNextLevel", NULL);
}

//Function to draw achievement screen
void achShow(control* c, eventData* d){
	ui_achievements->children.clear();
	
	if (progress.unlockedAch.size() == 0){
		control* ach = ui_ach_single->copy();
		ach->clearScriptVar();
		
		ach->area.xRef = 1;
		ach->area.yRef = 1;
		ach->area.x = 0;
		ach->area.y = 0;
		
		*ach->getContent("title")->data.text = "NO ACHIEVEMENTS UNLOCKED!";
		ach->getContent("icon")->data.img = NULL;
		
		ui_achievements->addChild(ach);
		
		ach->genScriptVar();
		ach->linkScriptVar();
		
		return;
	}
	
	int spacing = (*c)["spacing"]->getInt();
	int columns = (*c)["columns"]->getInt();
	
	int xOffset = -((ui_ach_single->area.w + spacing) * columns - spacing) / 2 + ui_ach_single->area.w / 2;
	int yOffset = -((ui_ach_single->area.h + spacing) * floor(progress.unlockedAch.size() / columns)) / 2 + ui_ach_single->area.h / 2;
	
	int col = 0, row = 0;
	
	if (progress.unlockedAch.size() < columns){
		xOffset = -((ui_ach_single->area.w + spacing) * (int(progress.unlockedAch.size()) - columns * row) - spacing) / 2 + ui_ach_single->area.w / 2;
	}
	
	LOG("Drawing " << progress.unlockedAch.size() << " achievements (" << xOffset << "," << yOffset << ")");
	
	for (deque<string>::iterator i = progress.unlockedAch.begin(); i != progress.unlockedAch.end(); i++) {
		achievement* a = NULL;
		int n = i->find(".");
		
		if (n != i->npos) {
			levelPack* p = getPack(i->substr(0, n));
			if (p) a = p->getAchievement(i->substr(n + 1));
			else continue;
		}
		
		else a = getAchievement(*i);
				
		if( !a ) continue;
		
		control* ach = ui_ach_single->copy();
		
		ach->clearScriptVar();
		
		ach->area.xRef = 1;
		ach->area.yRef = 1;
		ach->area.y = yOffset + row * (ach->area.h + spacing);
		ach->area.x = xOffset + col * (ach->area.w + spacing);
		
		ach->getContent("title")->data.text = &a->name;
		ach->getContent("info")->data.text = &a->info;		
		ach->getContent("icon")->data.img = new image(a->icon);
		
		ui_achievements->addChild(ach);
		
		ach->genScriptVar();
		ach->jsVar->ref();
		//ach->linkScriptVar();
				
		col++;
		if (col % columns == 0) {
			col = 0;
			row++;
			
			if (row == ceil(progress.unlockedAch.size() / columns)){
				xOffset = -((ui_ach_single->area.w + spacing) * (progress.unlockedAch.size() - columns * row) - spacing) / 2 + ui_ach_single->area.w / 2;
			}
		}
	}
}

//Function that draws needed ui pieces
void redrawUI(){
	if (mustDraw_levelSel) drawLevelSelect();
}
