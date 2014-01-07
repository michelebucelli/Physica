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

#include "utils.h"

//Functions to convert to string
string toString(int n){
	stringstream s;
	s << n;
	
	return s.str();
}

string toString(double n){
	stringstream s;
	s << n;
	
	return s.str();
}

//Function to list files in folder
list<string> listFiles(string folder){
	list<string> result;
	
	DIR* d = opendir(folder.c_str());
	
	if (!d) return result;
	
	for (struct dirent * entry = readdir(d); entry; entry = readdir(d)){
		string s = entry->d_name;
		
		if (s != ".." && s != ".")
			result.push_back(entry->d_name);
	}
	
	closedir(d);
	
	return result;
}