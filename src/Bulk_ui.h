//Buch's Library Kollection
//User interaface handling header

#ifndef _BULK_UI
#define _BULK_UI

//Script object types
#define OBJTYPE_FONT		"font"//Font objects
#define OBJTYPE_UIELEMENT	"element"//Ui element objects
#define OBJTYPE_THEME		"theme"//Theme objects
#define OBJTYPE_CONTENT		"content"//Content objects
#define OBJTYPE_CONTROL		"control"//Control objects
#define OBJTYPE_PANEL		"panel"//Panel objects
#define OBJTYPE_INPUTBOX	"inputBox"//Input box objects
#define OBJTYPE_FILLBAR		"fillbar"//Fillbar objects
#define OBJTYPE_WINDOW		"window"//Window objects
#define OBJTYPE_LISTBOX		"listBox"//List box objects
#define OBJTYPE_CHECKBOX	"checkBox"//Check box objects
#define OBJTYPE_KEYBOX		"keyBox"//Key box objects

//Control content types
#define CONTENT_TEXT		0//Text content
#define CONTENT_IMAGE		1//Image content

//Control types
#define CTYPE_CONTROL		0//Generic control
#define CTYPE_PANEL			1//Panel
#define CTYPE_INPUTBOX		2//Input box
#define CTYPE_FILLBAR		3//Fillbar
#define CTYPE_LISTBOX		4//List box
#define CTYPE_CHECKBOX		5//Check box
#define CTYPE_KEYBOX		6//Key box

//Macros
#define RENDERTEXT(FONT,TEXT)	TTF_RenderText_Blended(FONT.f, TEXT, FONT.color)//Macro to render text using font class instead of TTF_Font
#define GETTHEME(ID)			get <theme, list<theme> > (&themesDB, ID)//Macro to get a theme

//Prototypes
class control;//Control class
class inputBox;//Input box
class fillbar;//Fillbar
class panel;//Panel class
class window;//Window class

//Function that converts a string to a SDL color
//	string format is RRRGGGBBB
SDL_Color stringToColor(string s){
	SDL_Color result = {atoi(s.substr(0, 3).c_str()), atoi(s.substr(3,3).c_str()), atoi(s.substr(6,3).c_str())};//Result
	return result;//Returns result
}

//Function that converts a SDL color to a string
//	string format is RRRGGGBBB
string colorToString(SDL_Color c){
	return toString(c.r) + toString(c.g) + toString(c.b);//Returns complete string
}

//Font handling class
//	bundles a TTF_Font*, its color and object loading-saving functions
class font: public objectBased {
	public:
	string fontFile;//Font file path (for saving)
	int fontSize;//Font size
	
	TTF_Font* f;//Font
	SDL_Color color;//Color
	
	//Constructor
	font(){
		id = "";
		type = OBJTYPE_FONT;
		
		f = NULL;
		fontSize = 10;
		color = {255,255,255};
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succedeed loading base data
			var* fontFile = get <var, deque<var> > (&o.v, "fontFile");//Font file variable
			var* fontSize = get <var, deque<var> > (&o.v, "fontSize");//Font size variable
			var* color = get <var, deque<var> > (&o.v, "color");//Color variable
			
			if (fontFile) this->fontFile = fontFile->value;//Gets font file
			if (fontSize) this->fontSize = fontSize->intValue();//Gets font size
			
			if (fontFile) f = TTF_OpenFont(this->fontFile.c_str(), this->fontSize);//Loads font
			
			if (color) this->color = stringToColor(color->value);//Gets color
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();//Base data
		
		result.set("fontFile", fontFile);//Sets font file
		result.set("fontSize", fontSize);//Sets font size
		result.set("color", colorToString(color));//Sets font color
		
		return result;//Returns result
	}
	
	//Function to get if font is valid
	bool isValid(){
		return f != NULL;
	}
};

//Event class
//	represents an event together with its handlers
template <class eventData> class event {
	public:
	list <void (*)(eventData)> handlers;//Event handlers (pointer to functions)
	
	//Operator () to trigger event
	void operator () (eventData data){
		typename list < void (*)(eventData) > :: iterator i;//Handlers iterator
		
		for (i = handlers.begin(); i != handlers.end(); i++)//For each handler
			if (*i) (*i)(data);//Calls the event function
	}
};

//Theme element class
//	represents a single element to be printed on a fixed position inside a theme
class uiElement: public objectBased {
	public:
	image elementImage;//Element image
	
	int whichX, whichY;//Variables to indicate how to align the element
	int x;//X coord (from left, centre right)
	int y;//Y coord (from bottom, centre or top)
	
	//Constructor
	uiElement(){
		id = "";
		type = OBJTYPE_UIELEMENT;
		
		whichX = 0;
		whichY = 0;
		
		x = 0;
		y = 0;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succedeed loading base data
			object* elementImage = get <object> (&o.o, "elementImage");//Loads element image
			
			var* x_left = get <var> (&o.v, "x_left");//X coord referred to left
			var* x_centre = get <var> (&o.v, "x_centre");//X coord referred to centre
			var* x_right = get <var> (&o.v, "x_right");//X coord referred to right
			
			var* y_top = get <var> (&o.v, "y_top");//Y coord referred to top
			var* y_centre = get <var> (&o.v, "y_centre");//Y coord referred to centre
			var* y_bottom = get <var> (&o.v, "y_bottom");//Y coord referred to bottom
			
			if (elementImage) this->elementImage.fromScriptObj(*elementImage);//Loads image
			
			//Loads x coord
			if (x_left){
				whichX = 0;
				x = x_left->intValue();
			}
			
			else if (x_centre){
				whichX = 1;
				x = x_centre->intValue();
			}
			
			else if (x_right){
				whichX = 2;
				x = x_right->intValue();
			}
			
			//Loads y coord
			if (y_top){
				whichY = 0;
				y = y_top->intValue();
			}
			
			else if (y_centre){
				whichY = 1;
				y = y_centre->intValue();
			}
			
			else if (y_bottom){
				whichY = 2;
				y = y_bottom->intValue();
			}
		
			return true;//Returns true
		}
		
		return false;//Returns false
	}
};

//Theme class
//	gives all information for printing controls
class theme: public objectBased {
	public:
	//General
	image panel_topLeft, panel_top, panel_topRight, panel_right, panel_bottomRight, panel_bottom, panel_bottomLeft, panel_left;//Panel border images
	image panel_centre;//Panel fill image
	
