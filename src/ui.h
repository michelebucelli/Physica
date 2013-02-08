//User interface functions header

//Files
string themesFile = "data/cfg/ui/themes.cfg";//Themes file path
string graphicsFile = "data/cfg/graphics.cfg";//Global graphics file

string commonFile = "data/cfg/ui/common.cfg";//Common ui file path

string hudFile = "data/cfg/ui/hud.cfg";//Hud file path
string menuFile = "data/cfg/ui/menu.cfg";//Main menu file path
string levelSetSelFile = "data/cfg/ui/levelSets.cfg";//Level set selection file path
string levelSelectFile = "data/cfg/ui/levels.cfg";//Level selection file path
string pauseFile = "data/cfg/ui/pause.cfg";//Pause filepath
string successFile = "data/cfg/ui/success.cfg";//Success file path
string settingsUiFile = "data/cfg/ui/settings.cfg";//Settings window file path
string creditsFile = "data/cfg/ui/credits.cfg";//Credits window file path
string achievedFile = "data/cfg/ui/achievement.cfg";//Achievement window file path
string achievementsUiFile = "data/cfg/ui/achievements.cfg";//Achievements window file path

string msgFile = "data/cfg/ui/msg.cfg";//Message window file path
string inputFile = "data/cfg/ui/input.cfg";//Input window file path
string imgPreviewFile = "data/cfg/ui/imagePreview.cfg";//Image preview file path
string imgInputFile = "data/cfg/ui/imageInput.cfg";//Image input file path
string achDialogFile = "data/cfg/ui/achDialog.cfg";//Achievement dialog file path
string rulesDialogFile = "data/cfg/ui/rulesDialog.cfg";//Rules dialog file path

//Global graphics
image starOn, starOff;//Star images
image starOn_sm, starOff_sm;//Small star images

//User interface
window hud;//Hud window
control *btnPause, *btnRestart;//Hud buttons
control *labDeaths, *labTime;//Hud labels

window menu;//Menu window
panel *menuFrame;//Menu frame
control *btnPlay, *btnEditor, *btnSettings, *btnAchs, *btnCredits, *btnQuit;//Menu buttons

window levelSetSel;//Level set selection window
panel levelSetButton;//Level set button panel
int levelSetSel_spacing = 16;//Level set selection spacing
int levelSetSel_w = 2;//Level set selection grid width

window levelSelect;//Level select window
panel levelButton;//Level button panel
control lockedButton;//Locked level button
int levelSelect_spacing = 16;//Level selection spacing
int levelSelect_w = 4;//Level selection grid width

window pauseWindow;//Pause window
panel* pauseFrame;//Pause frame
control *btnResume, *btnBack;//Pause screen buttons

window success;//Success window
panel* successFrame;//Success frame
control *btnNext, *btnSuccessBack;//Success window buttons
control *labSuccessTime, *labSuccessDeaths;//Success timer and deaths count
control *ratingA, *ratingB, *ratingC;//Rating stars

window settings;//Settings window
panel* settingsFrame;//Settings frame
checkBox *setFullscreen, *setCamFollow, *setSound, *setDebug;//Settings check boxes
keyBox *setUp, *setLeft, *setRight;//Controls key boxes
control *setUpdates;//Updates check button

window achievements;//Achievements window
panel defaultAch;//Achievement info panel
int achs_spacing = 16;//Grid spacing
int achs_w = 2;//Achievements grid width

window credits;//Credits window
control* creditsLabel;//Credits label

window achieved;//Achieved window
panel* achFrame;//Achieved frame
control* achIcon;//Achieved icon
control* achName;//Achieved name
control* achInfo;//Achieved info

int achBegin = -1;//Achieved window time counter
int achDuration = 5000;//Achieved window duration
int achSpeed = 1;//Pixels per frame of achievement window movement

//UI mode enumeration
enum uiMode {
	ui_mainMenu,//Main menu
	
	ui_levelSets,//Level set selection
	ui_levels,//Level selection
	
	ui_game,//Game view
	ui_paused,//Paused game
	ui_success,//Level completion
	
	ui_settings,//Settings window
	
	ui_achievements,//Achievements window
	
	ui_credits//Credits
} curUiMode = ui_mainMenu;//Current UI mode

