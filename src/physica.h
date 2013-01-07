//Physica main header
//Provides a game interface based on Bulk

#include "Bulk.h"//Includes Bulk main header
#include "Bulk_graphics.h"//Includes Bulk graphics
#include "Bulk_physics.h"//Includes Bulk physics

#define OBJTYPE_LEVEL		"level"//Level objects

#define BKG					SDL_FillRect(video, &video->clip_rect, background)//Background applying macro
#define DARK				boxColor(video, 0, 0, video_w, video_h, 0x0000007F)//Dark transparent fill
#define UPDATE				SDL_Flip(video)//Video updating macro

#define FRAME_BEGIN			frameBegin = SDL_GetTicks()//Frame beginning macro
#define FRAME_END			if (SDL_GetTicks() - frameBegin < 1000 / fps) SDL_Delay(1000 / fps - SDL_GetTicks() + frameBegin); frames++//Frame end macro

//Video output
SDL_Surface* video = NULL;//Video surface
int video_w = 800;//Video width
int video_h = 400;//Video height

bool fullscreen = true;//Fullscreen flag

//Program flow
bool running = true;//If true, program is running

int frames = 0;//Frame counter
int fps = 60;//Frames per second
int printFps = 45;//Printing frames per second
int frameBegin = 0;//Current frame beginning time

//Event handling and input
SDL_Event ev;//Global event
Uint8* keys = NULL;//Keys array

//User interface
Uint32 background = 0x101010;//Background color

string themesFile = "data/cfg/ui/themes.cfg";//Themes file path

string hudFile = "data/cfg/ui/hud.cfg";//Hud file path
window hud;//Hud window
control *btnPause, *btnRestart;//Hud buttons
control *labDeaths, *labTime;//Hud labels

string menuFile = "data/cfg/ui/menu.cfg";//Main menu file path
window menu;//Menu window
panel *menuFrame;//Menu frame
control *btnPlay, *btnCredits, *btnQuit;//Menu buttons

string levelSelectFile = "data/cfg/ui/levels.cfg";//Level selection filepath
window levelSelect;//Level select window
control levelButton;//Level button
int levelSelect_spacing = 16;//Level selection spacing
int levelSelect_w = 800;//Level selection width

string pauseFile = "data/cfg/ui/pause.cfg";//Pause filepath
window pause;//Pause window
panel* pauseFrame;//Pause frame
control *btnResume, *btnBack;//Pause screen buttons

enum uiMode { ui_mainMenu, ui_levels, ui_paused, ui_game } curUiMode = ui_mainMenu;//Current UI mode

//Control scheme structure
struct controls {
	Uint8 up, left, down, right;//Movement keys
	Uint8 rollLeft, rollRight;//Rolling keys (use to rotate player)
	
	Uint8 action1, action2, action3;//Action keys
};

//Function to get an integer representing pressed controls
int controlsToInt(controls c, Uint8* keys){
	int result = 0;//Result
	
	//Sets result bits
	if (keys[c.up]) result += 0b10000000;
	if (keys[c.left]) result += 0b01000000;
	if (keys[c.down]) result += 0b00100000;
	if (keys[c.right]) result += 0b00010000;
	if (keys[c.rollLeft]) result += 0b0000100;
	if (keys[c.rollRight]) result += 0b0000010;
	
	return result;//Returns result
}

//Rules structure
struct rules {
	double jumpImpulse;//Module of jump impulse
	
	double groundSpeed;//Module of max speed when on the ground
	double groundForce;//Module of force applied when on the ground
	double groundDamping;//Extra damping factor when on the ground
	
	double airSpeed;//Module of speed when in the air
	double airForce;//Module of force applied when in air
	
	double rotateSpeed;//Rotation max speed
	double rotateForce;//Rotation force
	
	vector gravity;//Gravity vector
	
	int jumpCount;//Jumps counter
};

//Level class
//	adds a few stuff to the base scene class
class level: public scene {
	public:
	SDL_Surface* backgroundImage;//Level background image
	bool printLevelScene;//If true, prints the scene objects above the background
	
