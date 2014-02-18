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

#include "ui.h"

//Color constructor
color::color(){	
	r = 0;
	g = 0;
	b = 0;
}

//Color function to convert to int
int color::toInt(){
	return (r << 16) + (g << 8) + b;
}

//Color function to load from int
void color::fromInt (int col){
	r = (col & 0xFF0000) >> 16;
	g = (col & 0x00FF00) >> 8;
	b =  col & 0x0000FF;
}

//Color function to load from XML
void color::load(xml_node source){
	if (!source) return;
	
	content::load(source);
	
	//NOTE: loading from value needs to be improved with HEX support
	if (xml_attribute a = source.attribute("value")) fromInt(a.as_int());
	
	else {
		//If integer value was not found loads individual components
		if (xml_attribute a = source.attribute("r")) r = a.as_int();
		if (xml_attribute a = source.attribute("g")) g = a.as_int();
		if (xml_attribute a = source.attribute("b")) b = a.as_int();
		
	}
}

//Color function to store into javascript variable
void color::toJSVar(CScriptVar* c){
	content::toJSVar(c);
	
	c->findChildOrCreate("r")->var->setInt(r);
	c->findChildOrCreate("g")->var->setInt(g);
	c->findChildOrCreate("b")->var->setInt(b);
}

//Color function to load from javascript variable
void color::fromJSVar(CScriptVar* c){
	content::fromJSVar(c);
	
	if (CScriptVarLink* v = c->findChild("r")) r = v->var->getInt();
	if (CScriptVarLink* v = c->findChild("g")) g = v->var->getInt();
	if (CScriptVarLink* v = c->findChild("b")) b = v->var->getInt();
}

TTF_Font* openFont ( const char* path, int size ) {
	SDL_RWops *rw = SDL_RWFromFile ( path, "rb" );
	
	if (!rw) return NULL;
	
	else {
		TTF_Font* font = TTF_OpenFontRW(rw, 1, size );
		
		if (!font && find(fontErrors.begin(), fontErrors.end(), string(path) ) == fontErrors.end()){
			LOG_WARN("invalid font path: " << path << " - " << TTF_GetError());
			fontErrors.push_back ( string ( path ) );
		}
		
		return font;
	}
}

//Font constructor
font::font(){
	f = NULL;
}

//Font function to render a text
SDL_Surface* font::render(string text){
	return TTF_RenderUTF8_Blended(f, text.c_str(), col);
}

//List of the registered font errors
list<string> fontErrors;

//Font function to load from XML
void font::load(xml_node source){
	if (!source) return;
	
	content::load(source);
	
	xml_attribute a = source.attribute("file"), b = source.attribute("size");
	if (a && b){
		f = openFont(preprocessFilePath(a.value()).c_str(), b.as_int());		
		filePath = a.value();
	}
		
	if (xml_node n = lastChild(source, "color")) col.load(n);
}

//Font function to store into javascript variable
void font::toJSVar(CScriptVar* c){
	content::toJSVar(c);
	
	c->findChildOrCreate("file")->var->setString(filePath);
	c->findChildOrCreate("size")->var->setInt(TTF_FontHeight(f));
	
	col.toJSVar(c->findChildOrCreate("color")->var);
}

//Font function to load from javascript variable
void font::fromJSVar(CScriptVar* c){
	content::fromJSVar(c);
	
	CScriptVarLink* fileVar = c->findChild("file");
	CScriptVarLink* sizeVar = c->findChild("size");
	
	if (fileVar || sizeVar){
		string newFile = fileVar ? fileVar->var->getString() : filePath;
		int newSize = sizeVar ? sizeVar->var->getInt() : TTF_FontHeight(f);
		
		if (newFile != filePath || newSize != TTF_FontHeight(f)){
			filePath = newFile;
			f = TTF_OpenFont(preprocessFilePath(filePath).c_str(), newSize);
		}
	}
	
	if (CScriptVarLink* v = c->findChild("color")) col.fromJSVar(v->var);
}

//Position constructor
_position::_position(){	
	xRef = 0;
	yRef = 0;
	
	x = 0;
	y = 0;
}

//Position constructor
_position::_position(int x, int xRef, int y, int yRef){
	this->x = x;
	this->y = y;
	this->xRef = xRef;
	this->yRef = yRef;
}

//Function to get absolute position
_position _position::absolutePos(_rect r, int w, int h){
	_position result;
	
	//Determines x position
	if (xRef == 0) result.x = r.x + x;
	else if (xRef == 1) result.x = r.x + (r.w - w) / 2 + x;
	else if (xRef == 2) result.x = r.x + r.w - w - x;
	
	//Determines y position
	if (yRef == 0) result.y = r.y + y;
	else if (yRef == 1) result.y = r.y + (r.h - h) / 2 + y;
	else if (yRef == 2) result.y = r.y + r.h - h - y;
	
	return result;
}

//Position constructor
position::position(){
	x = 0;
	y = 0;
	xRef = 0;
	yRef = 0;
}

//Position constructor
position::position(_position p){
	x = p.x;
	y = p.y;
	xRef = p.xRef;
	yRef = p.yRef;
}

//Function to load position from XML
void position::load(xml_node source){
	content::load(source);
	
	//Gets x position
	if (source.attribute( "x_left")) { x = source.attribute( "x_left").as_int(); xRef = 0; }
	else if (source.attribute( "x_centre"))	 { x = source.attribute( "x_centre").as_int(); xRef = 1; }
	else if (source.attribute( "x_right")) { x = source.attribute( "x_right").as_int(); xRef = 2; }
	
	//Gets y position
	if (source.attribute( "y_top")) { y = source.attribute( "y_top").as_int(); yRef = 0; }
	else if (source.attribute( "y_centre"))	 { y = source.attribute( "y_centre").as_int(); yRef = 1; }
	else if (source.attribute( "y_bottom")) { y = source.attribute( "x_bottom").as_int(); yRef = 2; }
}