//Function to convert a time in msec into a string
string timeToString(int t){
	int t_min = floor (t / 1000 / 60);//Minutes
	int t_sec = int(floor (t / 1000)) % 60;//Seconds
	int t_hun = int(floor (t / 10)) % 100;//Hundreths of second
	
	return (t_min < 10 ? "0" : "") + toString(t_min) + ":" + (t_sec < 10 ? "0" : "") + toString(t_sec) + ":" + (t_hun < 10 ? "0" : "") + toString(t_hun);//Sets timer
}

//Function to redraw level set selection window
void redrawLevelSetSel(){
	levelSetSel.clearControls();//Clears window
	
	int rows = ceil (levelSets.size() / levelSetSel_w);//Rows needed
	int lsH = rows * levelSetButton.area.h + (rows - 1) * levelSetSel_spacing;//Selector height
	int offsetY = (video_h - lsH) / 2;//Y offset
	
	if (rows % 2 != 0) offsetY -= levelSetButton.area.h / 2;//Adjusts y
	
	int i;//Counter
	for (i = 0; i <= rows; i++){//For each row
		int n;//Counter
		int rowSize = levelSets.size() - i * achs_w > achs_w ? achs_w : levelSets.size() - i * achs_w;//Elements in row
		int rowW = rowSize * levelSetButton.area.w + (rowSize - 1) * levelSetSel_spacing;//Row width
		int rowOffsetX = (video_w - rowW) / 2;//Row x offset
		
		for (n = 0; n < rowSize; n++){//For each element of the row
			panel* p = levelSetButton.copy();//New panel
				
			p->id = toString(i * levelSetSel_w + n);//Sets id
			
			levelSet* a = levelSets[i * i * levelSetSel_w + n];//Level set to show
			
			control* t = p->getControl("name");//Name control
			control* u = p->getControl("icon");//Icon control
			
			if (u && a->icon.valid()){//If icon was found
				if (t) t->content.t = a->name + " (" + toString(progress.percent(a->id)) + "%)";//Sets text
				u->content.i = a->icon;//Sets icon
				u->content.contentType = CONTENT_IMAGE;//Sets content type
			}
			
			else {
				if (t) t->content.t = "";//Empty name label
				p->content.t = a->name + " (" + toString(progress.percent(a->id)) + "%)";//Sets text
			}
			
			p->area.x = rowOffsetX + n * (p->area.w + levelSetSel_spacing);//Sets x
			p->area.y = offsetY + i * (p->area.h + levelSetSel_spacing);//Sets y
			
			levelSetSel.push_back(p);//Adds to controls
		}
	}
}

//Function to redraw level selection window
void redrawLevelSelect(){
	levelSelect.clearControls();//Clears level selection window
	
	int rows = ceil (current.levels.size() / levelSelect_w);//Rows needed
	int lsH = rows * levelButton.area.h + (rows - 1) * levelSelect_spacing;//Selector height
	int offsetY = (video_h - lsH) / 2;//Y offset
	
	if (rows % 2 != 0) offsetY -= levelButton.area.h / 2;//Adjusts y
	
	int i;//Counter
	for (i = 0; i <= rows; i++){//For each row
		int n;//Counter
		int rowSize = current.levels.size() - i * levelSelect_w > levelSelect_w ? levelSelect_w : current.levels.size() - i * levelSelect_w;//Elements in row
		int rowW = rowSize * levelButton.area.w + (rowSize - 1) * levelSelect_spacing;//Row width
		int rowOffsetX = (video_w - rowW) / 2;//Row x offset
		
		for (n = 0; n < rowSize; n++){//For each element of the row
			level *toLoad = loadLevel (current.levels[i * levelSelect_w + n]);//Loaded level
			bool play = toLoad && progress.canPlay(current.levels.id + "." + toLoad->id);
			
			if (play){
				panel* p = levelButton.copy();//New panel
				
				control *star1, *star2, *star3;//Star controls
				star1 = p->getControl("rating1"); star2 = p->getControl("rating2"); star3 = p->getControl("rating3");//Gets stars
				
				int rating = progress.getRating(current.levels.id + "." + toLoad->id);//Level rating
				
				//Sets stars
				if (star1) star1->content.i = rating >= 1 ? starOn_sm : starOff_sm;
				if (star2) star2->content.i = rating >= 2 ? starOn_sm : starOff_sm;
				if (star3) star3->content.i = rating >= 3 ? starOn_sm : starOff_sm;
				
				p->id = toString(i * levelSelect_w + n);//Sets id
				p->content.t = toString(i * levelSelect_w + n + 1);//Sets text
				
				p->area.x = rowOffsetX + n * (p->area.w + levelSelect_spacing);//Sets x
				p->area.y = offsetY + i * (p->area.h + levelSelect_spacing);//Sets y
				
				levelSelect.push_back(p);//Adds to controls
			}
			
			else {
				control* c = new control;//Creates new control
				*c = lockedButton;//Sets locked button
			
				c->id = toString(i * levelSelect_w + n);//Sets id
				
				c->area.x = rowOffsetX + n * (c->area.w + levelSelect_spacing);//Sets x
				c->area.y = offsetY + i * (c->area.h + levelSelect_spacing);//Sets y
				
				levelSelect.push_back(c);//Adds to controls
			}
			
			delete toLoad;//Frees level
		}
	}
}