	font themeFont;//Theme font
	
	list<uiElement> elements;//Ui elements
	
	//Constructor
	theme(){
		id = "";
		type = OBJTYPE_THEME;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succedeed loading base data
			//Gets panel components objects
			object* panel_topLeft = get <object, deque<object> > (&o.o, "panel_topLeft");
			object* panel_top = get <object, deque<object> > (&o.o, "panel_top");
			object* panel_topRight = get <object, deque<object> > (&o.o, "panel_topRight");
			object* panel_right = get <object, deque<object> > (&o.o, "panel_right");
			object* panel_bottomRight = get <object, deque<object> > (&o.o, "panel_bottomRight");
			object* panel_bottom = get <object, deque<object> > (&o.o, "panel_bottom");
			object* panel_bottomLeft = get <object, deque<object> > (&o.o, "panel_bottomLeft");
			object* panel_left = get <object, deque<object> > (&o.o, "panel_left");
			object* panel_centre = get <object, deque<object> > (&o.o, "panel_centre");
			
			object* themeFont = get <object, deque<object> > (&o.o, "themeFont");//Gets theme font
			
			//Loads panel components
			if (panel_topLeft) this->panel_topLeft.fromScriptObj(*panel_topLeft);
			if (panel_top) this->panel_top.fromScriptObj(*panel_top);
			if (panel_topRight) this->panel_topRight.fromScriptObj(*panel_topRight);
			if (panel_right) this->panel_right.fromScriptObj(*panel_right);
			if (panel_bottomRight) this->panel_bottomRight.fromScriptObj(*panel_bottomRight);
			if (panel_bottom) this->panel_bottom.fromScriptObj(*panel_bottom);
			if (panel_bottomLeft) this->panel_bottomLeft.fromScriptObj(*panel_bottomLeft);
			if (panel_left) this->panel_left.fromScriptObj(*panel_left);
			if (panel_centre) this->panel_centre.fromScriptObj(*panel_centre);
			
			if (themeFont) this->themeFont.fromScriptObj(*themeFont);//Loads theme font
			
			deque<object>::iterator i;//Sub-object iterator
			for (i = o.o.begin(); i != o.o.end(); i++)//For each sub object
				if (i->type == OBJTYPE_UIELEMENT){//If object is an ui element
					uiElement e;//New element
					e.fromScriptObj(*i);//Loads element
					elements.push_back(e);//Adds to elements
				}
		
			return true;//Returns true
		}
		
		return false;//Returns false if failed
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();//Gets base data
		
		//Adds panel component data
		result.o.push_back(panel_topLeft.toScriptObj());
		result.o.push_back(panel_top.toScriptObj());
		result.o.push_back(panel_topRight.toScriptObj());
		result.o.push_back(panel_right.toScriptObj());
		result.o.push_back(panel_bottomRight.toScriptObj());
		result.o.push_back(panel_bottom.toScriptObj());
		result.o.push_back(panel_bottomLeft.toScriptObj());
		result.o.push_back(panel_left.toScriptObj());
		result.o.push_back(panel_centre.toScriptObj());
		
		result.o.push_back(themeFont.toScriptObj());//Adds font data
		
		return result;//Returns result
	}
	
	//Function that prints the given rectangle according to theme
	void printRect (SDL_Surface* target, rectangle rect){
		//Prints fill
		int curX, curY;//Current print coordinates
		
		SDL_SetClipRect (target, & SDL_Rect {	rect.x + panel_topLeft.w() + panel_topLeft.offsetX,
												rect.y + panel_topLeft.h() + panel_topLeft.offsetY, 
												rect.w - panel_topLeft.w() - panel_topLeft.offsetX - panel_topRight.w() + panel_topRight.offsetX,
												rect.h - panel_topLeft.h() - panel_topLeft.offsetY - panel_bottomLeft.h() + panel_bottomLeft.offsetY});//Sets surface clip rect
		
		if (panel_centre.valid())
			for (curY = panel_topLeft.h() + panel_topLeft.offsetY; curY < rect.h - panel_bottomLeft.h() + panel_bottomLeft.offsetY; curY += panel_centre.h())//For each line
				for (curX = panel_topLeft.w() + panel_topLeft.offsetX; curX < rect.w - panel_topRight.w() + panel_topRight.offsetX; curX += panel_centre.w())//For each column
					panel_centre.print(target, rect.x + curX, rect.y + curY);//Prints panel centre image
		
		SDL_SetClipRect (target, & SDL_Rect {	rect.x + panel_topLeft.w() + panel_topLeft.offsetX,
												0,
												rect.w - panel_topLeft.w() - panel_topLeft.offsetX - panel_topRight.w() + panel_topRight.offsetX,
												target->h});//Sets surface clip rect
		
		//Prints top side
		if (panel_top.valid())
			for (curX = panel_topLeft.w() + panel_topLeft.offsetX; curX < rect.w - panel_topRight.w(); curX += panel_top.w())//For each column
				panel_top.print(target, rect.x + curX, rect.y);//Prints panel top image
			
		//Prints bottom side
		if (panel_bottom.valid())
			for (curX = panel_bottomLeft.w() + panel_bottomLeft.offsetX; curX < rect.w - panel_bottomRight.w(); curX += panel_bottom.w())//For each column
				panel_bottom.print(target, rect.x + curX, rect.y + rect.h - panel_bottom.h());//Prints panel bottom image
			
		SDL_SetClipRect (target, & SDL_Rect {	0,
												rect.y + panel_topRight.h() + panel_topRight.offsetY,
												target->w,
												rect.h - panel_topRight.h() - panel_topRight.offsetY - panel_bottomRight.h() + panel_bottomRight.offsetY});//Sets surface clip rect
			
		//Prints right side
		if (panel_right.valid())
			for (curY = panel_topRight.h() + panel_topRight.offsetY; curY < rect.h - panel_bottomRight.h() + panel_bottomRight.offsetY; curY += panel_right.h())//For each column
				panel_right.print(target, rect.x + rect.w - panel_right.w(), rect.y + curY);//Prints panel right image
			
		//Prints left side
		if (panel_left.valid())
			for (curY = panel_topLeft.h() + panel_topLeft.offsetX; curY < rect.h - panel_bottomLeft.h() + panel_bottomLeft.offsetY; curY += panel_left.h())//For each column
				panel_left.print(target, rect.x, rect.y + curY);//Prints panel left image
		
		SDL_SetClipRect (target, NULL);//Resets surface clip rect
		
		//Prints corners
		if (panel_topRight.valid()) panel_topLeft.print(target, rect.x, rect.y);//Prints topleft corner
		if (panel_topRight.valid()) panel_topRight.print(target, rect.x + rect.w - panel_topRight.w(), rect.y);//Prints topright corner
		if (panel_topRight.valid()) panel_bottomRight.print(target, rect.x + rect.w - panel_bottomRight.w(), rect.y + rect.h - panel_bottomRight.h());//Prints bottomright corner
		if (panel_topRight.valid()) panel_bottomLeft.print(target, rect.x, rect.y + rect.h - panel_bottomLeft.h());//Prints bottomleft corner
		
		list<uiElement>::iterator i;//Element iterator
		for (i = elements.begin(); i != elements.end(); i++){//For each element
			int x = 0, y = 0;//Coords to print the element
			
			//Sets x coord
			if (i->whichX == 0) x = i->x;
			else if (i->whichX == 1) x = rect.w / 2 + i->x - i->elementImage.w() / 2;
			else if (i->whichX == 2) x = rect.w - i->x - i->elementImage.w();
			
			//Sets y coord
			if (i->whichY == 0) y = i->y;
			else if (i->whichY == 1) y = rect.h / 2 + i->y - i->elementImage.h() / 2;
			else if (i->whichY == 2) y = rect.h - i->y - i->elementImage.h();
			
			if (i->elementImage.valid()) i->elementImage.print(target, rect.x + x, rect.y + y);//Prints element
		}
	}
};

