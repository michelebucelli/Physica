//Editor header
Uint32 editorBkg = 0x181818;//Editor background
Uint32 editorSel = 0xFFFFFF30;//Editor selected overlay

image handle;//Transformation handles

string templateFile = "data/cfg/levels/levelPresets.cfg";//Level preset file
string outputSetFile = "data/cfg/levels/levels_editor.cfg";//Editor set file
string entitiesFile = "data/cfg/editor/entities.cfg";//Entities file
deque<entity*> entities;//Loaded entities

string editorFile = "data/cfg/ui/editor.cfg";//Editor window file path
window editor;//Editor window
control *edBack, *edNew, *edSave, *edOpen, *edProp;//Editor buttons
checkBox* edSpring;//Spring mode checkbox

string propertiesFile = "data/cfg/ui/editor_properties.cfg";//Properties window file path
window properties;//Properties window
panel* propFrame;//Properties frame
control *idField, *wField, *hField, *dTrField, *dRotField, *stars2, *stars3;//Level properties fields
bool showProperties = true;//Show level properties flag

string selPropFile = "data/cfg/ui/editor_selProp.cfg";//Selected properties file path
window selProp;//Selected properties window
panel* selPropFrame;//Selected properties frame
control *selIdField, *selMassField, *selEField, *selWField, *selHField, *selColorField;//Selected properties fields
checkBox *selLockTr, *selLockRot, *selPrint;//Selected properties check boxes

bool editing = false;//Editor running flag
bool springMode = false;//True if in spring mode
spring* adding = NULL;//Spring being added
double springK = 0.5;//Spring constant

level edited;//Edited level

box* selected = NULL;//Selected entity

box* dragged = NULL;//Dragged entity
vector* draggedNode = NULL;//Dragged node
int draggedNodeIndex;//Dragged node index

vector dragInitial;//Dragging intial position
vector dragDist;//Distance between dragging point and position of dragged entity
vector dragVector;//Dragging vector

string lastSaveId = "";//Last ID when saved

//Function to update properties content
void updateProperties(){
	idField->content.t = edited.id;
	wField->content.t = toString(edited.w);
	hField->content.t = toString(edited.h);
	dTrField->content.t = toString(edited.damping_tr);
	dRotField->content.t = toString(edited.damping_rot);
	stars2->content.t = toString(edited.twoStarsTime);
	stars3->content.t = toString(edited.threeStarsTime);
}

//Back button click
void editorBackClick(clickEventData data){
	editing = false;//No more editing
	PLAYSOUND(clickSfx);
}

//New button click
void editorNewClick(clickEventData data){
	edited.entities.clear();//Clears entities
	PLAYSOUND(clickSfx);
	
	edited = *loadLevel(templateFile);//Loads template
	lastSaveId = "";
	
	updateProperties();//Updates properties panel
}

//Save button click
void editorSaveClick(clickEventData data){
	string lFile = "data/cfg/levels/" + edited.id + ".cfg";//Level file path
	bool existing = false;//If true, level is already in level set
	deque<string>::iterator i;//String iterator
	
	ofstream o (lFile.c_str());//Output level file
	o << edited.toScriptObj().toString();//Saves level
	o.close();//Closes file
	
	levelSet *editorSet = get_ptr <levelSet> (&levelSets, "levels_editor");//Editor level seti
	
	if (!editorSet) return;//Fails if editor set doesn't exist
	
	if (lastSaveId != edited.id){//If id was changed
		remove(("data/cfg/levels/" + lastSaveId + ".cfg").c_str());//Deletes old file
		
		for (i = editorSet->begin(); i != editorSet->end(); i++){//For each level
			if (*i == "data/cfg/levels/" + lastSaveId + ".cfg"){//If level was this (with old id)
				i = editorSet->erase(i);//Erases
				i--;//Back
			}
		}
	}
	
	for (i = editorSet->begin(); i != editorSet->end(); i++)//For each level
		if (*i == lFile) existing = true;//Sets existing flag
	
	if (!existing){//If level doesn't exist
		editorSet->push_back(lFile);
		
		ofstream levels(outputSetFile.c_str());//Levels file
		int n;//Counter
		for (n = 0; n < editorSet->size(); n++)//For each level
			levels << "level" << n + 1 << " = " << (*editorSet)[n] << ";" << endl;//Adds level file
			
		levels << "id = levels_editor;" << endl;//Outputs id string
		levels << "name = Editor;" << endl;//Outputs name
			
		levels.close();//Closes file
	}
	
	lastSaveId = edited.id;//Saves ID
}

