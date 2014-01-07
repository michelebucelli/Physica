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
MISCELLANEOUS UTILS
*/

#ifndef _UTILS
#define _UTILS

#include <string>
#include <list>
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <dirent.h>

#define COMPLEMENTARY(COLOR)		((0xFF0000 - (0xFF0000 & COLOR)) + (0xFF00 - (0xFF00 & COLOR)) + (0xFF - (0xFF & COLOR)))


using namespace std;

//Functions to convert to string
string toString(int);
string toString(double);

//Function to get list of files in folder
list<string> listFiles(string);

#endif