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
FILES HEADER
Defines macros for file paths of configuration/data/save files
*/

#ifndef _FILES
#define _FILES

#include <string>
#include <iostream>
using namespace std;

//Macros to stringify other macros
#define STR_EXPAND(tok)		#tok
#define STR(tok) STR_EXPAND(tok)



//Folder path macros (strings without quotes)
#define _FOLDER_DATA		data
#define _FOLDER_GRAPHICS	_FOLDER_DATA/graphics
#define _FOLDER_AUDIO		_FOLDER_DATA/audio
#define _FOLDER_FONTS		_FOLDER_DATA/fonts
#define _FOLDER_SCRIPT		_FOLDER_DATA/js
#define _FOLDER_UI			_FOLDER_DATA/ui
#define _FOLDER_SETTINGS	data/settings
#define _FOLDER_LEVELS		_FOLDER_DATA/levels

//Folder path macros (strings with quotes)
#define FOLDER_DATA			STR(_FOLDER_DATA)
#define FOLDER_GRAPHICS		STR(_FOLDER_GRAPHICS)
#define FOLDER_AUDIO		STR(_FOLDER_AUDIO)
#define FOLDER_FONTS		STR(_FOLDER_FONTS)
#define FOLDER_SCRIPT		STR(_FOLDER_SCRIPT)
#define FOLDER_UI			STR(_FOLDER_UI)
#define FOLDER_SETTINGS		STR(_FOLDER_SETTINGS)
#define FOLDER_LEVELS		STR(_FOLDER_LEVELS)



//Individual files
#define FILE_MENU			STR(_FOLDER_UI/global.xml)
#define FILE_EASINGS		STR(_FOLDER_SCRIPT/easings.js)
#define FILE_UI_SCRIPT		STR(_FOLDER_SCRIPT/control_funcs.js)
#define FILE_SETTINGS		STR(_FOLDER_SETTINGS/settings.xml)
#define FILE_PROGRESS		STR(_FOLDER_DATA/progress.xml)
#define FILE_LOG			STR(output.log)



//Function to preprocess a file path
//Replaces preprocessor directives (@*) with the paths of macros above
//Known directives:
//		(@data),(@graphics),(@audio),(@fonts),(@script),(@ui) - folders
//		(#menu),(#easings) - individual files
string preprocessFilePath(string);

//Function for folders debug
void folderDebug(ostream&);

#endif
