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

//Buch Library Kollection
//UI dialog header

#ifndef _BULK_UI_DIALOG
#define _BULK_UI_DIALOG

//Dialog class
class dialog {
	public:
	window dialogWindow;//Dialog window
	panel* dialogFrame;//Dialog frame
	
	void (*frameBegin)();//Frame beginning function
	void (*frameEnd)();//Frame ending function
	void (*events)(SDL_Event);//Common event checking function
	bool* quitFlag;//Quit flag pointer
	void (*update)();//Update function
	
	//Constructor
	dialog(){
		dialogFrame = NULL;
		
		frameBegin = NULL;
		frameEnd = NULL;
		events = NULL;
		quitFlag = NULL;
	}
	
	//Function to load base dialog
	void loadDialog(string path){
		dialogWindow = loadWindow(path, "dialogWindow");//Loads window
		dialogFrame = (panel*) dialogWindow.getControl("frame");//Gets frame panel
	}
	
	//Function to center dialog to screen
	void centre(int screenW, int screenH){
		if (!dialogFrame) return;
		
		dialogFrame->area.x = (screenW - dialogFrame->area.w) / 2;//Centers on x
		dialogFrame->area.y = (screenH - dialogFrame->area.h) / 2;//Centers on y
	}
};

#endif