//Function to store position into javascript variable
void position::toJSVar(CScriptVar* c){
	content::toJSVar(c);
	
	c->findChildOrCreate("xRef")->var->setInt(xRef);
	c->findChildOrCreate("yRef")->var->setInt(yRef);
	c->findChildOrCreate("x")->var->setInt(x);
	c->findChildOrCreate("y")->var->setInt(y);
}

//Function to load position from javascript variable
void position::fromJSVar(CScriptVar* c){
	content::fromJSVar(c);
	
	if (CScriptVarLink *v = c->findChild("xRef")) xRef = v->var->getInt();
	if (CScriptVarLink *v = c->findChild("yRef")) yRef = v->var->getInt();
	if (CScriptVarLink *v = c->findChild("x")) x = v->var->getInt();
	if (CScriptVarLink *v = c->findChild("y")) y = v->var->getInt();
}

//Theme element constructor
themeElement::themeElement(){	
	xRef = 0;
	x = 0;
	
	yRef = 0;
	y = 0;
	
	img = NULL;
}

//Function to print theme element
void themeElement::print(SDL_Renderer* target, _rect r){
	position p = absolutePos(r, img->w(), img->h());
	
	img->print(target, p.x, p.y);
}

//Function to load theme element from XML
void themeElement::load(xml_node source){
	if (!source) return;
	
	position::load(source);	
	img = renderableFromXML(lastChild(source, "renderable"));
}

//Function to store theme element into javascript variable
void themeElement::toJSVar(CScriptVar* c){
	position::toJSVar(c);
	img->toJSVar(c->findChildOrCreate("gfx")->var);
}

//Function to load theme element from javascript variable
void themeElement::fromJSVar(CScriptVar* c){
	position::fromJSVar(c);	
	if (CScriptVarLink *v = c->findChild("gfx")) img->fromJSVar(v->var);
}

//Theme constructor
theme::theme(){	
	topLeft = NULL;
	top = NULL;
	topRight = NULL;
	left = NULL;
	centre = NULL;
	right = NULL;
	bottomLeft = NULL;
	bottom = NULL;
	bottomRight = NULL;
}

//Function to print a theme
void theme::print(SDL_Renderer* target, _rect r){
	
	//Pointers to the theme pieces images
	//If a piece is not specified, another one is used if available to fill its place
	renderable *topLeft, *top, *topRight, *left, *centre, *right, *bottomLeft, *bottom, *bottomRight;
	
	//Picks theme pieces
	//Discards the invalid ones
	if (this->topLeft && this->topLeft->valid()) topLeft = this->topLeft;
	else if (this->top && this->top->valid()) topLeft = this->top;
	else if (this->left && this->left->valid()) topLeft = this->left;
	else if (this->centre && this->centre->valid()) topLeft = this->centre;
	else topLeft = NULL;
	
	if (this->top && this->top->valid()) top = this->top;
	else if (this->centre && this->centre->valid()) top = this->centre;
	else top = NULL;
	
	if (this->topRight && this->topRight->valid()) topRight = this->topRight;
	else if (this->top && this->top->valid()) topRight = this->top;
	else if (this->right && this->right->valid()) topRight = this->right;
	else if (this->centre && this->centre->valid()) topRight = this->centre;
	else topRight = NULL;
	
	if (this->left && this->left->valid()) left = this->left;
	else if (this->centre && this->centre->valid()) left = this->centre;
	else left = NULL;
	
	if (this->centre && this->centre->valid()) centre = this->centre;
	else centre = NULL;
	
	if (this->right && this->right->valid()) right = this->right;
	else if (this->centre && this->centre->valid()) right = this->centre;
	else right = NULL;
	
	if (this->bottomLeft && this->bottomLeft->valid()) bottomLeft = this->bottomLeft;
	else if (this->bottom && this->bottom->valid()) bottomLeft = this->bottom;
	else if (this->left && this->left->valid()) bottomLeft = this->left;
	else if (this->centre && this->centre->valid()) bottomLeft = this->centre;
	else bottomLeft = NULL;
	
	if (this->bottom && this->bottom->valid()) bottom = this->bottom;
	else if (this->centre && this->centre->valid()) bottom = this->centre;
	else bottom = NULL;
	
	if (this->bottomRight && this->bottomRight->valid()) bottomRight = this->bottomRight;
	else if (this->bottom && this->bottom->valid()) bottomRight = this->bottom;
	else if (this->right && this->right->valid()) bottomRight = this->right;
	else if (this->centre && this->centre->valid()) bottomRight = this->centre;
	else bottomRight = NULL;
	
	//Individually fills pieces
	if (topLeft){
		_rect topLeftRect (r.x, r.y, topLeft->w(), topLeft->h());
		fillRect(topLeft, target, topLeftRect, 0b1100);
	}
	
	if (top){
		_rect topRect (r.x + (topLeft ? topLeft->w() : 0), r.y, r.w - (topLeft ? topLeft->w() : 0) - (topRight ? topRight->w() : 0), top->h());
		fillRect(top, target, topRect, 0b1000);
	}
	
	if (topRight){
		_rect topRightRect (r.x + r.w - topRight->w(), r.y, topRight->w(), topRight->h());
		fillRect(topRight, target, topRightRect, 0b1010);
	}
	
	if (left){
		_rect leftRect (r.x, r.y + (topLeft ? topLeft->h() : 0), left->w(), r.h - (topLeft ? topLeft->h() : 0) - (bottomLeft ? bottomLeft->h() : 0));
		fillRect(left, target, leftRect, 0b0100);
	}
	
	if (centre){
		_rect centreRect (	r.x + (topLeft ? topLeft->w() : 0),
								r.y + (topLeft ? topLeft->h() : 0),
								r.w - (topLeft ? topLeft->w() : 0) - (topRight ? topRight->w() : 0),
								r.h - (topLeft ? topLeft->h() : 0) - (bottomLeft ? bottomLeft->h() : 0));
		
		fillRect(centre, target, centreRect, 0b0000);
	}
	
	if (right){
		_rect rightRect (r.x + r.w - right->w(), r.y + (topRight ? topRight->h() : 0), left->w(), r.h - (topRight ? topRight->h() : 0) - (bottomRight ? bottomRight->h() : 0));
		fillRect(right, target, rightRect, 0b0010);
	}
	
	if (bottomLeft){
		_rect bottomLeftRect (r.x, r.y + r.h - bottomLeft->h(), bottomLeft->w(), bottomLeft->h());
		fillRect(bottomLeft, target, bottomLeftRect, 0b0101);
	}
	
	if (bottom){
		_rect bottomRect (r.x + (bottomLeft ? bottomLeft->w() : 0), r.y + r.h - bottom->h(), r.w - (bottomLeft ? bottomLeft->w() : 0) - (bottomRight ? bottomRight->w() : 0), top->h());
		fillRect(bottom, target, bottomRect, 0b0001);
	}
	
	if (bottomRight){
		_rect bottomRightRect (r.x + r.w - bottomRight->w(), r.y + r.h - bottomRight->h(), bottomRight->w(), bottomRight->h());
		fillRect(bottomRight, target, bottomRightRect, 0b0011);
	}
	
	//Prints elements
	for (list<themeElement>::iterator i = elements.begin(); i != elements.end(); i++)
		i->print(target, r);
}

