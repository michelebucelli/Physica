//Physica main header
//Provides a game interface based on Bulk

#include "Bulk.h"//Includes Bulk main header
#include "Bulk_graphics.h"//Includes Bulk graphics
#include "Bulk_physics.h"//Includes Bulk physics

#include "Bulk_expr_double.h"//Includes double expressions (for achievements)

#include "SDL/SDL_mixer.h"//Includes SDL sound effects library

#define OBJTYPE_LEVEL			"level"//Level objects
#define OBJTYPE_LEVELPROGRESS	"progress"//Level progress objects
#define OBJTYPE_GLOBALPROGRESS	"gProgress"//Global progress objects
#define OBJTYPE_ACHIEVEMENT		"achievement"//Achievement objects
#define OBJTYPE_RULES			"rules"//Rules objects

#define BKG						SDL_FillRect(video, &video->clip_rect, background)//Background applying macro
#define DARK					boxColor(video, 0, 0, video_w, video_h, 0x0000007F)//Dark transparent fill
#define UPDATE					SDL_Flip(video)//Video updating macro

#define FRAME_BEGIN				frameBegin = SDL_GetTicks()//Frame beginning macro
#define FRAME_END				if (SDL_GetTicks() > frameBegin) actualFps = 1000 / (SDL_GetTicks() - frameBegin); else actualFps = 1000; if (SDL_GetTicks() - frameBegin < 1000 / fps) SDL_Delay(1000 / fps - SDL_GetTicks() + frameBegin); frames++//Frame end macro

#define EVENTS_COMMON(E)		if (E.type == SDL_QUIT) running = false; if (E.type == SDL_VIDEORESIZE) resize(E.resize.w, E.resize.h, fullscreen)//Common events macro

#define PLAYSOUND(SND)			if (enableSfx) Mix_PlayChannel(-1, SND, 0)//Play sound macro

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
int frameBegin = 0;//Current frame beginning time
int actualFps = 0;//Actual fps

//Event handling and input
SDL_Event ev;//Global event
Uint8* keys = NULL;//Keys array

//Files
string settingsFile = "data/cfg/settings.cfg";//Global settings file
string sfxFile = "data/cfg/sfx.cfg";//Sound effects file
string levelsFile = "data/cfg/levels/levelSet_core.cfg";//Level set file
string progressFile = "data/cfg/progress.cfg";//Progress file
string achievementsFile = "data/cfg/achievements.cfg";//Achievements file
string rulesFile = "data/cfg/rules.cfg";//Rules file

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
bool debugMode = false;//Debug mode flag (all levels unlocked if true)
bool camFollow = false;//If true, camera will follow player

//Prototypes
void resize(int,int,bool,bool = true);//Resizing function
string getInput(string);//Function to get input

//Control scheme structure
struct controls {
	Uint8 up, left, down, right;//Movement keys
	Uint8 rollLeft, rollRight;//Rolling keys (use to rotate player)
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

//Rules class
class rules: public objectBased {
	public:
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
	