//Function to redraw achievements window
void redrawAchievements(){
	achievements.clearControls();//Clears window
	
	int rows = ceil (progress.unlockedAch.size() / achs_w);//Rows needed
	int lsH = rows * defaultAch.area.h + (rows - 1) * achs_spacing;//Selector height
	int offsetY = (video_h - lsH) / 2 - defaultAch.area.h / 2;//Y offset
	
	int i;//Counter
	for (i = 0; i <= rows; i++){//For each row
		int n;//Counter
		int rowSize = progress.unlockedAch.size() - i * achs_w > achs_w ? achs_w : progress.unlockedAch.size() - i * achs_w;//Elements in row
		int rowW = rowSize * defaultAch.area.w + (rowSize - 1) * achs_spacing;//Row width
		int rowOffsetX = (video_w - rowW) / 2;//Row x offset
		
		for (n = 0; n < rowSize; n++){//For each element of the row
			panel* p = defaultAch.copy();//New panel
				
			p->id = toString(i * achs_w + n);//Sets id
			
			control* icon = p->getControl("icon");//Icon
			control* name = p->getControl("name");//Name
			control* info = p->getControl("info");//Info
			
			string achId = progress.unlockedAch[i * achs_w + n];//Achievement id
			achievement* a;//Achievement to show
			
			if (achId.find(".") != achId.npos){//If part of a level set
				levelSet* l = get_ptr <levelSet> (&levelSets, achId.substr(0, achId.find(".")));//Level set
				a = get <achievement> (&l->lsAchs, achId.substr(achId.find(".") + 1));//Gets achievement from level set
			}
			
			else a = get <achievement> (&achs, achId);//Else gets from global achievements
			
			if (icon){ icon->content.contentType = CONTENT_IMAGE; icon->content.i = a->icon; }//Sets icon
			if (name) name->content.t = a->name;//Sets name
			if (info) info->content.t = a->info;//Sets info
			
			p->area.x = rowOffsetX + n * (p->area.w + achs_spacing);//Sets x
			p->area.y = offsetY + i * (p->area.h + achs_spacing);//Sets y
			
			achievements.push_back(p);//Adds to controls
		}
	}
}

//Function to handle pause click
void pauseClick(clickEventData data){
	current.paused = true;
	curUiMode = ui_paused;
	
	PLAYSOUND(clickSfx);
}

//Function to handler reset click
void restartClick(clickEventData data){
	current.reset();
}

//Function to update hud with current game info
void updateHud(){
	labTime->content.t = timeToString(current.time);//Sets timer
	labDeaths->content.t = (current.deaths < 10 ? "0" : "") + toString(current.deaths);//Sets deaths counter
}

//Function to handle play click
void playClick(clickEventData data){
	curUiMode = ui_levelSets;//Goes to level set selector
	PLAYSOUND(clickSfx);//Plays sound
	
	redrawLevelSetSel();//Draws level set selection window
}

//Function to handle editor click
void editorClick(clickEventData data){
	lpEditing = true;//Sets editing flag
	lpEditorLoop();//Starts editor loop
}

//Function to handle achievements click
void achsClick(clickEventData data){
	curUiMode = ui_achievements;
	redrawAchievements();
}

//Function to handle quit click
void quitClick(clickEventData data){
	running = false;
	PLAYSOUND(clickSfx);//Plays sound
}