//Function to load a theme from XML
void theme::load(xml_node source){
	if (!source) return;
	
	content::load(source);
	
	//Loads theme pieces
	if (xml_node n = lastChild(source, "topLeft")) topLeft = renderableFromXML(n);
	if (xml_node n = lastChild(source, "top")) top = renderableFromXML(n);
	if (xml_node n = lastChild(source, "topRight")) topRight = renderableFromXML(n);
	if (xml_node n = lastChild(source, "left")) left = renderableFromXML(n);
	if (xml_node n = lastChild(source, "centre")) centre = renderableFromXML(n);
	if (xml_node n = lastChild(source, "right")) right = renderableFromXML(n);
	if (xml_node n = lastChild(source, "bottomLeft")) bottomLeft = renderableFromXML(n);
	if (xml_node n = lastChild(source, "bottom")) bottom = renderableFromXML(n);
	if (xml_node n = lastChild(source, "bottomRight")) bottomRight = renderableFromXML(n);
	
	//Loads fonts
	if (xml_node n = lastChild(source, "fonts")){
		for (xml_node child = n.first_child(); child; child = child.next_sibling()){
			font f;
			f.load(child);			
			fonts.push_back(f);
		}
	}
	
	for (xml_node el = lastChild(source, "element"); el; el = el.next_sibling("element")){
		themeElement e;
		e.load(el);		
		elements.push_back(e);
	}
}

//Function to store a theme inside a javascript variable
void theme::toJSVar(CScriptVar* c){
	content::toJSVar(c);
	
	if (topLeft) topLeft->toJSVar(c->addChildNoDup("topLeft")->var);
	if (top) top->toJSVar(c->addChildNoDup("top")->var);
	if (topRight) topRight->toJSVar(c->addChildNoDup("topRight")->var);
	if (left) left->toJSVar(c->addChildNoDup("left")->var);
	if (centre) centre->toJSVar(c->addChildNoDup("centre")->var);
	if (right) right->toJSVar(c->addChildNoDup("right")->var);
	if (bottomLeft) bottomLeft->toJSVar(c->addChildNoDup("bottomLeft")->var);
	if (bottom) bottom->toJSVar(c->addChildNoDup("bottom")->var);
	if (bottomRight) bottomRight->toJSVar(c->addChildNoDup("bottomRight")->var);
	
	CScriptVar* elementsVar = c->addChildNoDup("elements")->var;
	CScriptVar* fontsVar = c->addChildNoDup("fonts")->var;
	
	for (list <themeElement> :: iterator i = elements.begin(); i != elements.end(); i++)
		i->toJSVar(elementsVar->addChildNoDup(i->id)->var);
		
	for (list <font> :: iterator i = fonts.begin(); i != fonts.end(); i++)
		i->toJSVar(fontsVar->addChildNoDup(i->id)->var);
}

//Function to load a theme from a javascript variable
void theme::fromJSVar(CScriptVar* c){
	content::fromJSVar(c);
	
	if (CScriptVarLink* v = c->findChild("topLeft")) topLeft->fromJSVar(v->var);
	if (CScriptVarLink* v = c->findChild("top")) top->fromJSVar(v->var);
	if (CScriptVarLink* v = c->findChild("topRight")) topRight->fromJSVar(v->var);
	if (CScriptVarLink* v = c->findChild("left")) left->fromJSVar(v->var);
	if (CScriptVarLink* v = c->findChild("centre")) centre->fromJSVar(v->var);
	if (CScriptVarLink* v = c->findChild("right")) right->fromJSVar(v->var);
	if (CScriptVarLink* v = c->findChild("bottomLeft")) bottomLeft->fromJSVar(v->var);
	if (CScriptVarLink* v = c->findChild("bottom")) bottom->fromJSVar(v->var);
	if (CScriptVarLink* v = c->findChild("bottomRight")) bottomRight->fromJSVar(v->var);
	
	elements.clear();
	
	if (CScriptVarLink* v = c->findChild("elements")) {
		for (CScriptVarLink* t = v->var->firstChild; t; t = t->nextSibling){
			themeElement e;
			e.fromJSVar(t->var);
			elements.push_back(e);
		}
	}
	
	fonts.clear();
	
	if (CScriptVarLink* v = c->findChild("fonts")){
		for (CScriptVarLink* t = v->var->firstChild; t; t = t->nextSibling){
			font e;
			e.fromJSVar(t->var);
			fonts.push_back(e);
		}
	}
}

