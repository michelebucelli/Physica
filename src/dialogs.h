//Physica - dialogs

//Message box
class msgbox: public dialog {
	public:
	panel answerButton;//Answer button
	control* text;//Text control
	
	int ansSpacing;//Answer spacing
	int margin;//Margin
	
	//Constructor
	msgbox() {
		dialogFrame = NULL;
		
		frameBegin = NULL;
		frameEnd = NULL;
		events = NULL;
		quitFlag = NULL;
		
		text = NULL;
		
		ansSpacing = 4;
		margin = 16;
	};
	
	//Load function
	void loadDialog(string path){
		dialog::loadDialog(path);
		
		panel* p = (panel*) dialogWindow.getControl("frame.answer");//Gets answer
		answerButton = *p;//Sets answer button
		
		clearAns();//Removes unused buttons
		
		text = dialogWindow.getControl("frame.text");//Gets text
	}
	
	//Function to clear answer buttons
	void clearAns(){
		list<control*>::iterator i;
		
		for (i = dialogFrame->children.begin(); i != dialogFrame->children.end(); i++){//For each child
			if ((*i)->id != "text"){//If not text
				delete *i;//Deletes
				i = dialogFrame->children.erase(i);//Erases from list
				i--;//Goes back
			}
		}
	}
	
	//Show function
	int show(SDL_Surface* target, string prompt, int ansc, string ansv[], bool kp = false){
		text->content.t = prompt;//Sets text
		
		SDL_Surface* old = SDL_CreateRGBSurface(SDL_SWSURFACE, target->w, target->h, 32, 0, 0, 0, 0);//Surface to store target
		SDL_BlitSurface(target, NULL, old, NULL);//Blits target on surface
		
		int n;//Counter
		int w = ansc * answerButton.area.w + (ansc - 1) * ansSpacing;//Total width
		
		if (dialogFrame->area.w < w + 2 * margin){//If message is too small
			dialogFrame->area.w = w + 2 * margin;//Sets frame width
			answerButton.area.x = margin;//Sets first button x coord
		}
		
		else {//Else
			answerButton.area.x = (dialogFrame->area.w - answerButton.area.w) / 2 - (ansc % 2 ? 0 : answerButton.area.w / 2);//Centers answers
		}
		
		centre(target->w, target->h);//Centers on target
	
		clearAns();//Clears answers
	
		for (n = 0; n < ansc; n++){//For each answer
			panel* p = new panel (answerButton);//New answer button
			
			p->area.x += n * p->area.w + ansSpacing;//Sets x
			p->content.t = ansv[n];//Sets content
			p->id = toString(n);//Sets id
			
			dialogFrame->children.push_back(p);//Adds to frame
		}
		
		SDL_Event e;//Event
		
		while (!quitFlag || *quitFlag){//While running
			if (frameBegin) frameBegin();//Frame beginnig
			
			while (SDL_PollEvent(&e)){//While there are events on stack
				if (events) events(e);//Checks events
				
				dialogWindow.checkEvents(e);//Checks dialog events
				
				list<control*>::iterator i;//Control iterator
				for (i = dialogFrame->children.begin(); i != dialogFrame->children.end(); i++)//For each control
					if ((*i)->id != "text" && (*i)->status == control::pressed) return atoi((*i)->id.c_str());//Returns index if pressed
					
				if (e.type == SDL_KEYDOWN && kp) return -1;//Returns -1 on key press
			}
			
			SDL_BlitSurface(old, NULL, target, NULL);//Prints target
			dialogWindow.print(target);//Prints dialog
			SDL_Flip(target);//Updates target
			
			if (frameEnd) frameEnd();//Frame ending
		}
		
		return -1;//Returns -1 if didn't return any answer
	}
} msgBox;

string msgBox_ans_ok[] = { "OK" };//OK msgbox answer
string msgBox_ans_okCancel[] = { "OK", "Cancel"};//OK/Cancel msgbox answer
string msgBox_ans_yn[] = { "Yes", "No"};//Yes/No msgbox answer

//Input box
class inputbox: public dialog {
	public:
	control* text;//Text control
	inputBox* field;//Field control

	//Constructor
	inputbox(){
		dialogFrame = NULL;
		
		frameBegin = NULL;
		frameEnd = NULL;
		events = NULL;
		quitFlag = NULL;
		
		text = NULL;
		field = NULL;
	}
	
	//Load function
	void loadDialog(string path){
		dialog::loadDialog(path);//Loads base dialog
		
		text = dialogWindow.getControl("frame.text");//Gets dialog
		field = (inputBox*) dialogWindow.getControl("frame.field");//Gets field
	}
	
