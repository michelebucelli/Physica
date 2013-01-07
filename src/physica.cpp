#include "physica.h"//Includes game main header

//Main function
int main(int argc, char* argv[]){
	gameInit();//Initializes the game
	
	current.loadLevelSet("data/cfg/levelSets/levelSet_core.cfg");//Loads core level set
	current.setup();//Sets up level
		
	cout << "Setup in " << SDL_GetTicks() << " msec" << endl;
	
	while (running){//While game is running
		FRAME_BEGIN;//Frame beginning
		
		int printOX = video_w / 2 - (*current.currentLevel)->w / 2;//Print offset x
		int printOY = video_h / 2 - (*current.currentLevel)->h / 2;//Print offset y
		
		while (SDL_PollEvent(&ev)){//While there are stacked events
			if (ev.type == SDL_QUIT)//If event type was quit
				running = false;//Stops execution
				
			if (ev.type == SDL_KEYDOWN){//If pressed a key
				if (ev.key.keysym.sym == SDLK_ESCAPE) running = false;//Stops execution on esc pression
				current.paused = false;//Unpause the game
			}
				
			hud.checkEvents(ev, printOX, printOY);//Checks hud events
		}
		
		current.frame(0.2, keys);//Game frame
		if (frames % (fps / printFps) == 0){//On printing frame
			BKG;//Prints background
			
			current.print(video, printOX, printOY);//Prints game scene
			
			updateHud();//Updates hud
			hud.print(video, printOX, printOY);//Prints hud
		}
		
		if (frames % (fps / animFps) == 0) current.animStep();//Steps animation

		UPDATE;//Updates
		
		FRAME_END;//Frame end
	}
}