//Event data constructor
eventData::eventData(){
	type = 0;
}

//Function to store event data to JS variable
void eventData::toJSVar(CScriptVar* c){
	if (type == 0){
		c->findChildOrCreate("x")->var->setInt(data.mouse.x);
		c->findChildOrCreate("y")->var->setInt(data.mouse.y);
		c->findChildOrCreate("button")->var->setInt(data.mouse.button);
	}
	
	if (type == 1){		
		c->findChildOrCreate("key")->var->setInt(data.keyboard.key.scancode);
	}
	
	if (type == 2) {
		c->addChild("custom", data.custom.var);
	}
	
	if (type == 3) {
		c->addChild("text")->var->setString(data.textInput.text);
	}
}

//Event constructor
event::event(){
	script = "";	
	status = -3;
}

//Event constructor
//Sets type and adds a handler
event::event(string type, void (*handler) (control*, eventData*), int status){
	this->type = type;
	this->handlers.push_back(handler);
	this->status = status;
}

//Event trigger function
void event::trigger(control* caller, eventData *data){
	if (script != ""){//If a script is specified
		CTinyJS* js = new CTinyJS();//Javascript context for event script execution
		
		//Registers common functions and variables
		registerMathFunctions(js);
		registerMiscFunctions(js);
		registerUIFunctions(js);
		registerViewportData(js);
		
		if (data) data->toJSVar(js->root->findChildOrCreate("event")->var);//Stores data
		
		js->root->addChild("caller", caller->jsVar);
		
		js->execute(script);
		
		caller->root->reload();
		
		delete js;
	}
	
	for (list < void(*)(control*, eventData*) > :: iterator i = handlers.begin(); i != handlers.end(); i++)//For each handler
		(*i)(caller, data);//Calls handler if it is valid
}

//Event loading function
void event::load(xml_node source){
	if (!source) return;//Exits if source node is invalid
	
	content::load(source);//Loads base data
	
	if (xml_node n = lastChild(source, "script")) script = n.text().get();//Gets script
	if (xml_attribute a = source.attribute("status")) status = a.as_int() * 3;
}

//Control content constructor
controlContent::controlContent(){
	content();

	xRef = 0;
	x = 0;
	yRef = 0;
	y = 0;
	
	data.text = NULL;
	fontId = "";
}

//Function to print control content
void controlContent::print(SDL_Renderer* target, rect r, theme* t){
	if (type == "text"){//If content type is text
		if (!t || !data.text || *data.text == "") return;//Exits function if theme or text is null
		
		list<font>::iterator i;//Font iterator
		for (i = t->fonts.begin(); i != t->fonts.end(); i++)//For each font in theme
			if (i->id == fontId) break;//Exits when font is found
		
		if (i == t->fonts.end()) return;//Exits function if fails finding the font
		if (!i->f) return;
		
		string s = _(data.text->c_str());
		
		int n = s.find("["), m = s.find("]", n);
		while (n != s.npos && m != s.npos) {
			string token = s.substr(n, m - n);
			
			int paramIndex, paddingLength;
			string paddingChar;
			
			int comma = token.find(",");
			int sndComma = comma != token.npos ? token.find(",", comma + 1) : token.npos;
			
			if (comma != token.npos) paramIndex = atoi(token.substr(1, comma - 1).c_str());
			else paramIndex = atoi(token.substr(1, token.size() - 2).c_str());
			
			if (sndComma != token.npos) paddingLength = atoi(token.substr(comma + 1, sndComma - comma).c_str());
			else paddingLength = atoi(token.substr(comma + 1, token.size() - 2 - comma).c_str());
			
			if (sndComma != token.npos) paddingChar = token.substr(sndComma + 1, token.size() - 1 - sndComma);
			else paddingChar = " ";
			
			if (textParameters.size() > paramIndex){
				string replace = textParameters[paramIndex];
				while ( replace.size() < paddingLength ) replace = paddingChar + replace;
				
				s.replace ( n, token.size() + 1, replace );
			}
			
			n = s.find("[", n + 1);
			m = s.find("]", n);
		}
		
		SDL_Surface* text = i->render(s);
		if (!text) { SDL_FreeSurface(text); return; }
		
		SDL_Texture* t = SDL_CreateTextureFromSurface(target, text);
		if (!t) { SDL_FreeSurface(text); SDL_DestroyTexture(t); return; }
		
		_position p = absolutePos(r, text->w, text->h);
		
		SDL_Rect offset = {p.x, p.y, text->w, text->h};//Offset rectangle
		SDL_RenderCopy(target, t, NULL, &offset);
		
		SDL_FreeSurface(text);
		SDL_DestroyTexture(t);
	}
	
	else if (type == "image"){//If content type is image
		if (!data.img) return;//Exits function if image is null
		
		int oX, oY;//Absolute coords
		
		_position p = absolutePos(r, data.img->w(), data.img->h());
		
		data.img->print(target, p.x, p.y);//Prints image
	}
}

//Function to load control content from XML
void controlContent::load(xml_node source){
	if (!source) return;//Exits function if source is not valid
	
	position::load(source);//Loads base content data
	
	xml_attribute a = source.attribute( "text");//Text attribute
	xml_node n = lastChild(source, "image");//Image node
	
	if (type == "text"){
		if (a)//If content type is text and text attribute is specified
			data.text = new string ( a.value() );//Sets text
		else data.text = NULL;//Sets text to null if type is text but attribute is not specified
	}
	
	if (type == "image"){
		if (n)//If type is image and image node is given
			{ data.img = renderableFromXML(n); }//Loads image
		else data.img = NULL;//Sets image to null if type is image but image node is not given
	}
	
	if (xml_attribute b = source.attribute( "fontId")) fontId = b.value();//Gets font id
}

