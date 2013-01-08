#include "physica.h"//Includes game main header

//Main function
int main(int argc, char* argv[]){
	gameInit();//Initializes the game
	
	while (running){//While game is running
		FRAME_BEGIN;//Frame beginning
		
		BKG;//Prints background
		
		if (curUiMode == ui_mainMenu){//If in main menu
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events					
				menu.checkEvents(ev);//Checks menu events
			}
			
			menu.print(video);//Prints menu
		}
		
		if (curUiMode == ui_levels){//If in level selection
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) curUiMode = ui_mainMenu;//Back to menu on esc
					
				levelSelect.checkEvents(ev);//Checks level selector events
			}
			
			levelSelect.print(video);//Prints level selector
		}
		
		if (curUiMode == ui_game){//If in game mode		
			int printOX = video_w / 2 - (camFollow ? current.player->position.x : current.currentLevel->w / 2);//Print offset x
			int printOY = video_h / 2 - (camFollow ? current.player->position.y : current.currentLevel->h / 2);//Print offset y
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN){//If pressed a key
					if (ev.key.keysym.sym == SDLK_ESCAPE) pauseClick ({});//Pauses on esc
					else current.paused = false;//Unpause
				}
					
				hud.checkEvents(ev, printOX, printOY);//Checks hud events
			}
			
			current.frame(0.2, keys);//Game frame
			
			if (frames % (fps / printFps) == 0){//On printing frame
				current.print(video, printOX, printOY);//Prints game scene
				
				updateHud();//Updates hud
				
				if (!camFollow) hud.print(video, printOX, printOY);//Prints hud on level corner if not following player
				else hud.print(video, (video->w - current.currentLevel->w) / 2, (video->h - current.currentLevel->h) / 2);//Else prints hud on upper-left level
			}
		}
		
		if (curUiMode == ui_paused){//If paused
			int printOX = video_w / 2 - (camFollow ? current.player->position.x : current.currentLevel->w / 2);//Print offset x
			int printOY = video_h / 2 - (camFollow ? current.player->position.y : current.currentLevel->h / 2);//Print offset y
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN){//If pressed a key
					if (ev.key.keysym.sym == SDLK_ESCAPE) backClick({});//Returns to level selector on esc
					else if (ev.key.keysym.sym == SDLK_RETURN) resumeClick({});//Resumes on enter
				}
					
				pause.checkEvents(ev);//Checks pause events
			}
			
			if (frames % (fps / printFps) == 0){//On printing frame
				current.print(video, printOX, printOY);//Prints game scene
				
				if (!camFollow) hud.print(video, printOX, printOY);//Prints hud on level corner if not following player
				else hud.print(video, (video->w - current.currentLevel->w) / 2, (video->h - current.currentLevel->h) / 2);//Else prints hud on upper-left level				
				
				DARK;//Dark transparent fill
				pause.print(video);//Prints pause screen
			}
		}
		
		if (curUiMode == ui_success){//If completed level
			int printOX = video_w / 2 - (camFollow ? current.player->position.x : current.currentLevel->w / 2);//Print offset x
			int printOY = video_h / 2 - (camFollow ? current.player->position.y : current.currentLevel->h / 2);//Print offset y
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN){//If pressed a key
					if (ev.key.keysym.sym == SDLK_ESCAPE) backClick({});//Goes back on esc
					else if (ev.key.keysym.sym == SDLK_RETURN) nextClick({});//Goes next on enter
				}
				
				success.checkEvents(ev);//Checks success window events
			}
			
			if (frames % (fps / printFps) == 0){//On printing frame
				current.print(video, printOX, printOY);//Prints game scene
				
				if (!camFollow) hud.print(video, printOX, printOY);//Prints hud on level corner if not following player
				else hud.print(video, (video->w - current.currentLevel->w) / 2, (video->h - current.currentLevel->h) / 2);//Else prints hud on upper-left level
				
				DARK;//Dark transparent fill
				
				updateSuccess();//Updates success screen
				success.print(video);//Prints success screen
			}
		}

		if (curUiMode == ui_settings){//If in settings view
			while (SDL_PollEvent(&ev)){//While there are events on stack
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN){//If pressed a key
					if (ev.key.keysym.sym == SDLK_ESCAPE) curUiMode = ui_mainMenu;//Goes back on esc
				}
				
				settings.checkEvents(ev);//Checks settings window events
				applySettings();//Applies settings changes
			}
			
			settings.print(video);//Prints settings window
		}
		
		UPDATE;//Updates
		
		FRAME_END;//Frame end
	}
	
	gameQuit();//Quits
	
	return 0;//Returns 0
}