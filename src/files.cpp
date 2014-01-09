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

#include "files.h"

//Function to preprocess file path
string preprocessFilePath(string filePath){
	int n;
	string path = filePath;
	
	for (n = path.find("(@data)"); n != path.npos; n = path.find("(@data)"))
		path.replace(n, 7, FOLDER_DATA);
		
	for (n = path.find("(@graphics)"); n != path.npos; n = path.find("(@graphics)"))
		path.replace(n, 11, FOLDER_GRAPHICS);
		
	for (n = path.find("(@audio)"); n != path.npos; n = path.find("(@audio)"))
		path.replace(n, 8, FOLDER_AUDIO);
		
	for (n = path.find("(@fonts)"); n != path.npos; n = path.find("(@fonts)"))
		path.replace(n, 8, FOLDER_FONTS);
		
	for (n = path.find("(@script)"); n != path.npos; n = path.find("(@script)"))
		path.replace(n, 9, FOLDER_SCRIPT);
		
	for (n = path.find("(@ui)"); n != path.npos; n = path.find("(@ui)"))
		path.replace(n, 5, FOLDER_UI);
		
	for (n = path.find("(@levels)"); n != path.npos; n = path.find("(@levels)"))
		path.replace(n, 9, FOLDER_LEVELS);
		
	for (n = path.find("(#menu)"); n != path.npos; n = path.find("(#menu)"))
		path.replace(n, 7, FILE_MENU);
		
	for (n = path.find("(#easings)"); n != path.npos; n = path.find("(#easings)"))
		path.replace(n, 10, FILE_EASINGS);
		
	for (n = path.find("(#uiScript)"); n != path.npos; n = path.find("(#uiScript)"))
		path.replace(n, 11, FILE_UI_SCRIPT);

	if (path != filePath) return preprocessFilePath(path);
	else return path;
}

//Function to debug folders
void folderDebug(ostream &o){
	o << "PROGRAM FOLDERS STRUCTURE:" << endl;
	o << "  DATA         " << FOLDER_DATA << endl;
	o << "  GRAPHICS     " << FOLDER_GRAPHICS << endl;
	o << "  AUDIO        " << FOLDER_AUDIO << endl;
	o << "  FONTS        " << FOLDER_FONTS << endl;
	o << "  SCRIPT       " << FOLDER_SCRIPT << endl;
	o << "  UI           " << FOLDER_UI << endl;
	o << "  SETTINGS     " << FOLDER_SETTINGS << endl;
	o << "  LEVELS       " << FOLDER_LEVELS << endl << endl;
	
	o << "RELEVANT FILES:" << endl;
	o << "  ACHIEVEMENTS " << FILE_ACHIEVEMENTS << endl;
	o << "  SETTINGS     " << FILE_SETTINGS << endl;
	o << "  PROGRESS     " << FILE_PROGRESS << endl;
	o << "  MENU         " << FILE_MENU << endl;
	o << "  EASINGS      " << FILE_EASINGS << endl;
	o << "  UI SCRIPT    " << FILE_UI_SCRIPT << endl;
}
