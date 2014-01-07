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

#include "easing.h"
#include <iostream>

string sc_easingsSetupScript;//Script to be executed before evaluating any easing expression

//Easing constructor
easing::easing(){
	func = NULL;
	
	pos_begin = 0;
	pos_dist = 0;
	time_begin = 0;
	time_total = 1;
	
	params[0] = 0;
}

//Easing constructor
easing::easing (int beginPos, int dist, int beginTime, int duration, CScriptVarLink* func, double params[EASING_PARAMS_COUNT]){
	pos_begin = beginPos;
	pos_dist = dist;
	time_begin = beginTime;
	time_total = duration;
	
	for (int i = 0; i < EASING_PARAMS_COUNT; i++) this->params[i] = params[i];
	
	this->func = func;
}

//Function to get position at given instant
int easing::pos(double time){
	if (time - time_begin >= time_total) return pos_begin + pos_dist;

	CTinyJS* js = new CTinyJS;	

	registerMathFunctions(js);
	registerMiscFunctions(js);
	registerEasings(js);
	
	js->root->addChild("easingFunction", func->var);	
	string s = "easingFunction (" + toString( double(time - time_begin) / time_total);
	
	for (int i = 0; i < EASING_PARAMS_COUNT && i < func->var->getChildren() - 1; i++)
		s += ", " + toString(params[i]);
		
	s += ")";
	
	double x = atof(js->evaluate(s).c_str());
	
	delete js;
	return pos_begin + pos_dist * x;
}

//Function to register easings
void registerEasings(CTinyJS* c){
	c->execute(sc_easingsSetupScript);
}
