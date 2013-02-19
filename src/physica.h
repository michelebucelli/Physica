//Physica main header
//Provides a game interface based on Bulk

#include "Bulk.h"//Includes Bulk main header
#include "Bulk_graphics.h"//Includes Bulk graphics
#include "Bulk_physics.h"//Includes Bulk physics

#include "Bulk_expr_double.h"//Includes double expressions (for achievements)

#include "SDL/SDL_mixer.h"//Includes SDL sound effects library

#include <curl/curl.h>//Includes CURL

#if defined(__WIN32__) || defined(__WIN64__)
#include "SDL/SDL_syswm.h"//Includes SDL window manager
#endif

#ifdef __linux__
#include <sys/stat.h>//Includes linux header
#endif

#define VERSION_STR				"1.2"//Version string

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
#define UPDATE					SDL_BlitSurface(video, NULL, actVideo, NULL); SDL_Flip(actVideo)//Video updating macro

#define FRAME_BEGIN				lastFrameBegin = frameBegin; frameBegin = SDL_GetTicks()//Frame beginning macro
#define FRAME_END				SDL_ShowCursor(hideCursor && frameBegin - lastMotion > cursorHideDelay ? SDL_DISABLE : SDL_ENABLE); if (SDL_GetTicks() > frameBegin) actualFps = 1000 / (SDL_GetTicks() - frameBegin); else actualFps = 1000;//Frame end macro

#define EVENTS_COMMON(E)		if (E.type == SDL_QUIT) running = false; if (E.type == SDL_VIDEORESIZE) resize(E.resize.w, E.resize.h, fullscreen); if (E.type == SDL_MOUSEMOTION) lastMotion = SDL_GetTicks()//Common events macro

#define PLAYSOUND(SND)			if (enableSfx) Mix_PlayChannel(-1, SND, 0)//Play sound macro

#define KEYNAME(KEY)			SDL_GetKeyName(SDLKey(KEY))//Key name macro

//Video output
SDL_Surface* video = NULL;//Video surface
SDL_Surface* actVideo = NULL;//Actual video surface

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
int lastFrameBegin = 0;//Last frame beginning time
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

string localePath = "data/cfg/locale/";//Localizations folder
string defaultLocale = "en";//Default locale

//Sound
bool enableSfx = true;//Enables sound

Mix_Chunk *clickSfx;//Click sound
Mix_Chunk *successSfx;//Success sound
Mix_Chunk *deathSfx;//Death sound

//Misc
bool debugMode = false;//Debug mode flag (all levels unlocked if true)

window common;//Common UI
control* fpsLabel;//Fps label
control* debugLabel;//Debug label

bool hideCursor = false;//Determines if the cursor must be hidden after cursorHideDelay msec
int lastMotion = 0;//Last mouse motion event time
int cursorHideDelay = 3000;//Inactivity time before hiding the cursor

SDL_Surface* destinationArrow = NULL;//Destination arrow (uses SDL_Surface* instead of image for rotation)

//Prototypes
class area;//Area class prototype
class rules;//Rules class prototype

void resize(int,int,bool,bool = true);//Resizing function

void updateHud();//Hud updating function
void updateCommon();//Function to update common ui

void showSuccess();//Function to show success window

void test(string, rules);//Level test function

void checkUpdates(bool = true, bool = false);//Updates function

//Common funcs
void frame_begin(){ frameBegin = SDL_GetTicks(); }//Frame beginning

//Camera class
class camera {
	public:
	vector position;//Camera position
	vector speed;//Camera movement speed

	entity* destination;//Camera destination
	
	double range;//Range
	double rangeFactor;//Range factor
	double sVal;//Speed value
	double maxS;//Max speed value
	
	//Constructor
	camera(){
		position = {0,0};
		speed = {0,0};
		
		destination = NULL;
		
		range = 5;
		rangeFactor = 0.1;
		sVal = 10;
		maxS = 60;
	}
	
	//Function to move the camera
	void move(double t){
		vector d = destination->position - position;//Distance vector
		
		double curS = sVal + (d.module() - range) * rangeFactor;
		if (curS > maxS) curS = maxS;
		
		double dt = (destination->speed * t + destination->force / destination->mass / 2 * 0.5 * t * t).module();
		double cdt = curS * t;
		
		if (cdt >= d.module()) position = destination->position;
		else if (d.module() >= range){ speed = d.setModule(curS); position += speed * t; }//Moves
		else if (dt < range){ position = destination->position; speed = vector (0,0); }//Centers
		else speed = vector(0,0);//Stops if too close
	}
	
