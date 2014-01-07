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
EASING HEADER
Defines the easing class to implement object movements
*/

#ifndef _EASING
#define _EASING

#include <string>
#include <cstdlib>

#include "TinyJS/TinyJS.h"
#include "TinyJS/TinyJS_MathFunctions.h"
#include "script_misc.h"

#include "utils.h"

#define EASING_PARAMS_COUNT 2

using namespace std;

extern string sc_easingsSetupScript;//Script to be executed before evaluating any easing expression

//Easing class
class easing {
	public:
	
	CScriptVarLink* func;//Javascript function content (where t, b, c, d represent time, initial position, distance and duration)
	
	int pos_begin, pos_dist;//Initial position and distance
	int time_begin, time_total;//Initial and total time
	
	double params[EASING_PARAMS_COUNT];//Easing parameters (mostly unused)
	
	easing();
	easing(int, int, int, int, CScriptVarLink*, double[3]);
	
	//Function to get position
	int pos(double);
};

//Function to register easings
void registerEasings(CTinyJS*);

#endif