	//Constructor
	rules(){
		id = "";
		type = OBJTYPE_RULES;
		
		jumpImpulse = 30;
		
		groundSpeed = 30;
		groundForce = 15;
		groundDamping = 0.8;
		
		airSpeed = 15;
		airForce = 7;
		
		rotateSpeed = 100;
		rotateForce = 25;
		
		gravity = {0,10};
		
		jumpCount = 2;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succeeded loading base data
			var* jumpImpulse = get <var> (&o.v, "jumpImpulse");
			var* groundSpeed = get <var> (&o.v, "groundSpeed");
			var* groundForce = get <var> (&o.v, "groundForce");
			var* groundDamping = get <var> (&o.v, "groundDamping");
			var* airSpeed = get <var> (&o.v, "airSpeed");
			var* airForce = get <var> (&o.v, "airForce");
			var* rotateSpeed = get <var> (&o.v, "rotateSpeed");
			var* rotateForce = get <var> (&o.v, "rotateForce");
			var* gravity = get <var> (&o.v, "gravity");
			var* jumpCount = get <var> (&o.v, "jumpCount");
			
			if (jumpImpulse) this->jumpImpulse = jumpImpulse->doubleValue();
			if (groundSpeed) this->groundSpeed = groundSpeed->doubleValue();
			if (groundForce) this->groundForce = groundForce->doubleValue();
			if (groundDamping) this->groundDamping = groundDamping->doubleValue();
			if (airSpeed) this->airSpeed = airSpeed->doubleValue();
			if (airForce) this->airForce = airForce->doubleValue();
			if (rotateSpeed) this->rotateSpeed = rotateSpeed->doubleValue();
			if (rotateForce) this->rotateForce = rotateForce->doubleValue();
			if (jumpCount) this->jumpCount = jumpCount->intValue();
			
			if (gravity) this->gravity.fromString(gravity->value);
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
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

double *getVar(string);//Function to get variable from current game and game progress data

//Achievement class
class achievement: public objectBased {
	public:
	expr<double> verifyExpr;//Achievement expression
	image icon;//Achievement icon
	string name, info;//Achievement name and information
	
	//Constructor
	achievement(){
		id = "";
		type = OBJTYPE_ACHIEVEMENT;
		
		name = "";
		info = "";
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succeeded loading base data
			var* verifyExpr = get <var> (&o.v, "verify");
			var* name = get <var> (&o.v, "name");
			var* info = get <var> (&o.v, "info");
			object* icon = get <object> (&o.o, "icon");
			
			if (verifyExpr) this->verifyExpr.fromString(verifyExpr->value, &doubleOps);
			if (name) this->name = name->value;
			if (info) this->info = info->value;
			if (icon) this->icon.fromScriptObj(*icon);
			
			return true;//Returns true
		}
		
		return false;//Else returns false
	}
	
	//Function to verify if achievement was unlocked
	bool verify(){
		return verifyExpr.calculate(getVar);//Calculates expression and returns
	}
};

deque<achievement> achs;//Achievements

//Level progress class
//best times, least deaths, best ratings
class levelProgress: public objectBased {
	public:
	bool unlocked;//If true, level has been unlocked
	int time, deaths, rating;//Best scores
	
	//Constructor
	levelProgress(){
		id = "";
		type = OBJTYPE_LEVELPROGRESS;
		
		time = -1;
		deaths = 0;
		rating = 0;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succedeed loading base data
			var* unlocked = get <var> (&o.v, "unlocked");//Unlocked flag
			var* time = get <var> (&o.v, "time");//Best time
			var* deaths = get <var> (&o.v, "deaths");//Best deaths
			var* rating = get <var> (&o.v, "rating");//Best rating
			
			//Gets data
			if (unlocked) this->unlocked = unlocked->intValue();
			if (time) this->time = time->intValue();
			if (deaths) this->deaths = deaths->intValue();
			if (rating) this->rating = rating->intValue();
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();//Stores base data
		
		//Sets members
		result.set("unlocked", unlocked);
		result.set("time", time);
		result.set("deaths", deaths);
		result.set("rating", rating);
		
		return result;
	}
};

void unlockedAchievement(achievement*);//Function to show the unlocked achievement on screen

//Global progress class
class globalProgress: public objectBased, public list<levelProgress> {
	public:
	deque<string> unlockedAch;//Unlocked achievements
	