	//Constructor
	level(){
		id = "";
		type = OBJTYPE_LEVEL;
		
		damping_tr = 0;
		damping_rot = 0;
		
		w = 0;
		h = 0;
		
		backgroundImage = NULL;
		printLevelScene = true;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (scene::fromScriptObj(o)){//If succeeded loading base data
			var* backgroundImage = get <var> (&o.v, "background");//Gets background
			var* print = get <var> (&o.v, "print");//Gets print flag
			
			if (backgroundImage) this->backgroundImage = CACHEDSURFACE(backgroundImage->value);//Loads background image
			
			if (!this->backgroundImage)//If background wasn't found
				this->backgroundImage = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, w, h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);//Creates background
				
			if (print) this->printLevelScene = print->intValue();//Sets print flag
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to print the level
	void print(SDL_Surface* target, int x, int y){
		SDL_Rect offset {x, y};//Offset rect
		SDL_BlitSurface(backgroundImage, NULL, target, &offset);//Prints background
		
		if (printLevelScene) printScene(this, target, x, y);//Prints scene elements
	}
};

//Function to load a level from file
level* loadLevel(string path){
	fileData source (path);//Loads and processes file
	
	object s = source.objGen(path);//level object
	s.type = OBJTYPE_LEVEL;//Sets object typ
	
	level *result = new level;//Result level
	result->fromScriptObj(s);//Loads result
	
	return result;//Returns level
}

//Game class
class game {
	public:
	deque<string> levels;//Game level files
	level* currentLevel;//Current level
	int levelIndex;//Current level index
	
	entity* player;//Entity controlled by player
	entity* goal;//Goal entity
	controls playerControls;//Player controls
	
	rules gameRules;//Game rules
	
	bool releasedJump;//If true, released jump
	int playerJumps;//Jump counter
	
	vector playerStart;//Player starting position
	double playerAngle;//Player starting angle
	
	bool paused;//If true, frame will do nothing
	
	int lastFrameTime;//Last frame time
	
	int time;//Start time
	int deaths;//Death counter
		
	//Constructor
	game(){
		player = NULL;
		goal = NULL;
		
		playerControls = {SDLK_w, SDLK_a, SDLK_s, SDLK_d, SDLK_q, SDLK_e, SDLK_i, SDLK_o, SDLK_p};
		gameRules = {30, 30, 15, 0.8, 15, 7, 100, 25, {0,10}, 2};
		
		releasedJump = true;
		playerJumps = 0;
		
		playerStart = {0,0};
		playerAngle = 0;
		
		paused = false;
		lastFrameTime = 0;
		
		currentLevel = NULL;
		levelIndex = 0;
	}
	
	//Function for controls handling
	void handleControls(Uint8* keys){
		if (keys && player){//If keys array is valid and there's a player
			int lowestSensor = 0;//Lowest player sensor
			int i;//Counter
			
			for (i = 0; i < player->sensors.size(); i++)//For each sensor in player
				if (player->sensors[i]->y > player->sensors[lowestSensor]->y) lowestSensor = i;//Sets lowest sensor
			
			bool ground = player->checkSensor(lowestSensor);//True if touching the ground
			
			if (!keys[playerControls.up] || ground) releasedJump = true;//Resets released jump flag if jump is not pressed
			if (ground) playerJumps = 0;//Resets jump count
			if (!ground && playerJumps == 0) playerJumps = 1;//If on air, at least one jump
			
			if (keys[playerControls.up] && releasedJump && playerJumps < gameRules.jumpCount){//If pressed up and can jump
				player->speed.y = 0;//Stops on y
				player->applyImpulse(player->position, {0, -gameRules.jumpImpulse});//Applies impulse
				
				playerJumps++;//Increases jump count
				releasedJump = false;//Not released jump
			}
			
			if (keys[playerControls.right]){//If pressing right
				if (ground && player->speed.x < gameRules.groundSpeed)//If on ground and moving slower
					player->applyForce(player->position, {gameRules.groundForce, 0});//Applies force
					
				else if (!ground && player->speed.x < gameRules.airSpeed)//If on air and moving slower
					player->applyForce(player->position, {gameRules.airForce, 0});//Applies force
			}
			
			else if (keys[playerControls.left]){//If pressing left
				if (ground && player->speed.x > -gameRules.groundSpeed)//If on ground and moving slower
					player->applyForce(player->position, {-gameRules.groundForce, 0});//Applies force
					
				else if (!ground && player->speed.x > -gameRules.airSpeed)//If on air and moving slower
					player->applyForce(player->position, {-gameRules.airForce, 0});//Applies force
			}
			
			else if (ground) player->applyForce(player->position, {-player->speed.x * gameRules.groundDamping, 0});//Applies damping on ground
		}
	}
	
