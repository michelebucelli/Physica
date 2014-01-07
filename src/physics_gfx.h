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
GRAPHICS for PHYSICS ENTITIES
Contains functions to print physics.h classes
*/

#ifndef _PHYSICS_GFX
#define _PHYSICS_GFX

#include "physics.h"
#include "utils.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL2_gfxPrimitives.h"

//Entity printing function
void printEntity(SDL_Renderer*, entity*, Uint32, int, int, bool = true);

//Scene printing function
void printScene(SDL_Renderer*, scene*, int, int, bool = false);

#endif