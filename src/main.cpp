#define _(STRING)	gettext(STRING)

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_thread.h>

#include "log.h"
#include "ui.h"
#include "ui_pieces.h"
#include "game.h"

bool running = true;
SDL_Event e;

int aveFPS = 0;
int tot = 0;
int frames = 1;

//Function to load settings
void loadSettings(){
	xml_document d;
	d.load_file(FILE_SETTINGS);
	xml_node n = d.child("settings");
	
	loadViewport(n);
	playerControls.load(n.child("controls"));
}

//Function to save settings
void saveSettings(){
	xml_document d;
	xml_node n = d.append_child("settings");
	
	saveViewport(n);
	playerControls.id = "controls";
	
	xml_node c = n.append_child("controls");
	playerControls.save(&c);
	
	d.save_file(FILE_SETTINGS);
}

//Function to load progress
void loadProgress(){
	xml_document d;
	d.load_file(FILE_PROGRESS);
	xml_node n = d.child("progress");
	
	progress.load(n);
}

//Function to save progress
void saveProgress(){
	xml_document d;
	xml_node n = d.append_child("progress");
	
	progress.save(&n);
	
	d.save_file(FILE_PROGRESS);
}

//Initialization
void init(){
	LOG_HEADING;
		
	folderDebug(outputLog);
	
	LOG_HR;
	LOG("Beginning basic initialization");
	
	REPORT_RES_TIME_0(SDL_Init(SDL_INIT_EVERYTHING), 	"  Initialized SDL");
	REPORT_RES_TIME  (IMG_Init(IMG_INIT_PNG), 		    "  Initialized SDL_image");
	REPORT_RES_TIME  (TTF_Init(),                       "  Initialized SDL_ttf");
	
	REPORT_TIME  (initLocale(),						"	Initialized locale");
	REPORT_TIME  (loadSettings(),                   "   Loaded settings");
	REPORT_TIME  (loadProgress(),					"   Loaded player progress");
	REPORT_TIME  (initViewport(),                   "   Initialized viewport");
	REPORT_TIME	 (loadLevelPacks(),                 "   Loaded level packs");
	REPORT_TIME  (loadAchievements(),				"   Loaded achievements");
	REPORT_TIME  (initUI(),                         "   Initialized UI");
	
	LOG_TIME_0("Done");
	LOG_HR;
	
	LOG("Loaded " << levelPacks.size() << " level packs");
	LOG("Loaded " << globalAchievements.size() << " global achievements");
	LOG_HR;
}

//Quit
void quit(){
	LOG_HR;
	LOG("Terminating program");
	LOG("Average FPS: " << aveFPS);
	
	REPORT_TIME(saveProgress(), "   Saved player progress");
	REPORT_TIME(saveSettings(), "   Saved settings");
	REPORT_TIME(SDL_Quit(), "   Quit SDL");
	
	LOG("Done");
	LOG_CLOSING_STD;
}

//JS quit function
void scQuit(CScriptVar* v, void* userdata) { running = false; }

//Function to setup js functions
void setupJS(CTinyJS* js){
	js->addNative("function quit()", scQuit, NULL);
	js->addNative("function dark()", dark, NULL);
	
	registerGameFuncs(js);
}

//Main function
int main(int argc, char** argv){
	try {		
		init();
		customSetupJS_UI = setupJS;
		
		Uint8* keys = (Uint8*) SDL_GetKeyboardState(NULL);
		
		while (running){
			int frameBegin = SDL_GetTicks();
			
			ui_menu.genScriptVar();
			ui_menu.linkScriptVar();
			
			while(SDL_PollEvent(&e)){
				if (e.type == SDL_QUIT) running = false;
					
				if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
					VPRECT(&windowRect);
					if (!fullscreen) VPRECT(&windowedRect);
				}
				
				ui_menu.handleEvents(&e, windowRect);
			}
			
			ui_menu.handleEvents(NULL, windowRect);
			ui_menu.refresh();
			
			progress.verifyAchs();
			progress.check();
			
			if (currentGame.completed && ui_levelClear->cStatus == cs_disabled){
				lcShow();
				ui_game->cStatus = cs_disabled;
			}
			
			if (ui_game->cStatus != cs_disabled && ui_game->area.y == 0) currentGame.frame(0.2, keys);
			if (ui_settingsMenu->cStatus != cs_disabled) applySettingsMenu();

			SDL_SetRenderDrawColor(mainRenderer, 0x10, 0x10, 0x10, 0xff);
			SDL_RenderClear(mainRenderer);
			
			if (ui_game->visible && ui_game->area.y < windowRect.h){
				currentGame.print(mainRenderer, windowRect.w / 2, windowRect.h / 2 + ui_game->area.y);
				updateHud();
			}
						
			ui_menu.print(mainRenderer, &windowRect);
			
			SDL_RenderPresent(mainRenderer);
			
			ui_menu.clearScriptVar();
			
			redrawUI();
			
			int fps = 1000 / (SDL_GetTicks() > frameBegin ? SDL_GetTicks() - frameBegin : 1);
			aveFPS = (tot += fps) / frames++;
		}
		
		quit();
		
		return 0;
	}
	
	catch (CScriptException* e){
		cout << e->text << endl;
		LOG_ERR("program terminated abnormally due to TinyJS exception");
		LOG_ERR(e->text);
		
		return 1;
	}
	
	catch (exception &e){
		cout << e.what() << endl;
		LOG_ERR("program terminated abnormally due to standard exception");
		LOG_ERR(e.what());
		
		return 2;
	}
}