	//Function to print
	void print(SDL_Surface* target, int x, int y){
		if (currentLevel){//If current level is valid
			currentLevel->print(target, x, y);//Prints current level
			
			if (!currentLevel->printLevelScene){//If level scene wasn't printed
				if (player) printEntity(player, target, 0, x, y);//Prints player
				if (goal) printEntity(goal, target, 0, x, y);//Prints goal
			}
		}
	}
	
	//Function to load level set
	void loadLevelSet(string path){
		fileData f (path);//Source file
		object o = f.objGen("");//Generates object
		
		int i = 1;//Counter
		while (true){//Endless loop
			var* levelFile = get <var> (&o.v, "level" + toString(i));//Gets variable
			
			if (levelFile) levels.push_back(levelFile->value);//Adds level to game if found the variable
			else break;//Else exits loop
			
			i++;//Next level
		}
	}
	
	//Function to setup a level
	void setup(int levelIndex, bool death = false){
		if (currentLevel) delete currentLevel;//Deletes level if existing
		currentLevel = loadLevel(levels[levelIndex]);//Loads level
		
		this->levelIndex = levelIndex;//Sets level index
		
		player = get_ptr <entity> (&currentLevel->entities, "player");//Gets player
		goal = get_ptr <entity> (&currentLevel->entities, "goal");//Gets goal
		
		playerStart = player->position;//Sets player starting position
		playerAngle = player->theta;//Sets player starting angle
		
		time = 0;//Resets timer
		if (!death) deaths = 0;//Resets death counter
		
		paused = false;//Unpauses
	}
	
	//Function to reset current level
	void reset(){	
		setup(levelIndex, true);//Resets level
		
		deaths++;//Increases death counter
		time = 0;//Resets timer
	}
	
	//Function to move onto next level (if any)
	void next(){
		if (levelIndex < levels.size() - 1) setup(levelIndex + 1);//Sets up next level
	}
	
	//Function to check relevant collisions
	void checkRelevant(list<collision> c){
		list<collision>::iterator i;//Iterator
		
		for (i = c.begin(); i != c.end(); i++){//For each collision
			if ((i->a == player && i->b->special == "hazard") || (i->a->special == "hazard" && i->b == player)){ reset(); break; }//Resets level on death
			if ((i->a == player && i->b == goal) || (i->a == goal && i->b == player)){ next(); break; }//Next level if hit goal
		}
	}
	
	//Function to reset forces
	void resetForces(){
		if (currentLevel) currentLevel->resetForces();//Resets forces on level
	}
	
	//Function for time step
	void step(double t){
		if (currentLevel){//If level exists
			currentLevel->applyGravity(gameRules.gravity);//Applies gravity
			checkRelevant(currentLevel->step(t));//Steps level
		}
	}
	
	//Function to handle a frame
	void frame(double t, Uint8* keys){
		if (paused) return;//Exits function if paused
		
		step(t);//Steps
		
		resetForces();//Resets forces
		handleControls(keys);//Handles controls
		
		time += SDL_GetTicks() - lastFrameTime;
		lastFrameTime = SDL_GetTicks();
	}
	