//Open button click
void editorOpenClick(clickEventData data){
	PLAYSOUND(clickSfx);
	
	string lFile = inputBox.show(video, "Insert level file name (data/cfg/levels/)");//Level file path
	if (lFile != "") edited = *loadLevel("data/cfg/levels/" + lFile);//Loads level
	
	lastSaveId = edited.id;
	updateProperties();
}

//Properties button click
void editorPropClick(clickEventData data){
	showProperties = !showProperties;
	PLAYSOUND(clickSfx);
	
	updateProperties();
}

//Spring button click
void editorSpringClick(clickEventData data){
	PLAYSOUND(clickSfx);
	springMode = edSpring->checked;
	
	selected = NULL;
	dragged = NULL;
}

//Function to apply properties
void applyProp(){
	edited.id = idField->content.t;
	edited.w = atoi(wField->content.t.c_str());
	edited.h = atoi(hField->content.t.c_str());
	edited.damping_tr = atof(dTrField->content.t.c_str());
	edited.damping_rot = atof(dRotField->content.t.c_str());
	edited.twoStarsTime = atoi(stars2->content.t.c_str());
	edited.threeStarsTime = atoi(stars3->content.t.c_str());
}

//Function to update selected properties
void updateSelProp(){
	if (!selected) return;//Exits function if no entity is selected

	selIdField->content.t = selected->id;
	selMassField->content.t = toString(selected->mass);
	selEField->content.t = toString(selected->e);
	selWField->content.t = toString(selected->w);
	selHField->content.t = toString(selected->h);
	selColorField->content.t = toString(selected->color, true);
	selLockTr->checked = selected->lockTranslation;
	selLockRot->checked = selected->lockRotation;
	selPrint->checked = selected->print;
}

//Function to apply properties to selected
void applySelProp(){
	if (!selected) return;//Exits function if no entity is selected
	
	selected->id = selIdField->content.t;
	selected->mass = atof(selMassField->content.t.c_str());
	selected->e = atof(selEField->content.t.c_str());
	selected->resize(atof(selWField->content.t.c_str()), atof(selHField->content.t.c_str()));
	selected->color = strtol(selColorField->content.t.c_str(), NULL, 0);
	
	selected->lockTranslation = selLockTr->checked;
	selected->lockRotation = selLockRot->checked;
	selected->print = selPrint->checked;
}

//Add button click
void addClick(clickEventData data){
	box* n = (box*) get_ptr <entity> (&entities, data.caller->id);//Requested entity
	
	if (n){//If entity was found
		box *nB = new box;//New box
		n->clone(nB);//Clones preset
		
		nB->translate(vector{edited.w / 2, edited.h / 2} - nB->position);//Sets position
		
		string id = nB->id;//Id
		int i = 1;//Counter
		
		while (get_ptr <entity> (&edited.entities, nB->id)){//While there's an entity with the same name
			nB->id = id + "_" + toString(i);//Serializes id
			i++;//Increases counter
		}
		
		edited.entities.push_back((entity*) nB);//Adds to level
	}
	
	PLAYSOUND(clickSfx);
}

//Function to get entity currently under the mouse
box* getSelected(int oX, int oY){
	int x, y;
	SDL_GetMouseState(&x, &y);//Gets mouse position
	
	vector p (x - oX, y - oY);//Vector
	
	list<entity*>::reverse_iterator i;//Iterator
	for (i = edited.entities.rbegin(); i != edited.entities.rend(); i++)//For each entity
		if ((*i)->isInside(p)) return (box*) *i;//Returns entity
		
	return NULL;//Returns null if nothing was found
}

//Function to print transformation controls
void printTrControls(int oX, int oY){
	if (!selected) return;//Exits function if no entity was selected
	box* b = (box*) selected;//Converts to box
	
	boxColor(video, b->point[0].x + oX, b->point[0].y + oY, b->point[2].x + oX, b->point[2].y + oY, editorSel);//Highlights
	
	//Prints handles
	handle.print_centre(video, oX + b->point[0].x, oY + b->point[0].y);
	handle.print_centre(video, oX + b->point[1].x, oY + b->point[1].y);
	handle.print_centre(video, oX + b->point[2].x, oY + b->point[2].y);
	handle.print_centre(video, oX + b->point[3].x, oY + b->point[3].y);
}

