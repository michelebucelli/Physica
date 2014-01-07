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
VIEWPORT
*/

#ifndef _VIEWPORT
#define _VIEWPORT

#include "SDL2/SDL.h"
#include "rect.h"

extern const char* windowCaption;
extern const int windowFlags;
extern const int rendererFlags;

extern bool fullscreen;

extern rect windowedRect;

extern int windowX, windowY;
extern rect windowRect;

extern SDL_Window* mainWindow;
extern SDL_Renderer* mainRenderer;

#define VPRECT(RECT)				(SDL_RenderGetViewport(mainRenderer, RECT))
#define SET_FULLSCREEN(B)			SDL_SetWindowFullscreen(mainWindow, B ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0); if (!(B)) { SDL_SetWindowSize(mainWindow, windowedRect.w, windowedRect.h); SDL_SetWindowPosition(mainWindow, 100, 100); }

//Initialization function
//Sets up mainWindow and mainRenderer
void initViewport();

//Function to register viewport data
void registerViewportData(CTinyJS*);

//Function to load viewport data
void loadViewport(xml_node);

//Function to save viewport data
void saveViewport(xml_node&);

#endif