list <theme> themesDB;//Available themes

//Function to load the themes database from a given file
void loadThemesDB(string path){
	fileData source (path);//Source file, loaded and processed
	object database = source.objGen(path);//Generates database object
	
	deque<object>::iterator i;//Object iterator
	for (i = database.o.begin(); i != database.o.end(); ++i){//For each object in database
		if (i->type == OBJTYPE_THEME){//If object is a theme
			theme newTheme;//New theme
			newTheme.fromScriptObj(*i);//Loads theme
			themesDB.push_back(newTheme);//Adds theme to database
		}
	}
}

//Control content class
//	may be a text or an image
class controlContent: public objectBased {
	public:
	
	//Horizontal alignment
	enum halign {
		left,
		hcentre,
		right
	};
	
	//Vertical alignment
	enum valign {
		top,
		vcentre,
		bottom
	};
	
	int contentType;//Content type (text or image, see defines above)
	halign hAlignment;//Content horizontal alignment
	valign vAlignment;//Content vertical alignment
	
	string t;//Text content (used if contentType == CONTENT_TEXT)
	image i;//Image content (used if contentType == CONTENT_IMAGE)
	
	int margin;//Alignment margin
	
	bool multiline;//Multiline flag
	
	//Constructor
	controlContent() {
		id = "";
		type = OBJTYPE_CONTENT;
		
		contentType = CONTENT_TEXT;
		hAlignment = left;
		vAlignment = top;
		
		t = "";
		
		margin = 0;
		multiline = false;
	}
	
	//Function to load from script object
	//	if text is specified or no image is given (e.g. neither text nor image), contentType is set to CONTENT_TEXT;
	//	else, contentType is seto to CONTENT_IMAGE
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succedeed loading base data
			var* hAlignment = get <var, deque<var> > (&o.v, "hAlignment");//Horizontal alignment variable
			var* vAlignment = get <var, deque<var> > (&o.v, "vAlignment");//Vertical alignment variable
			var* margin = get <var, deque<var> > (&o.v, "margin");//Content margin
			
			var* text = get <var, deque<var> > (&o.v, "text");//Text variable
			object* img = get <object, deque<object> > (&o.o, "image");//Image object
			
			if (hAlignment) this->hAlignment = (halign) hAlignment->intValue();//Gets horizontal alignment
			if (vAlignment) this->vAlignment = (valign) vAlignment->intValue();//Gets vertical alignment
			if (margin) this->margin = margin->intValue();//Gets content margin
			
			if (text || !img){//If a text was given
				contentType = CONTENT_TEXT;//Sets content type
				
				if (text) this->t = getText(text->value);//Gets text
			}
			
			else {//If an image was given
				contentType = CONTENT_IMAGE;//Sets content type
				this->i.fromScriptObj(*img);//Loads image
			}
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	//	only the current content type is saved (e.g. if contentType is CONTENT_TEXT, the image is not saved)
	object toScriptObj(){
		object result = objectBased::toScriptObj();//Saves base object data
		
		result.set("hAlignment", hAlignment);//Sets horizontal alignment
		result.set("vAlignment", vAlignment);//Sets vertical alignment
		
		if (contentType == CONTENT_TEXT)//If content is a text
			result.set("text", t);//Sets text variable
			
		else//If content is an image
			result.o.push_back(i.toScriptObj());//Adds image object
	}
};

//Click event structure
struct clickEventData {
	control* caller;//Caller control
	
	int x;//X coordinate (relative to control)
	int y;//Y coordinate (relative to control)
	
	int button;//Pressed button
};

//Function to give focus to control (prototype)
void getFocus(clickEventData);

//Function to check control on click (prototype)
void check(clickEventData);

//Control class
//	represents a generic user interface control
class control: public objectBased {
	public:
	
	//Control status enumeration
	enum controlStatus {
		normal,
		hover,
		pressed
	};
	
	//Misc
	int controlType;//Control type
	control* parent;//Control parent (NULL by default if there is no parent)
	
	//Graphics
	theme* themes [pressed + 1];//Themes for each status
	controlContent content;//Control content
	
	//Positioning
	rectangle area;//Control area
	
	//Interaction
	controlStatus status;//Current control status
	
	event <clickEventData> press;//Press event (triggered on mouse button press)
	event <clickEventData> release;//Release event (triggered on mouse button release)
	
	bool clickThrough;//If true, clicking this allows panel dragging
	
	int pressBtn;//Pressed mouse button
	