//Function to check links
void checkLinks(){
	deque<phLink*>::iterator i;//Iterator
	int n = 0;
	
	for (i = edited.links.begin(); i != edited.links.end(); i++, n++){//For each link
		spring* s = (spring*) *i;//Converts to spring
		
		if (!get_ptr <entity> (&edited.entities, s->a->id) || (!get_ptr <entity> (&edited.entities, s->b->id))){//If one of the two entities is no more in the level
			free (s);//Frees spring
			i = edited.links.erase(i);//Erases element
			i--;
		}
		
		else {//Else
			s->length_zero = (*s->a_point - *s->b_point).module();//Resets length
			s->id = n > 0 ? "spring_" + toString(n) : "spring";//Resets id
		}
	}
}

//Function to load the editor
void loadEditor(){
	fileData f (entitiesFile);//Entities file
	object o = f.objGen("e");//Generates object
	deque<object>::iterator i;//Iterator
	
	for (i = o.o.begin(); i != o.o.end(); i++){//For each object
		objectBased* newObj = NULL;//Object to load
		if (i->type == OBJTYPE_BOX) newObj = new box;//Creates box
		
		if (newObj){//If object was created
			newObj->fromScriptObj(*i);//Loads it
			entities.push_back ( (entity*) newObj);//Adds to entities
		}
	}
	
	editor = loadWindow(editorFile, "editor");//Loads editor UI
	control* addBtn = editor.getControl("defaultAdd");//Gets default add button
	addBtn->release.handlers.push_back(addClick);//Adds click handler
	
	editor.remove(addBtn);//Removes add control
	
	int n;//Counter
	for (n = 0; n < entities.size(); n++){//For each entity loaded
		control* nC = new control(*addBtn);//New control
		
		nC->area.x += (addBtn->area.w + 4) * n;//Sets position
		nC->id = entities[n]->id;//Sets id
		
		nC->content.contentType = CONTENT_IMAGE;//Sets content type
		nC->content.i.applyScriptCommandLine("new 16 16");//Creates image
		nC->content.i.applyScriptCommandLine("paint box " + toString(entities[n]->color) + " 0 0 15 15");//Paints rect
		
		editor.push_back(nC);//Adds to editor
	}
	
	//Gets buttons
	edBack = editor.getControl("back");
	edNew = editor.getControl("new");
	edSave = editor.getControl("save");
	edOpen = editor.getControl("open");
	edProp = editor.getControl("prop");
	edSpring = (checkBox*) editor.getControl("spring");
	
	//Sets handlers
	edBack->release.handlers.push_back(editorBackClick);
	edNew->release.handlers.push_back(editorNewClick);
	edSave->release.handlers.push_back(editorSaveClick);
	edOpen->release.handlers.push_back(editorOpenClick);
	edProp->release.handlers.push_back(editorPropClick);
	edSpring->release.handlers.push_back(editorSpringClick);
	
	properties = loadWindow(propertiesFile, "properties");//Loads properties
	
	//Gets controls
	propFrame = (panel*) properties.getControl("frame");
	idField = properties.getControl("frame.idField");
	wField = properties.getControl("frame.wField");
	hField = properties.getControl("frame.hField");
	dTrField = properties.getControl("frame.dTrField");
	dRotField = properties.getControl("frame.dRotField");
	stars2 = properties.getControl("frame.twoField");
	stars3 = properties.getControl("frame.threeField");
	
	selProp = loadWindow(selPropFile, "selProperties");//Loads selected properties
	
	//Gets controls
	selPropFrame = (panel*) selProp.getControl("frame");
	selIdField = selProp.getControl("frame.idField");
	selMassField = selProp.getControl("frame.massField");
	selEField = selProp.getControl("frame.eField");
	selWField = selProp.getControl("frame.wField");
	selHField = selProp.getControl("frame.hField");
	selColorField = selProp.getControl("frame.colorField");
	selLockTr = (checkBox*) selProp.getControl("frame.lockTrCheck");
	selLockRot = (checkBox*) selProp.getControl("frame.lockRotCheck");
	selPrint = (checkBox*) selProp.getControl("frame.printCheck");
	
	edited = *loadLevel(templateFile);//Loads template
	updateProperties();//Updates properties panel
}

