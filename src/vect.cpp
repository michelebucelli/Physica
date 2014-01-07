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

#include "vect.h"

vect::vect() { x = 0; y = 0; }
vect::vect (double x, double y) { this->x = x; this->y = y; }
vect::vect (string s) { fromString(s); }

double vect::module() { return sqrt (x*x + y*y); }

vect vect::setModule(double m){
	vect result;
	double mod = module();
	
	if (mod > 0) result = *this * (m / mod);
	else result = vect(0,0);
	
	return result;
}

double vect::angle(){ return atan2(x, y); }
double vect::angleDeg(){ return 360 * atan2(x, y) / 2 / PI; }

vect vect::operator + (vect v) { return vect (x + v.x, y + v.y); }
void vect::operator += (vect v) { *this = *this + v; }

vect vect::operator - () { return vect (-x, -y); }
vect vect::operator - (vect v) { return vect (x - v.x, y - v.y); }
void vect::operator -= (vect v) { *this = *this - v; }

vect vect::operator * (double d) { return vect (x * d, y * d); }
vect vect::operator / (double d) { return vect (x / d, y / d); }

bool vect::operator == (vect v) { return x == v.x && y == v.y; }
bool vect::operator != (vect v) { return x != v.x || y != v.y; }

double vect::dot (vect v) { return x * v.x + y * v.y; }
double vect::cross (vect v) { return -x * v.y + y * v.x; }

vect vect::perp(){ return vect (-y, x); }

vect vect::rotate(double angle) { return vect (x * cos(angle) - y * sin(angle), x * sin(angle) + y * cos(angle)); }

void vect::fromString(string s){
	int n = s.find(" ");
	
	if (n != s.npos){
		x = atof(s.substr(0, n).c_str());
		y = atof(s.substr(n + 1).c_str());
	}
}

vect projection(vect a, vect b){
	double dp = a.dot(b);
	
	if (dp != 0) return vect (dp / pow(b.module(), 2) * b.x, dp / pow(b.module(), 2) * b.y);
	else return vect (0, 0);
}

double intersection (vect p1, vect v1, vect p2, vect v2){
	return (p2 - p1).cross(v2) / v1.cross(v2);
}