//Function to save control content into javascript variable
void controlContent::toJSVar(CScriptVar* c){
	content::toJSVar(c);
	
	c->findChildOrCreate("xRef")->var->setInt(xRef);
	c->findChildOrCreate("yRef")->var->setInt(yRef);
	c->findChildOrCreate("x")->var->setInt(x);
	c->findChildOrCreate("y")->var->setInt(y);
	
	if (type == "text" && data.text) c->findChildOrCreate("text")->var->setString(*data.text);
	else if (type == "image" && data.img) data.img->toJSVar(c->findChildOrCreate("image")->var);
	
	c->findChildOrCreate("fontId")->var->setString(fontId);
}

//Function to load control content from javascript variable
void controlContent::fromJSVar(CScriptVar* c){
	content::fromJSVar(c);
	
	if (CScriptVarLink *v = c->findChild("xRef")) xRef = v->var->getInt();
	if (CScriptVarLink *v = c->findChild("yRef")) yRef = v->var->getInt();
	if (CScriptVarLink *v = c->findChild("x")) x = v->var->getInt();
	if (CScriptVarLink *v = c->findChild("y")) y = v->var->getInt();
	
	if (type == "text"){
		if (!data.text) data.text = new string ("");		
		if (CScriptVarLink *v = c->findChild("text")) *data.text = v->var->getString();
	}
	
	if (type == "image"){
		if (!data.img) data.img = new image;
		if (CScriptVarLink* v = c->findChild("image")) data.img->fromJSVar(v->var);
	}
	
	if (CScriptVarLink *v = c->findChild("fontId")) fontId = v->var->getString();
}

void controlContent::setTextParameter ( int index, string value ) {
	while ( textParameters.size() <= index ) textParameters.push_back("");
	textParameters[index] = value;
}

//Control constructor
control::control(){	
	root = this;
	parent = NULL;
	nextSibling = NULL;
	prevSibling = NULL;
	
	mStatus = ms_outside;
	cStatus = cs_normal;
	
	for ( int i = 0; i < 9; i++ )
		themes[i] = NULL;
	
	dontUseAsReference = false;
	draggable = false;
	dragging = false;
	dragInitialX = 0; dragInitialY = 0;
	dragGrid = 1;
	
	clickThrough = false;
	
	visible = true;
	
	jsVar = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
	jsVar->ref();
	
	eventGrabber = NULL;
}

//Function to print a control
void control::print(SDL_Renderer* target, _rect *ref){
	if (!visible) return;
	
	printBase(target, ref);
	printChildren(target, ref);
}

//Function to print control only
void control::printBase(SDL_Renderer* target, _rect* ref){
	if (!visible) return;
	
	_rect absRect = area.abs(ref ? *ref : getViewport(target));
	
	theme* t = themes[mStatus + cStatus];
	
	triggerEvent("prePrint", NULL);
		
	if (t){
		t->print(target, absRect);
		
		for (list<controlContent*>::iterator i = cContent.begin(); i != cContent.end(); i++)
			(*i)->print(target, absRect, t);
	}
}

//Function to print children
void control::printChildren(SDL_Renderer* target, _rect* ref){
	if (!visible) return;
	
	_rect absRect = area.abs(ref ? *ref : getViewport(target));
	
	//First prints disabled controls, then others
	
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++)
		if ((*i)->cStatus == cs_disabled) (*i)->printBase(target, dontUseAsReference ? ref : &absRect);
		
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++)
		if ((*i)->cStatus != cs_disabled) (*i)->printBase(target, dontUseAsReference ? ref : &absRect);
		
	//Prints children of children
	
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++)
		(*i)->printChildren(target, dontUseAsReference ? ref : &absRect);
}

//Function to trigger events
void control::triggerEvent(string type, eventData* data, bool disabled){
	for (list<event>::iterator i = events.begin(); i != events.end(); i++)
		if (i->type == type){
			if (i->status == -3 && cStatus != cs_disabled && !disabled) i->trigger(this, data);
			else if (i->status > 0 && cStatus == i->status) i->trigger(this, data);
			else if (i->status == -6) i->trigger(this, data);
		}
}

//Function to trigger events
void control::triggerEventChildren(string type, eventData* data, bool disabled){
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++)
		(*i)->triggerEventChildren(type, data, disabled || cStatus == cs_disabled);
	
	triggerEvent(type, data, disabled || cStatus == cs_disabled);
}

//Function to handle events on a control
void control::handleEvents(SDL_Event* e, _rect ref, bool disabled){
	disabled = disabled || cStatus == cs_disabled;
	
	if (!eventGrabber){
		handleEventsChildren(e, ref, disabled);
		handleEventsBase(e, ref, disabled);
	}
	
	else eventGrabber->handleEvents(e, dontUseAsReference ? ref : area.abs(ref));
}

