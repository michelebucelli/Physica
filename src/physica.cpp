#include "physica.h"//Includes game main header

game g;//Game

//Main function
int main(int argc, char* argv[]){
	gameInit();//Initializes the game
	
	g.loadLevelSet("data/cfg/levelSets/levelSet_core.cfg");//Loads core level set
	g.setup();//Sets up level
		
	cout << "Setup in " << SDL_GetTicks() << " msec" << endl;
	
	while (running){//While game is running
		FRAME_BEGIN;//Frame beginning
		
		while (SDL_PollEvent(&ev)){//While there are stacked events
			if (ev.type == SDL_QUIT)//If event type was quit
				running = false;//Stops execution
				
			if (ev.type == SDL_KEYDOWN && ev.key.keysym.sym == SDLK_ESCAPE)//If pressed esc
				running = false;//Stops execution
		}
		
		g.frame(0.2, keys);//Game frame
		if (frames % (fps / printFps) == 0){//On printing frame
			BKG;//Prints background
			g.print(video, video_w / 2 - (*g.currentLevel)->w / 2, video_h / 2 - (*g.currentLevel)->h / 2);
		}
		
		if (frames % (fps / animFps) == 0) g.animStep();//Steps animation

		UPDATE;//Updates
		
		FRAME_END;//Frame end
	}
}