	//Show function
	string show(SDL_Surface* target, string prompt){
		text->content.t = prompt;//Sets text
		
		SDL_Surface* old = SDL_CreateRGBSurface(SDL_SWSURFACE, target->w, target->h, 32, 0, 0, 0, 0);//Surface to store target
		SDL_BlitSurface(target, NULL, old, NULL);//Blits target on surface
		
		SDL_Event e;//Event
		
		centre(target->w, target->h);//Centres
		
		while (!quitFlag || *quitFlag){//While running
			if (frameBegin) frameBegin();//Frame beginnig
			
			while (SDL_PollEvent(&e)){//While there are events on stack
				if (events) events(e);//Checks events
				
				dialogWindow.checkEvents(e);//Checks dialog events
				
				if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_RETURN) return field->content.t;//Returns input field on enter press
				else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) return "";//Returns empty on esc
			}
			
			SDL_BlitSurface(old, NULL, target, NULL);//Prints target
			dialogWindow.print(target);//Prints dialog
			SDL_Flip(target);//Updates target
			
			if (frameEnd) frameEnd();//Frame ending
		}
		
		return "";//Returns "" if didn't return any answer
	}
} inputBoxDialog;

//Image dialog
class imgpreview: public dialog {
	public:
	int margin;//Margin
	
	//Constructor
	imgpreview(){
		dialogFrame = NULL;
		
		frameBegin = NULL;
		frameEnd = NULL;
		events = NULL;
		quitFlag = NULL;
		
		margin = 16;
	}
	
	//Load function
	void loadDialog(string path){
		dialog::loadDialog(path);//Loads normally
		dialogFrame->content.contentType = CONTENT_IMAGE;//Sets content type
	}
	
	//Show function
	void show(SDL_Surface* target, image i){
		if (!i.valid()) return;//Returns if image isn't valid
		
		dialogFrame->content.i = i;//Sets image
		dialogFrame->area.w = i.w() + 2 * margin;//Sets width
		dialogFrame->area.h = i.h() + 2 * margin;//Sets height
		
		centre(target->w, target->h);//Centers
		
		SDL_Surface* old = SDL_CreateRGBSurface(SDL_SWSURFACE, target->w, target->h, 32, 0, 0, 0, 0);//Surface to store target
		SDL_BlitSurface(target, NULL, old, NULL);//Blits target on surface
		
		SDL_Event e;//Event
		
		while (!quitFlag || *quitFlag){//While running
			if (frameBegin) frameBegin();//Frame beginnig
			
			while (SDL_PollEvent(&e)){//While there are events on stack
				if (events) events(e);//Checks events
				
				dialogWindow.checkEvents(e);//Checks dialog events
				
				if (e.type == SDL_KEYDOWN) return;//Returns on key press
			}
			
			SDL_BlitSurface(old, NULL, target, NULL);//Prints target
			dialogWindow.print(target);//Prints dialog
			SDL_Flip(target);//Updates target
			
			if (frameEnd) frameEnd();//Frame ending
		}
	}
} imgPreview;

//Image input
class imginput: public dialog {
	public:
	inputBox *idField, *pathField, *rectX, *rectY, *rectW, *rectH;//Input boxes
	control *ok, *preview, *cancel;//Buttons
	
	//Constructor
	imginput(){
		dialogFrame = NULL;
		
		frameBegin = NULL;
		frameEnd = NULL;
		events = NULL;
		quitFlag = NULL;
		
		idField = NULL;
		pathField = NULL;
		rectX = NULL;
		rectY = NULL;
		rectW = NULL;
		
		ok = NULL;
		preview = NULL;
		cancel = NULL;
	}
	
	//Load function
	void loadDialog(string path){
		dialog::loadDialog(path);//Loads dialog
		
		//Gets controls
		idField = (inputBox*) dialogWindow.getControl("frame.idField");
		pathField = (inputBox*) dialogWindow.getControl("frame.pathField");
		rectX = (inputBox*) dialogWindow.getControl("frame.rectX");
		rectY = (inputBox*) dialogWindow.getControl("frame.rectY");
		rectW = (inputBox*) dialogWindow.getControl("frame.rectW");
		rectH = (inputBox*) dialogWindow.getControl("frame.rectH");
		
		ok = dialogWindow.getControl("frame.ok");
		preview = dialogWindow.getControl("frame.preview");
		cancel = dialogWindow.getControl("frame.cancel");
	}
	