	//Constructor
	globalProgress(){
		id = "";
		type = OBJTYPE_GLOBALPROGRESS;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succeeded loading base data
			deque<object>::iterator i;//Iterator
			
			for (i = o.o.begin(); i != o.o.end(); i++){//For each object
				if (i->type == OBJTYPE_LEVELPROGRESS){//If object is progress info
					levelProgress p;//New progress
					p.fromScriptObj(*i);//Loads
					push_back(p);//Adds to progress
				}
			}
			
			var* ach = get <var> (&o.v, "achievements");//Achievements variable
			
			if (ach) this->unlockedAch = tokenize <deque<string> > (ach->value, ",");//Gets achievements
	
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	object toScriptObj(){
		object o = objectBased::toScriptObj();//Output object
		list<levelProgress>::iterator i;//Iterator
	
		for (i = begin(); i != end(); i++)//For each progress data
			o.o.push_back(i->toScriptObj());//Adds to output object
			
		if (unlockedAch.size() > 0) o.set("achievements", join(unlockedAch, ","));//Sets achievements
		
		return o;//Returns object
	}
	
	//Function to fill in progress info for given level
	void fillProgress(string id, int time, int deaths, int rating){
		levelProgress* p = get <levelProgress> (this, id);//Progress to read/change
		
		if (!p){//If progress doesn't exist
			levelProgress pr;//New progress
					
			//Sets members
			pr.id = id;
			pr.unlocked = true;
			pr.time = time;
			pr.deaths = deaths;
			pr.rating = rating;
			
			push_back(pr);//Adds to data
		}
		
		else {
			bool set = p->time > 0;//True if level has already been set
			
			//Picks best
			if (p->time > time || !set) p->time = time;
			if (p->deaths > deaths || !set) p->deaths = deaths;
			if (p->rating < rating || !set) p->rating = rating;
		}
	}
	
	//Function to get rating
	int getRating(string id){
		levelProgress* p = get <levelProgress> (this, id);
		
		if (p) return p->rating;
		else return 0;
	}

	//Function to get if a level is available to play
	bool canPlay(string id){
		levelProgress* p = get <levelProgress> (this, id);//Gets progress
		
		if ((p && p->unlocked) || debugMode) return true;//If level is unlocked and exists or game is in debug mode
		else return false;
	}

	//Function to unlock a level
	void unlock(string id){
		if (!canPlay(id)){//If level is locked
			levelProgress* p = get <levelProgress> (this, id);//Level progress
			if (p) p->unlocked = true;//Unlocks
			
			else {
				levelProgress pr;//New progress
				
				//Sets members
				pr.id = id;
				pr.unlocked = true;
				
				push_back(pr);//Adds to data
			}
		}
	}
	
	//Function to verify achievements
	void verifyAchs(){
		deque<achievement>::iterator i;//Iterator
		
		for (i = achs.begin(); i != achs.end(); i++){//For each achievement
			if (!find(&unlockedAch, i->id) && i->verify()){//If verified and not unlocked
				unlockedAch.push_back(i->id);//Adds to unlocked
				unlockedAchievement(&*i);//Calls unlock function
			}
		}
	}
	
	//Function to get completed levels
	int completed(){
		iterator i;//Iterator
		int result = 0;//Result
		
		for (i = begin(); i != end(); i++)//For each element
			if (i->time > 0) result++;//Increments counter if completed

		return result;//Returns result
	}
} progress;

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
	
	bool completed;//Completed flag
		
	void (*success)();//Success function
	