//Function to handle level set click
void levelSetClick(clickEventData data){
	curUiMode = ui_levels;//Goes to level selector
	PLAYSOUND(clickSfx);//Plays sound

	current.loadLevelSet(atoi(data.caller->id.c_str()));//Loads level set
	redrawLevelSelect();//Draws level select window
}

//Function to handle level click
void levelClick(clickEventData data){
	current.setup(atoi(data.caller->id.c_str()));//Sets up level
	curUiMode = ui_game;//Starts game
	PLAYSOUND(clickSfx);//Plays sound
}

//Function to handle resume click
void resumeClick(clickEventData data){
	curUiMode = ui_game;
	PLAYSOUND(clickSfx);//Plays sound
}

//Function to handle back click
void backClick(clickEventData data){
	curUiMode = ui_levels;
	PLAYSOUND(clickSfx);//Plays sound
	
	redrawLevelSelect();//Redraws level select ui
}

//Function to update success window
void updateSuccess(){
	labSuccessTime->content.t = timeToString(current.time);//Sets time label
	labSuccessDeaths->content.t = (current.deaths < 10 ? "0" : "") + toString(current.deaths);//Sets deaths counter
	
	int rating = current.rating();//Rating
	
	//Sets stars
	ratingA->content.i = rating >= 1 ? starOn : starOff;
	ratingB->content.i = rating >= 2 ? starOn : starOff;
	ratingC->content.i = rating >= 3 ? starOn : starOff;
}

//Function to show success window
void showSuccess(){
	current.paused = true;//Pauses game
	curUiMode = ui_success;//Shows success window
	
	PLAYSOUND(successSfx);//Plays sound
	
	if (!debugMode) progress.fillProgress(current.levels.id + "." + current.currentLevel->id, current.time, current.deaths, current.rating());//Fills progress data (if not in debug mode)
	
	if (current.levelIndex < current.levels.size() - 1 && !debugMode){//If level is not the last
		level* l = loadLevel(current.levels[current.levelIndex + 1]);//Loads level
		if (l) progress.unlock(current.levels.id + "." + l->id);//Unlocks it
	}
	
	updateSuccess();//Updates success window
}

//Function to handle next click
void nextClick(clickEventData data){
	if (current.levelIndex < current.levels.size() - 1){//If not on last level
		current.next();//Jumps to next level
		curUiMode = ui_game;//Sets game mode
		PLAYSOUND(clickSfx);//Plays sound
	}
	
	else backClick({});//Else goes back
}

//Function to handle credits click
void creditsClick(clickEventData data){
	curUiMode = ui_credits;
	PLAYSOUND(clickSfx);
}

//Standard click function (just plays sound)
void stdClick(clickEventData data){
	PLAYSOUND(clickSfx);
}

//Function to show settings window
void showSettings(clickEventData data){
	curUiMode = ui_settings;//Settings mode
	
	//Sets window content
	setFullscreen->checked = fullscreen;
	setCamFollow->checked = camFollow;
	setSound->checked = enableSfx;
	setDebug->checked = debugMode;
	
	setUp->key = playerControls.up;
	setRight->key = playerControls.right;
	setLeft->key = playerControls.left;
	
	PLAYSOUND(clickSfx);//Plays sound
}

//Unlock achievement function
void unlockedAchievement(achievement* a){
	achBegin = SDL_GetTicks();//Sets time beginning
	
	//Sets window parameters
	achIcon->content.i = a->icon;
	achName->content.t = a->name;
	achInfo->content.t = a->info;
}