//Editor loop function
void editorLoop(){
	while (editing && running){//While editor is in execution
		FRAME_BEGIN;//Begins frame
		
		int oX = (video_w - edited.w) / 2;//Print offset x
		int oY = (video_h - edited.h) / 2;//Print offset y
		
		while (SDL_PollEvent(&ev)){//While there are events on stack
			EVENTS_COMMON(ev);//Global events
			
			editor.checkEvents(ev);//Checks editor events
			if (showProperties){ properties.checkEvents(ev); applyProp(); }//Checks properties events
			if (selected) { selProp.checkEvents(ev); applySelProp(); }//Checks selected properties events
			
			if (ev.type == SDL_MOUSEBUTTONDOWN){//On mouse pressure
				if (ev.button.button == SDL_BUTTON_LEFT){//If clicked left
					if (springMode){//If in spring mode
						if (!adding){ adding = new spring; adding->k = springK; }//Creates new spring if it doesn't exist
						
						box* clicked = (box*) getSelected(oX, oY);//Selected entity
						vector* node;//Node vector
						
						if (clicked){//If clicked on an entity
							double min = (vector {double(ev.button.x - oX), double(ev.button.y - oY)} - clicked->point[0]).module();//Minimum distance
							int closest = 0;//Closest vertex index
							int i;//Counter
							
							for (i = 1; i < 4; i++){//For each vertex
								double d = (vector {double(ev.button.x - oX), double(ev.button.y - oY)} - clicked->point[i]).module();//Distance
								
								if (d < min){//If less than minimum
									min = d;//Sets min
									closest = i;//Sets closest
								}
							}
							
							node = &clicked->point[closest];//Gets clicked node
							
							if (!adding->a){//If first spring entity is not set
								adding->a = clicked;//Sets entity
								adding->a_point = node;//Sets node
							}
							
							else {
								adding->b = clicked;//Sets entity
								adding->b_point = node;//Sets node
								
								adding->a->addNode(*adding->a_point);//Adds node to a
								adding->b->addNode(*adding->b_point);//Adds node to b
								
								adding->aPointIndex = adding->a->nodes.size() - 1;//Sets a point index
								adding->bPointIndex = adding->b->nodes.size() - 1;//Sets b point index
								
								adding->length_zero = (*adding->a_point - *adding->b_point).module();//Sets zero length
								adding->id = "spring_" + toString(edited.links.size() + 1);//Sets id
								
								edited.links.push_back(adding);//Adds spring to level
								adding = NULL;
							}
						}
					}
					
					else {//If not in spring mode
						if (selected){//If there's a selected entity
							double min = (vector {double(ev.button.x - oX), double(ev.button.y - oY)} - selected->point[0]).module();//Minimum distance
							int closest = 0;//Closest vertex index
							int i;//Counter
							
							for (i = 1; i < 4; i++){//For each vertex
								double d = (vector {double(ev.button.x - oX), double(ev.button.y - oY)} - selected->point[i]).module();//Distance
								
								if (d < min){//If less than minimum
									min = d;//Sets min
									closest = i;//Sets closest
								}
							}						
							
							if (min < handle.w()){//If inside handle
								draggedNode = &selected->point[closest];//Sets dragged node
								draggedNodeIndex = closest;//Sets dragged node index
							}
							
							else draggedNode = NULL;//Else removes dragged
						}
						else draggedNode = NULL;
						
						if (!draggedNode && selPropFrame->status != control::pressed && propFrame->status != control::pressed){//If not dragging a node or pressing a panel
							selected = getSelected(oX, oY);//Gets entity
							updateSelProp();//Updates selected entity properties
						}
					
						dragged = selected;//Sets dragged
						if (dragged) dragInitial = {double(ev.button.x - oX), double(ev.button.y - oY)};//Sets dragging initial position
						
						if (!draggedNode && dragged) dragDist = dragInitial - dragged->position;//Gets dragging distance
						else if (draggedNode) dragDist = dragInitial - *draggedNode;//Gets dragging distance
					}
				}
				
				else if (ev.button.button == SDL_BUTTON_RIGHT){//If clicked right
					selected = NULL;//Unselects
					
					if (adding){//If adding a spring
						free(adding);//Frees spring
						adding = NULL;//Sets to null
					}
					
					else if (springMode){//Else if in spring mode
						springMode = false;
					}
				}
			}
			
			if (ev.type == SDL_MOUSEBUTTONUP){//On mouse release
				if (dragged) dragged = NULL;//Drops dragged entity
				if (draggedNode) draggedNode = NULL;//Drops dragged node
				
				dragVector = {0,0};//Resets drag vector
			}
			
			if (ev.type == SDL_KEYDOWN){//Keystrokes
				if (ev.key.keysym.sym == SDLK_DELETE){//On del press
					if (selected && !dragged){//If there's a selected entity
						edited.entities.remove(selected);//Removes
						selected = NULL;//Unselects
					}
				}
			}
		}
		
		int mX, mY;//Mouse coords
		int mBtn = SDL_GetMouseState(&mX, &mY);//Gets mouse state
		
		if (dragged && selPropFrame->status != control::pressed && propFrame->status != control::pressed){//If dragging something (entity or node but not panel)
			dragVector = vector {double(ev.button.x - oX), double(ev.button.y - oY)} - dragInitial;//Calculates drag vector
			
			SDLMod m = SDL_GetModState();//Modifier state
			
			if (m & KMOD_CTRL){//If pressing CTRL
				//Limits to one direction
				if (abs(dragVector.x) > abs(dragVector.y)) dragVector.y = 0;
				else dragVector.x = 0;
			}
			
			if (!(m & KMOD_SHIFT)){//Unless pressing shift
				//Limits to 8px steps
				dragVector.x = int(dragVector.x) - (int(dragVector.x) % 8);
				dragVector.y = int(dragVector.y) - (int(dragVector.y) % 8);
			}
			
			dragVector -= dragDist;//Applies dragging distance
			
			if (draggedNode){//If dragging a node
				int prev = draggedNodeIndex - 1, next = draggedNodeIndex + 1;//Previous and next node indices
				
				//Adjusts indices
				if (draggedNodeIndex == 0) prev = 3;
				if (draggedNodeIndex == 3) next = 0;
				
				deque<vector*>::iterator i;//Iterator for nodes
				
				//Drags points
				if (dragged->point[prev].x == draggedNode->x){
					for (i = dragged->nodes.begin(); i != dragged->nodes.end(); i++)//For each node
						if (**i == dragged->point[prev]) (*i)->x = dragInitial.x + dragVector.x;//Moves node
						else if (**i == dragged->point[next]) (*i)->y = dragInitial.y + dragVector.y;//Moves node
						
					dragged->point[prev].x = dragInitial.x + dragVector.x;
					dragged->point[next].y = dragInitial.y + dragVector.y;
				}
				
				else {
					for (i = dragged->nodes.begin(); i != dragged->nodes.end(); i++)//For each node
						if (**i == dragged->point[next]) (*i)->x = dragInitial.x + dragVector.x;//Moves node
						else if (**i == dragged->point[prev]) (*i)->y = dragInitial.y + dragVector.y;//Moves node
						
					dragged->point[next].x = dragInitial.x + dragVector.x;
					dragged->point[prev].y = dragInitial.y + dragVector.y;
				}
				
				for (i = dragged->nodes.begin(); i != dragged->nodes.end(); i++)//For each node
						if (**i == *draggedNode) **i = dragInitial + dragVector;//Moves node
						
				*draggedNode = dragInitial + dragVector;//Drags node
				
				dragged->position = (dragged->point[0] + dragged->point[1] + dragged->point[2] + dragged->point[3]) / 4;//Recalculates position
				
				//Recalculates size
				dragged->w = (dragged->point[0] - dragged->point[1]).module();
				dragged->h = (dragged->point[1] - dragged->point[2]).module();
				
				updateSelProp();//Updates selected properties with new info
			}
			
			else {//If dragging an entity
				dragged->translate(dragInitial - dragged->position + dragVector);//Drags whole entity
			}
		}
		
		BKG;//Prints background
		SDL_FillRect(video, & SDL_Rect { oX, oY, edited.w, edited.h }, editorBkg);//Fills editor background
		
		edited.print(video, oX, oY, true);//Prints level
		printTrControls(oX, oY);//Prints controls
		
		if (springMode && adding && adding->a) lineColor(video, oX + adding->a_point->x, oY + adding->a_point->y, mX, mY, 0x606060FF);//Prints spring
		
		editor.print(video);//Prints editor UI
		if (showProperties) properties.print(video);//Print properties
		if (selected) selProp.print(video);//Prints selected properties
		
		checkLinks();//Checks links
		
		UPDATE;//Updates
		FRAME_END;//Ends frame
	}
}