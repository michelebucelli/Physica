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

#include "rect.h"
#include <iostream>

//Rectangle constructor
_rect::_rect(){	
	x = 0;
	y = 0;
	w = 1;
	h = 1;
	
	xRef = 0;
	yRef = 0;
	
	xEasing = NULL;
	yEasing = NULL;
	wEasing = NULL;
	hEasing = NULL;
}

//Rectangle constructor from data
_rect::_rect(int x, int y, int w, int h){
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	
	xRef = 0;
	yRef = 0;
	
	xEasing = NULL;
	yEasing = NULL;
	wEasing = NULL;
	hEasing = NULL;
}

//Function to determine if a point is inside the rectangle
bool _rect::isInside(double px, double py){
	return px > x && px < x + w && py > y && py < y + h;
}

//Function to get absoulte rectangle with respect to rectangle
_rect _rect::abs(_rect ref){
	rect result = *this;
	
	if (xRef == 0) result.x += ref.x;
	else if (xRef == 1) result.x += ref.x + (ref.w - w) / 2;
	else if (xRef == 2) result.x = ref.x + ref.w - w - x;
	
	if (yRef == 0) result.y += ref.y;
	else if (yRef == 1) result.y += ref.y + (ref.h - h) / 2;
	else if (yRef == 2) result.y = ref.y + ref.h - h - y;
	
	return result;
}

//Function to move the rectangle following easings
void _rect::move(int t){
	if (xEasing){
		if (t < xEasing->time_begin + xEasing->time_total) x = xEasing->pos(t);
		else {
			x = xEasing->pos(xEasing->time_begin + xEasing->time_total);
			
			delete xEasing;
			xEasing = NULL;
		}
	}
	
	if (yEasing){
		if (t < yEasing->time_begin + yEasing->time_total) y = yEasing->pos(t);
		else {
			y = yEasing->pos(yEasing->time_begin + yEasing->time_total);
			
			delete yEasing;
			yEasing = NULL;
		}
	}
	
	if (wEasing){
		if (t < wEasing->time_begin + wEasing->time_total) w = wEasing->pos(t);
		else {
			w = wEasing->pos(wEasing->time_begin + wEasing->time_total);
			
			delete wEasing;
			wEasing = NULL;
		}
	}
	
	if (hEasing){
		if (t < hEasing->time_begin + hEasing->time_total) h = hEasing->pos(t);
		else {
			h = hEasing->pos(hEasing->time_begin + hEasing->time_total);
			
			delete hEasing;
			hEasing = NULL;
		}
	}
}

//Rectangle constructor
rect::rect(){
}

//Rectangle constructor from basic rectangle
rect::rect(_rect r){
	x = r.x;
	y = r.y;
	w = r.w;
	h = r.h;
	
	type = "rect";
	
	xRef = r.xRef;
	yRef = r.yRef;
	
	xEasing = NULL;
	yEasing = NULL;
	wEasing = NULL;
	hEasing = NULL;
}

rect::rect ( rect &r ) : content(r) {	
	this->x = r.x;
	this->y = r.y;
	this->w = r.w;
	this->h = r.h;
	
	xRef = r.xRef;
	yRef = r.yRef;
	
	xEasing = NULL;
	yEasing = NULL;
	wEasing = NULL;
	hEasing = NULL;
}

//Rectangle constructor from data
rect::rect(int x, int y, int w, int h){
	this->x = x;
	this->y = y;
	this->w = w;
	this->h = h;
	
	type = "rect";
	
	xRef = 0;
	yRef = 0;
	
	xEasing = NULL;
	yEasing = NULL;
	wEasing = NULL;
	hEasing = NULL;
}

//Function to load from XML
void rect::load(xml_node source){
	if (!source) return;
	
	content::load(source);//Loads content data
	
	xml_attribute a;//Attribute
	
	//Gets members
	if (xml_attribute a = source.attribute("x")) x = a.as_int();
	if (xml_attribute a = source.attribute("y")) y = a.as_int();
	if (xml_attribute a = source.attribute("w")) w = a.as_int();
	if (xml_attribute a = source.attribute("h")) h = a.as_int();
	
	if (xml_attribute a = source.attribute( "xRef")) xRef = a.as_int();
	if (xml_attribute a = source.attribute( "yRef")) yRef = a.as_int();
}