	//Function to get shown area
	rectangle viewport(int w, int h){
		rectangle result;//Result
		
		//Sets rectangle data
		result.x = position.x - w / 2;
		result.y = position.y - h / 2;
		result.w = w;
		result.h = h;
		
		return result;//Returns result		
	}
} cam;

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

//Update function
void update(){
	updateCommon();
	common.print(video);

	UPDATE;
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
	Uint32 color;//Area color
	
	//Constructor
	area(){
		rules::id = "";
		rectangle::id = "";
		
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
		if (rules::fromScriptObj(o) && rectangle::fromScriptObj(o)){//If succeeded loading base data	
			var* color = get <var> (&o.v, "color");//Area color
			if (color) this->color = strtol(color->value.c_str(), NULL, 0);//Gets color
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = rectangle::toScriptObj();//Rules data
		result += rules::toScriptObj();//Adds rectangle
		
		result.set("color", color);
		
		return result;//Returns result
	}
	
	//Assignment for rules
	void setRules (rules r){
		jumpImpulse = r.jumpImpulse;
		
		groundSpeed = r.groundSpeed;
		groundForce = r.groundForce;
		groundDamping = r.groundDamping;
		
		airSpeed = r.airSpeed;
		airForce = r.airForce;
		
		gravity = r.gravity;
		
		jumpCount = r.jumpCount;
		
		setMask = r.setMask;
	}
	
	//Equal operator
	bool operator == (area a){
		return rectangle::id == a.rectangle::id;
	}
};

//Specific get for areas
template <> area* get<area> (list<area>* c, string id){
	list<area>::iterator i;//Iterator
	
	for (i = c->begin(); i != c->end(); i++)//For each area
		if (i->rules::id == id) return &*i;//Returns if matching
		
	return NULL;
}

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
	list<area> areas;//Areas
	
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
		
		lvlRules.id = "rules";
		result.o.push_back(lvlRules.toScriptObj());//Adds rules
		
		list<area>::iterator a;//Area iterator
		for (a = areas.begin(); a != areas.end(); a++)//For each area
			result.o.push_back(a->toScriptObj());//Adds areas
			
		result.set("message", join(message, "_"));//Sets msg var
		
		return result;//Returns result
	}
	
	//Function to print the level
	void print(SDL_Surface* target, int x, int y, bool hidden = false){
		SDL_Rect offset {x, y};//Offset rect
		SDL_BlitSurface(backgroundImage, NULL, target, &offset);//Prints background
		
		list<area>::iterator a;//Area iterator
		for (a = areas.begin(); a != areas.end(); a++){//For each area
			SDL_Rect r = {a->x, a->y, a->w, a->h};//Rectangle
			r.x += x;//Offset x
			r.y += y;//Offset y
			
			SDL_FillRect(target, &r, a->color);//Fills area
		}
		
		if (printLevelScene) printScene(this, target, x, y, hidden);//Prints scene elements
	}
	
	//Function to get a copy of the level
	level* copy(){
		level* result = new level(*this);
		
		result->entities.clear();//Removes entities
		result->links.clear();//Removes links
		
		list<entity*>::iterator e;//Entity iterator
		for (e = entities.begin(); e != entities.end(); e++){//For each entity
			entity* ne = NULL;//New entity
			
			if ((*e)->eType == etype_box)//If entity is a box
				ne = new box(* (box*) (*e));//Allocates as box
			
			else if ((*e)->eType == etype_ball)//If entity is a ball
				ne = new ball(* (ball*) (*e));//Allocates as ball
				
			if (ne) result->entities.push_back(ne);//Adds new entity
		}
		
		
		////NOT WORKING!!!!!!///////////////////////////////////////
		deque<phLink*>::iterator l;//Link iterator
		for (l = links.begin(); l != links.end(); l++){//For each link
			phLink* nl = NULL;//New link
			
			if ((*l)->type == OBJTYPE_SPRING)//If link is a spring
				nl = new spring (* (spring*) (*l));//Allocates as spring
				
			if (nl->a) nl->a = get_ptr <entity> (&result->entities, nl->a->id);
			if (nl->b) nl->b = get_ptr <entity> (&result->entities, nl->b->id);
			
			nl->a_point = nl->a->addNode(*nl->a_point);
			nl->b_point = nl->b->addNode(*nl->b_point);
		}
		
		return result;//Returns result
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
	
	deque<language> langs;//Languages
	
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
			
			deque<object>::iterator ob;//Object iterator
			for (ob = o.o.begin(); ob != o.o.end(); ob++){//For each object
				if (ob->type == OBJTYPE_LANGUAGE){//If object is a language
					langs.push_back(*ob);//Adds to level set languages
					addLang(*ob);//Adds to language database
				}
			}
			
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
		
		int n;
		for (n = 0; n < langs.size(); n++) result.o.push_back(langs[n]);
		
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
	
	deque<language> langs;//Languages bound to level set
		
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
				
				if (ob->type == OBJTYPE_LANGUAGE){//If object is a language
					langs.push_back(*ob);//Adds to level set languages
					addLang(*ob);//Adds to language database
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
		for (n = 0; n < langs.size(); n++) result.o.push_back(langs[n]);
		
		return result;
	}
};

