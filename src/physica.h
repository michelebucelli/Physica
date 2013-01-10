//Physica main header
//Provides a game interface based on Bulk

#include "Bulk.h"//Includes Bulk main header
#include "Bulk_graphics.h"//Includes Bulk graphics
#include "Bulk_physics.h"//Includes Bulk physics

#include "SDL/SDL_mixer.h"//Includes SDL sound effects library

#define OBJTYPE_LEVEL		"level"//Level objects

#define BKG					SDL_FillRect(video, &video->clip_rect, background)//Background applying macro
#define DARK				boxColor(video, 0, 0, video_w, video_h, 0x0000007F)//Dark transparent fill
#define UPDATE				SDL_Flip(video)//Video updating macro

#define FRAME_BEGIN			frameBegin = SDL_GetTicks()//Frame beginning macro
#define FRAME_END			if (SDL_GetTicks() - frameBegin < 1000 / fps) SDL_Delay(1000 / fps - SDL_GetTicks() + frameBegin); frames++//Frame end macro

#define EVENTS_COMMON(E)	if (E.type == SDL_QUIT) running = false; if (E.type == SDL_VIDEORESIZE) resize(E.resize.w, E.resize.h, fullscreen)//Common events macro

#define PLAYSOUND(SND)		if (enableSfx) Mix_PlayChannel(-1, SND, 0)//Play sound macro

//Video output
SDL_Surface* video = NULL;//Video surface
int video_w = 800;//Video width
int video_h = 400;//Video height

int videoWin_w = 800;//Video width if windowed
int videoWin_h = 400;//Video height if windowed

bool fullscreen = true;//Fullscreen flag
Uint32 background = 0x101010;//Background color

//Program flow
bool running = true;//If true, program is running

int frames = 0;//Frame counter
int fps = 60;//Frames per second
int printFps = 45;//Printing frames per second
int frameBegin = 0;//Current frame beginning time

//Event handling and input
SDL_Event ev;//Global event
Uint8* keys = NULL;//Keys array

//Files
string settingsFile = "data/cfg/settings.cfg";//Global settings file
string sfxFile = "data/cfg/sfx.cfg";//Sound effects file
string levelsFile = "data/cfg/levelSets/levelSet_core.cfg";//Level set file

//Sound
bool enableSfx = true;//Enables sound

Mix_Chunk *clickSfx;//Click sound
Mix_Chunk *successSfx;//Success sound
Mix_Chunk *deathSfx;//Death sound

//Global user interface (needed in editor, can't be declared in ui.h)
string inputFile = "data/cfg/ui/input.cfg";//Input window file path
window input;//Input window
panel* inputFrame;//Input frame
control* inputPrompt;//Input prompt
inputBox* inputField;//Input field

//Misc
bool camFollow = false;//If true, camera will follow player

//Prototypes
void resize(int,int,bool);//Resizing function
string getInput(string);//Function to get input

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
	string bkg;//Background image file
	SDL_Surface* backgroundImage;//Level background image
	bool printLevelScene;//If true, prints the scene objects above the background
	
	int twoStarsTime, threeStarsTime;//Time required for two and three stars rating (in seconds)
	
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
		
		twoStarsTime = 0;
		threeStarsTime = 0;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (scene::fromScriptObj(o)){//If succeeded loading base data
			var* id = get <var> (&o.v, "id");//Id variable (when loaded directly from file)
			
			var* backgroundImage = get <var> (&o.v, "background");//Gets background
			var* print = get <var> (&o.v, "print");//Gets print flag
			var* twoStarsTime = get <var> (&o.v, "twoStarsTime");//Gets two stars time
			var* threeStarsTime = get <var> (&o.v, "threeStarsTime");//Gets three stars time
			
			if (id) this->id = id->value;//Gets id
			
			if (backgroundImage){ this->bkg = backgroundImage->value; this->backgroundImage = CACHEDSURFACE(bkg); }//Gets background image
			
			if (!this->backgroundImage)//If background wasn't found
				this->backgroundImage = SDL_CreateRGBSurface(SDL_SWSURFACE | SDL_SRCALPHA, w, h, 32, 0xFF000000, 0x00FF0000, 0x0000FF00, 0x000000FF);//Creates background
				
			if (print) this->printLevelScene = print->intValue();//Sets print flag
			if (twoStarsTime) this->twoStarsTime = twoStarsTime->intValue();//Gets two stars time
			if (threeStarsTime) this->threeStarsTime = threeStarsTime->intValue();//Gets three stars time
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = scene::toScriptObj();//Result object
		
		//Sets variables
		result.set("id", id);
		result.set("backgroudImage", bkg);
		result.set("print", printLevelScene);
		result.set("twoStarsTime", twoStarsTime);
		result.set("threeStarsTime", threeStarsTime);
		
		return result;//Returns result
	}
	
	//Function to print the level
	void print(SDL_Surface* target, int x, int y, bool hidden = false){
		SDL_Rect offset {x, y};//Offset rect
		SDL_BlitSurface(backgroundImage, NULL, target, &offset);//Prints background
		
		if (printLevelScene) printScene(this, target, x, y, hidden);//Prints scene elements
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
		
	void (*success)();//Success function
	
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
		
		success = NULL;
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
		
		releasedJump = true;
		playerJumps = 0;
		
		if (!death){//If not setting up cause death
			time = 0;//Resets timer
			deaths = 0;//Resets death counter
			lastFrameTime = SDL_GetTicks();//Resets frame time
		}
		
		paused = false;//Unpauses
	}
	
	//Function to reset current level
	void reset(){	
		setup(levelIndex, true);//Resets level
		deaths++;//Increases death counter
		
		PLAYSOUND(deathSfx);//Plays sound
		
		releasedJump = true;
		playerJumps = 0;
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
			if ((i->a == player && i->b == goal) || (i->a == goal && i->b == player)){ if (success) success(); break; }//Success if hit goal
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
		if (paused){ lastFrameTime = SDL_GetTicks(); return; }//Exits function if paused
		
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
	
	//Function to calculate rating (1 to 3, 0 for no level)
	int rating(){
		if (!currentLevel) return 0;
		
		if (time / 1000 + deaths < currentLevel->threeStarsTime) return 3;
		else if (time / 1000 + deaths < currentLevel->twoStarsTime) return 2;
		else return 1;
	}
} current;