	//Constructor
	control(){
		id = "";
		type = OBJTYPE_CONTROL;
		controlType = CTYPE_CONTROL;
		
		parent = NULL;
		
		status = normal;//Sets normal status
	
		int i;//Counter
		for (i = 0; i <= pressed; i++){//For each status
			themes[i] = NULL;//Sets null theme for that status
		}
			
		clickThrough = false;
			
		release.handlers.push_back(getFocus);//Adds getFocus to click events
	}
	
	//Content printing function
	void printContent(SDL_Surface* target, int x = 0, int y = 0){
		if (content.contentType == CONTENT_TEXT){//If content is text
			if (content.t != "" && themes[status] && themes[status]->themeFont.isValid()){//If text is valid
				SDL_Surface* text = RENDERTEXT(themes[status]->themeFont, content.t.c_str());//Renders text
				
				if (text){//If text was generated successfully
					SDL_Rect offset;//Offset rectangle
					
					//Sets X according to horizontal alignment
					switch (content.hAlignment){
						case controlContent::left: offset.x = area.x + content.margin; break;
						case controlContent::hcentre: offset.x = area.x + (area.w - text->w) / 2; break;
						case controlContent::right: offset.x = area.x + area.w - text->w - content.margin; break;
					}
					
					//Sets Y according to vertical alignment
					switch (content.vAlignment){
						case controlContent::top: offset.y = area.y + content.margin; break;
						case controlContent::vcentre: offset.y = area.y + (area.h - text->h) / 2 + 1; break;
						case controlContent::bottom: offset.y = area.y + area.h - text->h - content.margin; break;
					}
					
					SDL_BlitSurface(text, NULL, target, &offset);//Blits text
					SDL_FreeSurface(text);//Frees text
				}
			}
		}
		
		else if (content.contentType == CONTENT_IMAGE){//If content is image
			int x, y;//Image print coords
			
			//Sets X according to horizontal alignment
			switch (content.hAlignment){
				case controlContent::left: x = area.x + content.margin; break;
				case controlContent::hcentre: x = area.x + (area.w - content.i.w()) / 2; break;
				case controlContent::right: x = area.x + area.w - content.i.w() - content.margin; break;
			}
			
			//Sets Y according to vertical alignment
			switch (content.vAlignment){
				case controlContent::top: y = area.y + content.margin; break;
				case controlContent::vcentre: y = area.y + (area.h - content.i.h()) / 2; break;
				case controlContent::bottom: y = area.y + area.h - content.i.h() - content.margin; break;
			}
			
			content.i.print(target, x, y);//Prints image
		}
	}
	
	//Print function
	virtual void print(SDL_Surface* target, int x = 0, int y = 0, bool printTheme = true){
		//Adds offset
		area.x += x;
		area.y += y;
		
		if (printTheme && themes[status])//If there's a valid theme for current status
			themes[status]->printRect(target, area);//Prints the control rectangle
			
		printContent(target, x, y);//Prints content
		
		//Removes offset
		area.x -= x;
		area.y -= y;
	}
	
	//Function to check events on control
	virtual void checkEvents(SDL_Event e, int x = 0, int y = 0){
		int mX, mY, mBtn;//Mouse info
		mBtn = SDL_GetMouseState(&mX, &mY);//Gets mouse info
		
		//Applies offset
		area.x += x;
		area.y += y;
		
		if (e.type == SDL_MOUSEBUTTONUP && (e.button.button == SDL_BUTTON_WHEELUP || e.button.button == SDL_BUTTON_WHEELDOWN)){//Wheel events
			if (e.button.x > area.x && e.button.x < area.x + area.w && e.button.y > area.y && e.button.y < area.y + area.h){//If mouse is inside control
				clickEventData data = {this, e.button.x - area.x, e.button.y - area.y, SDL_BUTTON(e.button.button)};//Click data
				release(data);//Throws click event
			}
		}
		
		if (mX > area.x && mX < area.x + area.w && mY > area.y && mY < area.y + area.h){//If mouse is inside control
			if (mBtn & SDL_BUTTON(1) || mBtn & SDL_BUTTON(3) || mBtn & SDL_BUTTON(2)){//If mouse button was pressed
				clickEventData data = {this, mX - area.x, mY - area.y, mBtn};//Click data
				press(data);//Throws click event
				
				pressBtn = mBtn;//Sets pressed mouse button
				
				status = pressed;//Sets pressed status
			}
			
			else {//Else
				if (status == pressed){//If control was pressed
					clickEventData data = {this, mX - area.x, mY - area.y, pressBtn};//Click data
					release(data);//Throws click event
				}
				
				status = hover;//Mouse hovering over control
			}
		}
		
		else {//Else
			status = normal;//Resets normal status (no release event triggered if mouse outside control)
		}
		
		//Removes offset
		area.x -= x;
		area.y -= y;
	}
	
	//Function to load from script object
	virtual bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succedeed loading base data
			object* area = get <object, deque<object> > (&o.o, "area");//Gets control area
			
			var* theme_normal = get <var, deque<var> > (&o.v, "normalTheme");//Control normal theme
			var* theme_hover = get <var, deque<var> > (&o.v, "hoverTheme");//Control hover theme
			var* theme_pressed = get <var, deque<var> > (&o.v, "pressedTheme");//Control pressed theme
			
			object* content = get <object, deque<object> > (&o.o, "content");//Control content
			
			var* clickThrough = get <var> (&o.v, "clickThrough");//Gets clickThrough variable
			
			if (area) this->area.fromScriptObj(*area);//Loads area
			
			if (theme_normal) this->themes[normal] = get <theme, list<theme> > (&themesDB, theme_normal->value);//Gets normal theme
			if (theme_hover) this->themes[hover] = get <theme, list<theme> > (&themesDB, theme_hover->value);//Gets hover theme
			if (theme_pressed) this->themes[pressed] = get <theme, list<theme> > (&themesDB, theme_pressed->value);//Gets pressed theme
			
			if (content) this->content.fromScriptObj(*content);//Loads content

			if (clickThrough) this->clickThrough = clickThrough->intValue();//Gets click through flag
			
			return true;//Returns true
		}
		
		return false;//Returns false if failed
	}
	
	//Function to save to script object
	virtual object toScriptObj(){
		object result = objectBased::toScriptObj();//Saves base data
		
		result.set("normalTheme", themes[normal]->id);//Sets normal theme
		result.set("pressedTheme", themes[pressed]->id);//Sets pressed theme
		
		result.o.push_back(area.toScriptObj());//Adds area object
		result.o.push_back(content.toScriptObj());//Adds content object
		
		return result;//Returns result
	}
};

