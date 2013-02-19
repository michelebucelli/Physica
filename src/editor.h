//Editor header
Uint32 editorBkg = 0x181818;//Editor background
Uint32 editorSel = 0xFFFFFF10;//Editor selected overlay

image handle;//Transformation handles

string templateFile = "data/cfg/levels/levelPresets.cfg";//Level preset file
string outputSetFile = "data/cfg/levels/levels_editor.cfg";//Editor set file
string entitiesFile = "data/cfg/editor/entities.cfg";//Entities file
deque<entity*> entities;//Loaded entities

string editorFile = "data/cfg/ui/editor.cfg";//Editor window file path
window editor;//Editor window
control *edBack, *edNew, *edSave, *edOpen, *edProp, *edTest;//Editor buttons
control *edArea;//Add area button
checkBox* edSpring;//Spring mode checkbox

string propertiesFile = "data/cfg/ui/editor_properties.cfg";//Properties window file path
window properties;//Properties window
panel* propFrame;//Properties frame
control *idField, *wField, *hField, *dTrField, *dRotField, *stars2, *stars3;//Level properties fields
control *editRules;//Edit rules button
bool showProperties = true;//Show level properties flag

string selPropFile = "data/cfg/ui/editor_selProp.cfg";//Selected properties file path
window selProp;//Selected properties window
panel* selPropFrame;//Selected properties frame
control *selIdField, *selMassField, *selEField, *selWField, *selHField, *selColorField, *selDampTrField, *selDampRotField;//Selected properties fields
checkBox *selLockTr, *selLockX, *selLockY, *selLockRot, *selPrint;//Selected properties check boxes

string areaPropFile = "data/cfg/ui/editor_areaProp.cfg";//Selected area properties file path
window areaProp;//Selected area properties window
panel* areaPropFrame;//Area properties frame
control* areaRules;//Area rules edit button
control* areaColorField;//Area color field

bool editing = false;//Editor running flag
bool springMode = false;//True if in spring mode
spring* adding = NULL;//Spring being added
double springK = 0.5;//Spring constant

level edited;//Edited level

box* selected = NULL;//Selected entity
area* selectedArea = NULL;//Selected area

box* dragged = NULL;//Dragged entity
vector* draggedNode = NULL;//Dragged node
area* draggedArea = NULL;//Dragged area
int draggedAreaNodeIndex;//Dragged area node index
int draggedNodeIndex = -1;//Dragged node index

vector dragInitial;//Dragging intial position
vector dragDist;//Distance between dragging point and position of dragged entity
vector dragVector;//Dragging vector

int scrollMargin = 1;//Scroll margin
int scrollSpeed = 10;//Scroll speed

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
	
	string path = edited.path;//Gets path
	
	edited = *loadLevel(templateFile);//Loads template
	edited.path = path;//Re-sets path
	
	updateProperties();//Updates properties panel
}

//Save button click
void editorSaveClick(clickEventData data){
	string lFile = inputBoxDialog.show(video, getText("insertFile"), edited.path);//Level file path
	
	if (lFile == "")//Invalid file
		return;//Returns
		
	else if (edited.id == ""){//Invalid id
		msgBox.show(video, getText("idRequired"), 1, msgBox_ans_ok);//Message function
		return;//Returns
	}
	
	ofstream o (lFile.c_str());//Output level file

	if (!o.good()){//If file couldn't be opened
		msgBox.show(video, getText("fileCreationFailure"), 1, msgBox_ans_ok);//Message
		return;//Quits
	}
	
	edited.path = lFile;//Sets path
	
	o << edited.toScriptObj().toString();//Saves level
	o.close();//Closes file
}

