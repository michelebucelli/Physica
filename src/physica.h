//Physica main header
//Provides a game interface based on Bulk

#include "Bulk.h"//Includes Bulk main header
#include "Bulk_graphics.h"//Includes Bulk graphics
#include "Bulk_physics.h"//Includes Bulk physics

#include "Bulk_expr_double.h"//Includes double expressions (for achievements)

#include "SDL/SDL_mixer.h"//Includes SDL sound effects library

#include <curl/curl.h>

#define OBJTYPE_LEVEL			"level"//Level objects
#define OBJTYPE_LEVELPROGRESS	"progress"//Level progress objects
#define OBJTYPE_GLOBALPROGRESS	"gProgress"//Global progress objects
#define OBJTYPE_ACHIEVEMENT		"achievement"//Achievement objects
#define OBJTYPE_RULES			"rules"//Rules objects
#define OBJTYPE_CONTROLS		"controls"//Controls objects
#define OBJTYPE_LEVELSET		"levelSet"//Level set objects
#define OBJTYPE_AREA			"area"//Area objects

#define BKG						SDL_FillRect(video, &video->clip_rect, background)//Background applying macro
#define DARK					boxColor(video, 0, 0, video_w, video_h, 0x0000007F)//Dark transparent fill
#define UPDATE					SDL_Flip(video)//Video updating macro

#define FRAME_BEGIN				frameBegin = SDL_GetTicks()//Frame beginning macro
#define FRAME_END				if (SDL_GetTicks() > frameBegin) actualFps = 1000 / (SDL_GetTicks() - frameBegin); else actualFps = 1000; if (SDL_GetTicks() - frameBegin < 1000 / fps) SDL_Delay(1000 / fps - SDL_GetTicks() + frameBegin); frames++//Frame end macro

#define EVENTS_COMMON(E)		if (E.type == SDL_QUIT) running = false; if (E.type == SDL_VIDEORESIZE) resize(E.resize.w, E.resize.h, fullscreen)//Common events macro

#define PLAYSOUND(SND)			if (enableSfx) Mix_PlayChannel(-1, SND, 0)//Play sound macro

#define KEYNAME(KEY)			SDL_GetKeyName(SDLKey(KEY))//Key name macro

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
Uint8 *keys;//Keys array

//Files
string settingsFile = "data/cfg/settings.cfg";//Global settings file
string sfxFile = "data/cfg/sfx.cfg";//Sound effects file
string levelsFile = "data/cfg/levels/levelPacks.cfg";//Level set file
string progressFile = "data/cfg/progress.cfg";//Progress file
string achievementsFile = "data/cfg/achievements.cfg";//Achievements file
string rulesFile = "data/cfg/rules.cfg";//Rules file

string updatesFile = "https://raw.github.com/buch415/Physica/master/updates/updates.cfg";//Updates file path
int updatesCount = 0;//Installed updates count

//Sound
bool enableSfx = true;//Enables sound

Mix_Chunk *clickSfx;//Click sound
Mix_Chunk *successSfx;//Success sound
Mix_Chunk *deathSfx;//Death sound

//Misc
bool debugMode = false;//Debug mode flag (all levels unlocked if true)
bool camFollow = false;//If true, camera will follow player

//Prototypes
class area;//Area class prototype

void resize(int,int,bool,bool = true);//Resizing function
string getInput(string);//Function to get input

//Common funcs
void frame_begin(){ frameBegin = SDL_GetTicks(); }//Frame beginning

//Frame end
void frame_end(){
	if (SDL_GetTicks() > frameBegin) actualFps = 1000 / (SDL_GetTicks() - frameBegin);
	else actualFps = 1000;
	
	if (SDL_GetTicks() - frameBegin < 1000 / fps) SDL_Delay(1000 / fps - SDL_GetTicks() + frameBegin);
	
	frames++;
}

//Common events
void events_common(SDL_Event e){
	if (e.type == SDL_QUIT) running = false;
	if (e.type == SDL_VIDEORESIZE) resize(e.resize.w, e.resize.h, fullscreen);
}

//Function to convert text into keysym
SDLKey strToKey(string s){
	int i;//Counter
	for (i = 0; i < SDLK_LAST; i++)//For each key
		if (string(KEYNAME(i)) == s) return SDLKey(i);//If name matches, returns code
	
	return SDLKey(0);//Returns -1 if failed
}

//Control scheme class
class controls: public objectBased {
	public:
	SDLKey up, left, right;//Movement keys
	
