//Physica main header
//Provides a game interface based on Bulk

#include "Bulk.h"//Includes Bulk main header
#include "Bulk_graphics.h"//Includes Bulk graphics
#include "Bulk_physics.h"//Includes Bulk physics

#define OBJTYPE_LEVEL		"level"//Level objects

#define BKG					SDL_FillRect(video, &video->clip_rect, background)//Background applying macro
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
int animFps = 15;//Animation frames per second
int frameBegin = 0;//Current frame beginning time

//Event handling and input
SDL_Event ev;//Global event
Uint8* keys = NULL;//Keys array

//User interface
Uint32 background = 0x101010;//Background color

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
	list<level*> levels;//Game levels
	list<level*>::iterator currentLevel;//Current level scene
	
	entity* player;//Entity controlled by player
	entity* goal;//Goal entity
	controls playerControls;//Player controls
	
	rules gameRules;//Game rules
	
	bool releasedJump;//If true, released jump
	int playerJumps;//Jump counter
	
	vector playerStart;//Player starting position
	double playerAngle;//Player starting angle
		
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
		if (*currentLevel){//If current level is valid
			(*currentLevel)->print(target, x, y);//Prints current level
			
			if (!(*currentLevel)->printLevelScene){//If level scene wasn't printed
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
			
			if (levelFile) levels.push_back(loadLevel(levelFile->value));//Adds level to game if found the variable
			else break;//Else exits loop
			
			i++;//Next level
		}
		
		currentLevel = levels.begin();//Sets first level
	}
	
	//Function to setup current level
	void setup(){
		player = get_ptr <entity> (&(*currentLevel)->entities, "player");//Gets player
		goal = get_ptr <entity> (&(*currentLevel)->entities, "goal");//Gets goal
		
		playerStart = player->position;//Sets player starting position
		playerAngle = player->theta;//Sets player starting angle
	}
	
	//Function to reset current level
	void reset(){	
		player->translate ({playerStart - player->position});//Resets starting position
		player->rotate (playerAngle - player->theta);//Resets starting angle
		
		player->speed = {0,0};//Resets speed
		player->omega = 0;//Resets angular speed
	}
	
	//Function to move onto next level (if any)
	void next(){
		if ((*currentLevel)->id != levels.back()->id){//If not on last level
			currentLevel++;//Moves onto next level
			setup();//Sets up level
		}
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
		if (*currentLevel) (*currentLevel)->resetForces();//Resets forces on level
	}
	
	//Function for time step
	void step(double t){
		if (*currentLevel){//If level exists
			(*currentLevel)->applyGravity(gameRules.gravity);//Applies gravity
			checkRelevant((*currentLevel)->step(t));//Steps level
		}
	}
	
	//Function to handle a frame
	void frame(double t, Uint8* keys){
		step(t);//Steps
		
		resetForces();//Resets forces
		handleControls(keys);//Handles controls
	}
	
	//Function for animation step
	void animStep(){
		if (*currentLevel) (*currentLevel)->animStep();//Steps animation
	}
};

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
	
	keys = SDL_GetKeyState(NULL);//Gets keys
	
	loadThemesDB("data/cfg/themes/blocks.cfg");//Loads block themes
}