levelSet lpEdited;//Edited level set

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
	
	double globalDeaths, globalTime;//Global deaths, global time played
	
	//Constructor
	globalProgress(){
		id = "";
		type = OBJTYPE_GLOBALPROGRESS;
		
		globalDeaths = 0;
		globalTime = 0;
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
			var* deaths = get <var> (&o.v, "deaths");//Deaths variable
			var* time = get <var> (&o.v, "time");//Time variable
			
			if (ach) this->unlockedAch = tokenize <deque<string> > (ach->value, ",");//Gets achievements
			if (deaths) this->globalDeaths = deaths->intValue();//Gets deaths
			if (time) this->globalTime = time->doubleValue();//Gets time
	
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
		
		o.set("deaths", globalDeaths);//Sets deaths
		o.set("time", globalTime);//Sets time
		
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
				
				if (!l){//If level set doesn't exist
					s = unlockedAch.erase(s);//Erases element
					s--;//Goes back
					continue;//Next loop
				}
				
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
		rules currentRules = getAreaRules();//Gets rules
		
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
			
			if (keys[playerControls.up] && releasedJump && (playerJumps < currentRules.jumpCount || currentRules.jumpCount < 0)){//If pressed up and can jump
				player->speed.y = 0;//Stops on y
				player->applyImpulse(player->position, {0, -currentRules.jumpImpulse * currentRules.gravity.y / abs(currentRules.gravity.y)});//Applies impulse
				
				playerJumps++;//Increases jump count
				releasedJump = false;//Not released jump
			}
			
			if (right){//If pressing right
				if (ground && player->speed.x < currentRules.groundSpeed)//If on ground and moving slower
					player->applyForce(player->position, {currentRules.groundForce, 0});//Applies force
					
				else if (!ground && player->speed.x < currentRules.airSpeed)//If on air and moving slower
					player->applyForce(player->position, {currentRules.airForce, 0});//Applies force
			}
			
			else if (left){//If pressing left
				if (ground && player->speed.x > -currentRules.groundSpeed)//If on ground and moving slower
					player->applyForce(player->position, {-currentRules.groundForce, 0});//Applies force
					
				else if (!ground && player->speed.x > -currentRules.airSpeed)//If on air and moving slower
					player->applyForce(player->position, {-currentRules.airForce, 0});//Applies force
			}
			
			else if (ground) player->applyForce(player->position, {-player->speed.x * currentRules.groundDamping, 0});//Applies damping on ground
		}
	}
	
	//Function to check for area rules
	rules getAreaRules(){
		list<area>::iterator a;//Area iterator
		
		rules result = defaultRules + loadedRules + levels.lsRules + currentLevel->lvlRules;//Sets rules
		
		for (a = currentLevel->areas.begin(); a != currentLevel->areas.end(); a++)//For each area
			if (a->isInside(player->position.x, player->position.y)) result += *a;//Adds area rules if player is inside
			
		return result;//Returns result
	}
	
	//Function to get rules in given point
	rules getAreaRules(vector p){
		list<area>::iterator a;//Area iterator
		
		rules result = defaultRules + loadedRules + levels.lsRules + currentLevel->lvlRules;//Sets rules
		
		for (a = currentLevel->areas.begin(); a != currentLevel->areas.end(); a++)//For each area
			if (a->isInside(p.x, p.y)) result += *a;//Adds area rules if point is inside
			
		return result;//Returns result
	}
	
	//Function to print
	void print(SDL_Surface* target, int x, int y){
		if (currentLevel){//If current level is valid
			currentLevel->print(target, x, y);//Prints current level
			
			if (!currentLevel->printLevelScene){//If level scene wasn't printed
				if (player) printEntity(player, target, 0, x, y);//Prints player
				if (goal) printEntity(goal, target, 0, x, y);//Prints goal
			}
			
			if (goal && !cam.viewport(video_w, video_h).isInside(goal->position.x, goal->position.y)){//If goal is out of screen
				vector dist = goal->position - cam.position;//Vector from goal to camera
				vector pos = dist;//Position
				
				//Viewport corners
				vector a (video_w / 2, video_h / 2);
				vector b (-video_w / 2, video_h / 2);
				vector c (-video_w / 2, -video_h / 2);
				vector d (video_w / 2, -video_h / 2);
				
				double ang = dist.angleDeg();//Angle
				
				//Gets posiion vector
				if ((ang > a.angleDeg() && ang < d.angleDeg()) || (ang > c.angleDeg() && ang < b.angleDeg())) pos = pos * video_w / 2 / abs(pos.x);
				else pos = pos * video_h / 2 / abs(pos.y);
				
				SDL_Surface* src = rotozoomSurface(destinationArrow, ang, 1, SMOOTHING_ON);//Rotated surface
				SDL_Rect offset = {video_w / 2 + pos.x - src->w, video_h / 2 + pos.y - src->h};//Offset rect
				SDL_BlitSurface(src, NULL, target, &offset);//Prints arrow
				
				SDL_FreeSurface(src);//Frees surface
			}
		}
	}
	
	//Function to setup a level
	void setup(int levelIndex, bool death = false, bool moveCam = true){
		if (currentLevel) delete currentLevel;//Deletes level if existing
		currentLevel = loadLevel(levels[levelIndex]);//Loads level
		
		this->levelIndex = levelIndex;//Sets level index
		
		setup(death, moveCam);//Sets general data
	}
	
	//Function for general setup
	void setup(bool death = false, bool moveCam = true){
		player = get_ptr <entity> (&currentLevel->entities, "player");//Gets player
		goal = get_ptr <entity> (&currentLevel->entities, "goal");//Gets goal
		
		playerStart = player->position;//Sets player starting position
		playerAngle = player->theta;//Sets player starting angle
		
		releasedJump = true;
		playerJumps = 0;
		
		if (!death){//If not setting up cause death
			deque<string>::iterator i;//String iterator
			string ans[] = { getText("continue") };//Answer
			
			for (i = currentLevel->message.begin(); i != currentLevel->message.end(); i++){//For each message
				BKG;
				msgBox.show(video, getText(*i), 1, ans, true, false);//Shows message
			}
		
			time = 0;//Resets timer
			deaths = 0;//Resets death counter
			lastFrameTime = SDL_GetTicks();//Resets frame time
			
			if (player && moveCam) cam.position = player->position;//Centers camera
			
		}
		
		if (death) releasedJump = false;
		if (player) cam.destination = player;//Sets cam destination
		
		paused = false;//Unpauses
		completed = false;//Not completed
	}
	
	//Function to reset current level
	void reset(){	
		setup(levelIndex, true);//Resets level
		deaths++;//Increases death counter
		if (!debugMode) progress.globalDeaths++;//Increases global deaths counter
	}
	
	//Function to move onto next level (if any)
	void next(){
		if (levelIndex < levels.size() - 1) setup(levelIndex + 1);//Sets up next level
	}
	
	//Function to check relevant collisions
	void checkRelevant(){
		list<collision>::iterator i;//Iterator
		
		if (!player) return;//Exits function if no player was given
		
		for (i = c.begin(); i != c.end(); i++){//For each collision
			if ((i->a == player && i->b->special == "hazard") || (i->a->special == "hazard" && i->b == player)){//If player hit hazard
				PLAYSOUND(deathSfx);//Plays sound
				reset();//Resets
				break;//Exits loop
			}
			
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
			list<entity*>::iterator i;//Entity iterator
			deque<area>::iterator a;//Area iterator
			
			for (i = currentLevel->entities.begin(); i != currentLevel->entities.end(); i++)//For each entity in level
				(*i)->applyForce((*i)->position, getAreaRules((*i)->position).gravity * (*i)->mass);//Applies gravity
			
			c = currentLevel->step(t);//Steps
			checkRelevant();//Steps level
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
		
		if (!debugMode) progress.globalTime += double(frameBegin - lastFrameBegin) / 60000;//Increases global time counter
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

	else if (id == "globalDeaths")//If required global deaths
		return new double(progress.globalDeaths);
		
	else if (id == "globalTime")//If required global time
		return new double(progress.globalTime);
	
	else { cout << "Failed: " << id << endl; return new double(0); }//Returns 0 if failed
}

//Load sets prototype
void loadSets();

//Install set prototype
void installSet(string);

#include "editor.h"//Includes editor
#include "lpEditor.h"//Includes level set editor
#include "ui.h"//Includes user interface header

//Function to test a level
void test(string levelPath, rules lsRules){
	current.levels = * new levelSet;//New level set
	current.levels.lsRules = lsRules;//Sets rules
	
	current.levels.push_back(levelPath);//Adds level
	
	current.success = NULL;//Disables success function
	
	current.setup(0, false, false);//Setup game
	cam.position = vector(current.currentLevel->w / 2, current.currentLevel->h / 2);
	
	SDL_Event e;//Event
	bool testing = true;//Testing flag
	
	uiMode u = curUiMode;//Stores old ui mode
	curUiMode = ui_game;//Sets game mode
	
	btnBack->release.handlers.clear();//Removes back button handlers
	
	while (testing){//Game loop
		FRAME_BEGIN;
		
		if (curUiMode == ui_game){//If in game mode
			hideCursor = true;//Sets hide cursor flag
			
			BKG;//Prints background
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN){//If pressed a key
					if (ev.key.keysym.sym == SDLK_ESCAPE) testing = false;//Stops test on esc
					else current.paused = false;//Unpause
				}
					
				//Checks hud events
				hud.checkEvents(ev, 2, 2);
			}
			
			if (current.completed) {//If completed level
				PLAYSOUND(successSfx);//Plays sound
				current.setup(current.levelIndex, false, false);//Resets level
			}
			
			current.print(video, video_w / 2 - cam.position.x, video_h / 2 - cam.position.y);//Prints game scene
			
			updateHud();//Updates hud
			hud.print(video, 2, 2);//Prints hud on upper-left level
			current.frame(double((frameBegin - lastFrameBegin)) * 0.0125, keys);//Game frame
			cam.move(double((frameBegin - lastFrameBegin)) * 0.0125);//Moves camera
		}
		
		if (curUiMode == ui_paused){//If paused
			hideCursor = false;//Sets hide cursor flag
			
			BKG;//Prints background
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN){//If pressed a key
					if (ev.key.keysym.sym == SDLK_ESCAPE) testing = false;//Stops test on esc
					else if (ev.key.keysym.sym == SDLK_RETURN) resumeClick({});//Resumes on enter
				}
				
				if (btnBack->release.triggered) testing = false;//Quits on back
				
				pauseWindow.checkEvents(ev);//Checks pause events
			}
			
			current.print(video, video_w / 2 - cam.position.x, video_h / 2 - cam.position.y);//Prints game scene
			
			hud.print(video, 2, 2);//Prints hud on upper-left level
			
			pauseWindow.print(video);//Prints pause screen
		}
		
		UPDATE;
		FRAME_END;
	}
	
	btnBack->release.handlers.push_back(backClick);
	current.success = showSuccess;
	
	curUiMode = u;//Resets ui mode
}

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
	var* v_videoW = get <var> (&g.v, "video_w");
	var* v_videoH = get <var> (&g.v, "video_h");
	var* v_sound = get <var> (&g.v, "enableSfx");
	var* v_debug = get <var> (&g.v, "debugMode");
	
	object* c = get <object> (&g.o, "controls");
	
	var* v_updatesFile = get <var> (&g.v, "updatesFile");
	var* v_updatesCount = get <var> (&g.v, "updatesCount");
	
	if (v_fullscreen) fullscreen = v_fullscreen->intValue();
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
bool processUpdateScript(script u){
	script::iterator i;//Iterator for script lines
	bool failed = false;
	
	for (i = u.begin(); i != u.end(); i++){//For each line
		deque<string> t = tokenize <deque<string> > (*i, " ");//Splits into tokens
				
		if (t[0] == "download" && t.size() >= 3){//Download command
			CURLcode result = downloadFile(t[1], t[2]);//Downloads file
			
			if (result != CURLE_OK){//If failed
				failed = true;//Sets failed flag
				cerr << "ERROR - DOWNLOADING:\nfile:    " << t[1] << "\ndest:    " << t[2] << "\nerror   :" << curl_easy_strerror(result) << "\n\n";//Error message
			}
			
			if (!ifstream(t[2].c_str())){ cerr << "ERROR - DOWNLOADING:\nfile:    " << t[1] << "\ndest:    " << t[2] << "\nerror   : UNKNOWN\n\n"; failed = true; }//Error message
		}
			
		else if (t[0] == "mkdir" && t.size() >= 2)//Makedir command
			#ifdef __WIN32__
			CreateDirectory  (t[1].c_str(), NULL);//Creates directory
			#else if defined __linux
			mkdir(t[1].c_str(), 0666);//Creates directory
			#endif
		
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
	
	if (failed) msgBox.show(video, "Download failed.", 1, msgBox_ans_ok);//Error message
	
	return !failed;
}

//Function to provess an update script from file path
bool processUpdateScript(string path){
	script s;//New script
	s.fromFile(path);//Opens
	return processUpdateScript(s);//Processes script
}

//Function to check for updates
void checkUpdates(bool silent, bool dark){
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
		
		if (s > 0 && msgBox.show(video, getText("upd_install", toString(s).c_str()), 2, msgBox_ans_yn, false, dark) == 0){//If user decides to download
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
		
		else if (s == 0 && !silent) msgBox.show(video, "Database is up to date", 1, msgBox_ans_ok, false, dark);//up to date message
	}
	
	else switch (result){
		case CURLE_COULDNT_CONNECT: break;//Connection error: breaks
		case CURLE_COULDNT_RESOLVE_HOST: BKG; msgBox.show(video, getText("upd_listNotFound"), 1, msgBox_ans_ok); break; //Error message if couldn't find file
		default: cerr << "UPDATES ERROR: " << curl_easy_strerror(result); break;//Unexpected error
	}

	remove("tmp_updates");//Removes updates file
}

//Function to initialize language
void initLang(){
	char lang[3] = "";//Locale string
	
	#if defined(__WIN32__) || defined(__WIN64__)//On Windows
	GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SISO639LANGNAME, lang, sizeof(char) * 2);//Locale id
	#else ifdef __linux__//On linux
	lang = getenv("LANG");//Gets environmental language variable
	#endif
	
	if (!loadLanguagesDB(localePath + string(lang) + ".cfg")){//If fails loading language
		loadLanguagesDB(localePath + defaultLocale + ".cfg");//Loads default language
		SETLANG(defaultLocale);//Sets default language
	}
	
	else SETLANG(string(lang));//Else sets loaded language
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
	SDL_WM_SetIcon(NULL, NULL);//Sets icon

	current.success = showSuccess;//Sets success function
		
	loadSettings();//Loads settings
	loadGraphics();//Loads graphics
	
	loadSound();//Loads sound
	
	loadSets();//Loads level sets
	loadAchievements();//Loads achievements
	
	initLang();//Inits language
	
	loadRules();//Loads rules
	loadProgress();//Loads game progress
	
	loadUI();//Loads ui
	
	loadEditor();//Loads editor
	loadLpEditor();//Loads level pack editor

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

//Windows-specific init and close function
#if defined(__WIN32__) || defined(__WIN64__)
HICON icon;
HWND hwnd;

//Initialization
void win32_init(int argc, char* argv[]){
	SDL_putenv ("SDL_VIDEODRIVER=directx");//Sets video driver to directx
	gameInit(argc, argv);//Normally inits
	
	HINSTANCE handle = GetModuleHandle(NULL);
	icon = LoadIcon(handle, MAKEINTRESOURCE(1));//Loads icon
	
	SDL_SysWMinfo wminfo;//Window manager information
	SDL_VERSION(&wminfo.version);
	SDL_GetWMInfo(&wminfo);//Gets window manager info
	
	hwnd = wminfo.window;//Gets window handler
	
	SetClassLong(hwnd, GCL_HICON, (LONG) icon);
}

#endif