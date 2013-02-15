//Physica
//Level pack editor

string lpEditorFile = "data/cfg/ui/lpackEditor.cfg";

window lpEditor;//Level pack editor window

panel* lpProperties;//Properties panel
inputBox *lpIdField, *lpNameField;//Fields
control *lpIcon, *lpIconEdit;//Icon and edit icon controls
control *lpRulesEdit;//Rules editor

panel *lpLevPanel;//Levels panel
panel lpLevItem;//Level item panel
control *lpLevAdd;//Add level button
int lpLevSpacing = 3;//Levels spacing

panel *lpAchPanel;//Achievements panel
panel lpAchItem;//Achievement item
control *lpAchAdd;//Add achievement button
int lpAchSpacing = 3;//Achievements spacing

control *lpBtnBack, *lpBtnNew, *lpBtnSave, *lpBtnOpen;//Buttons

bool lpEditing = false;//Editing flag

bool forceLevRedraw = false;//If true, redraws level
bool forceAchRedraw = false;//If true, redraws achievements

//Function to redraw level panel
void lpLevelRedraw(){
	list<control*>::iterator i;//Iterator
	
	for (i = lpLevPanel->children.begin(); i != lpLevPanel->children.end(); i++){//For each child control
		if ((*i)->id != "scroll" && (*i)->id != "title" && (*i)->id != "add"){//If control has to be removed
			control* c = *i;//Stores control
			i = lpLevPanel->children.erase(i);//Removes from panel
			i--;//Goes back
			
			delete c;//Frees control
		}
	}
		
	levelSet::iterator l;//Iterator
	int n = 0;//Counter
	
	for (l = lpEdited.begin(); l != lpEdited.end(); l++, n++){//For each level
		panel* item = lpLevItem.copy();//New panel
		
		item->id = toString(n);//Sets item id
		item->area.y += (item->area.h + lpLevSpacing) * n;//Sets item y
		
		inputBox* fileField = (inputBox*) item->getControl("fileField");//File field
		fileField->content.t = *l;//Sets text
		
		lpLevPanel->children.push_back(item);//Adds to children
	}
}

//Function to redraw achievements panel
void lpAchRedraw(){
	list<control*>::iterator i;//Iterator
	
	for (i = lpAchPanel->children.begin(); i != lpAchPanel->children.end(); i++){//For each child control
		if ((*i)->id != "scroll" && (*i)->id != "title" && (*i)->id != "add"){//If control has to be removed
			control* c = *i;//Stores control
			i = lpAchPanel->children.erase(i);//Removes from panel
			i--;//Goes back
			
			delete c;//Frees control
		}
	}
	
	deque<achievement>::iterator a;//Achievement iterator
	int n = 0;//Counter
	
	for (a = lpEdited.lsAchs.begin(); a != lpEdited.lsAchs.end(); a++, n++){//For each achievement
		panel* item = lpAchItem.copy();//New panel
		
		item->id = toString(n);//Sets item id
		item->area.y += (item->area.h + lpAchSpacing) * n;//Sets item y
		
		control* idField = item->getControl("idField");//Id field
		idField->content.t = a->id;//Sets text
		
		lpAchPanel->children.push_back(item);//Adds to children
	}
}

//Function to update properties
void lpUpdateProp(){
	lpIdField->content.t = lpEdited.id;//Sets id
	lpNameField->content.t = lpEdited.name;//Sets name
	
	lpIcon->content.i = lpEdited.icon;//Sets icon
}

