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

//Bulk library collection
//Primitives handling header
//(no printing included!)

#ifndef _BULK_GEOM
#define _BULK_GEOM

#define PI 3.14159265358979323846//Pi value

//Class vector
class vector {
	public:
	double x, y;//Vector components
	
	//Constructor
	vector(){
		x = 0;
		y = 0;
	}
	
	//Constructor
	vector(double x, double y){
		this->x = x;
		this->y = y;
	}
	
	//Constructor
	vector(string s){
		fromString(s);
	}
	
	//Function to get module
	double module(){
		return sqrt(x*x + y*y);//Returns result
	}
	
	//Function to set module without changing direction
	vector setModule(double m){
		vector result;//Result vector
		
		if (module() > 0)//If this vector is not null
			result = *this * (m / module());//Sets result
		else result = {0, 0};//Else result is null
			
		return result;//Returns result
	}
	
	//Function to get angle
	double angle(){
		return atan2(x, y);//Returns result
	}
	
	//Function to get angle in degrees
	double angleDeg(){
		return 360 * atan2(x, y) / 2 / PI;
	}
	
	//Vectors sum
	vector operator + (vector v){
		vector result (x + v.x, y + v.y);//Result vector
		return result;//Returns result
	}
	
	//Vector increment
	vector operator += (vector v){
		*this = *this + v;
	}
	
	//Inversion
	vector operator - (){
		return {-x, -y};
	}
	
	//Vector difference
	vector operator - (vector v){
		vector result (x - v.x, y - v.y);//Result vector
		return result;//Returns result
	}
	
	//Vector decrement
	vector operator -= (vector v){
		*this = *this - v;
	}
	
	//Scalar-vector multiplication
	vector operator * (double d){
		vector result (x * d, y * d);//Result vector
		return result;//Returns result
	}
	
	//Scalar-vector division
	vector operator / (double d){
		vector result (x / d, y / d);//Result vector
		return result;//Returns result
	}
	
	//Equal operator
	bool operator == (vector v){
		return v.x == x && v.y == y;
	}
	
	//Unequal operator
	bool operator != (vector v){
		return v.x != x || v.y != y;
	}
	
	//Dot product
	double dot (vector v){
		return x * v.x + y * v.y;//Returns result
	}
	
	//Cross product
	double cross (vector v){
		return -x * v.y + y * v.x;
	}
	
	//Perpendicular vector
	vector perp(){
		return {-y, x};
	}
	
	//Function to rotate vector
	vector rotate (double angle){
		vector result (x * cos(angle) - y * sin(angle), x * sin(angle) + y * cos(angle));//Result vector
		return result;//Returns result
	}
	
	//Loads vector from string
	void fromString(string s){
		deque<string> toks = tokenize <deque<string> > (s, " \t");//Splits into tokens
		
		if (toks.size() >= 2){//If there are enough tokens
			x = atof(toks[0].c_str());//Gets x
			y = atof(toks[1].c_str());//Gets y
		}
	}
};

//Function to get the projection of a vector onto another
vector projection(vector a, vector b){
	double dp = a.dot(b);//Dot product of vectors
	vector result;//Result
	
	if (dp != 0)//If product is not 0
		return {dp / pow(b.module(), 2) * b.x, dp / pow(b.module(), 2) * b.y};//Returns result
	else return {0, 0};//Else returns null vector
}

#endif