//Function to handle events on a control
void control::handleEventsBase(SDL_Event *e, _rect ref, bool disabled){
	int mX, mY, mState;//Mouse coords and state
	
	mState = SDL_GetMouseState(&mX, &mY);//Gets mouse state
	
	disabled = disabled || cStatus == cs_disabled;
	
	if (e && e->type == SDL_MOUSEMOTION && dragging){
		area.x = floor((mX - dragInitialX) / dragGrid) * dragGrid;
		area.y = floor((mY - dragInitialY) / dragGrid) * dragGrid;
	}
	
	bool inside = isInside(mX, mY, ref);
	
	//Gets mouse status
	if (inside && mState & SDL_BUTTON_LEFT) mStatus = ms_pressed;
	else if (inside) mStatus = ms_over;
	else mStatus = ms_outside;
		
	if (e){
		if (e->type == SDL_MOUSEBUTTONDOWN && inside){
			eventData data;
			data.type = 0;
			
			//Sets data members
			data.data.mouse.x = e->button.x;
			data.data.mouse.y = e->button.y;
			data.data.mouse.button = e->button.button;
			
			if (e->button.button == 1) triggerEvent("onMouseDown", &data, disabled);
			
			if (draggable){
				bool insideChildren = false;
				
				for (list<control*>::iterator i = children.begin(); i != children.end(); i++) {
					if (((*i)->mStatus == ms_over || (*i)->mStatus == ms_pressed) && !(*i)->clickThrough){ insideChildren = true; break; }
				}
				
				if (!insideChildren){
					dragging = true;
					dragInitialX = mX - area.x;
					dragInitialY = mY - area.y;
				}
			}
		}
		
		if (e->type == SDL_MOUSEBUTTONUP && inside){
			eventData data;
			data.type = 0;
			
			//Sets data members
			data.data.mouse.x = e->button.x;
			data.data.mouse.y = e->button.y;
			data.data.mouse.button = e->button.button;
						
			if (e->button.button == 1) triggerEvent("onMouseUp", &data, disabled);
			
			if (dragging) dragging = false;
		}
		
		if (e->type == SDL_MOUSEBUTTONDOWN && !inside){
			eventData data;
			data.type = 0;
			
			//Sets data members
			data.data.mouse.x = e->button.x;
			data.data.mouse.y = e->button.y;
			data.data.mouse.button = e->button.button;
			
			triggerEvent("onMouseDown_outside", &data, disabled);
		}
		
		if (e->type == SDL_MOUSEBUTTONUP && !inside){
			eventData data;
			data.type = 0;
			
			//Sets data members
			data.data.mouse.x = e->button.x;
			data.data.mouse.y = e->button.y;
			data.data.mouse.button = e->button.button;
			
			triggerEvent("onMouseUp_outside", &data, disabled);
		}
		
		if (dragging && !inside) dragging = false;
		
		if (e->type == SDL_KEYDOWN){//On key down event
			eventData data;
			data.type = 1;
			
			//Sets data members
			data.data.keyboard.key = e->key.keysym;
			
			triggerEvent("onKeyDown", &data, disabled);
		}
		
		if (e->type == SDL_TEXTINPUT){//On text input
			eventData data;
			data.type = 3;
			
			data.data.textInput.text = e->text.text;
			
			triggerEvent("onTextInput", &data, disabled);
		}
		
		if (e->type == SDL_WINDOWEVENT && e->window.event == SDL_WINDOWEVENT_SIZE_CHANGED) triggerEvent("onWindowResize", NULL, disabled);
	}
}

//Function to handle events on children
void control::handleEventsChildren(SDL_Event*e, _rect ref, bool disabled){
	disabled = disabled || cStatus == cs_disabled;
	
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++)
		(*i)->handleEvents(e, dontUseAsReference ? ref : area.abs(ref), disabled);
}

//Function to load a control from XML
void control::load(xml_node source){
	if (!source) return;
	
	content::load(source);//Loads base data

	//Loads area
	if (xml_node n = lastChild(source,  "area")) area.load(n);
	
	//Loads themes
	if (xml_node n = lastChild(source, "theme_normal_outside")){
		themes[0] = new theme;
		themes[0]->load(n);
	}
	
	if (xml_node n = lastChild(source, "theme_normal_over")){
		themes[1] = new theme;
		themes[1]->load(n);
	}
	
	if (xml_node n = lastChild(source, "theme_normal_pressed")){
		themes[2] = new theme;
		themes[2]->load(n);
	}
	
	if (xml_node n = lastChild(source, "theme_active_outside")){
		themes[3] = new theme;
		themes[3]->load(n);
	}
	
	if (xml_node n = lastChild(source, "theme_active_over")){
		themes[4] = new theme;
		themes[4]->load(n);
	}
	
	if (xml_node n = lastChild(source, "theme_active_pressed")){
		themes[5] = new theme;
		themes[5]->load(n);
	}
	
	if (xml_node n = lastChild(source, "theme_disabled_outside")){
		themes[6] = new theme;
		themes[6]->load(n);
	}
	
	if (xml_node n = lastChild(source, "theme_disabled_over")){
		themes[7] = new theme;
		themes[7]->load(n);
	}
	
	if (xml_node n = lastChild(source, "theme_disabled_pressed")){
		themes[8] = new theme;
		themes[8]->load(n);
	}
	
	if (xml_node n = lastChild(source, "mask")) mask.load(n);//Loads mask
	
	if (xml_attribute a = source.attribute("dontUseAsReference")) dontUseAsReference = a.as_int();
	
	if (xml_attribute a = source.attribute("status")) cStatus = controlStatus (a.as_int() * 3);
	if (xml_attribute a = source.attribute("visible")) visible = a.as_int();
	
	if (xml_attribute a = source.attribute("draggable")) draggable = a.as_int();
	if (xml_attribute a = source.attribute("clickThrough")) clickThrough = a.as_int();
	if (xml_attribute a = source.attribute("dragGrid")) dragGrid = a.as_int();
	
	//Gets content
	for (xml_node n = source.child("content"); n; n = n.next_sibling("content")){//If content node is found
		for (xml_node child = n.first_child(); child; child = child.next_sibling()){//For each child
			controlContent * newContent = new controlContent;//New content
			newContent->load(child);//Loads content
			
			cContent.push_back(newContent);//Adds to content
		}
	}
	
	//Gets events
	for (xml_node a = source.child("event"); a; a = a.next_sibling("event")){
		event e;
		e.load(a);
		events.push_back(e);
	}
	
	//Gets children
	for (xml_node n = source.child("children"); n; n = n.next_sibling("children")){//If children node is found
		for (xml_node child = n.first_child(); child; child = child.next_sibling()){//For each child
			control *childControl = new control;
			addChild(childControl);
			childControl->load(child);
		}
	}
	
	if (parent == NULL){//If node is a root node
		genScriptVar();
		linkScriptVar();
	
		triggerEventChildren("onLoad", NULL);
	}
}