//Function to handle back click
void lpBackClick(clickEventData data){
	lpEditing = false;//Not editing anymore
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle add level click
void lpAddLevClick(clickEventData data){
	lpEdited.push_back("");//Adds level
	forceLevRedraw = true;//Forces redraw
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle add ach click
void lpAddAchClick(clickEventData data){
	achievement * a = achDialog.show(video, NULL);//Gets achievement
	
	if (a){//If achievement was valid
		lpEdited.lsAchs.push_back(*a);//Adds achievement
		forceAchRedraw = true;//Forces redraw
	}
	
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle level text changing
void lpLevTextChange (editEventData data){
	lpEdited[atoi(data.caller->parent->id.c_str())] = data.text;//Sets level id
}

//Function to handle delete click button
void lpLevDeleteClick(clickEventData data){
	lpEdited.erase(lpEdited.begin() + atoi(data.caller->parent->id.c_str()));//Erases level
	forceLevRedraw = true;//Forces level redraw
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle delete click button
void lpAchDeleteClick(clickEventData data){
	lpEdited.lsAchs.erase(lpEdited.lsAchs.begin() + atoi(data.caller->parent->id.c_str()));//Erases achievement
	forceAchRedraw = true;//Forces achievements redraw
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle edit click
void lpLevEditClick(clickEventData data){
	panel *p = (panel*) data.caller->parent;//Parent
	inputBox *fileField = (inputBox*) p->getControl("fileField");//File field
	
	if (fileField->content.t == "") return;//Returns if field is empty
	
	level *l = loadLevel(fileField->content.t);//Loads level
	
	if (!ifstream(fileField->content.t.c_str())){//If file doesn't exist
		if (msgBox.show(video, "Level doesn't exist. Create it?", 2, msgBox_ans_yn) == 1) return;//Exits if msgbox ans is no
		
		ofstream o (fileField->content.t.c_str());//Creates new file
		if (!o.good()){//If file couldn't be opened
			msgBox.show(video, "Failed creating level: invalid path (probably unexisting directory)", 1, msgBox_ans_ok);//Message
			return;//Quits
		}
		
		else {//Else
			l = loadLevel(templateFile);//Loads template
			l->path = fileField->content.t;//Sets path
			
			o << edited.toScriptObj().toString();//Saves level
			o.close();//Closes file
		}
		
	}
	
	openLevel(l);
	editing = true;//Sets editing flag
	editorLoop();//Runs editor
	
	springMode = false;
	edSpring->checked = false;
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle edit click
void lpAchEditClick(clickEventData data){
	int n = atoi(data.caller->parent->id.c_str());//Ach index
	achievement* a = achDialog.show(video, &lpEdited.lsAchs[n]);//Edits
	
	if (a){//If edited successfully
		lpEdited.lsAchs[n] = *a;//Sets achievement
		forceAchRedraw = true;//Forces redraw
	}
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle up click
void lpLevUpClick(clickEventData data){
	int lev = atoi(data.caller->parent->id.c_str());//Level index
	
	if (lev > 0){//If level is not first
		swap(lpEdited[lev], lpEdited[lev - 1]);//Swaps with previous
	}
	
	forceLevRedraw = true;
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle down click
void lpLevDownClick(clickEventData data){
	int lev = atoi(data.caller->parent->id.c_str());//Level index
	
	if (lev < lpEdited.size() - 1){//If level is not last
		swap(lpEdited[lev], lpEdited[lev + 1]);//Swaps with next
	}
	
	forceLevRedraw = true;
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle the open click
void lpOpenClick(clickEventData data){
	string lpFile = inputBoxDialog.show(video, "Enter level pack file:");//Gets file
	levelSet* l = get_ptr <levelSet> (&levelSets, lpFile);//Opens level pack according to id
	
	if (l)//If found
		lpFile = l->path;//Sets path
	
	l = levelSetFromFile(lpFile);//Loads level set
	
	if (l){//If level was found
		lpEdited = *l;//Opens pack
		lpUpdateProp();//Updates properties
		lpLevelRedraw();//Redraws level
		lpAchRedraw();//Redraws achievements
	}
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle the save click
void lpSaveClick(clickEventData data){
	if (lpIdField->content.t == ""){//If empty id field
		msgBox.show(video, "Id field required", 1, msgBox_ans_ok);//Message function
		return;//Returns
	}
	
	string lpFile = inputBoxDialog.show(video, "Enter level pack file", lpEdited.path);//Gets file
	
	//Sets metadata
	lpEdited.id = lpIdField->content.t;
	lpEdited.name = lpNameField->content.t;
	
	if (lpFile != ""){//If path is valid
		if (levelSetsFiles.find(lpFile) == levelSetsFiles.npos) installSet(lpFile);//Installs set if not installed
		
		lpEdited.path = lpFile;//Sets path
		
		ofstream o (lpFile.c_str());//Opens file
		o << lpEdited.toScriptObj().toString();//Outputs data
		o.close();//Closes
	}
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle the new click
void lpNewClick(clickEventData data){
	lpEdited = *new levelSet;//Reloads level set
	
	lpUpdateProp();//Updates properties
	lpLevelRedraw();//Redraws level
	lpAchRedraw();//Redraws achievements
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle icon edit button click
void lpIconEditClick(clickEventData data){
	image *i = imgInput.show(video, &lpEdited.icon);//Edits icon
	
	if (i){//If image was valid
		lpEdited.icon = *i;//Sets icon
		lpIcon->content.i = lpEdited.icon;//Sets icon
	}
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to handle rules edit button click
void lpRulesEditClick(clickEventData data){
	rules *r = rulesDialog.show(video, &lpEdited.lsRules);//Edits rules
	
	if (r){//If rules were valid
		lpEdited.lsRules = *r;//Sets rules
	}
	PLAYSOUND(clickSfx);//Plays click sound
}

//Function to load level pack editor
void loadLpEditor(){
	lpEditor = loadWindow(lpEditorFile, "lpEditor");//Loads window
	
	//Gets controls
	lpProperties = (panel*) lpEditor.getControl("properties");
	lpIdField = (inputBox*) lpEditor.getControl("properties.idField");
	lpNameField = (inputBox*) lpEditor.getControl("properties.nameField");
	lpIcon = lpEditor.getControl("properties.icon");
	lpIconEdit = lpEditor.getControl("properties.iconEdit");
	lpRulesEdit = lpEditor.getControl("properties.rulesEdit");
	
	lpLevPanel = (panel*) lpEditor.getControl("levels");
	
	panel* p = (panel*) lpEditor.getControl("levels.levelItem");
	lpLevItem = *p;
	lpLevPanel->children.remove(p);
	
	lpLevAdd = lpEditor.getControl("levels.add");
	
	lpAchPanel = (panel*) lpEditor.getControl("achievements");
	
	p = (panel*) lpEditor.getControl("achievements.achItem");
	lpAchItem = *p;
	lpAchPanel->children.remove(p);
	
	lpAchAdd = lpEditor.getControl("achievements.add");
	
	lpBtnBack = lpEditor.getControl("back");
	lpBtnNew = lpEditor.getControl("new");
	lpBtnSave = lpEditor.getControl("save");
	lpBtnOpen = lpEditor.getControl("open");
	
	//Sets handlers
	lpBtnBack->release.handlers.push_back(lpBackClick);
	lpBtnOpen->release.handlers.push_back(lpOpenClick);
	lpBtnSave->release.handlers.push_back(lpSaveClick);
	lpBtnNew->release.handlers.push_back(lpNewClick);
	lpLevAdd->release.handlers.push_back(lpAddLevClick);
	lpAchAdd->release.handlers.push_back(lpAddAchClick);
	lpIconEdit->release.handlers.push_back(lpIconEditClick);
	lpRulesEdit->release.handlers.push_back(lpRulesEditClick);
	
	lpIcon->content.contentType = CONTENT_IMAGE;//Sets content type for icon
	
	//Sets level item and ach item events
	inputBox* lpLevFileField = (inputBox*) lpLevItem.getControl("fileField");
	control* lpLevDelete = lpLevItem.getControl("delete");
	control* lpLevEdit = lpLevItem.getControl("edit");
	control* lpLevUp = lpLevItem.getControl("moveUp");
	control* lpLevDown = lpLevItem.getControl("moveDown");
	
	control* lpAchDelete = lpAchItem.getControl("delete");
	control* lpAchEdit = lpAchItem.getControl("edit");
	
	lpLevFileField->editEvent.handlers.push_back(lpLevTextChange);
	lpLevDelete->release.handlers.push_back(lpLevDeleteClick);
	lpLevEdit->release.handlers.push_back(lpLevEditClick);
	lpLevUp->release.handlers.push_back(lpLevUpClick);
	lpLevDown->release.handlers.push_back(lpLevDownClick);
	
	lpAchDelete->release.handlers.push_back(lpAchDeleteClick);
	lpAchEdit->release.handlers.push_back(lpAchEditClick);
}

//Function to show editor
void lpEditorLoop(){
	lpEditing = true;//Sets editing flag
	
	while (running && lpEditing){//While running program and still editing level pack
		FRAME_BEGIN;//Begins frame
		
		while (SDL_PollEvent(&ev)){//While there are events on stack
			EVENTS_COMMON(ev);//Common events
			
			lpEditor.checkEvents(ev);//Checks editor events
		}
		
		BKG;//Prints background
		lpEditor.print(video);//Prints editor
		
		updateCommon();//Updates common ui
		common.print(video);//Prints common ui
		
		UPDATE;//Updates
		
		if (forceLevRedraw){//If has to force redraw
			lpLevelRedraw();//Redraws levels
			forceLevRedraw = false;//Unflags
		}
		
		if (forceAchRedraw){//If has to force redraw
			lpAchRedraw();//Redraws achievements
			forceAchRedraw = false;//Unflags
		}
		
		FRAME_END;
	}
	
	levelSets.clear();//Clears loaded sets
	loadSets();//Reloads s
}