//Function to resize video
void resize(int newW, int newH, bool fs, bool redraw){
	if (fs){//If setting fullscreen
		videoWin_w = video_w;//Saves video width in windowed mode
		videoWin_h = video_h;//Saves video height in windowed mode
	}
	
	else {//Else
		videoWin_w = newW;//Sets new windowed width
		videoWin_h = newH;//Sets new windowed height
	}
	
	//Sets size
	video_w = newW;
	video_h = newH;
	
	fullscreen = fs;//Sets fullscreen
	
	if (fullscreen){//If in fullscreen mode
		SDL_Rect best = *(SDL_ListModes(NULL, SDL_HWSURFACE | SDL_FULLSCREEN | SDL_DOUBLEBUF)[0]);//Best video mode
		
		//Gets video size
		video_w = best.w;
		video_h = best.h;
	}
	
	video = SDL_CreateRGBSurface(SDL_SWSURFACE, video_w, video_h, 32, 0, 0, 0, 0);
	actVideo = SDL_SetVideoMode(video_w, video_h, 32, SDL_HWSURFACE | SDL_DOUBLEBUF | (fullscreen ? SDL_FULLSCREEN : SDL_RESIZABLE));//Creates video surface
	
	if (redraw){//If has to redraw
		if (menuFrame){//If menu frame is available
			menuFrame->area.x = (video_w - menuFrame->area.w) / 2;//Re-centers on x
			menuFrame->area.y = (video_h - menuFrame->area.h) / 2;//Re-centers on y
		}
		
		if (pauseFrame){//If pause frame is available
			pauseFrame->area.x = (video_w - pauseFrame->area.w) / 2;//Re-centers on x
			pauseFrame->area.y = (video_h - pauseFrame->area.h) / 2;//Re-centers on y
		}
		
		if (successFrame){//If successFrame frame is available
			successFrame->area.x = (video_w - successFrame->area.w) / 2;//Re-centers on x
			successFrame->area.y = (video_h - successFrame->area.h) / 2;//Re-centers on y
		}
		
		if (settingsFrame){//If settingsFrame is available
			settingsFrame->area.x = (video_w - settingsFrame->area.w) / 2;//Centers settings on x
			settingsFrame->area.y = (video_h - settingsFrame->area.h) / 2;//Centers settings on y
		}
		
		if (creditsLabel){//If credits is available
			creditsLabel->area.x = (video_w - creditsLabel->area.w) / 2;//Centers settings on x
			creditsLabel->area.y = (video_h - creditsLabel->area.h) / 2;//Centers settings on y
		}
		
		if (fpsLabel){//If fps label is available
			fpsLabel->area.x = video_w - fpsLabel->area.w;//Positions
		}
		
		if (debugLabel){//If debug label is available
			debugLabel->area.x = video_w - debugLabel->area.w;//Positions
		}
		
		if (achFrame){
			achFrame->area.x = (video_w - achFrame->area.w) / 2;//Centers on x
		}
		
		if (curUiMode == ui_levels) redrawLevelSelect();//Redraws level selection window
		if (curUiMode == ui_achievements) redrawAchievements();//Redraws achievements
		
		msgBox.centre(video_w, video_h);//Centers message box
		inputBoxDialog.centre(video_w, video_h);//Centers input box
	}
}

//Function to handle click update
void updateClick(clickEventData data){
	PLAYSOUND(clickSfx);
	
	checkUpdates(false, true);//Checks for updates
}

//Function to apply settings
void applySettings(){
	if (setFullscreen->checked != fullscreen) resize(videoWin_w, videoWin_h, setFullscreen->checked);//Applies fullscreen
	camFollow = setCamFollow->checked;//Applies cam follow
	enableSfx = setSound->checked;//Applies sound settings
	debugMode = setDebug->checked;//Applies debug settings
	
	playerControls.up = setUp->key;
	playerControls.left = setLeft->key;
	playerControls.right = setRight->key;
}

//Function to update common UI
void updateCommon(){
	fpsLabel->content.t = "fps:" + toString(actualFps);//Sets fps label
	
	if (debugMode) debugLabel->content.t = "[DEBUG] video:" + toString(video_w) + "x" + toString(video_h) + ":" + toString(fullscreen);//Debug info
	else debugLabel->content.t = "";
}

