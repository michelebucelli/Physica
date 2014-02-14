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
IMAGE HEADER
Provides an image class, suitable for image objects. Surfaces are cached
upon loading for memory saving; all images feature a pointer to the surface
and a rectangle representing the area of the surface to be printed.
*/

#ifndef _IMAGE
#define _IMAGE

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <list>
#include <string>

#include "files.h"
#include "content.h"
#include "rect.h"
#include "renderable.h"
#include "viewport.h"

using namespace std;

//Cached item type
struct cachedItem {
	SDL_Texture* texture;//Loaded texture
	string filePath;//File path (used to identify the item within cache)
};

extern list <cachedItem> cachedTextures;//Cached textures list

//Function to load a texture from cache (loads from file if doesn't find texture)
SDL_Texture* textureFromCache(string);

class image: public renderable {
	public:
	rect area;//Portion of the image to be taken (used as clip rectangle when blitting)
	
	string filePath;//Image file path (keep track to store into JS variable and/or XML)
	SDL_Texture* texture;//Image texture
	
	bool useRect;//Use rectangle flag (if true, area is used to print only a portion of the picture; else the whole surface is print)
	
	int tw, th;//Actual size of the texture
	
	image();
	image(const image&);
	~image();
	
	//Function to print the image
	void print(SDL_Renderer*, int, int);
	
	//Function to print relative to centre position (just calls print adjusting coords)
	void print_centre(SDL_Renderer*, int, int);
	
	//Function to load from XML
	virtual void load(xml_node);
	
	//Function to get width (takes clip rectangle in account)
	int w();
	
	//Function to get height (takes clip rectangle in account)
	int h();
	
	//Function to store into javascript variable
	void toJSVar(CScriptVar*);
	
	//Function to load from javascript variable
	void fromJSVar(CScriptVar*);
	
	//Function to get a pixel (considering clip rectangle)
	Uint8 getPixel(unsigned int, unsigned int);
	
	//Function to get if image is valid
	bool valid();
};

#endif
