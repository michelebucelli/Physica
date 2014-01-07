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
VECTOR CLASS
*/

#ifndef _VECTOR
#define _VECTOR

#include <string>
#include <cmath>
#include <cstdlib>

using namespace std;

#define PI 3.14159265358979323846

class vect {
	public:
	double x, y;
	
	vect();
	vect(double, double);
	vect(string);
	
	//Returns the module of the vector
	double module();
	
	//Returns a vector with same direction as this but given module
	vect setModule(double);
	
	//Returns vector direction
	double angle(); //Radiants
	double angleDeg(); //Degrees
	
	//Vector aritmethics
	vect operator +  (vect);
	void   operator += (vect);
	
	vect operator -  ();
	vect operator -  (vect);
	void   operator -= (vect);
	
	vect operator *  (double);
	vect operator /  (double);
	
	bool   operator == (vect);
	bool   operator != (vect);
	
	double dot         (vect);
	double cross       (vect);
	
	//Returns same vector but perpendicular vector (same as rotate(PI/2))
	vect perp();
	
	//Returns vector rotated by given angle (in radiants)
	vect rotate (double);
	
	//Loads vector from string
	void fromString(string);
};

//Returns the projection of a vector onto another
vect projection (vect, vect);

//Intersects two lines (returns a double; intersection is calculated p1 + v1 * result)
double intersection(vect, vect, vect, vect);

#endif