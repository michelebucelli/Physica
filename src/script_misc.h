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
MISC SCRIPT FUNCTIONS
*/

#ifndef _SCRIPT_MISC
#define _SCRIPT_MISC

#include "TinyJS/TinyJS.h"
#include <string>
#include "SDL2/SDL.h"

#include "log.h"
#include "files.h"

#include <list>

using namespace std;

class cachedScript {
	public:
	string path;
	string script;
};

extern list<cachedScript> cachedScripts;

string getScript (string);

void registerMiscFunctions(CTinyJS*);

#endif