	//Constructor
	game(){
		player = NULL;
		goal = NULL;
		
		playerControls = {SDLK_w, SDLK_a, SDLK_s, SDLK_d, SDLK_q, SDLK_e};
		
		releasedJump = true;
		playerJumps = 0;
		
		playerStart = {0,0};
		playerAngle = 0;
		
		paused = false;
		lastFrameTime = 0;
		
		currentLevel = NULL;
		levelIndex = 0;
		
		success = NULL;
		
		completed = false;
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
			
			if (!ground){//If not on ground
				if (keys[playerControls.rollRight] && player->omega < gameRules.rotateSpeed){//If rolling right
					player->applyForce(player->position - vector {0, 1}, {gameRules.rotateForce, 0});
					player->applyForce(player->position - vector {0, -1}, {-gameRules.rotateForce, 0});
				}
				
				else if (keys[playerControls.rollLeft] && player->omega > -gameRules.rotateSpeed){//If rolling left
					player->applyForce(player->position - vector {0, 1}, {-gameRules.rotateForce, 0});
					player->applyForce(player->position - vector {0, -1}, {gameRules.rotateForce, 0});
				}
			}
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
		completed = false;//Not completed
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
			if ((i->a == player && i->b == goal) || (i->a == goal && i->b == player)){ completed = true; if (success) success(); break; }//Success if hit goal
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

//Function to get variable
double *getVar(string id){
	if (id == "unlockedLevels"){//If requested the unlocked levels
		double *i = new double(0);//Counter
		deque<string>::iterator l;//Level iterator
		
		for (l = current.levels.begin(); l != current.levels.end(); l++)//For each level
			if (progress.canPlay(loadLevel(*l)->id)) (*i)++;//Increases counter
			
		return i;
	}

	else if (id == "completedLevels")//If requested the completed levels
		return new double(progress.completed());//Returns result
	
	else if (id == "totalLevels")//If requested the total levels
		return new double(current.levels.size());//Returns result
	
	else if (id == "deaths")//If requested the deaths
		return new double(current.deaths);//Returns result
	
	else if (id == "finished")//If requested the finished flag
		return new double(current.completed);//Returns result
		
	else if (id == "aveRating"){//If requested the average rating
		if (progress.size() == 0) return new double (0);//Returns 0 if no level was unlocked
		
		globalProgress::iterator i;//Iterator
		int result = 0;//Result
		
		for (i = progress.begin(); i != progress.end(); i++)//For each progress info
			result += i->rating;//Adds to total
		
		return new double (result / progress.size());//Returns result
	}
	
	else if (id == "achs")//If requested unlocked achievements
		return new double(progress.unlockedAch.size());
		
	else if (id == "totalAchs")//If requested total achievements
		return new double(achs.size());//Returns result
}

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
	var* v_debug = get <var> (&g.v, "debugMode");
	
	if (v_fullscreen) fullscreen = v_fullscreen->intValue();
	if (v_camFollow) camFollow = v_camFollow->intValue();
	if (v_videoW) videoWin_w = v_videoW->intValue();
	if (v_videoH) videoWin_h = v_videoH->intValue();
	if (v_sound) enableSfx = v_sound->intValue();
	if (v_debug) debugMode = v_debug->intValue();
	
	resize(videoWin_w, videoWin_h, fullscreen, false);//Resizes window
}

//Function to load progress
void loadProgress(){
	fileData f (progressFile);//Source file
	object o = f.objGen("progress");//Generates object
	
	o.type = OBJTYPE_GLOBALPROGRESS;//Sets type
	
	progress.fromScriptObj(o);//Loads data
}

//Function to load achievements
void loadAchievements(){
	fileData f (achievementsFile);//Source file
	object o = f.objGen("ach");//Generates data
	
	deque<object>::iterator i;//Iterator
	for (i = o.o.begin(); i != o.o.end(); i++){//For each sub object
		if (i->type == OBJTYPE_ACHIEVEMENT){//If object is an achievement
			achievement a;//New achievement
			a.fromScriptObj(*i);//Loads
			achs.push_back(a);//Adds to database
		}
	}
}

//Function to load rules
void loadRules(){
	fileData f (rulesFile);//Source file
	object o = f.objGen("rules");//Generated object
	
	o.type = OBJTYPE_RULES;//Sets rules
	current.gameRules.fromScriptObj(o);//Loads rules
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
	ob.set("debugMode", debugMode);
	
	o << ob.toString();//Outputs settings
	
	o.close();//Closes file
}

//Function to save progress
void saveProgress(){
	ofstream of (progressFile.c_str());//Output file
	of << progress.toScriptObj().toString();//Outputs data
	of.close();//Closes file
}

//Game initialization function
void gameInit(int argc, char* argv[]){
	Bulk_image_init();//Initializes Bulk image
	Bulk_ui_init();//Initializes Bulk user interface
	Bulk_physGraphics_init();//Initializes Bulk physics graphic functions

	Bulk_doubleExpr_init();//Initializes for double expressions
	
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);//Opens audio
	
	SDL_WM_SetCaption("Physica", "PHY");//Sets window caption
	
	current.loadLevelSet(levelsFile);//Loads core level set
	current.success = showSuccess;//Sets success function
	
	loadRules();//Loads rules
	loadProgress();//Loads game progress
	
	level* first = loadLevel(current.levels[0]);//First level
	if (first && !progress.canPlay(first->id)) progress.unlock(first->id);//Unlocks first level
	
	loadSettings();//Loads settings
	loadGraphics();//Loads graphics
	loadAchievements();//Loads achievements
	loadSound();//Loads sound
	loadUI();//Loads ui
	
	loadEditor();//Loads editor
	
	keys = SDL_GetKeyState(NULL);//Gets keys
}

//Game quitting function
void gameQuit(){
	saveProgress();//Saves progress
	saveSettings();//Saves settings
	
	while (Mix_Playing(-1)){}//Waits for all channels to finish playing
	
	Mix_CloseAudio();//Closes audio
	IMG_Quit();//Quits SDL image
	TTF_Quit();//Quits SDL ttf
	SDL_Quit();//Quits SDL
}