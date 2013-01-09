//Editor header
Uint32 editorBkg = 0x181818;//Editor background
Uint32 editorSel = 0xFFFFFF30;//Editor selected overlay

image handle;//Transformation handles

string entitiesFile = "data/cfg/editor/entities.cfg";//Entities file
deque<entity*> entities;//Loaded entities

string editorFile = "data/cfg/ui/editor.cfg";//Editor window file path
window editor;//Editor window

bool editing = false;//Editor running flagg

level edited;//Edited level

box* selected = NULL;//Selected entity

box* dragged = NULL;//Dragged entity
vector* draggedNode = NULL;//Dragged node
int draggedNodeIndex;//Dragged node index

vector dragInitial;//Dragging intial position
vector dragDist;//Distance between dragging point and position of dragged entity
vector dragVector;//Dragging vector

//Add button click
void addClick(clickEventData data){
	box* n = (box*) get_ptr <entity> (&entities, data.caller->id);//Requested entity
	
	if (n){//If entity was found
		box *nB = new box (*n);//New box
		nB->translate(vector{edited.w / 2, edited.h / 2} - nB->position);//Sets position
		edited.entities.push_back((entity*) nB);//Adds to level
	}
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
	
	edited.w = 800;
	edited.h = 400;
}

//Editor loop function
void editorLoop(){
	while (editing && running){//While editor is in execution
		FRAME_BEGIN;//Begins frame
		BKG;//Prints background
		
		int oX = (video_w - edited.w) / 2;//Print offset x
		int oY = (video_h - edited.h) / 2;//Print offset y
		
		while (SDL_PollEvent(&ev)){//While there are events on stack
			EVENTS_COMMON(ev);//Global events
			editor.checkEvents(ev);//Checks editor events
			
			if (ev.type == SDL_MOUSEBUTTONDOWN){//On mouse pressure
				if (ev.button.button == SDL_BUTTON_LEFT){//If clicked left
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
					
					if (!draggedNode) selected = getSelected(oX, oY);//Gets entity
				
					dragged = selected;//Sets dragged
					if (dragged) dragInitial = {double(ev.button.x - oX), double(ev.button.y - oY)};//Sets dragging initial position
					
					if (!draggedNode && dragged) dragDist = dragInitial - dragged->position;//Gets dragging distance
					else if (draggedNode) dragDist = dragInitial - *draggedNode;//Gets dragging distance
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
		
		if (dragged){//If dragging something (entity or node)
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
				
				//Drags points
				if (dragged->point[prev].x == draggedNode->x){
					dragged->point[prev].x = dragInitial.x + dragVector.x;
					dragged->point[next].y = dragInitial.y + dragVector.y;
				}
				
				else {
					dragged->point[next].x = dragInitial.x + dragVector.x;
					dragged->point[prev].y = dragInitial.y + dragVector.y;
				}
				
				*draggedNode = dragInitial + dragVector;//Drags node
				
				dragged->position = (dragged->point[0] + dragged->point[1] + dragged->point[2] + dragged->point[3]) / 4;//Recalculates position
				
				//Recalculates size
				dragged->w = (dragged->point[0] - dragged->point[1]).module();
				dragged->h = (dragged->point[1] - dragged->point[2]).module();
			}
			
			else {//If dragging an entity
				dragged->translate(dragInitial - dragged->position + dragVector);//Drags whole entity
			}
		}
		
		SDL_FillRect(video, & SDL_Rect { oX, oY, edited.w, edited.h }, editorBkg);//Fills editor background
		
		edited.print(video, oX, oY);//Prints level
		printTrControls(oX, oY);//Prints controls
		
		editor.print(video);//Prints editor UI
		
		UPDATE;//Updates
		FRAME_END;//Ends frame
	}
}