	//Constructor
	controls(){
		id = "";
		type = OBJTYPE_CONTROLS;
		
		up = SDLK_UP;
		left = SDLK_LEFT;
		right = SDLK_RIGHT;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succeeded loading base data
			var* up = get <var> (&o.v, "up");//Up key
			var* left = get <var> (&o.v, "left");//Left key
			var* right = get <var> (&o.v, "right");//Right key
			
			if (up) this->up = strToKey(up->value);//Gets up
			if (left) this->left = strToKey(left->value);//Gets left
			if (right) this->right = strToKey(right->value);//Gets right
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();//Result
		
		//Sets members
		result.set("up", KEYNAME(up));
		result.set("left", KEYNAME(left));
		result.set("right", KEYNAME(right));
		
		return result;//Returns result
	}
} playerControls;

//Function to get an integer representing pressed controls
int controlsToInt(controls c, Uint8* keys){
	int result = 0;//Result
	
	//Sets result bits
	if (keys[c.up]) result += 0b10000000;
	if (keys[c.left]) result += 0b01000000;
	if (keys[c.right]) result += 0b00010000;
	
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
	
	vector gravity;//Gravity vector
	
	int jumpCount;//Jumps counter
	
	int setMask;//Mask for set values
	
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
		
		gravity = {0,10};
		
		jumpCount = 2;
		
		setMask = 0;
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
			var* gravity = get <var> (&o.v, "gravity");
			var* jumpCount = get <var> (&o.v, "jumpCount");
			
			setMask = 0;
			
			if (jumpImpulse){ this->jumpImpulse = jumpImpulse->doubleValue(); setMask |= 0b10000000; }
			if (groundSpeed){ this->groundSpeed = groundSpeed->doubleValue(); setMask |= 0b01000000; }
			if (groundForce){ this->groundForce = groundForce->doubleValue(); setMask |= 0b00100000; }
			if (groundDamping){ this->groundDamping = groundDamping->doubleValue(); setMask |= 0b00010000; }
			if (airSpeed){ this->airSpeed = airSpeed->doubleValue(); setMask |= 0b00001000; }
			if (airForce){ this->airForce = airForce->doubleValue(); setMask |= 0b00000100; }
			if (gravity){ this->gravity.fromString(gravity->value); setMask |= 0b00000010; }
			
			if (jumpCount){ this->jumpCount = jumpCount->intValue(); setMask |= 0b00000001; }
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();
		
		if (setMask & 0b10000000) result.set("jumpImpulse", jumpImpulse);
		if (setMask & 0b01000000) result.set("groundSpeed", groundSpeed);
		if (setMask & 0b00100000) result.set("groundForce", groundForce);
		if (setMask & 0b00010000) result.set("groundDamping", groundDamping);
		if (setMask & 0b00001000) result.set("airSpeed", airSpeed);
		if (setMask & 0b00000100) result.set("airForce", airForce);
		if (setMask & 0b00000010) result.set("gravity", toString(gravity.x) + " " + toString(gravity.y));
		if (setMask & 0b00000001) result.set("jumpCount", jumpCount);
		
		return result;
	}
	
	//Function to stack rules
	template <class t> rules operator + (t r){
		rules result = *this;
		
		if (r.setMask & 0b10000000) result.jumpImpulse = r.jumpImpulse;
		if (r.setMask & 0b01000000) result.groundSpeed = r.groundSpeed;
		if (r.setMask & 0b00100000) result.groundForce = r.groundForce;
		if (r.setMask & 0b00010000) result.groundDamping = r.groundDamping;
		if (r.setMask & 0b00001000) result.airSpeed = r.airSpeed;
		if (r.setMask & 0b00000100) result.airForce = r.airForce;
		if (r.setMask & 0b00000010) result.gravity = r.gravity;
		if (r.setMask & 0b00000001) result.jumpCount = r.jumpCount;
		
		result.setMask = result.setMask | r.setMask;
		
		return result;//Returns result
	}
	
	//Stack operator
	template <class t> void operator += (t r){
		*this = *this + r;
	}
};

rules defaultRules;//Default rules (same as constructor values)
rules loadedRules;//Loaded rules

//Area class - a rectangle plus rules definitions
class area: public rectangle, public rules {
	public:
	string id, type;
	Uint32 color;//Area color
	