//Function to store into javascript variable
void rect::toJSVar(CScriptVar* c){
	content::toJSVar(c);
	
	c->addChildNoDup("x")->var->setInt(x);
	c->addChildNoDup("y")->var->setInt(y);
	c->addChildNoDup("w")->var->setInt(w);
	c->addChildNoDup("h")->var->setInt(h);
	c->addChildNoDup("xRef")->var->setInt(xRef);
	c->addChildNoDup("yRef")->var->setInt(yRef);
	
	//Adds easing functions
	CScriptVar* setEasingXVar = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE);
	setEasingXVar->setCallback(scRectSetEasingX, this);
	
	//Adds function arguments
	setEasingXVar->addChildNoDup("b");
	setEasingXVar->addChildNoDup("c");
	setEasingXVar->addChildNoDup("d");
	setEasingXVar->addChildNoDup("func");
	
	for (int i = 0; i < EASING_PARAMS_COUNT; i++) setEasingXVar->addChildNoDup("param"+ toString(i));
	
	c->addChild("setEasingX", setEasingXVar);
	
	CScriptVar* setEasingYVar = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE);
	setEasingYVar->setCallback(scRectSetEasingY, this);
	
	//Adds function arguments
	setEasingYVar->addChildNoDup("b");
	setEasingYVar->addChildNoDup("c");
	setEasingYVar->addChildNoDup("d");
	setEasingYVar->addChildNoDup("func");
	
	for (int i = 0; i < EASING_PARAMS_COUNT; i++) setEasingYVar->addChildNoDup("param"+ toString(i));
	
	c->addChild("setEasingY", setEasingYVar);
	
	CScriptVar* setEasingWVar = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE);
	setEasingWVar->setCallback(scRectSetEasingW, this);
	
	//Adds function arguments
	setEasingWVar->addChildNoDup("b");
	setEasingWVar->addChildNoDup("c");
	setEasingWVar->addChildNoDup("d");
	setEasingWVar->addChildNoDup("func");
	
	for (int i = 0; i < EASING_PARAMS_COUNT; i++) setEasingWVar->addChildNoDup("param"+ toString(i));
	
	c->addChild("setEasingW", setEasingWVar);
	
	CScriptVar* setEasingHVar = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_FUNCTION | SCRIPTVAR_NATIVE);
	setEasingHVar->setCallback(scRectSetEasingH, this);
	
	//Adds function arguments
	setEasingHVar->addChildNoDup("b");
	setEasingHVar->addChildNoDup("c");
	setEasingHVar->addChildNoDup("d");
	setEasingHVar->addChildNoDup("func");
	
	for (int i = 0; i < EASING_PARAMS_COUNT; i++) setEasingHVar->addChildNoDup("param"+ toString(i));
	
	c->addChild("setEasingH", setEasingHVar);
}
	
//Function to load from javascript variable
void rect::fromJSVar(CScriptVar* c){
	content::fromJSVar(c);
	
	if (CScriptVarLink* v = c->findChild("x")) x = v->var->getInt();
	if (CScriptVarLink* v = c->findChild("y")) y = v->var->getInt();
	if (CScriptVarLink* v = c->findChild("w")) w = v->var->getInt();
	if (CScriptVarLink* v = c->findChild("h")) h = v->var->getInt();
	if (CScriptVarLink* v = c->findChild("xRef")) xRef = v->var->getInt();
	if (CScriptVarLink* v = c->findChild("yRef")) yRef = v->var->getInt();
}

//Function to get rectangle corresponding to viewport
_rect getViewport(SDL_Renderer* r){
	SDL_Rect rc;
	SDL_RenderGetViewport(r, &rc);
	
	return _rect (rc.x, rc.y, rc.w, rc.h);
}

//Function that sets an easing on a rectangle (X axis)
void scRectSetEasingX(CScriptVar* c, void* userdata){
	rect* target = (rect*) userdata;
	
	if (target->xEasing) delete target->xEasing;
	
	double params[EASING_PARAMS_COUNT];
	for (int i = 0; i < EASING_PARAMS_COUNT; i++) params[i] = c->findChildOrCreate("param" + toString(i))->var->getDouble();
	
	target->xEasing = new easing ( 
			c->getParameter("b")->getInt(), 
			(target->xRef == 2 ? -1 : 1) * c->getParameter("c")->getInt(),
			SDL_GetTicks(),
			c->getParameter("d")->getInt(),
			new CScriptVarLink (*c->findChildOrCreate("func")),
			params
	);
}

//Function that sets an easing on a rectangle (Y axis)
void scRectSetEasingY(CScriptVar* c, void* userdata){
	rect* target = (rect*) userdata;
	
	if (target->yEasing) delete target->yEasing;
	
	double params[EASING_PARAMS_COUNT];
	for (int i = 0; i < EASING_PARAMS_COUNT; i++) params[i] = c->findChildOrCreate("param" + toString(i))->var->getDouble();
	
	target->yEasing = new easing ( 
			c->getParameter("b")->getInt(), 
			(target->yRef == 2 ? -1 : 1) * c->getParameter("c")->getInt(),
			SDL_GetTicks(),
			c->getParameter("d")->getInt(),
			new CScriptVarLink (*c->findChildOrCreate("func")),
			params
	);
}

//Function that sets an easing on a rectangle (width)
void scRectSetEasingW(CScriptVar* c, void* userdata){
	rect* target = (rect*) userdata;
	
	if (target->wEasing) delete target->wEasing;
	
	double params[EASING_PARAMS_COUNT];
	for (int i = 0; i < EASING_PARAMS_COUNT; i++) params[i] = c->findChildOrCreate("param" + toString(i))->var->getDouble();
	
	target->wEasing = new easing ( 
			c->getParameter("b")->getInt(), 
			(target->xRef == 2 ? -1 : 1) * c->getParameter("c")->getInt(),
			SDL_GetTicks(),
			c->getParameter("d")->getInt(),
			new CScriptVarLink (*c->findChildOrCreate("func")),
			params
	);
}

//Function that sets an easing on a rectangle (height)
void scRectSetEasingH(CScriptVar* c, void* userdata){
	rect* target = (rect*) userdata;
	
	if (target->hEasing) delete target->hEasing;
	
	double params[EASING_PARAMS_COUNT];
	for (int i = 0; i < EASING_PARAMS_COUNT; i++) params[i] = c->findChildOrCreate("param" + toString(i))->var->getDouble();
	
	target->hEasing = new easing ( 
			c->getParameter("b")->getInt(), 
			(target->yRef == 2 ? -1 : 1) * c->getParameter("c")->getInt(),
			SDL_GetTicks(),
			c->getParameter("d")->getInt(),
			new CScriptVarLink (*c->findChildOrCreate("func")),
			params
	);
}