//UI loading and setup function
void loadUI(){
	loadThemesDB(themesFile);//Loads themes
	
	common = loadWindow(commonFile, "common");//Loads common ui
	fpsLabel = common.getControl("fps");//Gets fps label
	debugLabel = common.getControl("debug");//Gets debug label
	
	fpsLabel->area.x = video_w - fpsLabel->area.w;//Positions fps label
	debugLabel->area.x = video_w - debugLabel->area.w;//Positions fps label
	
	hud = loadWindow(hudFile, "hud");//Loads hud
	btnPause = hud.getControl("pause");//Gets pause button
	btnRestart = hud.getControl("restart");//Gets restart button
	labDeaths = hud.getControl("deaths");//Gets deaths counter
	labTime = hud.getControl("timer");//Gets timer
	
	btnPause->release.handlers.push_back(pauseClick);//Adds click handler to pause
	btnRestart->release.handlers.push_back(restartClick);//Adds click handler to restart
	
	menu = loadWindow(menuFile, "menu");//Loads menu
	menuFrame = (panel*) menu.getControl("frame");//Gets frame
	btnPlay = menu.getControl("frame.play");//Gets play button
	btnEditor = menu.getControl("frame.editor");//Gets editor button
	btnSettings = menu.getControl("frame.settings");//Gets settings button
	btnAchs = menu.getControl("frame.achievements");//Gets achievements button
	btnCredits = menu.getControl("frame.credits");//Gets credits button
	btnQuit = menu.getControl("frame.quit");//Gets quit button
	
	menuFrame->area.x = (video_w - menuFrame->area.w) / 2;//Centers menu on x
	menuFrame->area.y = (video_h - menuFrame->area.h) / 2;//Centers menu on y
	
	btnPlay->release.handlers.push_back(playClick);//Adds click handler to play
	btnEditor->release.handlers.push_back(editorClick);//Adds click handler to editor
	btnAchs->release.handlers.push_back(achsClick);//Adds click handler to achievements
	btnSettings->release.handlers.push_back(showSettings);//Adds click handler to settings
	btnCredits->release.handlers.push_back(creditsClick);//Adds click handler to credits
	btnQuit->release.handlers.push_back(quitClick);//Adds click handler to quit
	
	levelSetSel = loadWindow(levelSetSelFile, "levelSets");//Loads level set selection window
	levelSetButton = * (panel*) levelSetSel.getControl("setButton");//Sets default button
	levelSetButton.release.handlers.push_back(levelSetClick);//Adds click handler
	
	levelSelect = loadWindow(levelSelectFile, "levels");//Loads level selection window
	levelButton = * (panel*) levelSelect.getControl("levelButton");//Sets default level button
	lockedButton = *levelSelect.getControl("lockedButton");//Gets default locked button
	levelButton.release.handlers.push_back(levelClick);//Adds click handler to level button
	
	pauseWindow = loadWindow(pauseFile, "pause");//Loads pause window
	pauseFrame = (panel*) pauseWindow.getControl("frame");//Gets frame panel
	btnResume = pauseWindow.getControl("frame.resume");//Gets resume button
	btnBack = pauseWindow.getControl("frame.back");//Gets back button
	
	pauseFrame->area.x = (video_w - pauseFrame->area.w) / 2;//Centers pause on x
	pauseFrame->area.y = (video_h - pauseFrame->area.h) / 2;//Centers pause on y
	
	btnResume->release.handlers.push_back(resumeClick);//Adds resume click handler
	btnBack->release.handlers.push_back(backClick);//Adds back click handler
	
	success = loadWindow(successFile, "success");//Loads success window
	successFrame = (panel*) success.getControl("frame");//Gets frame
	btnNext = success.getControl("frame.next");//Gets next button
	btnSuccessBack = success.getControl("frame.back");//Gets back button
	labSuccessTime = success.getControl("frame.time");//Gets time label
	labSuccessDeaths = success.getControl("frame.deaths");//Gets deaths label
	ratingA = success.getControl("frame.ratingA");//Gets first star
	ratingB = success.getControl("frame.ratingB");//Gets second star
	ratingC = success.getControl("frame.ratingC");//Gets third star
	
	successFrame->area.x = (video_w - successFrame->area.w) / 2;//Centers success on x
	successFrame->area.y = (video_h - successFrame->area.h) / 2;//Centers success on y
	
	btnSuccessBack->release.handlers.push_back(backClick);//Adds back click handler
	btnNext->release.handlers.push_back(nextClick);//Adds next click handler
	
	settings = loadWindow(settingsUiFile, "settings");//Loads settings file
	settingsFrame = (panel*) settings.getControl("frame");//Gets frame
	setFullscreen = (checkBox*) settings.getControl("frame.fullscreen");//Gets fullscreen checkbox
	setCamFollow = (checkBox*) settings.getControl("frame.camFollow");//Gets camera follow checkbox
	setSound = (checkBox*) settings.getControl("frame.enableSfx");//Gets sfx checkbox
	setDebug = (checkBox*) settings.getControl("frame.debug");//Gets debug checkbox
	setUp = (keyBox*) settings.getControl("frame.upKey");//Gets up key box
	setRight = (keyBox*) settings.getControl("frame.rightKey");//Gets right key box
	setLeft = (keyBox*) settings.getControl("frame.leftKey");//Gets left key box
	setUpdates = settings.getControl("frame.update");//Gets updates button
	
	settingsFrame->area.x = (video_w - settingsFrame->area.w) / 2;//Centers settings on x
	settingsFrame->area.y = (video_h - settingsFrame->area.h) / 2;//Centers settings on y
	
	setFullscreen->release.handlers.push_back(stdClick);//Adds click handler
	setCamFollow->release.handlers.push_back(stdClick);//Adds click handler
	setSound->release.handlers.push_back(stdClick);//Adds click handler
	setUpdates->release.handlers.push_back(updateClick);//Adds updates click handler
	
	credits = loadWindow(creditsFile, "credits");//Loads credits file
	creditsLabel = credits.getControl("credits");//Gets credits label
	
	creditsLabel->area.x = (video_w - creditsLabel->area.w) / 2;//Centers credits on x
	creditsLabel->area.y = (video_h - creditsLabel->area.h) / 2;//Centers credits on y
	
	achieved = loadWindow(achievedFile, "achieved");//Loads achieved window
	achFrame = (panel*) achieved.getControl("frame");//Gets frame
	achIcon = achieved.getControl("frame.icon");//Gets icon
	achName = achieved.getControl("frame.name");//Gets name
	achInfo = achieved.getControl("frame.info");//Gets info
	
	achFrame->area.x = (video_w - achFrame->area.w) / 2;//Centers on x
	achFrame->area.y = -achFrame->area.h;//Hides on y
	achIcon->content.contentType = CONTENT_IMAGE;//Sets content type
	
	achievements = loadWindow(achievementsUiFile, "achievements");//Loads achievements window
	defaultAch = * (panel*) achievements.getControl("defaultAchievement");//Gets achievement info
	
	msgBox.loadDialog(msgFile);//Loads message dialog
	inputBoxDialog.loadDialog(inputFile);//Loads input dialog
	imgPreview.loadDialog(imgPreviewFile);//Loads image preview
	imgInput.loadDialog(imgInputFile);//Loads image input
	achDialog.loadDialog(achDialogFile);//Loads ach dialog
	rulesDialog.loadDialog(rulesDialogFile);//Loads rules dialog
	
	//Sets dialog members
	msgBox.frameBegin = &frame_begin;
	msgBox.frameEnd = &frame_end;
	msgBox.events = &events_common;
	msgBox.quitFlag = &running;
	msgBox.update = &update;
	
	inputBoxDialog.frameBegin = &frame_begin;
	inputBoxDialog.frameEnd = &frame_end;
	inputBoxDialog.events = &events_common;
	inputBoxDialog.quitFlag = &running;
	inputBoxDialog.update = &update;
	
	imgPreview.frameBegin = &frame_begin;
	imgPreview.frameEnd = &frame_end;
	imgPreview.events = &events_common;
	imgPreview.quitFlag = &running;
	imgPreview.update = &update;
	
	imgInput.frameBegin = &frame_begin;
	imgInput.frameEnd = &frame_end;
	imgInput.events = &events_common;
	imgInput.quitFlag = &running;
	imgInput.update = &update;
	
	achDialog.frameBegin = &frame_begin;
	achDialog.frameEnd = &frame_end;
	achDialog.events = &events_common;
	achDialog.quitFlag = &running;
	achDialog.update = &update;
	
	rulesDialog.frameBegin = &frame_begin;
	rulesDialog.frameEnd = &frame_end;
	rulesDialog.events = &events_common;
	rulesDialog.quitFlag = &running;
	rulesDialog.update = &update;
}

//Graphics info file loading function
void loadGraphics(){
	fileData f (graphicsFile);//Source file
	object g = f.objGen("graphics");//Generates object
	
	//Gets data
	object* o_starOn = get <object> (&g.o, "starOn");
	object* o_starOff = get <object> (&g.o, "starOff");
	object* o_starOn_sm = get <object> (&g.o, "starOn_sm");
	object* o_starOff_sm = get <object> (&g.o, "starOff_sm");
	object* o_handle = get <object> (&g.o, "handle");
	
	if (o_starOn) starOn.fromScriptObj(*o_starOn);
	if (o_starOff) starOff.fromScriptObj(*o_starOff);
	if (o_starOn_sm) starOn_sm.fromScriptObj(*o_starOn_sm);
	if (o_starOff_sm) starOff_sm.fromScriptObj(*o_starOff_sm);
	if (o_handle) handle.fromScriptObj(*o_handle);
}