	//Constructor
	area(){
		id = "";
		type = OBJTYPE_AREA;
		rules::type = OBJTYPE_AREA;
		rectangle::type = OBJTYPE_AREA;
		
		x = 0;
		y = 0;
		w = 0;
		h = 0;
		
		jumpImpulse = 30;
		
		groundSpeed = 30;
		groundForce = 15;
		groundDamping = 0.8;
		
		airSpeed = 15;
		airForce = 7;
		
		gravity = {0,10};
		
		jumpCount = 2;
		
		setMask = 0;
		
		color = 0;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (rectangle::fromScriptObj(o) && rules::fromScriptObj(o)){//If succeeded loading base data	
			var* color = get <var> (&o.v, "color");//Area color
			if (color) this->color = strtol(color->value.c_str(), NULL, 0);//Gets color
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = rules::toScriptObj();//Rules data
		result += rectangle::toScriptObj();//Adds rectangle
		
		result.set("color", color);
		
		return result;//Returns result
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

	deque<string> message;//Level startup messages
	
	rules lvlRules;//Level specific rules
	deque<area> areas;//Areas
	
	string path;//Level path
	
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
		
		path = "";
	}
	
	//Destructor
	~level(){
		list<entity*>::iterator i;//Iterator for entities
		deque<phLink*>::iterator l;//Iterator for links
		
		for (i = entities.begin(); i != entities.end(); i++){ delete *i; i = entities.erase(i); i--; }//Deletes each entity
		for (l = links.begin(); l != links.end(); l++){ delete *l; l = links.erase(l); l--; }//Deletes each link
		
		if (backgroundImage) SDL_FreeSurface(backgroundImage);//Frees background
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (scene::fromScriptObj(o)){//If succeeded loading base data
			var* id = get <var> (&o.v, "id");//Id variable (when loaded directly from file)
			
			var* backgroundImage = get <var> (&o.v, "background");//Gets background
			var* print = get <var> (&o.v, "print");//Gets print flag
			var* twoStarsTime = get <var> (&o.v, "twoStarsTime");//Gets two stars time
			var* threeStarsTime = get <var> (&o.v, "threeStarsTime");//Gets three stars time
			
			var* message = get <var> (&o.v, "message");//Gets message
			
			object* lvlRules = get <object> (&o.o, "rules");//Gets rules
			
			if (id) this->id = id->value;//Gets id
			
			if (backgroundImage){ this->bkg = backgroundImage->value; this->backgroundImage = CACHEDSURFACE(bkg); }//Gets background image
			
			if (print) this->printLevelScene = print->intValue();//Sets print flag
			if (twoStarsTime) this->twoStarsTime = twoStarsTime->intValue();//Gets two stars time
			if (threeStarsTime) this->threeStarsTime = threeStarsTime->intValue();//Gets three stars time
			
			if (message) this->message = tokenize <deque<string> > (message->value, "_");//Gets messages
			
			if (lvlRules) this->lvlRules.fromScriptObj(*lvlRules);//Loads rules
			
			deque<object>::iterator i;//Iterator
			for (i = o.o.begin(); i != o.o.end(); i++){//For each object
				if (i->type == OBJTYPE_AREA){//If object's an area
					area a;//New area
					a.fromScriptObj(*i);//Loads
					areas.push_back(a);//Adds to areas
				}
			}
			
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
		
		result.o.push_back(lvlRules.toScriptObj());//Adds rules
		
		deque<area>::iterator a;//Area iterator
		for (a = areas.begin(); a != areas.end(); a++)//For each area
			result.o.push_back(a->toScriptObj());//Adds areas
		
		return result;//Returns result
	}
	
	//Function to print the level
	void print(SDL_Surface* target, int x, int y, bool hidden = false){
		SDL_Rect offset {x, y};//Offset rect
		SDL_BlitSurface(backgroundImage, NULL, target, &offset);//Prints background
		
		deque<area>::iterator a;//Area iterator
		for (a = areas.begin(); a != areas.end(); a++){//For each area
			SDL_Rect r = *a;//Rectangle
			r.x += x;//Offset x
			r.y += y;//Offset y
			
			SDL_FillRect(target, &r, a->color);//Fills area
		}
		
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
	
	bool checkOnce;//If false, checks for this achievement every time
	
	//Constructor
	achievement(){
		id = "";
		type = OBJTYPE_ACHIEVEMENT;
		
		name = "";
		info = "";
		
		checkOnce = true;
	}
	
	//Constructor
	achievement (string id, string name, string info, string verify, image icon, bool checkOnce){
		this->id = id;
		type = OBJTYPE_ACHIEVEMENT;
		
		this->name = name;
		this->info = info;
		
		this->verifyExpr.fromString(verify, &doubleOps);
		
		this->icon = icon;
		this->icon.id = "icon";
		
		this->checkOnce = checkOnce;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succeeded loading base data
			var* verifyExpr = get <var> (&o.v, "verify");
			var* name = get <var> (&o.v, "name");
			var* info = get <var> (&o.v, "info");
			var* checkOnce = get <var> (&o.v, "checkOnce");
			object* icon = get <object> (&o.o, "icon");
			
			if (verifyExpr) this->verifyExpr.fromString(verifyExpr->value, &doubleOps);
			if (name) this->name = name->value;
			if (info) this->info = info->value;
			if (checkOnce) this->checkOnce = checkOnce->intValue();
			if (icon) this->icon.fromScriptObj(*icon);
			
			return true;//Returns true
		}
		
		return false;//Else returns false
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();
		
		result.set("name", name);
		result.set("info", info);
		result.set("verify", verifyExpr.exprToString());
		result.set("checkOnce", checkOnce);
		
		icon.id = "icon";
		result.o.push_back(icon.toScriptObj());
		
		return result;
	}
	
	//Function to verify if achievement was unlocked
	bool verify(){
		return verifyExpr.calculate(getVar);//Calculates expression and returns
	}
};

deque<achievement> achs;//Global achievements

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
	
	//Function to get level set
	string set(){
		int dot = id.find(".");//Dot position
		
		if (dot != id.npos) return id.substr(0, dot);//Returns result
		else return "";//Returns empty string if no set was specified
	}
};

void unlockedAchievement(achievement*);//Function to show the unlocked achievement on screen

//Level set class
class levelSet: public deque<string>, public objectBased {
	public:
	string path;//Level set path (set only when loaded using levelSetFromFile)
	string name;//Level set name (shown on level set selection)
	
