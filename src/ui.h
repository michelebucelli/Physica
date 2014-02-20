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
USER INTERFACE
This provides a framework for user interface handling
User interface is based on a tree structure, where each node is
a control
*/

#ifndef _UI
#define _UI

#include <list>
#include <deque>
#include <algorithm>

#include "TinyJS/TinyJS.h"
#include "TinyJS/TinyJS_MathFunctions.h"

#include "image.h"
#include "viewport.h"
#include "files.h"
#include "locale.h"

#include "SDL2/SDL_ttf.h"

class color;
class font;
class themeElement;
class theme;
class event;
class controlContent;
class control;

class cachedFont {
	public:
	TTF_Font* font;
	int size;
	string path;
};

extern list<cachedFont> cachedFonts;

//Color class
//Joins a standard SDL color with functions to convert in other formats
//and to load from XML
class color: public content, public SDL_Color {
	public:
	
	color();
	
	//Function to convert to RGB integer
	int toInt();
	
	//Function to load from RGB integer
	void fromInt(int);
	
	//Function to load from XML
	void load(xml_node);
	
	//Function to store into javascript variable
	void toJSVar(CScriptVar*);
	
	//Function to load from javascript variable
	void fromJSVar(CScriptVar*);
};

TTF_Font* openFont ( const char*, int );

//Font class
//Joins a TTF_Font with color and a function to load from XML
class font: public content {
	public:
	
	string filePath;//Font file path
	TTF_Font* f;//Font
	
	color col;//Font color
	
	font();
	
	//Function to load from XML
	void load(xml_node);
	
	//Function to render a text
	SDL_Surface* render(string);
	
	//Function to store into javascript variable
	void toJSVar(CScriptVar*);
	
	//Function to load from javascript variable
	void fromJSVar(CScriptVar*);
};

//Registered font errors
extern list<string> fontErrors;

//Minimal position class
//Gives information about relative position
//No xml/js support
class _position {
	public:
	
	int xRef, yRef;//X and Y reference (0,1,2 for left-centre-right and top-middle-bottom)
	int x, y;//Actual x and y coords
	
	_position();
	_position(int, int, int, int);
	
	//Function to get absolute position relative to given rectangle (requires width and height of element)
	//Result has reference 0,0
	_position absolutePos(_rect, int, int);
};

//Position class
//Gives position information relative to rectangle
class position: public content, public _position {
	public:
	
	position();
	
	//Constructor from minimal
	position(_position);
	
	//Function to load from xml
	void load(xml_node);
	
	//Function to store into js variable
	void toJSVar(CScriptVar*);
	
	//Function to load from js variable
	void fromJSVar(CScriptVar*);
};

//Theme element class
//Represents a single image within a theme. It joins an image with info
//about its position
class themeElement: public position {
	public:	
	renderable* img;//Renderable of the element
	
	themeElement();
	
	//Function that prints the element within given rectangle
	void print(SDL_Renderer*, _rect);
	
	//Function to load theme element
	void load(xml_node);
	
	//Function to store into javascript variable
	void toJSVar(CScriptVar*);
	
	//Function to load from javascript variable
	void fromJSVar(CScriptVar*);
};

//Theme class
//Provides an interface to print a rectangle object on a SDL_Surface
//according to theme specific parameters
class theme: public content {
	public:
	
	//Rectangle pieces
	renderable* topLeft;
	renderable* top;
	renderable* topRight;
	renderable* left;
	renderable* centre;
	renderable* right;
	renderable* bottomLeft;
	renderable* bottom;
	renderable* bottomRight;
	
	list<themeElement> elements;//Individual elements list
	list<font> fonts;//Theme fonts list
	
	theme();
	
	//Function to print the theme on a rectangle
	void print(SDL_Renderer*, _rect);
	
	//Function to load theme
	void load(xml_node);
	
	//Function to store into javascript variable
	void toJSVar(CScriptVar*);
	
	//Function to load from javascript variable
	void fromJSVar(CScriptVar*);
};

//Mouse status enumeration
enum mouseStatus {
	ms_outside = 0,//Mouse outside the control
	ms_over = 1,//Mouse placed over the control
	ms_pressed = 2//Mouse pressed inside the control
};

//Control status enumeration
enum controlStatus {
	cs_normal = 0,//Normal status
	cs_active = 3,//Active status (e.g. checked checkbox)
	cs_disabled = 6//Disabled status (not printed, doesn't trigger events)
};

//Event data union
union _eventData {
	struct { int x; int y; int button; } mouse;//Mouse event data
	struct { SDL_Keysym key; } keyboard;//Keyboard event data
	struct { char* text; } textInput;//Text input event data
	struct { CScriptVar* var; } custom;//Custom event data
};

//Event data class
class eventData {
	public:
	int type;//Data type (mouse = 0, keyboard = 1, custom = 2, textInput = 3)
	_eventData data;//Data
	
	eventData();
	
	//Function to store to JS variable
	void toJSVar(CScriptVar*);
};

//Event class
class event: public content {
	public:
	list < void (*) (control*, eventData*) > handlers;//Event handlers: get called when event is triggered
	string script;//Javascript to be executed on event triggered
	int status;
	
	event();
	event(string, void (*) (control*, eventData*), int = -3);
	
	//Function to trigger the event
	void trigger(control*, eventData*);
	
	//Function to load the event from XML
	void load(xml_node);
};

//Control content class
//Represents any kind of content of a control (besides children controls)
class controlContent: public position {
	public:
	
