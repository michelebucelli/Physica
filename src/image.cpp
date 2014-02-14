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

#include "image.h"
#include <iostream>

list <cachedItem> cachedTextures;//Cached textures

//Function to load a texture from cache
SDL_Texture* textureFromCache(string filePath){
	for (list<cachedItem>::iterator i = cachedTextures.begin(); i != cachedTextures.end(); i++)//For each texture in cache
		if (i->filePath == filePath) return i->texture;//Returns texture if file matches
		
	//If texture was not found, then loads from file
	
	SDL_Surface* loaded = IMG_Load(filePath.c_str());//Loads surface
	SDL_Texture* result = SDL_CreateTextureFromSurface(mainRenderer, loaded);
	SDL_FreeSurface(loaded);
	
	return result;
}

//Image constructor
image::image(){
	texture = NULL;
	useRect = false;
	
	tw = 0;
	th = 0;
	
	type = "image";
}

image::image ( const image& i ) : renderable (i) {
	texture = i.texture;
	useRect = i.useRect;
	
	tw = i.tw;
	th = i.th;
	
	area = rect ( i.area );
}

//Image destructor
image::~image(){
}

//Image print function
void image::print(SDL_Renderer* target, int x, int y){
	SDL_Rect vp;
	SDL_RenderGetViewport(target, &vp);
	
	SDL_Rect dstrect = {x - vp.x, y - vp.y, w(), h()};//Offset rectangle
	_rect r (0, 0, tw, th);
	
	rect t = area.abs(r);
	SDL_RenderCopy(target, texture, useRect ? &t : &r, &dstrect);
}

//Function to print image with position relative to centre
void image::print_centre(SDL_Renderer* target, int x, int y){
	print(target, x - w() / 2, y - h() / 2);//Prints
}

//Function to load an image from xml
void image::load(xml_node source){
	if (!source) return;
	
	content::load(source);//Loads base content data
	
	if (xml_attribute a = source.attribute( "file")){//If file attribute is specified
		texture = textureFromCache(preprocessFilePath(a.value()));//Gets image
		filePath = a.value();
		
		if (texture) SDL_QueryTexture(texture, NULL, NULL, &tw, &th);
	}
	
	if (xml_node area = lastChild(source, "area")){//If area node is valid
		useRect = true;//Sets use rectangle flag to true
		this->area.load(area);//Loads area
	}
}

//Function to get image width
int image::w(){
	if (useRect) return area.w;
	else if (texture) return tw;
	else return 0;
}

//Function to get image height
int image::h(){
	if (useRect) return area.h;
	else if (texture) return th;
	else return 0;
}

//Function to store into a javascript variable
void image::toJSVar(CScriptVar* c){
	content::toJSVar(c);
	
	c->addChildNoDup("file")->var->setString(filePath);
	area.toJSVar(c->addChildNoDup("area")->var);
}

//Function to load from javascript variable
void image::fromJSVar(CScriptVar* c){
	content::fromJSVar(c);
	
	if (CScriptVarLink* v = c->findChild("file")){
		string newFile = v->var->getString();
		
		if (newFile != filePath){
			if (texture) SDL_DestroyTexture(texture);
			texture = NULL;
			
			filePath = newFile;
			texture = textureFromCache(preprocessFilePath(filePath));
			
			if (texture) SDL_QueryTexture(texture, NULL, NULL, &tw, &th);
		}
	}
	
	if (CScriptVarLink* v = c->findChild("area")) { area.fromJSVar(v->var); useRect = true; }
	else useRect = false;
}

//Function to get pixel of image (considering clip) (INCOMPLETE)
Uint8 image::getPixel(unsigned int x, unsigned int y){
	return 0;
}

//Function to get if image is valid
bool image::valid(){
	return texture != NULL;
}
