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

#include "renderable.h"
#include "image.h"

//Function to load a renderable from XML
renderable* renderableFromXML(xml_node source){
	renderable* result = NULL;
	string type = "";
	
	if (xml_attribute a = source.attribute( "type")) type = a.value();
	else type = source.name();
	
	if (type == "image") result = new image;
	else result = new image;//Default type: image
	
	//Other renderable types should be placed there
	
	if (result) result->load(source);
	
	return result;
}

//Function for renderable copy
renderable* renderableCopy(renderable* source){
	renderable* result = NULL;
	
	if (source->type == "image") result = new image ( * (image*) source);
	
	return result;
}

//Function to print a renderable within a rectangle
//The renderable is repeated until the requested rectangle is filled
//starting from the topleft corner
void fillRect(renderable* img, SDL_Renderer* target, _rect r, int noClipSides){
	int x, y;//Coordinate counters
	
	_rect clip = r;//Clip rectangle
	SDL_Rect vp;//Viewport
	
	SDL_RenderGetViewport(target, &vp);
	
	//Checks possible sides with no clip
	if (noClipSides & 0b1000) { clip.h += r.y; clip.y = 0; }
	if (noClipSides & 0b0100) { clip.w += r.x; clip.x = 0; }
	if (noClipSides & 0b0010) { clip.w = vp.w - r.x; }
	if (noClipSides & 0b0001) { clip.h = vp.h - r.y; }
	
	SDL_IntersectRect(&vp, &clip, &clip);
	
	SDL_RenderSetClipRect(target, &clip);//Sets clip rectangle
	
	for (x = r.x; x < r.x + r.w; x += img->w())
		for (y = r.y; y < r.y + r.h; y += img->h())
			img->print(target, x, y);//Prints image
	
	SDL_RenderSetClipRect(target, NULL);//Resets clip rectangle
}