	image icon;//Level set icon
	
	deque<achievement> lsAchs;//Level set achievements
	
	rules lsRules;//Level set custom rules (overrides normal rules)
		
	//Constructor
	levelSet(){
		id = "";
		type = OBJTYPE_LEVELSET;
		
		path = "";
		name = "";
		
		lsRules.setMask = 0;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succeeded loading base data
			var* id = get <var> (&o.v, "id");//Id variable
			var* name = get <var> (&o.v, "name");//Name variable
			object* icon = get <object> (&o.o, "icon");//Icon object
			object* rules = get <object> (&o.o, "rules");//Rules object
			
			if (id) this->id = id->value;//Gets id
			if (name) this->name = name->value;//Gets name
			if (icon) this->icon.fromScriptObj(*icon);//Loads icon
			if (rules) lsRules.fromScriptObj(*rules);//Loads rules
		
			int i = 1;//Counter
			while (true){//Endless loop
				var* levelFile = get <var> (&o.v, "level" + toString(i));//Gets variable
				
				if (levelFile) push_back(levelFile->value);//Adds level to game if found the variable
				else break;//Else exits loop
				
				i++;//Next level
			}
			
			deque<object>::iterator ob;//Object iterator
			for (ob = o.o.begin(); ob != o.o.end(); ob++){//For each object
				if (ob->type == OBJTYPE_ACHIEVEMENT){//If object is an achievement
					achievement a;//New achievement
					a.fromScriptObj(*ob);//Loads
					lsAchs.push_back(a);//Adds to achievements
				}
			}
		}
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();
		
		result.set("id", id);
		result.set("name", name);
		
		icon.id = "icon";
		result.o.push_back(icon.toScriptObj());
		
		lsRules.id = "rules";
		result.o.push_back(lsRules.toScriptObj());
		
		int n;
		for (n = 0; n < size(); n++) result.set("level" + toString(n + 1), (*this)[n]);
		for (n = 0; n < lsAchs.size(); n++) result.o.push_back(lsAchs[n].toScriptObj());
		
		return result;
	}
};

string levelSetsFiles = "";//Level sets files string
deque<levelSet*> levelSets;//Level sets