	deque<string> textParameters;//Text parameters
	union { string *text; renderable *img; } data;//Content data
	string fontId;//Name of the font used to print this content (relative to fonts within given theme)
	
	controlContent();
	
	//Function to print content
	void print(SDL_Renderer*, rect, theme*);
	
	//Function to load from XML
	void load(xml_node);
	
	//Function to store into javascript variable
	void toJSVar(CScriptVar*);
	
	//Function to load from javascript variable
	void fromJSVar(CScriptVar*);
	
	//Function to set a parameter to text
	void setTextParameter(int, string);
};

//Basic timeout structure
struct timeout {	
	int startTime;//Time when timeout was setup
	int time;//Time after which timeout is triggered
	
	int type = 0;//Type (0 for event, 1 for script)
};

//Event timeout structure
struct eventTimeout: public timeout {	
	string eventType;//Type of the triggered event
	eventData* data;//Event data (unused at the moment)
};

//Script timeout structure
struct scriptTimeout: public timeout {
	string script;
};

//Control class
//Each control is a node of the UI tree. It links towards its parent
//and its children
class control: public content {
	public:
	
	control* root;//Root control (the topmost of the control tree)
	control* parent;//Parent control
	list<control*> children;//Children controls
	
	//If eventGrabber is pointing to a child control, all events are passed only to the eventGrabber control
	//and events are ignored within other controls
	control* eventGrabber;
	
	control *nextSibling, *prevSibling;
	
	list<controlContent*> cContent;//Content of the control
	
	rect area;//Control area
	bool dontUseAsReference;//If true, control isn't used as reference when printing child controls
	
	bool draggable;//If true, control can be dragged around
	bool dragging;//If true, control is being dragged
	int dragInitialX, dragInitialY;//Drag initial positions
	int dragGrid;//Drag grid size
	
	bool clickThrough;//If true, parent control can be dragged even clicking on this
	
	mouseStatus mStatus;//Mouse status
	controlStatus cStatus;//Control status
	
	bool visible;
	
	theme* themes[9];//Themes array
	
	image mask;//Control mask (black for empty, anything else for full)
	
	//Events are triggered according to their type
	//For example: on mouse pressure, all events with type "onMouseDown" are triggered
	//Known event types:
	//onMouseDown / onMouseUp - triggered on mouse pressure/release
	//onKeyDown - triggered when pressing key
	//onRefresh - triggered when refreshing control
	//onDraw - triggered when drawing control
	list <event> events;//Control events
	
	//Event timeouts
	list <timeout*> timeouts;
	
	CScriptVar* jsVar;//Script variable associated to the control
	
	control();
	
	//Function to load control
	void load(xml_node);
	
	//Function to print control
	void print(SDL_Renderer*, _rect*);
	
	//Function to print control only
	void printBase(SDL_Renderer*, _rect*);
	
	//Function to print children
	void printChildren(SDL_Renderer*, _rect*);
	
	//Function to trigger all events with given type
	void triggerEvent(string, eventData*, bool = false);
	
	//Function to trigger all events with given type, including children events
	void triggerEventChildren(string, eventData*, bool = false);
	
	//Function to check events on control
	void handleEvents(SDL_Event*, _rect, bool = false);
	
	//Function to check events on control only
	void handleEventsBase(SDL_Event*, _rect, bool = false);
	
	//Function to check events on children
	void handleEventsChildren(SDL_Event*, _rect, bool = false);
	
	//Function to save control data inside a javascript variable
	void toJSVar(CScriptVar*);
	
	//Function to load control from javascript variable
	void fromJSVar(CScriptVar*);
	
	//Function to determine if a point is inside the control
	bool isInside(int, int, rect);
	
	//Function to get a child by id
	control* getChild(string);
	
	//Function to refresh
	void refresh();
	
	//Function to draw control content
	void draw();
	
	//Function to clear script variable
	void clearScriptVar();
	
	//Function that generates the script variable
	//Generates only basic content (no links between controls within same tree)
	void genScriptVar();
	
	//Function that links script var towards parent and children
	void linkScriptVar();
	
	//Function to reload control from its own js variable
	void reload();
	
	//Function to execute a script on the control
	void runScript(string);
	
	//Function to get specific content
	controlContent* getContent(string);
	
	//Function for control copy
	control* copy();
	
	//Function to add child control
	void addChild(control*);
	
	//Function to clear children
	void clear();
	
	//Function to get if any of the children has an event grabber
	//If so, even events of this control are not triggered
	//Returns the child with event grabber, or null if none
	control* childHasGrabber();
	
	//Sets root of this and of all children
	void setRoot(control*);
};

//Scripting:
//Functions and variables to handle event scripting on controls

extern control* scriptTargetControl;//Target for the script actions for UI events
extern void (*customSetupJS_UI)(CTinyJS*);//Custom javascript setup function - called when setting up javascript context for UI events

//Javascript setup function
void registerUIFunctions(CTinyJS*);

//Script functions
void scAddChildFromFile(CScriptVar*, void*);//Adds child
void scRemoveChild(CScriptVar*, void*);//Removes child

void scGrabEvents(CScriptVar*, void*);//Grabs events
void scReleaseEvents(CScriptVar*, void*);//Releases events

void scTriggerEvent(CScriptVar*, void*);//Triggers events
void scEventTimeout(CScriptVar*, void*);//Triggers events after some time
void scScriptTimeout(CScriptVar*, void*);//Executes script after some time

void scStartTextInput(CScriptVar*, void*);//Starts text input
void scStopTextInput(CScriptVar*, void*);//Stops text input

#endif
