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

#include "viewport.h"

SDL_Window* mainWindow = NULL;
SDL_Renderer* mainRenderer = NULL;

const char* windowCaption = "Physica";
const int windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
const int rendererFlags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE;

bool fullscreen = false;

int windowX = 100, windowY = 100;
rect windowRect (0,0,640,480);
rect windowedRect (0,0,640,480);

//Viewport initialization function
void initViewport(){
	mainWindow = SDL_CreateWindow(windowCaption, windowX, windowY, windowRect.w, windowRect.h, windowFlags | (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
	mainRenderer = SDL_CreateRenderer(mainWindow, -1, rendererFlags);
}

//Function to register viewport data for script
void registerViewportData(CTinyJS* c){
	windowRect.id = "viewport";
	
	windowRect.toJSVar(c->root->addChildNoDup("viewport")->var);
}

//Function to load viewport data from xml node
void loadViewport(xml_node source){
	if (xml_attribute a = source.attribute("windowX")) windowX = a.as_int();
	if (xml_attribute a = source.attribute("windowY")) windowY = a.as_int();
	if (xml_attribute a = source.attribute("windowW")) windowRect.w = a.as_int();
	if (xml_attribute a = source.attribute("windowH")) windowRect.h = a.as_int();
	if (xml_attribute a = source.attribute("windowedW")) windowedRect.w = a.as_int();
	if (xml_attribute a = source.attribute("windowedH")) windowedRect.h = a.as_int();
	if (xml_attribute a = source.attribute("fullscreen")) fullscreen = a.as_bool();
}

//Function to save viewport data from xml node
void saveViewport(xml_node& target){	
	SDL_GetWindowPosition(mainWindow, &windowX, &windowY);;
	
	target.append_attribute("windowX") = windowX;
	target.append_attribute("windowY") = windowY;
	target.append_attribute("windowW") = windowRect.w;
	target.append_attribute("windowH") = windowRect.h;
	target.append_attribute("windowedW") = windowedRect.w;
	target.append_attribute("windowedH") = windowedRect.h;
	target.append_attribute("fullscreen") = fullscreen;
}