//Input box class
//	the text content can be edited via keyboard
//	when edit flag is true
class inputBox: public control {
	public:
	bool edit;//Flag indicating if reading input from keyboard
		
	//Constructor
	inputBox(){
		id = "";
		type = OBJTYPE_INPUTBOX;
		controlType = CTYPE_INPUTBOX;
		
		parent = NULL;
		
		status = normal;
		
		int i;//Counter
		for (i = 0; i < pressed; i++)//For each status
			themes[i] = NULL;//Sets null theme for that status
		
		clickThrough = false;
		
		edit = false;
	}
	
	//Print function
	void print(SDL_Surface* target, int x = 0, int y = 0, bool printTheme = true){
		string oldText = content.t;//Old text

		if (edit) content.t += "_";//Adds caret to text if in edit mode
	
		control::print(target, x, y, printTheme);//Normally prints control
		
		content.t = oldText;//Resets text
	}
	
	//Function to check events
	//	checks base control events, plus typing events
	void checkEvents(SDL_Event e, int x = 0, int y = 0){
		control::checkEvents(e, x, y);//Checks base control events
		
		if (edit){//If input box is being edited
			if (e.type == SDL_KEYDOWN){//If a key was released
				char c = e.key.keysym.unicode;//Gets character
				
				if (isprint(c)) content.t.push_back(c);//If character is printable adds it to text
				else if (c == '\b' && content.t.size() > 0) content.t.erase(content.t.size() - 1);//Erases last character if pressed backspace
			}
		}
	}
	
	//Function to load from script object not redefined - using base control one
	
	//Function to save to script object not redefined - using base control one
};

//Fillbar class
class fillbar: public control {
	public:
	unsigned int maxFill, fill;//Maximum fill and current fill
	
	theme* fillTheme;//Fill theme
	int border;//Border size
	
	//Constructor
	fillbar(){
		id = "";
		type = OBJTYPE_FILLBAR;
		controlType = CTYPE_FILLBAR;
		
		parent = NULL;
		
		status = normal;//Sets normal status
	
		int i;//Counter
		for (i = 0; i <= pressed; i++)//For each status
			themes[i] = NULL;//Sets null theme for that status
			
		release.handlers.push_back(getFocus);//Adds getFocus to click events
		
		maxFill = 1;
		fill = 1;
		fillTheme = NULL;
		border = 0;
		
		clickThrough = false;
	}
	
	//Print method
	void print(SDL_Surface* target, int x = 0, int y = 0, bool printTheme = true){
		control::print(target, x, y, printTheme);//Prints control normally
		
		if (fillTheme && fill > 0){//If fill theme is available
			rectangle r = area;//Fill rectangle
			
			//Adjusts fill rectangle
			r.x += x + border;
			r.y += y + border;
			r.w = (r.w - 2 * border) * fill / maxFill;
			r.h -= 2 * border;
			
			if (r.w < fillTheme->panel_topLeft.w() + fillTheme->panel_topRight.w()) r.w = fillTheme->panel_topLeft.w() + fillTheme->panel_topRight.w();//Adjust width
			
			fillTheme->printRect(target, r);//Prints fill
		}
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (control::fromScriptObj(o)){//If succedeed loading base control data
			var* fillTheme = get <var> (&o.v, "fillTheme");//Fill theme variable
			var* border = get <var> (&o.v, "border");//Border size variable
			var* maxFill = get <var> (&o.v, "maxFill");//Maximum fill variable
			var* fill = get <var> (&o.v, "fill");//Fill variable
			
			if (fillTheme) this->fillTheme = get <theme> (&themesDB, fillTheme->value);//Gets fill theme
			if (border) this->border = border->intValue();//Gets border
			if (maxFill) this->maxFill = maxFill->intValue();
			if (fill) this->fill = fill->intValue();
			
			return true;
		}
		
		return false;
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = control::toScriptObj();//Saves base data
		
		result.set("fillTheme", fillTheme->id);//Sets fill theme
		result.set("border", border);//Sets border
	
		return result;//Returns result
	}
};

//Checkbox class
class checkBox: public control {
	public:
	theme* checkedTheme;//Theme when checked
	theme* checkedHoverTheme;//Theme when checked and mouse hover
	bool checked;//Checked if true
	
	//Constructor
	checkBox(){
		id = "";
		type = OBJTYPE_CHECKBOX;
		controlType = CTYPE_CHECKBOX;
		
		parent = NULL;
		
		status = normal;//Sets normal status
	
		int i;//Counter
		for (i = 0; i <= pressed; i++)//For each status
			themes[i] = NULL;//Sets null theme for that status
			
		release.handlers.push_back(check);//Adds check to click events
		
		checkedTheme = NULL;
		checkedHoverTheme = NULL;
		checked = false;
		
		clickThrough = false;
	}
	
	//Function to print
	void print(SDL_Surface* target, int x = 0, int y = 0, bool printTheme = true){
		theme *oldHover = themes[hover], *oldNormal = themes[normal];//Hover and normal themes
		
		if (checked){//If checked
			themes[hover] = checkedHoverTheme;//Replaces hover with checked hover
			themes[normal] = checkedTheme;//Replaces normal with checked
		}
		
		control::print(target, x, y, printTheme);//Prints control
		
		if (checked){//If checked
			themes[hover] = oldHover;//Resets hover theme
			themes[normal] = oldNormal;//Resets normal theme
		}
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (control::fromScriptObj(o)){//If control loaded successfully
			var* checkedTheme = get <var> (&o.v, "checkedTheme");
			var* checkedHoverTheme = get <var> (&o.v, "checkedHoverTheme");
			var* checked = get <var> (&o.v, "checked");
			
			if (checkedTheme) this->checkedTheme = get <theme> (&themesDB, checkedTheme->value);
			if (checkedHoverTheme) this->checkedHoverTheme = get <theme> (&themesDB, checkedHoverTheme->value);
			if (checked) this->checked = checked->intValue();
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}	
};

//Function to check a checkbox
void check (clickEventData data){
	if (data.caller->controlType == CTYPE_CHECKBOX)//If control is a checkbox
		((checkBox*) data.caller)->checked = !((checkBox*) data.caller)->checked;//Toggles checked
}

//Fixed size list box class
class listBox: public control {
	public:
	int itemCount;//List items count
	string* items;//Items in box
	
