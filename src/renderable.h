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
RENDERABLE
Defines the base renderable class, used to refer to any kind of printable
objects
*/

#ifndef _RENDERABLE
#define _RENDERABLE

#include "content.h"
#include "rect.h"
#include <SDL2/SDL.h>

class renderable: public content {
	public:
	
	//Print function
	//Prints the renderable object with position relative to top left corner
	virtual void print(SDL_Renderer*, int, int) = 0;
	
	//Print function
	//Prints the renderable object with position relative to centre
	virtual void print_centre(SDL_Renderer*, int, int) = 0;
	
	//Functions to get renderable size
	virtual int w() = 0;
	virtual int h() = 0;
	
	//Function to get if renderable is valid
	virtual bool valid() = 0;
	
	virtual void load(xml_node) = 0;
	virtual void toJSVar(CScriptVar*) = 0;
	virtual void fromJSVar(CScriptVar*) = 0;
	
};

//Function to get a renderable object from XML (decides between renderable types)
//Renderable type can be picked according either to node name or type
//For example: both '<image ... > ... </image>' and '<renderable type="image" ... > ... </renderable>' nodes
//Are loaded with type "image"
renderable* renderableFromXML(xml_node);

//Function to fill a rect with given renderable (tiles the renderable inside the rectangle)
void fillRect(renderable*, SDL_Renderer*, _rect, int = 0);

//Function for renderable copy
renderable* renderableCopy(renderable*);

#endif