#include "editor.h"//Includes editor
#include "ui.h"//Includes user interface header

//Sound file loading function
void loadSound(){
	fileData f (sfxFile);//Loads file
	object o = f.objGen("sfx");//Generates object
	
	//Gets data
	var* success = get <var> (&o.v, "success");
	var* death = get <var> (&o.v, "death");
	var* click = get <var> (&o.v, "click");
	
	if (success) successSfx = Mix_LoadWAV(success->value.c_str());
	if (death) deathSfx = Mix_LoadWAV(death->value.c_str());
	if (click) clickSfx = Mix_LoadWAV(click->value.c_str());
}

//Global settings file loading function
void loadSettings(){
	fileData f (settingsFile);//Source file
	object g = f.objGen("settings");//Generates object
	
	//Gets data
	var* v_fullscreen = get <var> (&g.v, "fullscreen");
	var* v_camFollow = get <var> (&g.v, "camFollow");
	var* v_videoW = get <var> (&g.v, "video_w");
	var* v_videoH = get <var> (&g.v, "video_h");
	var* v_sound = get <var> (&g.v, "enableSfx");
	
	if (v_fullscreen) fullscreen = v_fullscreen->intValue();
	if (v_camFollow) camFollow = v_camFollow->intValue();
	if (v_videoW) videoWin_w = v_videoW->intValue();
	if (v_videoH) videoWin_h = v_videoH->intValue();
	if (v_sound) enableSfx = v_sound->intValue();
	
	resize(videoWin_w, videoWin_h, fullscreen);//Resizes window
}

//Function to save settings
void saveSettings(){
	ofstream o (settingsFile.c_str());//Output file
	object ob ("", "settings");//Settings object
	
	//Sets settings data
	ob.set("video_w", videoWin_w);
	ob.set("video_h", videoWin_h);
	ob.set("fullscreen", fullscreen);
	ob.set("camFollow", camFollow);
	ob.set("enableSfx", enableSfx);
	
	o << ob.toString();//Outputs settings
	
	o.close();//Closes file
}

//Game initialization function
void gameInit(){
	Bulk_image_init();//Initializes Bulk image
	Bulk_ui_init();//Initializes Bulk user interface
	Bulk_physGraphics_init();//Initializes Bulk physics graphic functions

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);//Opens audio
	
	SDL_WM_SetCaption("Physica", "PHY");//Sets window caption
	
	current.loadLevelSet(levelsFile);//Loads core level set
	current.success = showSuccess;//Sets success function
	
	loadSettings();//Loads settings
	loadGraphics();//Loads graphics
	loadSound();//Loads sound
	loadUI();//Loads ui
	
	loadEditor();//Loads editor
	
	keys = SDL_GetKeyState(NULL);//Gets keys
}

//Game quitting function
void gameQuit(){
	saveSettings();//Saves settings
	
	while (Mix_Playing(-1)){}//Waits for all channels to finish playing
	
	Mix_CloseAudio();//Closes audio
	IMG_Quit();//Quits SDL image
	TTF_Quit();//Quits SDL ttf
	SDL_Quit();//Quits SDL
}