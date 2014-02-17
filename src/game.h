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
GAME LOGICS
*/

#ifndef _GAME
#define _GAME

#include "content.h"
#include "image.h"
#include "files.h"
#include "utils.h"
#include "log.h"

#include "physics.h"
#include "physics_gfx.h"

#include <algorithm>

class game;
class levelPack;
class achievement;

#include "ui_pieces.h"

//Control scheme class
class controls: public content {
	public:
	SDL_Scancode up, left, right;//Movement keys
	
	controls();
	controls(const controls&);
	void load(xml_node);
	void save(xml_node*);
};

//Camera class
class camera {
	public:
	vect position;
	vect speed;
	
	entity* destination;
	
	double range, rangeFactor, speedModule, maxSpeed;
	
	camera();
	void move(double);
	_rect viewport(int w, int h);
};

extern controls playerControls;

//Basic ruleset class
class _rules {
	public:
	double jumpImpulse;//Module of impulse applied when jumping
	
	double groundSpeed;//Module of maximum horizontal speed on ground
	double groundForce;//Module of horizontal force when moving on ground
	double groundDamping;//Damping factor on the ground
	
	double airSpeed;//Module of maximum horizontal speed in air
	double airForce;//Module of horizontal force when moving in air
	
	vect gravity;//Gravity acceleration
	
	int jumpCount;//Total available jumps
	
	//Mask for set values
	//When stacking rules, only values with corresponding bit set to 1
	//are considered
	int mask;
	
	_rules();
	
	//Function to stack rules
	_rules operator + (_rules);
	void operator += (_rules);
};

//Ruleset class
class rules: public content, public _rules {
	public:
	
	rules();
	rules(const rules&);
	void load(xml_node);
};

extern rules defaultRules;
extern rules loadedRules;

//Area class
//Joins a rectangle with the rules to be applied inside
//that rectangle
class area: public rect {
	public:
	rules areaRules;//Rules applied within the area
	
	area();
	area(const area&);
	void load(xml_node);
};

//Level class
class level: public scene {
	public:
	int twoStarsTime, threeStarsTime;//Time required for two and three stars rating (in seconds)
	
	rules lvlRules;//Level specific rules
	list<area> areas;//Areas
	
	string path;//Level file path
	
	level();
	void load(xml_node);
	
	//Print function
	void print(SDL_Renderer*, int, int, bool = false);
	
	//Returns a copy of the level
	level* copy();
	
	//Reloads level from file
	void reload();
};

//Achievement class
class achievement: public content {
	public:
	string expression;//Javascript expression to be verified when checking achievement
	image icon;//Achievement icon
	string name, info;//Achievement name and information
	
	//If checkOnce is true, achievement is no more checked once it has been unlocked
	//Else, expression is checked even later and possibly the achievement is re-locked
	//if expression isn't verified anymore
	bool checkOnce;
	
	achievement();
	achievement(const achievement&);
	achievement(string, string, string, string, image, bool);
	void load(xml_node);
	
	bool verify(CTinyJS*);
};

extern list <achievement> globalAchievements;

//Function to get an achievement by id
achievement* getAchievement ( string );

//Function to load all achievements
void loadAchievements ();

//Level progress
//Best times, least deaths, best rating
class levelProgress: public content {
	public:
	bool unlocked;//If true, level is unlocked and playable
	int time, deaths, rating;//Best results
	
	levelProgress();
	void load(xml_node);
	void save(xml_node*);
	
	//Gets level pack id of corresponding level
	string packId();
};

//Level pack
class levelPack: public content {
	public:
	image icon;//Level pack icon
	string name;
	int order;//Order (used to sort level packs)
	
	//Levels are not stored immediately as levels themselves but keeping their file path and loading them on request
	//This allows to load levels only when they're needed instead of all at game startup (which would add to
	//the long loading time with no relevant advantage) and is useful if levels are modified within the editor
	//The first time the level pack is effectively opened (e.g. when selecting it in the level pack selector menu)
	//levels are loaded and stored in the levels member
	deque<string> levelFiles;
	deque<level*> levels;
	
	list<achievement> lpAchs;//Level pack specific achievements
	
	rules lpRules;//Level pack specific rules
	
	//Constructor
	levelPack();
	
	//Save and load function
	void load(xml_node);
	
	//Function to load levels
	void loadLevels();
	
	//Function to get achievement
	achievement* getAchievement(string);
};

//Comparison function (used to sort level packs)
bool levelPack_compare(levelPack*, levelPack*);

extern list<levelPack*> levelPacks;//Loaded level packs

//Returns required level pack
levelPack* getPack(string);

//Function to load level packs
void loadLevelPacks();

//Global progress class
//Stores progress for all levels
class globalProgress: public content, public list<levelProgress> {
	public:
	deque<string> unlockedAch;//Unlocked achievements IDs
	
	double globalDeaths, globalTime;//Total deaths and total time played
	
	globalProgress();
	void load(xml_node);
	void save(xml_node*);
	
	//Fills in progress data for given level
	void fillProgress(string, int, int, int);
	
	//Gets rating of required level
	int getRating(string);
	
	//Returns if a level is playable or not
	bool canPlay(string);
	
	//Unlocks a level
	void unlock(string);
	
	//Verifies achievements
	void verifyAchs();
	
	//Returns completed levels count
	int completed();
	
	//Returns completed levels of specific pack
	int completed(string);
	
	//Returns completion percent of given set
	int percent(string);
	
	//Checks progress data and removes invalid levels
	void check();
};

//Current progress
extern globalProgress progress;

//Game class
//Stores all information about current game
class game {
	public:
	levelPack* currentLevelPack;
	level* currentLevel;
	int levelIndex;//Index of current level within level pack
	
	entity* player;
	entity* goal;
	
	rules gameRules;
	
	//Control data
	bool releasedUp;//If true, up key has been released and can be checked again
	int playerJumps;//Counts the number of time player has jumped
	
	bool paused;//Pause flag
	int lastFrameTime;//Time before pause
	
	int time;//Total time
	int deaths;//Deaths counter
	
	bool completed;//Completed flag
	
	list<collision> c;//List of collisions happened during last frame
	
	camera cam;//Camera used when printing game scene
	
	game();
	
	//Handles controls
	void handleControls(Uint8*);
	
	//Returns rules to be applied to player, considering areas
	_rules getAreaRules();
	
	//Gets rules in given point, considering areas
	_rules getAreaRules(vect);
	
	//Prints game
	void print(SDL_Renderer*, int, int);
	
	//Sets up a level
	void setup(int, bool = false, bool = true);
	
	//General setup (level has already been loaded)
	void setup(bool = false, bool = true);
	
	//Resets current level after death
	void reset();
	
	//Moves onto next level (if any) and returns true if succeeded
	bool next();
	
	//Checks relevant collisions
	void checkRelevant();
	
	//Time step
	void step (double);
	
	//Handles frame
	void frame (double, Uint8*);
	
	//Calculates rating of current level
	int rating();
	
	//Stores variables inside js context
	void storeVariables ( CTinyJS* );
};

extern game currentGame;

//Function to setup JS game functions
void registerGameFuncs(CTinyJS*);

#endif