//Function to store control into javascrpt variable
void control::toJSVar(CScriptVar* c){
	content::toJSVar(c);
	
	c->findChildOrCreate("childrenCount")->var->setInt(children.size());
	c->findChildOrCreate("status")->var->setInt(cStatus);
	c->findChildOrCreate("dontUseAsReference")->var->setInt(dontUseAsReference);
	c->findChildOrCreate("visible")->var->setInt(visible);
	area.toJSVar(c->findChildOrCreate("area")->var);
	mask.toJSVar(c->findChildOrCreate("mask")->var);
	c->findChildOrCreate("draggable")->var->setInt(draggable);
	c->findChildOrCreate("dragGrid")->var->setInt(dragGrid);
	c->findChildOrCreate("clickThrough")->var->setInt(clickThrough);
	
	CScriptVar* contentVar = c->addChildNoDup("content")->var;
	
	//Creates content variable
	for (list <controlContent*> :: iterator i = cContent.begin(); i != cContent.end(); i++)
		(*i)->toJSVar(contentVar->findChildOrCreate((*i)->id)->var);
	
	//Function variables
	CScriptVar 	*addChildFunc = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE),
				*removeChildFunc = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE),
				*grabEventsFunc = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE),
				*releaseEventsFunc = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE),
				*triggerEventFunc = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE),
				*timeoutEventFunc = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE);
	
	addChildFunc->addChildNoDup("sourceFile");
	addChildFunc->setCallback(scAddChildFromFile, this);
	
	removeChildFunc->addChildNoDup("childId");
	removeChildFunc->setCallback(scRemoveChild, this);
	
	grabEventsFunc->setCallback(scGrabEvents, this);
	
	releaseEventsFunc->setCallback(scReleaseEvents, this);
	
	triggerEventFunc->addChildNoDup("event");
	triggerEventFunc->setCallback(scTriggerEvent, this);
	
	timeoutEventFunc->addChildNoDup("time");
	timeoutEventFunc->addChildNoDup("event");
	timeoutEventFunc->setCallback(scTimeout, this);
	
	c->addChild("addChild", addChildFunc);
	c->addChild("removeChild", removeChildFunc);
	c->addChild("grabEvents", grabEventsFunc);
	c->addChild("releaseEvents", releaseEventsFunc);
	c->addChild("triggerEvent", triggerEventFunc);
	c->addChild("timeout", timeoutEventFunc);
}

//Function to reload control from its own js variable
void control::reload(){
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++)
		(*i)->reload();
		
	fromJSVar(jsVar);
}

//Function to load control from javascript variable
void control::fromJSVar(CScriptVar* c){
	content::fromJSVar(c);
	
	if (CScriptVarLink* contentVar = c->findChild("content")){
		for (list <controlContent*>::iterator i = cContent.begin(); i != cContent.end(); i++){
			if (CScriptVarLink* v = contentVar->var->findChild((*i)->id)) (*i)->fromJSVar(v->var);
		}
	}
	
	if (CScriptVarLink* v = c->findChild("status"))	cStatus = (controlStatus) v->var->getInt();	
	if (CScriptVarLink* v = c->findChild("area")) area.fromJSVar(v->var);
	if (CScriptVarLink* v = c->findChild("mask")) mask.fromJSVar(v->var);
	if (CScriptVarLink* v = c->findChild("dontUseAsReference")) dontUseAsReference = v->var->getInt();	
	if (CScriptVarLink* v = c->findChild("visible")) visible = v->var->getInt();
	if (CScriptVarLink* v = c->findChild("draggable")) draggable = v->var->getInt();
	if (CScriptVarLink* v = c->findChild("clickThrough")) clickThrough = v->var->getInt();
	if (CScriptVarLink* v = c->findChild("dragGrid")) dragGrid = v->var->getInt();
}

//Function to clear script variable
void control::clearScriptVar(){
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++){ (*i)->clearScriptVar();}
	
	if (jsVar) jsVar->removeAllChildren();	
	
	if (jsVar) jsVar->unref();
	jsVar = NULL;	
}

//Function to generate control script variable
void control::genScriptVar(){
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++)
		(*i)->genScriptVar();
	
	jsVar = new CScriptVar (TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
	jsVar->ref();
	
	toJSVar(jsVar);
}

//Function to link script variable
void control::linkScriptVar(){
	if (parent) jsVar->addChildNoDup("parent", parent->jsVar);
	
	if (nextSibling) jsVar->addChildNoDup("nextSibling", nextSibling->jsVar);
	else jsVar->addChildNoDup("nextSibling", new CScriptVar(0));
	
	if (prevSibling) jsVar->addChildNoDup("prevSibling", prevSibling->jsVar);
	else jsVar->addChildNoDup("prevSibling", new CScriptVar(0));
	
	if (children.size() > 0){
		jsVar->addChildNoDup("firstChild", children.front()->jsVar);
		jsVar->addChildNoDup("lastChild", children.front()->jsVar);
	}
	
	for (list <control*>::iterator i = children.begin(); i != children.end(); i++){
		jsVar->addChildNoDup((*i)->id, (*i)->jsVar);
		(*i)->linkScriptVar();
	}
}

//Function to determine if a point is inside the control
bool control::isInside(int x, int y, rect ref){
	rect r = area.abs(ref);
	
	if (!r.isInside(x, y)) return false;//Returns false if outside boundaries
	else if (mask.valid() && mask.getPixel(x - r.x, y - r.y) == 0) return false;//Returns false if outside mask
	
	return true;//Returns true
}

//Function to get control child by id
control* control::getChild(string id){
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++)//For each child
		if ((*i)->id == id) return *i;//Returns control if it matches
		
	return NULL;//Returns null if no child was found
}

//Function to refresh control
void control::refresh(){
	triggerEvent("onRefresh", NULL);
	
	//Refreshes children
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++)
		(*i)->refresh();
		
	area.move(SDL_GetTicks());
	
	//Checks for timeouts
	for (list<eventTimeout>::iterator i = eventTimeouts.begin(); i != eventTimeouts.end(); i++){
		if (SDL_GetTicks() - i->startTime >= i->time) {
			triggerEvent ( i->eventType, NULL );
			eventTimeouts.erase(i);
			i--;
		}
	}
}