	controlContent::halign itemHAlign;//Items horizontal alignment
	controlContent::valign itemVAlign;//Items vertical alignment
	int itemMargin;//Item margin
	int itemHeight;//Item height
	
	theme* itemTheme;//Item theme
	
	//Constructor
	listBox(){
		id = "";
		type = OBJTYPE_LISTBOX;
		controlType = CTYPE_LISTBOX;
		
		parent = NULL;
		
		status = normal;//Sets normal status
	
		int i;//Counter
		for (i = 0; i <= pressed; i++)//For each status
			themes[i] = NULL;//Sets null theme for that status
			
		release.handlers.push_back(getFocus);//Adds getFocus to click events
		
		itemCount = 0;
		items = NULL;
		
		clickThrough = false;
	}
	
	//Function to resize box items (deletes previous items)
	void resize(int count){
		itemCount = count;//Sets item count
		
		if (items) free(items);//Frees previous items
		items = new string [count];//Creates new items
	}
	
	//Function to add an item on bottom of list
	void add(string s){
		int i;//Counter
		for (i = 0; i < itemCount - 1; i++)//For each item except the last
			items[i] = items[i + 1];//Scrolls back
			
		items[itemCount - 1] = s;//Sets last item
	}
	
	//Print function
	void print(SDL_Surface* target, int x = 0, int y = 0, bool printTheme = true){
		control::print(target, x, y, printTheme);//Prints control
		
		int i;//Counter
		for (i = 0; i < itemCount; i++){//For each item
			if (items[i] != ""){//If item is not empty
				rectangle r;//Rectangle to fill
				
				//Sets rectangle
				r.x = x + area.x;
				r.y = y + area.y + i * itemHeight;
				r.w = area.w;
				r.h = itemHeight;
				
				if (itemTheme){//If item theme was given
					itemTheme->printRect(target, r);//Prints rectangle
					
					SDL_Surface* text = RENDERTEXT(itemTheme->themeFont, items[i].c_str());//Renders text
					
					if (text){//If text was generated successfully
						SDL_Rect offset;//Offset rectangle
						
						//Sets X according to horizontal alignment
						switch (itemHAlign){
							case controlContent::left: offset.x = x + area.x + itemMargin; break;
							case controlContent::hcentre: offset.x = x + area.x + (area.w - text->w) / 2; break;
							case controlContent::right: offset.x = x + area.x + area.w - text->w - itemMargin; break;
						}
						
						//Sets Y according to vertical alignment
						switch (itemVAlign){
							case controlContent::top: offset.y = y + i * itemHeight + area.y + itemMargin; break;
							case controlContent::vcentre: offset.y = y + i * itemHeight + area.y + (itemHeight - text->h) / 2 + 1; break;
							case controlContent::bottom: offset.y = y + i * itemHeight + area.y + area.h - text->h - itemMargin; break;
						}
						
						SDL_BlitSurface(text, NULL, target, &offset);//Blits text
						SDL_FreeSurface(text);//Frees text
					}
				}
			}
		}
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (control::fromScriptObj(o)){//If succedeed loading base control data
			var* itemCount = get <var> (&o.v, "size");//Size variable
			var* itemHAlign = get <var> (&o.v, "itemHAlign");//Item horizontal alignment
			var* itemVAlign = get <var> (&o.v, "itemVAlign");//Item vertical alignment
			var* itemMargin = get <var> (&o.v, "itemMargin");//Item margin
			var* itemHeight = get <var> (&o.v, "itemHeight");//Item height
			var* itemTheme = get <var> (&o.v, "itemTheme");//Item theme
			
			if (itemCount) resize(itemCount->intValue());//Resizes
			
			if (itemHAlign) this->itemHAlign = (controlContent::halign) itemHAlign->intValue();
			if (itemVAlign) this->itemVAlign = (controlContent::valign) itemVAlign->intValue();
			if (itemMargin) this->itemMargin = itemMargin->intValue();
			if (itemHeight) this->itemHeight = itemHeight->intValue();
			if (itemTheme) this->itemTheme = get<theme> (&themesDB, itemTheme->value);
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
};

//Key box
//when focuses, inputs and stores a single key from the user
class keyBox: public control {
	public:
	SDLKey key;//Stored key
	
	bool active;
	theme* activeTheme;//Active theme
	
	//Constructor
	keyBox(){
		id = "";
		type = OBJTYPE_KEYBOX;
		controlType = CTYPE_KEYBOX;
		
		parent = NULL;
		
		status = normal;//Sets normal status
	
		int i;//Counter
		for (i = 0; i <= pressed; i++)//For each status
			themes[i] = NULL;//Sets null theme for that status
		
		active = false;
		activeTheme = NULL;
			
		release.handlers.push_back(getFocus);//Adds getFocus to click events
		
		clickThrough = false;
	}
	
	//Printing function
	void print(SDL_Surface* target, int x = 0, int y = 0, bool printTheme = true){
		theme *oldNormal = themes[normal], *oldHover = themes[hover];//Old themes
		
		if (active){//If active
			themes[normal] = activeTheme;//Sets active theme
			themes[hover] = activeTheme;//Sets hover theme
		}
		
		content.t = SDL_GetKeyName(key);//Gets text
		control::print(target, x, y, printTheme);//Prints
		
		if (active){//If active
			themes[normal] = oldNormal;//Resets normal theme
			themes[hover] = oldHover;//Resets hover theme
		}
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (control::fromScriptObj(o)){//If succeeded loading control
			var* activeTheme = get <var> (&o.v, "activeTheme");//Active theme variable
			
			if (activeTheme) this->activeTheme = get <theme> (&themesDB, activeTheme->value);//Gets active theme
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to check events
	void checkEvents(SDL_Event e, int x = 0, int y = 0){
		control::checkEvents(e, x, y);//Checks control events
		
		if (e.type == SDL_KEYUP && active){//If released a key while active
			key = e.key.keysym.sym;//Sets stored key
			active = false;//Deactivates
		}
	}
};

//Function to give focus to control (used to handle focus on click)
void getFocus(clickEventData data){
	if (data.caller->controlType == CTYPE_INPUTBOX)//If control is an input box
		((inputBox*) data.caller)->edit = true;//Sets edit flag to true
		
	else if (data.caller->controlType == CTYPE_KEYBOX)//If control is a keybox
		((keyBox*) data.caller)->active = true;//Activates
}

//Panel class
//	sort of control container
class panel: public control {
	public:
	list <control*> children;//Sub-controls
	bool allowDrag;//If true, allows dragging
	
	int pressX, pressY;//Press X and Y positions
	int initialX, initialY;//Initial X and Y coords, before dragging
	
	//Constructor
	panel(){
		id = "";
		type = OBJTYPE_PANEL;
		controlType = CTYPE_PANEL;
		
		parent = NULL;
		
		status = normal;
		
		allowDrag = false;
		
		pressX = -1;
		pressY = -1;
		initialX = -1;
		initialY = -1;
		
		int i;//Counter
		for (i = 0; i < pressed; i++)//For each status
			themes[i] = NULL;//Sets null theme for that status
			
		clickThrough = false;
	}
	
	//Print function
	//	prints panel and children controls
	void print(SDL_Surface* target, int x = 0, int y = 0, bool printTheme = true){
		control::print(target, x, y, printTheme);//Prints panel
		
		//Adds offset
		area.x += x;
		area.y += y;
		
		list<control*>::iterator i;//Iterator
		for (i = children.begin(); i != children.end(); i++)//For each child control
			(*i)->print(target, area.x, area.y);//Prints control
			
		//Removes offset
		area.x -= x;
		area.y -= y;
	}
	
	//Function to check panel events
	//	checks main panel events and children events
	//	allows drag, too
	void checkEvents(SDL_Event e, int x = 0, int y = 0){
		//Applies offset
		area.x += x;
		area.y += y;
		
		bool anyPressed = false;//True if at least one control is pressed
		
		list<control*>::iterator i;//Children iterator
		for (i = children.begin(); i != children.end(); i++){//For each child control
			if ((*i)->status == pressed && !(*i)->clickThrough) anyPressed = true;//Checks for pressed control
		}
		
		if (status == pressed && !anyPressed && allowDrag){//If only the panel was pressed
			int mX, mY;//Mouse position
			SDL_GetMouseState(&mX, &mY);
			
			if (pressX < 0 || pressY < 0){//If first frame pressed
				pressX = mX;//Sets press x
				pressY = mY;//Sets press y
				
				initialX = area.x;//Sets initial x
				initialY = area.y;//Sets initial y
			}
			
			area.x = initialX + (mX - pressX);
			area.y = initialY + (mY - pressY);
		}
		
		else {//If not pressed or can't drag
			pressX = -1;//Resets press x
			pressY = -1;//Resets press y
		}
		
		control::checkEvents(e, 0, 0);//Checks control events
		
		for (i = children.begin(); i != children.end(); i++){//For each child control
			(*i)->checkEvents(e, area.x, area.y);//Checks child events
		}
		
		//Removes offset
		area.x -= x;
		area.y -= y;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (control::fromScriptObj(o)){//If succedeed loading base data
			var* allowDrag = get <var> (&o.v, "allowDrag");//Allow drag variable
			
			if (allowDrag) this->allowDrag = allowDrag->intValue();//Sets allow drag variable
			
			deque<object>::iterator i;//Iterator for child objects
			for (i = o.o.begin(); i != o.o.end(); i++){//For each child object
				control* newControl = NULL;//New control
				
				if (i->type == OBJTYPE_CONTROL) newControl = new control;//Creates control
				else if (i->type == OBJTYPE_FILLBAR) newControl = new fillbar;//Creates fillbar
				else if (i->type == OBJTYPE_INPUTBOX) newControl = new inputBox;//Creates input box
				else if (i->type == OBJTYPE_LISTBOX) newControl = new listBox;//Creates list box
				else if (i->type == OBJTYPE_PANEL) newControl = new panel;//Creates panel
				else if (i->type == OBJTYPE_CHECKBOX) newControl = new checkBox;//Creates checkbox
				else if (i->type == OBJTYPE_KEYBOX) newControl = new keyBox;//Creates keybox
				
				if (newControl){//If control was successfully created
					newControl->fromScriptObj(*i);//Loads control
					newControl->parent = this;//Sets parent
					children.push_back(newControl);//Adds control
				}
			}
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	//	all child controls are stored as sub-objects
	object toScriptObj(){
		object result = control::toScriptObj();//Saves control data
		
		result.type = OBJTYPE_PANEL;//Corrects object type to match panel
		
		list<control*>::iterator i;//Iterator for child controls
		for (i = children.begin(); i != children.end(); i++)//For each child control
			result.o.push_back((*i)->toScriptObj());//Adds child control info
			
		return result;//Returns result
	}
	
	//Function to get a pointer to the requested child control
	//	returns null if control wasn't found
	//	child controls can be accessed with a . (e.g. child_panel.child_control)
	control* getControl(string s){
		if (s.find(".") == s.npos)//If there are no . in the string
			return get_ptr <control, list<control*> > (&children, s);//Directly returns the control
			
		else {//If there are . in the string
			string pId = s.substr(0, s.find("."));//Gets requested panel id
			
			control* c = get_ptr <control, list<control*> > (&children, pId);//Gets requested panel
			
			if (c && c->controlType == CTYPE_PANEL)//If control is a panel
				return ((panel*) c)->getControl(s.substr(s.find(".") + 1));//Returns panel's requested child control
				
			else return NULL;//Returns null if control is not a panel
		}
	}
	
	//Function to remove focus from all child controls
	//	at the moment, only inputBox::edit is set to false for input boxes
	void unfocusAll(){
		list <control*> ::iterator i;//Iterator
		
		for (i = children.begin(); i != children.end(); i++)//For each child control
			if ((*i)->controlType == CTYPE_INPUTBOX) ((inputBox*) (*i))->edit = false;//Removes edit mode from input boxes
			else if ((*i)->controlType == CTYPE_PANEL) ((panel*) (*i))->unfocusAll();//Unfocus child controls for all panels
			else if ((*i)->controlType == CTYPE_KEYBOX) ((keyBox*) (*i))->active = false;//Unactivates key boxes
	}
	
	//Function to copy panel
	//	can't be done via the = operator, since controls are stored as pointers;
	//	need to copy each control de-referencing it
	//	what's more, each control has to be copied according to its type, so the
	//	new control must be declared as control, inputBox or panel according to original
	panel* copy(){
		panel* result = new panel;//Result panel
		*result = *this;//Copies current panel
		
		result->children.clear();//Removes all current pointers
		
		list<control*>::iterator i;//Iterator for child controls
		
		for (i = children.begin(); i != children.end(); i++){//For each child control
			control *c;//New control
			
			switch ((*i)->controlType){//According to the type of the control being copied
				case CTYPE_CONTROL: c = new control; break;//Declares new control as generic control
				case CTYPE_INPUTBOX: c = new inputBox; break;//Declares new control as input box
				case CTYPE_PANEL: c = new panel; break;//Declares new control as panel
				case CTYPE_FILLBAR: c = new fillbar; break;//Declares new control as fillbar
				case CTYPE_LISTBOX: c = new listBox; break;//Declares new control as listbox
			}
			
			if (c->controlType != CTYPE_PANEL)//If control is not a panel
				*c = **i;//Directly assigns value
			else c = ((panel*)(*i))->copy();//Else copies the panel
			
			c->parent = result;//Sets control parent
			
			result->children.push_back(c);//Adds control to new panel
		}
		
		return result;//Returns result
	}
	
	//Destructor
	~panel(){
		list<control*>::iterator i;//Children iterator
		
		for (i = children.begin(); i != children.end(); i++){//For each child control
			delete *i;//Deletes control
		}
	}
};

//Window class
//	derived from list of control pointers, includes functions to handle all their events,
//	print them all, load the whole window from a script object and save it
class window: public objectBased, public list<control*> {
	public:
	
	event <clickEventData> press;//Mouse press event
	event <clickEventData> release;//Mouse release event
	
	//Constructor
	window(){
		id = "";
		type = OBJTYPE_WINDOW;
	}
	
	//Print function
	//	simply prints all controls
	void print(SDL_Surface* target, int x = 0, int y = 0){
		iterator i;//Iterator

		for (i = begin(); i != end(); i++)//For each control
			(*i)->print(target, x, y);//Prints the control
	}
	
	//Function to check events
	//	first checks global events, then checks events for all controls
	//	using their respective functions
	void checkEvents(SDL_Event e, int x = 0, int y = 0){
		if (e.type == SDL_MOUSEBUTTONDOWN){//If mouse wass pressed
			unfocusAll();//Removes focus for all controls
			
			clickEventData data = {NULL, e.button.x, e.button.y, e.button.button};//Click event data
			press(data);//Throws click event
		}
		
		else if (e.type == SDL_MOUSEBUTTONUP){//If mouse was released
			clickEventData data = {NULL, e.button.x, e.button.y, e.button.button};//Click event data
			release(data);//Throws click event
		}
	
		iterator i;//Iterator
		
		for (i = begin(); i != end(); i++)//For each control
			(*i)->checkEvents(e, x, y);//Checks events for that control
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succedeed loading base data
			deque<object>::iterator i;//Iterator
			
			for (i = o.o.begin(); i != o.o.end(); i++){//For each sub-object
				control* newControl = NULL;//New control
				
				if (i->type == OBJTYPE_CONTROL) newControl = new control;//Creates control
				else if (i->type == OBJTYPE_FILLBAR) newControl = new fillbar;//Creates fillbar
				else if (i->type == OBJTYPE_INPUTBOX) newControl = new inputBox;//Creates input box
				else if (i->type == OBJTYPE_LISTBOX) newControl = new listBox;//Creates list box
				else if (i->type == OBJTYPE_PANEL) newControl = new panel;//Creates panel
				else if (i->type == OBJTYPE_CHECKBOX) newControl = new checkBox;//Creates checkbox
				else if (i->type == OBJTYPE_KEYBOX) newControl = new keyBox;//Creates keybox
				
				if (newControl){
					newControl->fromScriptObj(*i);//Loads control
					push_back(newControl);//Adds control
				}
			}
		}
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();//Saves base data
		
		iterator i;//Iterator
		for (i = begin(); i != end(); i++)//For each control
			result.o.push_back((*i)->toScriptObj());//Adds control data
			
		return result;//Returns result
	}
	
	//Function to get a pointer to the requested control
	//	returns null if control wasn't found
	//	child controls can be accessed with a . (e.g. a_panel.a_child_control)
	control* getControl(string s){
		if (s.find(".") == s.npos)//If there are no . in the string
			return get_ptr <control, window> (this, s);//Directly returns the control
			
		else {//If there are . in the string
			string pId = s.substr(0, s.find("."));//Gets requested panel id
			
			control* c = get_ptr <control, window> (this, pId);//Gets requested panel
			
			if (c && c->controlType == CTYPE_PANEL)//If control is a panel
				return ((panel*) c)->getControl(s.substr(s.find(".") + 1));//Returns panel's requested child control
				
			else return NULL;//Returns null if control is not a panel
		}
	}
	
	//Function to remove focus for all controls
	void unfocusAll(){
		iterator i;//Iterator
		
		for (i = begin(); i != end(); i++)//For each control
			if ((*i)->controlType == CTYPE_INPUTBOX) ((inputBox*) (*i))->edit = false;//Removes edit mode from input boxes
			else if ((*i)->controlType == CTYPE_PANEL) ((panel*) (*i))->unfocusAll();//Unfocus child controls for all panels
	}
	
	//Function to clear all controls (deleting them)
	void clearControls(){
		iterator i;//Iterator
		
		for (i = begin(); i != end(); i++){//For each control
			delete *i;//Deletes control
			i = erase(i);//Erases control
			i--;//Back to previous
		}
	}
};

//Function that loads a window from a file
window loadWindow(string path, string id){
	fileData source (path);//Source file, loaded and processed
	object win = source.objGen(id);//Window object
	win.type = OBJTYPE_WINDOW;//Sets object type
	
	window result;//Result window
	result.fromScriptObj(win);//Loads window
	return result;//Returns result
}

//Initialization function
bool Bulk_ui_init() {
	if (TTF_Init() == -1){//If failed initializing SDL_ttf
		cerr << "BULK WARNING: failed initializing SDL_ttf - " << __FILE__ << ":" << __LINE__ - 1 << endl;//Error message on stderr
		return false;//Returns false
	}
	
	SDL_EnableUNICODE(SDL_ENABLE);//Enables unicode
	
	return true;//Returns true if succedeed
}

#endif