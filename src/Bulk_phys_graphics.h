//Buch's Library Kollection
//Phys graphics

#ifdef _BULK_PHYS
#ifndef _BULK_PHYS_GRAPHICS
#define _BULK_PHYS_GRAPHICS

//Initialization function
void Bulk_physGraphics_init(){
	SDL_Init(SDL_INIT_EVERYTHING);//Initializes SDL
}

//Function to print an entity
void printEntity(entity* e, SDL_Surface* target, Uint32 color, int x, int y, bool border = true){
	if (e->useAnim){//If entity uses animation	
		e->anims.print_centre(target, x + e->position.x, y + e->position.y);//Prints current frame
	}
	
	else if (e->eType == etype_ball){//If entity's a ball
		ball* b = (ball*) e;//Converts to ball
		filledCircleColor(target, b->position.x + x, b->position.y + y, b->radius, color);//Prints a circle
		if (border) aacircleColor(target, b->position.x + x, b->position.y + y, b->radius, color);//Prints a circle
	}
	
	else if (e->eType == etype_box){//Else if entity's a box
		box* b = (box*) e;//Converts to box
		
		Sint16 px[] = {floor(b->point[0].x) + x, floor(b->point[1].x) + x, floor(b->point[2].x) + x, floor(b->point[3].x) + x};//Box points, x coords
		Sint16 py[] = {floor(b->point[0].y) + y, floor(b->point[1].y) + y, floor(b->point[2].y) + y, floor(b->point[3].y) + y};//Box points, y coords
		
		filledPolygonColor(target, px, py, 4, color);
		if (border) aapolygonColor(target, px, py, 4, color);
	}
}

//Function to print a scene
void printScene(scene *s, SDL_Surface* target, int x, int y, bool hidden = false){
	list <entity*> :: iterator i;//Iterator for entities
	deque <phLink*>::iterator j;
	
	for (i = s->entities.begin(); i != s->entities.end(); i++)//For each entity
		if ((*i)->print) printEntity(*i, target, ((*i)->color << 8) + 0xFF, x, y);//Prints the entity
		else if (hidden) printEntity(*i, target, ((*i)->color << 8) + 0x7F, x, y, false);//Prints hidden entity
		
	for (i = s->particles.begin(); i != s->particles.end(); i++)//For each particle
		if ((*i)->print) printEntity(*i, target, ((*i)->color << 8) + 0xFF, x, y);//Prints the particle
		else if (hidden) printEntity(*i, target, ((*i)->color << 8) + 0x7F, x, y, false);//Prints hidden particle
		
	for (j = s->links.begin(); j != s->links.end(); j++)//For each link
		if (((!(*j)->a || (*j)->a->print) && (!(*j)->b || (*j)->b->print)) || hidden)//If prints entities
			lineColor(target, x + (*j)->a_point->x, y + (*j)->a_point->y, x + (*j)->b_point->x, y + (*j)->b_point->y, 0x606060FF);//Prints link
}

#endif
#endif