//Function to draw control content
void control::draw(){
	triggerEvent("onDraw", NULL);
	
	//Draws children
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++)
		(*i)->draw();
}

//Function to execute a script on the control
void control::runScript(string script){
	CTinyJS* js = new CTinyJS();//Javascript context for event script execution
	
	//Registers common functions and variables
	registerMathFunctions(js);
	registerMiscFunctions(js);
	registerUIFunctions(js);
	registerViewportData(js);
	
	js->root->addChild("caller", jsVar);
	
	js->execute(script);
	
	root->reload();
	
	delete js;
}

//Function to get specific control content
controlContent* control::getContent(string id){
	for (list<controlContent*>::iterator i = cContent.begin(); i != cContent.end(); i++)
		if ((*i)->id == id) return *i;
		
	return NULL;
}

//Function for control copy
control* control::copy(){
	control* result = new control (*this);
	
	result->clearScriptVar();
	
	result->userDefined = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
	result->userDefined->ref();
	result->userDefined->setString("userDefined");
	
	if (userDefined) result->userDefined->copyValue(userDefined);	
	
	result->cContent.clear();
	result->children.clear();
	
	result->parent = NULL;
	result->root = NULL;
	result->nextSibling = NULL;
	result->prevSibling = NULL;
	
	result->genScriptVar();
	result->jsVar->ref();
	
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++)
		addChild((*i)->copy());
		
	for (list<controlContent*>::iterator i = cContent.begin(); i != cContent.end(); i++){
		controlContent* nc = new controlContent (**i);
		
		if (nc->type == "text") nc->data.text = new string (*(*i)->data.text);
		else if (nc->type == "image") nc->data.img = renderableCopy((*i)->data.img);
		
		result->cContent.push_back(nc);
	}
	
	return result;
}

//Function to add child control
void control::addChild(control* childControl){
	childControl->root = this->root;
	childControl->parent = this;
	
	if (children.size() > 0){
		childControl->prevSibling = children.back();
		children.back()->nextSibling = childControl;
	}
	
	children.push_back(childControl);
}

//Function to clear control children (children are deleted!)
void control::clear(){
	for (list<control*>::iterator i = children.begin(); i != children.end(); i++){
		(*i)->clear();
	}
	
	children.clear();
}

//Script handling

control* scriptTargetControl = NULL;//Control used in current script
void (*customSetupJS_UI)(CTinyJS*) = NULL;//Custom javascript setup function

//Javascript setup function
void registerUIFunctions(CTinyJS* js){
	//Stores statuses variables
	js->execute("var status_normal = 0;");
	js->execute("var status_active = 3;");
	js->execute("var status_disabled = 6;");
	
	js->execute("var key_escape = " + toString(SDL_SCANCODE_ESCAPE) + ";");
	js->execute("var key_enter = " + toString(SDL_SCANCODE_RETURN) + ";");
	js->execute("var key_backspace = " + toString(SDL_SCANCODE_BACKSPACE) + ";");
	
	int mX, mY;
	SDL_GetMouseState(&mX, &mY);
	
	js->execute("var mouse.x = " + toString(mX) + ";");
	js->execute("var mouse.y = " + toString(mY) + ";");
	
	js->addNative ("function startTextInput()", scStartTextInput, NULL);
	js->addNative ("function stopTextInput()", scStopTextInput, NULL);
	
	if (customSetupJS_UI) customSetupJS_UI(js);//Custom setup
}

//Function to add a child
void scAddChildFromFile(CScriptVar* v, void* userdata){
	control* target = (control*) userdata;//Control to be targeted
	
	if (target){//If target is valid
		control* c = new control;//New control
		
		c->load_file(preprocessFilePath(v->getParameter("sourceFile")->getString()));//Loads control
		target->addChild(c);
		
		v->setReturnVar ( c->jsVar );
	}
	
	else LOG("Failed adding child: invalid target");
}

//Function to remove a child
void scRemoveChild(CScriptVar* v, void* userdata){
	control* target = (control*) userdata;//Control to be targeted
	
	if (target){//If target is valid
		control* toRemove = target->getChild(v->getParameter("childId")->getString());//Gets control
		if (toRemove){ target->children.remove(toRemove); v->getReturnVar()->setInt(1); }
		else v->getReturnVar()->setInt(0);
	}
}

//Function to set event grabber
void scGrabEvents(CScriptVar* v, void* userdata){
	control* caller = (control*) userdata;
	
	if (caller->parent) caller->parent->eventGrabber = caller;
}

//Function to release events
void scReleaseEvents(CScriptVar* v, void* userdata){
	control* caller = (control*) userdata;
	
	if (caller->parent && caller->parent->eventGrabber == caller) caller->parent->eventGrabber = NULL;
}

//Function to trigger event on control
//WARNING: using this function from inside an event to trigger another event
//on the same control should be avoided, for it causes instability
void scTriggerEvent(CScriptVar*v, void* userdata){
	control* caller = (control*) userdata;
	string event = v->getParameter("event")->getString();
	
	caller->triggerEvent(event, NULL);
}

//Function to add a timeout to a control
void scTimeout ( CScriptVar* v, void* userdata ) {
	control* caller = (control*) userdata;
	
	eventTimeout t;
	t.startTime = SDL_GetTicks();
	t.time = v->getParameter("time")->getInt();
	t.eventType = v->getParameter("event")->getString();
	
	caller->eventTimeouts.push_back(t);
}

void scStartTextInput ( CScriptVar* v, void* userdata ) {
	SDL_StartTextInput();
}

void scStopTextInput ( CScriptVar* v, void* userdata ) {
	SDL_StopTextInput();
}