//Function to load a level set from a file
levelSet *levelSetFromFile(string path){
	fileData f (path);//Source file
	
	if (f.valid){//If file is valid
		levelSet *levels = new levelSet;//Level set
		object o = f.objGen("");//Generates object
		
		o.type = OBJTYPE_LEVELSET;//Sets type
		levels->fromScriptObj(o);//Loads levels
		levels->path = path;//Sets path
		
		return levels;//Returns result
	}
	
	else return NULL;//Else returns null
}

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
		deque<levelSet*>::iterator l;//Level set iterator
		
		for (i = achs.begin(); i != achs.end(); i++){//For each achievement
			if (!find(&unlockedAch, i->id) && i->verify()){//If verified and not unlocked
				unlockedAch.push_back(i->id);//Adds to unlocked
				unlockedAchievement(&*i);//Calls unlock function
			}
		}
		
		for (l = levelSets.begin(); l != levelSets.end(); l++){//For each level set
			for (i = (*l)->lsAchs.begin(); i != (*l)->lsAchs.end(); i++){//For each achievement
				if (!find(&unlockedAch, (*l)->id + "." + i->id) && i->verify()){//If verified and not unlocked
					unlockedAch.push_back((*l)->id + "." + i->id);//Adds to unlocked
					unlockedAchievement(&*i);//Calls unlock function
				}
			}
		}
		
		deque<string>::iterator s;//Iterator
		
		for (s = unlockedAch.begin(); s != unlockedAch.end(); s++){//For each unlocked achievement
			achievement* a;//Achievement
			
			if (s->find(".") != s->npos){//If related to a level set
				levelSet* l = get_ptr <levelSet> (&levelSets, s->substr(0, s->find(".")));//Level set
				a = get <achievement> (&l->lsAchs, s->substr(s->find(".") + 1));//Gets achievement
			}
			
			else a = get <achievement> (&achs, *s);//Else gets from global achievements
			
			if (!a || (!a->checkOnce && !a->verify())){//If achievement doesn't exist or is not verified anymore
				s = unlockedAch.erase(s);//Erases element
				s--;//Goes back
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
	
	//Function to get completed levels of specific set
	int completed(string set){
		iterator i;//Iterator
		int result = 0;//Result
		
		for (i = begin(); i != end(); i++)//For each element
			if (i->set() == set && i->time > 0) result++;//Adds to result

		return result;//Returns result
	}
	
	//Function to get completion percent of given set
	int percent(string set){
		iterator i;//Iterator
		double result = 0;//Result
		
		for (i = begin(); i != end(); i++)//For each element
			if (i->time > 0 && i->set() == set) result += i->rating;//Adds rating to result
			
		levelSet* s = get_ptr <levelSet> (&levelSets, set);//Requested set
		
		if (s && s->size()) return result / (s->size() * 3) * 100;//Returns result
		else if (s && !s->size()) return 100;//Returns 100 if no levels in set
		else return 0;//Returns 0 if failed
	}
	
	//Function to check progress and erase invalid levels
	void check(){
		iterator i;//Iterator
		
		for (i = begin(); i != end(); i++){//For each element
			string setId = "";//Set and level id
			
			setId = i->id.substr(0, i->id.find("."));//Set id
			
			levelSet* s = get_ptr <levelSet> (&levelSets, setId);//Gets set
			
			if (!s){//If set is invalid
				i = erase(i);//Erases element
				i--;//Goes back
			}
		}
	}
} progress;

//Function to load a level from file
level* loadLevel(string path){
	fileData source (path);//Loads and processes file
	
	if (source.valid){//If file is valid
		object s = source.objGen(path);//level object
		s.type = OBJTYPE_LEVEL;//Sets object typ
		
		level *result = new level;//Result level
		result->path = path;//Sets path
		result->fromScriptObj(s);//Loads result
		
		return result;//Returns level
	}
	
	else return NULL;//Else returns NULL
}

#include "dialogs.h"//Includes dialogs

//Game class
class game {
	public:
	levelSet levels;//Game level files
	level* currentLevel;//Current level
	int levelIndex;//Current level index
	
	entity* player;//Entity controlled by player
	entity* goal;//Goal entity
	
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
	
	list <collision> c;//Frame collision list
	
	rules currentRules;//Current rules
	
	//Constructor
	game(){
		player = NULL;
		goal = NULL;
		
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
		
		gameRules.setMask = 0b11111111;
	}
	
	//Function for controls handling
	void handleControls(Uint8* keys){
		rules resultRules = currentRules;//Current rules
		
		if (keys && player){//If keys array is valid and there's a player
			int lowestSensor = 0;//Lowest player sensor
			int i;//Counter
			
			bool up = int(keys[playerControls.up]) == 1;
			bool right = int(keys[playerControls.right]) == 1;
			bool left = int(keys[playerControls.left]) == 1;
			
			for (i = 0; i < player->sensors.size(); i++)//For each sensor in player
				if ((currentRules.gravity.y > 0 && player->sensors[i]->y > player->sensors[lowestSensor]->y) || (currentRules.gravity.y < 0 && player->sensors[i]->y < player->sensors[lowestSensor]->y)) lowestSensor = i;//Sets lowest sensor
			
			bool ground = player->checkSensor(lowestSensor);//True if touching the ground
			
			list<collision>::iterator c;//Collision iterator
			for (c = this->c.begin(); c != this->c.end(); c++){//For each collision
				if ((c->a == player || c->b == player) && ((currentRules.gravity.y > 0 && c->p.y > player->position.y) || (currentRules.gravity.y < 0 && c->p.y < player->position.y)) && abs(c->p.x - player->position.x) < ((box*) player)->w / 3) ground = true;//Sets ground flag if touching lower entity
			}
			
			if (!up) releasedJump = true;//Resets released jump flag if jump is not pressed
			if (ground) playerJumps = 0;//Resets jump count
			if (!ground && playerJumps == 0) playerJumps = 1;//If on air, at least one jump
			
			if (keys[playerControls.up] && releasedJump && playerJumps < resultRules.jumpCount){//If pressed up and can jump
				player->speed.y = 0;//Stops on y
				player->applyImpulse(player->position, {0, -resultRules.jumpImpulse * currentRules.gravity.y / abs(currentRules.gravity.y)});//Applies impulse
				
				playerJumps++;//Increases jump count
				releasedJump = false;//Not released jump
			}
			
			if (right){//If pressing right
				if (ground && player->speed.x < resultRules.groundSpeed)//If on ground and moving slower
					player->applyForce(player->position, {resultRules.groundForce, 0});//Applies force
					
				else if (!ground && player->speed.x < resultRules.airSpeed)//If on air and moving slower
					player->applyForce(player->position, {resultRules.airForce, 0});//Applies force
			}
			
			else if (left){//If pressing left
				if (ground && player->speed.x > -resultRules.groundSpeed)//If on ground and moving slower
					player->applyForce(player->position, {-resultRules.groundForce, 0});//Applies force
					
				else if (!ground && player->speed.x > -resultRules.airSpeed)//If on air and moving slower
					player->applyForce(player->position, {-resultRules.airForce, 0});//Applies force
			}
			
			else if (ground) player->applyForce(player->position, {-player->speed.x * resultRules.groundDamping, 0});//Applies damping on ground
		}
	}
	
	//Function to check for area rules
	void checkAreaRules(){
		deque<area>::iterator a;//Area iterator
		
		currentRules = defaultRules + loadedRules + levels.lsRules;//Sets rules
		
		for (a = currentLevel->areas.begin(); a != currentLevel->areas.end(); a++)//For each area
			if (a->isInside(player->position.x, player->position.y)) currentRules += *a;//Adds area rules if player is inside
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
			deque<string>::iterator i;//String iterator
			string ans[] = { "Continue"};//Answer
			
			for (i = currentLevel->message.begin(); i != currentLevel->message.end(); i++){//For each message
				BKG;
				msgBox.show(video, *i, 1, ans, true, false);//Shows message
			}
		
			time = 0;//Resets timer
			deaths = 0;//Resets death counter
			lastFrameTime = SDL_GetTicks();//Resets frame time
		}
		
		paused = false;//Unpauses
		completed = false;//Not completed
		
		currentRules = defaultRules + loadedRules + levels.lsRules + currentLevel->lvlRules;//Sets rules
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
	void checkRelevant(){
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
			currentLevel->applyGravity(currentRules.gravity);//Applies gravity
			c = currentLevel->step(t);//Steps
			checkRelevant();//Steps level
		}
	}
	
	//Function to handle a frame
	void frame(double t, Uint8* keys){
		if (paused){ lastFrameTime = SDL_GetTicks(); return; }//Exits function if paused
		
		checkAreaRules();//Checks rules
		
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
	
	//Function to load a level set
	void loadLevelSet(int n){
		levels = *levelSets[n];//Sets levels
		
		level* first = loadLevel(levels[0]);//First level
		if (first && !progress.canPlay(levels.id + "." + first->id)) progress.unlock(levels.id + "." + first->id);//Unlocks first level
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
		
	else if (id.substr(0,16) == "completedLevels[" && id[id.size() - 1] == ']')//If requested specific set completed levels
		return new double(progress.completed(id.substr(16, id.size() - 17)));//Returns result
		
	else if (id == "level")//If requested current level
		return new double(current.levelIndex);//Returns result
	
	else if (id == "totalLevels")//If requested the total levels
		return new double(current.levels.size());//Returns result
		
	else if (id.substr(0,12) == "totalLevels[" && id[id.size() - 1] == ']'){//If requested specific level set total levels
		string lSetId = id.substr(12, id.size() - 13);//Level set id
		levelSet* s = get_ptr <levelSet> (&levelSets, lSetId);//Requested set
		
		if (s) return new double (s->size());//Returns result if set was found
		else return new double (0);//Returns null
	}
	
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
	
	else if (id.substr(0, 10) == "aveRating[" && id[id.size() - 1] == ']'){//If requested the average rating of a specific set
		globalProgress::iterator i;//Iterator
		int result = 0;//Result
		int n = 0;//Counter
		
		string set = id.substr(10, id.size() - 11);//Set id
		
		for (i = progress.begin(); i != progress.end(); i++){//For each progress info
			if (i->set() == set){//If level belongs to requested set
				result += i->rating;//Adds to total
				n++;//Increments counter
			}
		}
		
		if (n > 0) return new double (result / n);//Returns result
		else return new double (0);//Returns 0 if no level was found
	}
	
	else if (id == "achs")//If requested unlocked achievements
		return new double(progress.unlockedAch.size());
		
	else if (id == "totalAchs"){//If requested total achievements
		int result = 0;//Result
		deque<levelSet*>::iterator i;//Level set iterator
		
		for (i = levelSets.begin(); i != levelSets.end(); i++)//For each set
			result += (*i)->lsAchs.size();//Adds to result
		
		return new double(result + achs.size());//Returns result
	}
		
	else { cout << "Failed: " << id << endl; return new double(0); }//Returns 0 if failed
}

//Load sets prototype
void loadSets();

//Install set prototype
void installSet(string);

#include "editor.h"//Includes editor
#include "lpEditor.h"//Includes level set editor
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
	fileData f (settingsFile, false);//Source file
	object g = f.objGen("settings");//Generates object
	
	//Gets data
	var* v_fullscreen = get <var> (&g.v, "fullscreen");
	var* v_camFollow = get <var> (&g.v, "camFollow");
	var* v_videoW = get <var> (&g.v, "video_w");
	var* v_videoH = get <var> (&g.v, "video_h");
	var* v_sound = get <var> (&g.v, "enableSfx");
	var* v_debug = get <var> (&g.v, "debugMode");
	
	object* c = get <object> (&g.o, "controls");
	
	var* v_updatesFile = get <var> (&g.v, "updatesFile");
	var* v_updatesCount = get <var> (&g.v, "updatesCount");
	
	if (v_fullscreen) fullscreen = v_fullscreen->intValue();
	if (v_camFollow) camFollow = v_camFollow->intValue();
	if (v_videoW) videoWin_w = v_videoW->intValue();
	if (v_videoH) videoWin_h = v_videoH->intValue();
	if (v_sound) enableSfx = v_sound->intValue();
	if (v_debug) debugMode = v_debug->intValue();
	
	if (c) playerControls.fromScriptObj(*c);
	playerControls.id = "controls";
	
	if (v_updatesFile) updatesFile = v_updatesFile->value;
	if (v_updatesCount) updatesCount = v_updatesCount->intValue();
	
	resize(videoWin_w, videoWin_h, fullscreen, false);//Resizes window
}

//Function to load all level sets (according to levelsFile definitions)
void loadSets(){
	fileData f (levelsFile);//Loads level sets file
	object o = f.objGen("sets");//Object
	
	var* ls = get <var> (&o.v, "levelPacks");//Gets level sets list
	
	if (ls){//If variable was found
		levelSetsFiles = ls->value;//Gets files
		
		deque<string> t = tokenize <deque<string> > (ls->value, ",");//Splits into tokens
		deque<string>::iterator i;//Iterator
		
		for (i = t.begin(); i != t.end(); i++){//For each set
			levelSet* s = levelSetFromFile(*i);//Gets level set
			if (s) levelSets.push_back(s);//Adds to loaded
		}
	}
}

//Function to save level sets
void saveSets(){
	ofstream o (levelsFile.c_str());//Output
	o << "levelPacks = " << levelSetsFiles << ";";
	o.close();
}

//Function to install a set
void installSet(string setFile){
	if (levelSetsFiles.find(setFile) != levelSetsFiles.npos) return;//Exits if set already exists
	
	if (levelSetsFiles != "") levelSetsFiles += ",";//Adds the comma
	levelSetsFiles += setFile;//Adds path
	
	saveSets();//Saves sets
}

//Function to uninstall a set
void uninstallSet(string setFile){
	if (levelSetsFiles.find(setFile) != levelSetsFiles.npos){//If set is installed
		levelSetsFiles.erase(levelSetsFiles.find(setFile), setFile.size() + 1);//Erases set
	}
	
	saveSets();//Saves sets
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
	loadedRules.fromScriptObj(o);//Loads rules
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
	ob.set("updatesFile", updatesFile);
	ob.set("updatesCount", updatesCount);
	
	ob.o.push_back(playerControls.toScriptObj());
	
	o << ob.toString();//Outputs settings
	
	o.close();//Closes file
}

//Function to save progress
void saveProgress(){
	ofstream of (progressFile.c_str());//Output file
	of << progress.toScriptObj().toString();//Outputs data
	of.close();//Closes file
}

//Curl write funtion
size_t libCurlWrite(char* data, size_t size, size_t nmemb, FILE* out){
	return fwrite(data, size, nmemb, out);//Writes data
}

//Function to download a file to a specific location
CURLcode downloadFile(string source, string dest){
	CURLcode result;//Result code
	CURL* curlHandle;//Curl handle
	
	FILE* fp = fopen(dest.c_str(), "wb");//Opens file
	
	curlHandle = curl_easy_init();//Initializes handle
	
	curl_easy_setopt(curlHandle, CURLOPT_SSL_VERIFYPEER, false);//Disables certificate checking
	curl_easy_setopt(curlHandle, CURLOPT_WRITEFUNCTION, libCurlWrite);//Sets write function
	curl_easy_setopt(curlHandle, CURLOPT_WRITEDATA, fp);//Sets write buffer
	curl_easy_setopt(curlHandle, CURLOPT_URL, source.c_str());//Sets URL
	
	result = curl_easy_perform(curlHandle);//Performs curl operation
	
	curl_easy_cleanup(curlHandle);//Cleans
	
	fclose(fp);//Closes file
	
	return result;//Returns result
}

//Function to process an update script
void processUpdateScript(script u){
	script::iterator i;//Iterator for script lines
	
	for (i = u.begin(); i != u.end(); i++){//For each line
		deque<string> t = tokenize <deque<string> > (*i, " ");//Splits into tokens
				
		if (t[0] == "download" && t.size() >= 3)//Download command
			downloadFile(t[1], t[2]);//Downloads file
		
		else if (t[0] == "kill" && t.size() >= 2)//Kill command
			remove(t[1].c_str());//Kills file
		
		else if (t[0] == "install" && t.size() >= 2)//Install command
			installSet(t[1]);//Installs set
			
		else if (t[0] == "uninstall" && t.size() >= 2)//Uninstall command (removes set from list)
			uninstallSet(t[1]);//Uninstalls set
		
		else if (t[0] == "message" && t.size() >= 2){//Message command
			{BKG; msgBox.show(video, i->substr(8), 1, msgBox_ans_ok); }//Shows message box
		}
		
	}
}

//Function to provess an update script from file path
void processUpdateScript(string path){
	script s;//New script
	s.fromFile(path);//Opens
	processUpdateScript(s);//Processes script
}

//Function to check for updates
void checkUpdates(){
	CURLcode result = downloadFile (updatesFile, "tmp_updates");//Result of download
	
	if (result == 0) {//If downloaded successfully
		fileData up ("tmp_updates", false);//Opens updates file
		object u = up.objGen("updates");//Generated object
		
		deque<string> toDownload;//Updates to download
		
		int n;//Counter
		for (n = updatesCount + 1; ; n++){//Starting from next update
			var* v = get <var> (&u.v, "update" + toString(n));//Update file variable
			
			if (!v) break;//Exits if didn't find variable
			else toDownload.push_back(v->value);//Else adds to download
		}
		
		int s = toDownload.size();//Download size
		
		if (s > 0 && msgBox.show(video, toString(s) + (s > 1 ? " updates" : " update") + " available. Download?", 2, msgBox_ans_yn, false, false) == 0){//If user decides to download
			deque<string>::iterator i;//Iterator
			
			for (i = toDownload.begin(); i != toDownload.end(); i++){//For each file
				if (downloadFile(*i, "tmp_update_file") == 0){//If downloaded successfully
					processUpdateScript("tmp_update_file");//Processes script
					updatesCount++;//Increases updates count
				}
			}
			
			remove("tmp_update_file");//Removes temporary file
			
			levelSets.clear();//Clears sets
			loadSets();//Reloads sets
		}
	}
	
	else switch (result){
		case CURLE_COULDNT_CONNECT: break;//Connection error: breaks
		case CURLE_COULDNT_RESOLVE_HOST: BKG; msgBox.show(video, "Couldn't get updates list.", 1, msgBox_ans_ok); break; //Error message if couldn't find file
		default: cerr << "UPDATES ERROR: " << curl_easy_strerror(result); break;//Unexpected error
	}

	
	
	remove("tmp_updates");//Removes updates file
}

//Game initialization function
void gameInit(int argc, char* argv[]){
	Bulk_image_init();//Initializes Bulk image
	Bulk_ui_init();//Initializes Bulk user interface
	Bulk_physGraphics_init();//Initializes Bulk physics graphic functions

	Bulk_doubleExpr_init();//Initializes for double expressions
	
	curl_global_init(CURL_GLOBAL_ALL);//Initializes CURL
	
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);//Opens audio
	
	SDL_WM_SetCaption("Physica", NULL);//Sets window caption

	current.success = showSuccess;//Sets success function
	
	loadSettings();//Loads settings
	loadGraphics();//Loads graphics
	loadUI();//Loads ui
	
	loadSets();//Loads level sets
	loadRules();//Loads rules
	loadProgress();//Loads game progress
	
	loadAchievements();//Loads achievements
	loadSound();//Loads sound
	
	loadEditor();//Loads editor
	loadLpEditor();//Loads level pack editor
	
	checkUpdates();//Checks for updates

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