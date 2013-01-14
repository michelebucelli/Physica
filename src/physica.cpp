#include "physica.h"//Includes game main header

//Main function
int main(int argc, char* argv[]){
	gameInit(argc, argv);//Initializes the game
	
	while (running){//While game is running
		FRAME_BEGIN;//Frame beginning
		
		if (curUiMode == ui_mainMenu){//If in main menu
			BKG;//Prints background
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events					
				menuFrame->checkEvents(ev);//Checks menu events
			}
		
			if (running) menu.print(video);//Prints menu
		}
		
		if (curUiMode == ui_levels){//If in level selection
			BKG;//Prints background
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) curUiMode = ui_mainMenu;//Back to menu on esc
					
				levelSelect.checkEvents(ev);//Checks level selector events
			}
			
			levelSelect.print(video);//Prints level selector
		}
		
		if (curUiMode == ui_game){//If in game mode
			BKG;//Prints background
			
			int printOX = video_w / 2 - (camFollow ? current.player->position.x : current.currentLevel->w / 2);//Print offset x
			int printOY = video_h / 2 - (camFollow ? current.player->position.y : current.currentLevel->h / 2);//Print offset y
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN){//If pressed a key
					if (ev.key.keysym.sym == SDLK_ESCAPE) pauseClick ({});//Pauses on esc
					else current.paused = false;//Unpause
				}
					
				//Checks hud events
				if (!camFollow) hud.checkEvents(ev, printOX + 2, printOY + 2);
				else hud.checkEvents(ev, (video->w - current.currentLevel->w) / 2 + 2, (video->h - current.currentLevel->h) / 2 + 2);
			}
			
			current.print(video, printOX, printOY);//Prints game scene
				
			updateHud();//Updates hud
			
			if (!camFollow) hud.print(video, printOX + 2, printOY + 2);//Prints hud on level corner if not following player
			else hud.print(video, (video->w - current.currentLevel->w) / 2 + 2, (video->h - current.currentLevel->h) / 2 + 2);//Else prints hud on upper-left level
			
			current.frame(0.2, keys);//Game frame
		}
		
		if (curUiMode == ui_paused){//If paused
			BKG;//Prints background
			
			int printOX = video_w / 2 - (camFollow ? current.player->position.x : current.currentLevel->w / 2);//Print offset x
			int printOY = video_h / 2 - (camFollow ? current.player->position.y : current.currentLevel->h / 2);//Print offset y
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN){//If pressed a key
					if (ev.key.keysym.sym == SDLK_ESCAPE) backClick({});//Returns to level selector on esc
					else if (ev.key.keysym.sym == SDLK_RETURN) resumeClick({});//Resumes on enter
				}
					
				pauseWindow.checkEvents(ev);//Checks pause events
			}
			
			current.print(video, printOX, printOY);//Prints game scene
			
			if (!camFollow) hud.print(video, printOX + 2, printOY + 2);//Prints hud on level corner if not following player
			else hud.print(video, (video->w - current.currentLevel->w) / 2 + 2, (video->h - current.currentLevel->h) / 2 + 2);//Else prints hud on upper-left level				
			
			pauseWindow.print(video);//Prints pause screen
		}
		
		if (curUiMode == ui_success){//If completed level
			BKG;//Prints background
			
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
			
			current.print(video, printOX, printOY);//Prints game scene
			
			if (!camFollow) hud.print(video, printOX + 2, printOY + 2);//Prints hud on level corner if not following player
			else hud.print(video, (video->w - current.currentLevel->w) / 2 + 2, (video->h - current.currentLevel->h) / 2 + 2);//Else prints hud on upper-left level
			
			success.print(video);//Prints success screen
		}

		if (curUiMode == ui_settings){//If in settings view
			BKG;//Prints background
			
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

		if (curUiMode == ui_credits){//If in credits view
			BKG;//Prints background
			
			while (SDL_PollEvent(&ev)){//While there are events on stack
				EVENTS_COMMON(ev);//Common events
				
				if (ev.type == SDL_KEYDOWN || ev.type == SDL_MOUSEBUTTONUP) curUiMode = ui_mainMenu;//Back to main menu on key press
			}
			
			credits.print(video);//Prints credits
		}
		
		if (!debugMode){//If not in debug mode
			if (achBegin > 0 && SDL_GetTicks() - achBegin < achDuration){//If has to print achievement
				if (achFrame->area.x < achFrame->area.y) achFrame->area.x = achFrame->area.y;//Moves window
				if (achFrame->area.x > achFrame->area.y) achFrame->area.x = achFrame->area.y;//Adjusts position
				
				achieved.print(video);//Prints achieved window
			}
			
			else if (achBegin > 0 && achFrame->area.x > achStartX){//If has to hide achievement
				achFrame->area.x = achStartX;//Moves window
				if (achFrame->area.x < achStartX) achFrame->area.x = achStartX;//Adjusts position
				
				achieved.print(video);//Prints window
			}
			
			else if (achBegin > 0) achBegin = -1;//Disables achievement
			
			progress.verifyAchs();//Verifies achievements
		}
		
		updateCommon();//Updates common UI
		common.print(video);//Prints common UI
		
		UPDATE;//Updates
		FRAME_END;//Frame end
	}
	
	gameQuit();//Quits
	
	return 0;//Returns 0
}