	//Show function
	image *show(SDL_Surface* target, image *i){
		if (i && i->valid()){//If image's valid
			pathField->content.t = i->imageFilePath;
			rectX->content.t = toString(i->rect.x);
			rectY->content.t = toString(i->rect.y);
			rectW->content.t = toString(i->rect.w);
			rectH->content.t = toString(i->rect.h);
		}
		
		centre(target->w, target->h);//Centers
		
		SDL_Surface* old = SDL_CreateRGBSurface(SDL_SWSURFACE, target->w, target->h, 32, 0, 0, 0, 0);//Surface to store target
		SDL_BlitSurface(target, NULL, old, NULL);//Blits target on surface
		
		SDL_Event e;//Event
		
		while (!quitFlag || *quitFlag){//While running
			if (frameBegin) frameBegin();//Frame beginnig
			
			while (SDL_PollEvent(&e)){//While there are events on stack
				if (events) events(e);//Checks events
				
				dialogWindow.checkEvents(e);//Checks dialog events
				
				if (ok->status == control::pressed)//If pressed ok
					return new image(
									idField->content.t,
									pathField->content.t,
									atoi(rectX->content.t.c_str()),
									atoi(rectY->content.t.c_str()),
									atoi(rectW->content.t.c_str()),
									atoi(rectH->content.t.c_str()));//Result
										
				if (preview->status == control::pressed){//If pressed preview
					image i (idField->content.t,
							pathField->content.t,
							atoi(rectX->content.t.c_str()),
							atoi(rectY->content.t.c_str()),
							atoi(rectW->content.t.c_str()),
							atoi(rectH->content.t.c_str()));//Image
							
					imgPreview.show(target, i);//Shows image
				}
										
				if (cancel->status == control::pressed)//If pressed cancel
					return NULL;//Returns null
			}
			
			SDL_BlitSurface(old, NULL, target, NULL);//Prints target
			dialogWindow.print(target);//Prints dialog
			SDL_Flip(target);//Updates target
			
			if (frameEnd) frameEnd();//Frame ending
		}
	}
} imgInput;

//Achievement dialog
class achdialog: public dialog {
	public:
	control *icon;//Achievement icon
	control *iconEdit;//Edit icon
	
	inputBox *idField, *nameField, *infoField, *verifyField;//Fields
	checkBox *checkOnce;//Check once flag
	
	control *ok, *cancel;//Ok and cancel buttons
	
	//Constructor
	achdialog(){
		dialogFrame = NULL;
		
		frameBegin = NULL;
		frameEnd = NULL;
		events = NULL;
		quitFlag = NULL;
		
		icon = NULL;
		iconEdit = NULL;
		idField = NULL;
		nameField = NULL;
		infoField = NULL;
		verifyField = NULL;
		checkOnce = NULL;
		
		ok = NULL;
		cancel = NULL;
	}
	
	//Load function
	void loadDialog(string path){
		dialog::loadDialog(path);//Loads normally
		
		//Gets controls
		icon = dialogWindow.getControl("frame.icon");
		iconEdit = dialogWindow.getControl("frame.iconEdit");
		idField = (inputBox*) dialogWindow.getControl("frame.idField");
		nameField = (inputBox*) dialogWindow.getControl("frame.nameField");
		infoField = (inputBox*) dialogWindow.getControl("frame.infoField");
		verifyField = (inputBox*) dialogWindow.getControl("frame.verifyField");
		checkOnce = (checkBox*) dialogWindow.getControl("frame.checkOnce");
		ok = dialogWindow.getControl("frame.ok");
		cancel = dialogWindow.getControl("frame.cancel");
		
		icon->content.contentType = CONTENT_IMAGE;//Sets image content type for icon
	}
	
	//Show function
	achievement* show(SDL_Surface* target, achievement *a = NULL){
		if (a){
			icon->content.i = a->icon;//Sets icon
			idField->content.t = a->id;//Sets id
			nameField->content.t = a->name;//Sets name
			infoField->content.t = a->info;//Sets info
			verifyField->content.t = a->verifyExpr.exprToString();//Sets expression
		}
		
		centre(target->w, target->h);//Centers on screen
		
		SDL_Surface* old = SDL_CreateRGBSurface(SDL_SWSURFACE, target->w, target->h, 32, 0, 0, 0, 0);//Surface to store target
		SDL_BlitSurface(target, NULL, old, NULL);//Blits target on surface
		
		SDL_Event e;//Event
		
		while (!quitFlag || *quitFlag){//While running
			if (frameBegin) frameBegin();//Frame beginnig
			
			while (SDL_PollEvent(&e)){//While there are events on stack
				if (events) events(e);//Checks events
				
				dialogWindow.checkEvents(e);//Checks dialog events
				
				if (ok->status == control::pressed)//If pressed ok
					return new achievement(
									idField->content.t,
									nameField->content.t,
									infoField->content.t,
									verifyField->content.t,
									icon->content.i,
									checkOnce->checked);//Result
										
				if (iconEdit->status == control::pressed || icon->status == control::pressed){//If pressed icon
					image* i = imgInput.show(target, &icon->content.i);//Edits image
					
					if (i) icon->content.i = *i;//Sets icon
				}
										
				if (cancel->status == control::pressed)//If pressed cancel
					return NULL;//Returns null
			}
			
			SDL_BlitSurface(old, NULL, target, NULL);//Prints target
			dialogWindow.print(target);//Prints dialog
			SDL_Flip(target);//Updates target
			
			if (frameEnd) frameEnd();//Frame ending
		}
	}
} achDialog;