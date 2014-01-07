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
RECTANGLE HEADER
Defines a rectangle class, together with a way to load it from XML
files (basing upon pugixml library)
*/

#ifndef _RECT
#define _RECT

#include <SDL2/SDL.h>
#include "content.h"
#include "easing.h"

#define SURFRECT(SURF)						(_rect( SURF->clip_rect.x, SURF->clip_rect.y, SURF->clip_rect.w, SURF->clip_rect.h ))

class _rect: public SDL_Rect {
	public:
	
	int xRef, yRef;
	easing *xEasing, *yEasing, *wEasing, *hEasing;
	
	_rect();
	_rect (int, int, int, int);
	
	//Function to determine if a point is inside rectangle
	bool isInside(double, double);
	
	//Function to get absolute rectangle with respect to given rectangle
	_rect abs(_rect);
	
	//Function to move the rectangle according to easings
	void move(int);
};

class rect: public content, public _rect {
	public:
	
	rect();
	rect(_rect);
	rect (int, int, int, int);
	
	//Function to load from XML
	virtual void load(xml_node);
	
	//Function to store into javascript variable
	void toJSVar(CScriptVar*);
	
	//Function to load from javascript variable
	void fromJSVar(CScriptVar*);	
};

_rect getViewport(SDL_Renderer*);

//Functions that set an easing on a rectangle (JS)
void scRectSetEasingX(CScriptVar*, void*);
void scRectSetEasingY(CScriptVar*, void*);
void scRectSetEasingW(CScriptVar*, void*);
void scRectSetEasingH(CScriptVar*, void*);

#endif