#include "physica.h"//Includes game main header

//Main function
int main(int argc, char* argv[]){
	#ifdef __WIN32__//On win32
	win32_init(argc, argv);//Inits
	#else//On other systems
	gameInit(argc, argv);//Initializes the game
	#endif
	
	while (running){//While game is running
		FRAME_BEGIN;//Frame beginning
		
		if (curUiMode == ui_mainMenu){//If in main menu
			hideCursor = false;//Sets hide cursor flag
			
			BKG;//Prints background
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events					
				menu.checkEvents(ev);//Checks menu events
			}
		
			if (running) menu.print(video);//Prints menu
		}
		
		if (curUiMode == ui_levelSets){//If in level set selection
			hideCursor = false;//Sets hide cursor flag
			
			BKG;//Prints background
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) curUiMode = ui_mainMenu;//Back to menu on esc
					
				levelSetSel.checkEvents(ev);//Checks level set selector events
			}
			
			levelSetSel.print(video);//Prints level set selector
		}
		
		if (curUiMode == ui_paused){//If paused
			hideCursor = false;//Sets hide cursor flag
			
			BKG;//Prints background
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN){//If pressed a key
					if (ev.key.keysym.sym == SDLK_ESCAPE) backClick({});//Returns to level selector on esc
					else if (ev.key.keysym.sym == SDLK_RETURN) resumeClick({});//Resumes on enter
				}
					
				pauseWindow.checkEvents(ev);//Checks pause events
			}
			
			current.print(video, video_w / 2 - cam.position.x, video_h / 2 - cam.position.y);//Prints game scene
			
			hud.print(video, 2, 2);//Prints hud on upper-left level
			
			pauseWindow.print(video);//Prints pause screen
		}
		
		if (curUiMode == ui_game){//If in game mode
			hideCursor = true;//Sets hide cursor flag
			
			BKG;//Prints background
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN){//If pressed a key
					if (ev.key.keysym.sym == SDLK_ESCAPE) pauseClick ({});//Pauses on esc
					else current.paused = false;//Unpause
				}
					
				//Checks hud events
				hud.checkEvents(ev, 2, 2);
			}
			
			current.print(video, video_w / 2 - cam.position.x, video_h / 2 - cam.position.y);//Prints game scene
				
			updateHud();//Updates hud
			hud.print(video, 2, 2);//Prints hud on upper-left level
			current.frame(double((frameBegin - lastFrameBegin)) * 0.0125, keys);//Game frame
			cam.move(double((frameBegin - lastFrameBegin)) * 0.0125);//Moves camera
		}
		
		if (curUiMode == ui_levels){//If in level selection
			hideCursor = false;//Sets hide cursor flag
			
			BKG;//Prints background
			
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE){ curUiMode = ui_levelSets; redrawLevelSetSel(); }//Back to level sets on esc
					
				levelSelect.checkEvents(ev);//Checks level selector events
			}
			
			if (curUiMode == ui_levels) levelSelect.print(video);//Prints level selector
			
			window::iterator i;//Iterator
			bool found = false;//Found flag
			
			for (i = levelSelect.begin(); i != levelSelect.end(); i++){//For each control
				if ((*i)->status == control::hover){//If control is under mouse
					int n = atoi((*i)->id.c_str());//Gets index
					
					if (tooltipN != n){//If selected new level
						tooltipN = n;//Sets level index
						
						string file = current.levels[n];//Gets file
						tooltipId = loadLevel(file)->id;//Gets id
						
						tooltipBegin = frameBegin;//Sets tooltip begin time
					}
					
					levelProgress* p = get <levelProgress> (&progress, current.levels.id + "." + tooltipId);//Requested progress
					
					if (p && p->unlocked && p->time > 0){//If progress is available
						levttTime->content.t = timeToString(p->time);//Sets time
						levttDeaths->content.t = (current.deaths < 10 ? "0" : "") + toString(p->deaths);//Sets deaths
						
						if (frameBegin - tooltipBegin > tooltipDelay) levTooltip.print(video, (*i)->area.x + ((*i)->area.w - levttFrame->area.w) / 2, (*i)->area.y - levttFrame->area.h - 10);//Prints tooltip
					}
					
					found = true;//Found level under mouse
				}
			}
			
			if (!found){//If not hovering
				tooltipN = -1;
				tooltipBegin = -1;
				tooltipId = "";
			}
		}
		
		if (curUiMode == ui_success){//If completed level
			hideCursor = false;//Sets hide cursor flag
			
			BKG;//Prints background
		
			while (SDL_PollEvent(&ev)){//While there are stacked events
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN){//If pressed a key
					if (ev.key.keysym.sym == SDLK_ESCAPE) backClick({});//Goes back on esc
					else if (ev.key.keysym.sym == SDLK_RETURN) nextClick({});//Goes next on enter
				}
				
				success.checkEvents(ev);//Checks success window events
			}
			
			if (curUiMode == ui_success){//If still in success mode
				current.print(video, video_w / 2 - cam.position.x, video_h / 2 - cam.position.y);//Prints game scene
				hud.print(video, 2, 2);//Prints hud on upper-left level
				success.print(video);//Prints success screen
			}
		}
		
		if (curUiMode == ui_settings){//If in settings view
			hideCursor = false;//Sets hide cursor flag
			
			while (SDL_PollEvent(&ev)){//While there are events on stack
				EVENTS_COMMON(ev);//Common events
					
				if (ev.type == SDL_KEYDOWN){//If pressed a key
					if (ev.key.keysym.sym == SDLK_ESCAPE) curUiMode = ui_mainMenu;//Goes back on esc
				}
				
				settings.checkEvents(ev);//Checks settings window events
				applySettings();//Applies settings changes
			}
			
			BKG;//Prints background
			settings.print(video);//Prints settings window
		}

		if (curUiMode == ui_achievements){//If in achievements view
			hideCursor = false;//Sets hide cursor flag
			
			BKG;//Prints background
			
			while (SDL_PollEvent(&ev)){//While there are events on stack
				EVENTS_COMMON(ev);//Common events
				
				if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE) curUiMode = ui_mainMenu;//Back to main menu on key press
			}
			
			int o = (video_h - achsH) / 2 - 20 - achs_oY;//Top achievement y position
			
			if (keys[SDLK_UP] && achsH > video_h && o < 0) achs_oY--;//Moves up
			if (keys[SDLK_DOWN] && achsH > video_h && o > video_h - achsH - 40) achs_oY++;//Moves down
			
			achievements.print(video, 0, -achs_oY);//Prints achievements
		}
		
		if (curUiMode == ui_credits){//If in credits view
			hideCursor = false;//Sets hide cursor flag
			
			BKG;//Prints background
			
			while (SDL_PollEvent(&ev)){//While there are events on stack
				EVENTS_COMMON(ev);//Common events
				
				if (ev.type == SDL_KEYDOWN || ev.type == SDL_MOUSEBUTTONUP) curUiMode = ui_mainMenu;//Back to main menu on key press
			}
			
			credits.print(video);//Prints credits
		}
		
		if (!debugMode){//If not in debug mode
			achFrameStep();//Steps achievement animation			
			progress.verifyAchs();//Verifies achievements
		}
		
		progress.check();//Checks broken progress info
		
		if (achieved.size() > 0) achieved.print(video);//Prints achievements
		
		updateCommon();//Updates common UI
		common.print(video);//Prints common UI
		
		UPDATE;//Updates
		FRAME_END;
	}
	
	gameQuit();//Quits
	
	return 0;//Returns 0
}