//Open button click
void editorOpenClick(clickEventData data){
	PLAYSOUND(clickSfx);
	
	string lFile = inputBoxDialog.show(video, getText("insertFile"));//Level file path
	if (lFile != "") edited = *loadLevel(lFile);//Loads level
	
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

//Rules button click
void editRulesClick(clickEventData data){
	rules *r = rulesDialog.show(video, &edited.lvlRules);//Edits rules
	if (r) edited.lvlRules = *r;//Sets rules
}

//Area rules button click
void areaRulesEdit(clickEventData data){
	if (selectedArea){//If there's a selected area
		rules* r = rulesDialog.show(video, selectedArea);//Gets rules
		if (r) selectedArea->setRules(*r);//Sets rules
	}
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
	selDampTrField->content.t = toString(selected->damping_tr);
	selDampRotField->content.t = toString(selected->damping_rot);
	selWField->content.t = toString(selected->w);
	selHField->content.t = toString(selected->h);
	selColorField->content.t = toString(selected->color, true);
	selLockTr->checked = selected->lockTranslation;
	selLockX->checked = selected->lockX;
	selLockY->checked = selected->lockY;
	selLockRot->checked = selected->lockRotation;
	selPrint->checked = selected->print;
}

//Function to update area properties
void updateAreaProp(){
	areaColorField->content.t = toString(selectedArea->color, true);
}

//Function to apply properties to selected
void applySelProp(){
	if (!selected) return;//Exits function if no entity is selected
	
	selected->id = selIdField->content.t;
	selected->mass = atof(selMassField->content.t.c_str());
	selected->e = atof(selEField->content.t.c_str());
	selected->damping_tr = atof(selDampTrField->content.t.c_str());
	selected->damping_rot = atof(selDampRotField->content.t.c_str());
	selected->resize(atof(selWField->content.t.c_str()), atof(selHField->content.t.c_str()));
	selected->color = strtol(selColorField->content.t.c_str(), NULL, 0);
	
	selected->lockTranslation = selLockTr->checked;
	selected->lockX = selLockX->checked;
	selected->lockY = selLockY->checked;
	selected->lockRotation = selLockRot->checked;
	selected->print = selPrint->checked;
}

//Function to apply properties to area
void applyAreaProp(){
	selectedArea->color = strtol(areaColorField->content.t.c_str(), NULL, 0);
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

//Add area button click
void addAreaClick(clickEventData data){
	area newArea;//New area
	
	//Sets parameters
	newArea.x = edited.w / 2 - 48;
	newArea.y = edited.h / 2 - 48;
	newArea.w = 96;
	newArea.h = 96;
	newArea.color = 0x131B24;
	
	int n = 0;//Counter
	newArea.rectangle::id = "area_" + toString(n);//Sets id
	while (get <area> (&edited.areas, "area_" + toString(n))){//While there are areas with the same name
		n++;//Increases counter
		newArea.rectangle::id = "area_" + toString(n);//Sets id
	}
	
	edited.areas.push_back(newArea);//Adds to level
}

//Test button click
void testClick(clickEventData data){
	test(edited.path, lpEdited.lsRules);
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

//Function to get area currently under the mouse
area* getSelArea(int oX, int oY){
	int x, y;
	SDL_GetMouseState(&x, &y);//Gets mouse position
	
	list<area>::reverse_iterator i;//Iterator
	for (i = edited.areas.rbegin(); i != edited.areas.rend(); i++)//For each area
		if (i->isInside(x - oX, y - oY)) return &*i;//Returns area
		
	return NULL;//Returns null if nothing was found
}

//Function to print transformation controls
void printTrControls(int oX, int oY){
	if (selected){//If there's a selected entity
		box* b = (box*) selected;//Converts to box
		
		rectangleColor(video, b->point[0].x + oX, b->point[0].y + oY, b->point[2].x + oX, b->point[2].y + oY, editorSel);//Highlights
		
		//Prints handles
		handle.print_centre(video, oX + b->point[0].x, oY + b->point[0].y);
		handle.print_centre(video, oX + b->point[1].x, oY + b->point[1].y);
		handle.print_centre(video, oX + b->point[2].x, oY + b->point[2].y);
		handle.print_centre(video, oX + b->point[3].x, oY + b->point[3].y);
	}
	
	else if (selectedArea){//If there's a selected area
		rectangleColor(video, selectedArea->x + oX, selectedArea->y + oY, selectedArea->x + selectedArea->w + oX, selectedArea->y + selectedArea->h + oY, editorSel);//Highlights
		
		//Prints handles
		handle.print_centre(video, oX + selectedArea->x, oY + selectedArea->y);
		handle.print_centre(video, oX + selectedArea->x + selectedArea->w, oY + selectedArea->y);
		handle.print_centre(video, oX + selectedArea->x, oY + selectedArea->y + selectedArea->h);
		handle.print_centre(video, oX + selectedArea->x + selectedArea->w, oY + selectedArea->y + selectedArea->h);
	}
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

//Function to load a level into the editor
void openLevel(level *l){
	edited = *l;//Loads template
	updateProperties();//Updates properties panel
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
	edArea = editor.getControl("area");
	edTest = editor.getControl("test");
	
	//Sets handlers
	edBack->release.handlers.push_back(editorBackClick);
	edNew->release.handlers.push_back(editorNewClick);
	edSave->release.handlers.push_back(editorSaveClick);
	edOpen->release.handlers.push_back(editorOpenClick);
	edProp->release.handlers.push_back(editorPropClick);
	edSpring->release.handlers.push_back(editorSpringClick);
	edArea->release.handlers.push_back(addAreaClick);
	edTest->release.handlers.push_back(testClick);
	
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
	editRules = properties.getControl("frame.editRules");
	
	editRules->release.handlers.push_back(editRulesClick);//Edit rules click handler
	
	selProp = loadWindow(selPropFile, "selProperties");//Loads selected properties
		
	//Gets controls
	selPropFrame = (panel*) selProp.getControl("frame");
	selIdField = selProp.getControl("frame.idField");
	selMassField = selProp.getControl("frame.massField");
	selDampTrField = selProp.getControl("frame.dampTrField");
	selDampRotField = selProp.getControl("frame.dampRotField");
	selEField = selProp.getControl("frame.eField");
	selWField = selProp.getControl("frame.wField");
	selHField = selProp.getControl("frame.hField");
	selColorField = selProp.getControl("frame.colorField");
	selLockTr = (checkBox*) selProp.getControl("frame.lockTrCheck");
	selLockX = (checkBox*) selProp.getControl("frame.lockXCheck");
	selLockY = (checkBox*) selProp.getControl("frame.lockYCheck");
	selLockRot = (checkBox*) selProp.getControl("frame.lockRotCheck");
	selPrint = (checkBox*) selProp.getControl("frame.printCheck");
	
	areaProp = loadWindow(areaPropFile, "areaProperties");//Loads area properties
	
	//Gets controls
	areaPropFrame = (panel*) areaProp.getControl("frame");
	areaRules = areaProp.getControl("frame.editRules");
	areaColorField = areaProp.getControl("frame.colorField");
	
	//Sets handlers
	areaRules->release.handlers.push_back(areaRulesEdit);
	
	openLevel(loadLevel(templateFile));//Loads template
}

//Editor loop function
void editorLoop(){
	int oX = (video_w - edited.w) / 2;//Print offset x
	int oY = (video_h - edited.h) / 2;//Print offset y
	
	while (editing && running){//While editor is in execution
		FRAME_BEGIN;//Begins frame
		
		while (SDL_PollEvent(&ev)){//While there are events on stack
			EVENTS_COMMON(ev);//Global events
			
			hideCursor = false;
			
			editor.checkEvents(ev);//Checks editor events
			if (showProperties){ properties.checkEvents(ev); applyProp(); }//Checks properties events
			if (selected) { selProp.checkEvents(ev); applySelProp(); }//Checks selected properties events
			else if (selectedArea) { areaProp.checkEvents(ev); applyAreaProp(); }//Checks area properties events
			
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
						
						if (selectedArea){//If there's a selected area
							int pX = ev.button.x - oX, pY = ev.button.y - oY;//Clicked point
							
							if (abs(pX - selectedArea->x) < handle.w()){//If selected one of the left points
								if (abs(pY - selectedArea->y) < handle.h())//If selected the top left node
									draggedAreaNodeIndex = 2;//Sets index
									
								else if (abs(pY - selectedArea->y - selectedArea->h) < handle.h())//If selected the bottom left node
									draggedAreaNodeIndex = 1;//Sets index
									
								else draggedAreaNodeIndex = -1;
							}
							
							else if (abs(pX - selectedArea->x - selectedArea->w) < handle.w()){//If selected one of the right points
								if (abs(pY - selectedArea->y) < handle.h())//If selected the top right node
									draggedAreaNodeIndex = 3;//Sets index
									
								else if (abs(pY - selectedArea->y - selectedArea->h) < handle.h())//If selected the bottom right node
									draggedAreaNodeIndex = 0;//Sets index
									
								else draggedAreaNodeIndex = -1;
							}
							
							else draggedAreaNodeIndex = -1;
						}
						else draggedAreaNodeIndex = -1;
						
						if (!draggedNode && draggedAreaNodeIndex == -1 && selPropFrame->status != control::pressed && propFrame->status != control::pressed && areaPropFrame->status != control::pressed){//If not dragging a node or pressing a panel
							selected = getSelected(oX, oY);//Gets entity
							
							if (selected) updateSelProp();//Updates selected entity properties
							if (!selected) selectedArea = getSelArea(oX, oY);//Selected area
							if (selectedArea) updateAreaProp();//Updates selected area properties
						}
					
						dragged = selected;//Sets dragged
						draggedArea = selectedArea;//Sets dragged area
						if (dragged || draggedArea) dragInitial = {double(ev.button.x - oX), double(ev.button.y - oY)};//Sets dragging initial position
						
						if (!draggedNode && dragged) dragDist = dragInitial - dragged->position;//Gets dragging distance
						else if (draggedNode) dragDist = dragInitial - *draggedNode;//Gets dragging distance
						else if (draggedArea && draggedAreaNodeIndex == -1) dragDist = dragInitial - vector (draggedArea->x, draggedArea->y);//Gets dragging distance
						else if (draggedArea && draggedAreaNodeIndex != -1){//If dragging area node
							int pX = draggedArea->x + (draggedAreaNodeIndex % 3 == 0 ? draggedArea->w : 0);//X coord
							int pY = draggedArea->y + (draggedAreaNodeIndex < 2 ? draggedArea->h : 0);//Y coord
							
							dragDist = dragInitial - vector (pX, pY);//Gets dragging distance
						}
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
						edSpring->checked = false;
					}
				}
			}
			
			if (ev.type == SDL_MOUSEBUTTONUP){//On mouse release
				dragged = NULL;//Drops dragged entity
				draggedNode = NULL;//Drops dragged node
				draggedArea = NULL;//Drops dragged area
				
				dragVector = {0,0};//Resets drag vector
			}
			
			if (ev.type == SDL_KEYDOWN){//Keystrokes
				if (ev.key.keysym.sym == SDLK_DELETE){//On del press
					if (selected && !dragged){//If there's a selected entity
						edited.entities.remove(selected);//Removes
						selected = NULL;//Unselects
					}
					
					else if (selectedArea && draggedAreaNodeIndex == -1){//If deleting area
						edited.areas.remove(*selectedArea);//Removes
						selectedArea = NULL;//Unselects
					}
				}
				
				else if (ev.key.keysym.sym == SDLK_HOME){//On home press
					oX = (video_w - edited.w) / 2;//Resets x offset
					oY = (video_h - edited.h) / 2;//Resets y offset
				}
			}
		}
		
		int mX, mY;//Mouse coords
		int mBtn = SDL_GetMouseState(&mX, &mY);//Gets mouse state
		
		if ((dragged || draggedArea) && selPropFrame->status != control::pressed && propFrame->status != control::pressed && areaPropFrame->status != control::pressed){//If dragging something (area, entity or node but not panel)
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
			
			else if (dragged) {//If dragging an entity
				dragged->translate(dragInitial - dragged->position + dragVector);//Drags whole entity
			}
			
			else if (draggedArea && draggedAreaNodeIndex != -1){//If dragging area node
				switch(draggedAreaNodeIndex){//According to dragged index
					case 0:
					draggedArea->w = -draggedArea->x + dragInitial.x + dragVector.x;
					draggedArea->h = -draggedArea->y + dragInitial.y + dragVector.y;
					break;
					
					case 1:
					draggedArea->w += draggedArea->x - dragInitial.x - dragVector.x;
					draggedArea->h = -draggedArea->y + dragInitial.y + dragVector.y;
					
					draggedArea->x = dragInitial.x + dragVector.x;
					break;
					
					case 2:
					draggedArea->w += draggedArea->x - dragInitial.x - dragVector.x;
					draggedArea->h += draggedArea->y - dragInitial.y - dragVector.y;
					
					draggedArea->x = dragInitial.x + dragVector.x;
					draggedArea->y = dragInitial.y + dragVector.y;
					break;
					
					case 3:
					draggedArea->w = -draggedArea->x + dragInitial.x + dragVector.x;
					draggedArea->h += draggedArea->y - dragInitial.y - dragVector.y;
					
					draggedArea->y = dragInitial.y + dragVector.y;
					break;
				}
			}
			
			else if (draggedArea) {//If dragging an area
				draggedArea->x = dragInitial.x + dragVector.x;
				draggedArea->y = dragInitial.y + dragVector.y;
			}
		}
		
		//Checks for scrolling
		if (mX < scrollMargin) oX += scrollSpeed;
		if (mX >= video_w - scrollMargin) oX -= scrollSpeed;
		if (mY < scrollMargin) oY += scrollSpeed;
		if (mY >= video_h - scrollMargin) oY -= scrollSpeed;
		
		BKG;//Prints background
		SDL_FillRect(video, & SDL_Rect { oX, oY, edited.w, edited.h }, editorBkg);//Fills editor background
		
		edited.print(video, oX, oY, true);//Prints level
		printTrControls(oX, oY);//Prints controls
		
		if (springMode && adding && adding->a) lineColor(video, oX + adding->a_point->x, oY + adding->a_point->y, mX, mY, 0x606060FF);//Prints spring
		
		editor.print(video);//Prints editor UI
		if (showProperties) properties.print(video);//Print properties
		if (selected) selProp.print(video);//Prints selected properties
		else if (selectedArea) areaProp.print(video);//Prints area properties
		
		checkLinks();//Checks links
		
		updateCommon();//Updates common ui
		common.print(video);//Prints common ui
		
		UPDATE;//Updates
		FRAME_END;//Ends frame
	}
}