	//Function for animation step
	void animStep(){
		if (currentLevel) currentLevel->animStep();//Steps animation
	}
} current;

//Function to handle pause click
void pauseClick(clickEventData data){
	current.paused = true;
	curUiMode = ui_paused;
}

//Function to handler reset click
void restartClick(clickEventData data){
	current.reset();
}

//Function to update hud with current game info
void updateHud(){
	int t = current.time;//Time lapsed since level beginning
	
	int t_min = floor (t / 1000 / 60);//Minutes
	int t_sec = int(floor (t / 1000)) % 60;//Seconds
	int t_hun = int(floor (t / 10)) % 100;//Hundreths of second
	
	labTime->content.t = (t_min < 10 ? "0" : "") + toString(t_min) + ":" + (t_sec < 10 ? "0" : "") + toString(t_sec) + ":" + (t_hun < 10 ? "0" : "") + toString(t_hun);//Sets timer
	
	labDeaths->content.t = (current.deaths < 10 ? "0" : "") + toString(current.deaths);//Sets deaths counter
}

//Function to handle play click
void playClick(clickEventData data){
	curUiMode = ui_levels;//Goes to level selector
}

//Function to handle quit click
void quitClick(clickEventData data){
	running = false;
}

//Function to handle level click
void levelClick(clickEventData data){
	current.setup(atoi(data.caller->id.c_str()));//Sets up level
	curUiMode = ui_game;//Starts game
}

//Function to handle resume click
void resumeClick(clickEventData data){
	curUiMode = ui_game;
}

//Function to handle back click
void backClick(clickEventData data){
	curUiMode = ui_levels;
}

//Game initialization function
void gameInit(){
	Bulk_image_init();//Initializes Bulk image
	Bulk_ui_init();//Initializes Bulk user interface
	Bulk_physGraphics_init();//Initializes Bulk physics graphic functions
	
	if (fullscreen){//If in fullscreen mode
		SDL_Rect best = *(SDL_ListModes(NULL, SDL_SWSURFACE | SDL_FULLSCREEN)[0]);//Best video mode
		
		//Gets video size
		video_w = best.w;
		video_h = best.h;
	}
	
	video = SDL_SetVideoMode(video_w, video_h, 32, SDL_SWSURFACE | (fullscreen ? SDL_FULLSCREEN : 0));//Creates video surface
	
	current.loadLevelSet("data/cfg/levelSets/levelSet_core.cfg");//Loads core level set
	
	keys = SDL_GetKeyState(NULL);//Gets keys
	
	loadThemesDB(themesFile);//Loads themes
	
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
	btnCredits = menu.getControl("frame.credits");//Gets credits button
	btnQuit = menu.getControl("frame.quit");//Gets quit button
	
	menuFrame->area.x = (video_w - menuFrame->area.w) / 2;//Centers menu on x
	menuFrame->area.y = (video_h - menuFrame->area.h) / 2;//Centers menu on y
	
	btnPlay->release.handlers.push_back(playClick);//Adds click handler to play
	btnQuit->release.handlers.push_back(quitClick);//Adds click handler to quit
	
	levelSelect = loadWindow(levelSelectFile, "levels");//Loads level selection window
	levelButton = *levelSelect.getControl("levelButton");//Sets default level button
	levelButton.release.handlers.push_back(levelClick);//Adds click handler to level button
	
	levelSelect.clear();//Clears level selection window
	
	int max = floor((levelSelect_w + levelSelect_spacing) / (levelButton.area.w + levelSelect_spacing));//Maximum number of buttons in a row
	int rows = ceil (current.levels.size() / max);//Rows needed
	int lsH = rows * levelButton.area.h + (rows - 1) * levelSelect_spacing;//Selector height
	int offsetY = (video_h - lsH) / 2;//Y offset
	
	int i;//Counter
	for (i = 0; i <= rows; i++){//For each row
		int n;//Counter
		int rowSize = current.levels.size() - i * max > max ? max : current.levels.size() - i * max;//Elements in row
		int rowW = rowSize * levelButton.area.w + (rowSize - 1) * levelSelect_spacing;//Row width
		int rowOffsetX = (video_w - rowW) / 2;//Row x offset
		
		for (n = 0; n < rowSize; n++){//For each element of the row
			control* c = new control;//New control
			*c = levelButton;//Sets control
			
			c->id = toString(i * max + n);//Sets id
			c->content.t = toString(i * max + n + 1);//Sets text
			
			c->area.x = rowOffsetX + n * (c->area.w + levelSelect_spacing);//Sets x
			c->area.y = offsetY + i * (c->area.h + levelSelect_spacing);//Sets y
			
			levelSelect.push_back(c);//Adds to controls
		}
	}
	
	pause = loadWindow(pauseFile, "pause");//Loads pause window
	pauseFrame = (panel*) pause.getControl("frame");//Gets frame panel
	btnResume = pause.getControl("frame.resume");//Gets resume button
	btnBack = pause.getControl("frame.back");//Gets back button
	
	pauseFrame->area.x = (video_w - pauseFrame->area.w) / 2;//Centers pause on x
	pauseFrame->area.y = (video_h - pauseFrame->area.h) / 2;//Centers pause on y
	
	btnResume->release.handlers.push_back(resumeClick);//Adds resume click handler
	btnBack->release.handlers.push_back(backClick);//Adds back click handler
}