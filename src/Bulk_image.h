//Buch Library Kollection
//Image handling header

#ifndef _BULK_IMAGE
#define _BULK_IMAGE

//Macros
#define CACHEDSURFACE(NAME)		surfaceCache.getObj(NAME)//Macro to get a cached surface

//Script object types
#define OBJTYPE_RECTANGLE		"rectangle"//Rectangle objects
#define OBJTYPE_IMAGE			"image"//Image objects
#define OBJTYPE_ANIMATION		"animation"//Animation objects
#define OBJTYPE_ANIMSET			"animSet"//Animation set objects

//Function for optimized surface loading
SDL_Surface* loadImg (const char* source){
	SDL_Surface* s = IMG_Load(source);//Loads surface
	if (s) s = SDL_DisplayFormatAlpha(s);//Converts surface to display format
	
	return s;//Returns surface
}

cache <SDL_Surface*, loadImg> surfaceCache;//Surface cache

//Initialization function
bool Bulk_image_init() {
	if (SDL_Init(SDL_INIT_EVERYTHING) == -1){//If failed initializing SDL
		cerr << "BULK WARNING: failed initializing SDL - " << __FILE__ << ":" << __LINE__ - 1 << endl;//Error message on stderr
		return false;//Returns false
	}
	
	else if (IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) == -1){//If failed initializing SDL_image
		cerr << "BULK WARNING: failed initializing SDL_image - " << __FILE__ << ":" << __LINE__ - 1 << endl;//Error message on stderr
		return false;//Returns false
	}
	
	else//Else
		return true;//Returns true
}

//Rectangle class
//	basically, the SDL_rect class, plus some methods to load from and save to script object
class rectangle: public SDL_Rect, public objectBased {
	public:
	
	//Constructor
	rectangle(){
		id = "";
		type = OBJTYPE_RECTANGLE;
		
		x = 0;
		y = 0;
		w = 1;
		h = 1;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If can load an object of this kind from the given object
			var* x = get <var> (&o.v, "x");//X coordinate
			var* y = get <var> (&o.v, "y");//Y coordinate
			var* w = get <var> (&o.v, "w");//Width
			var* h = get <var> (&o.v, "h");//Height
			
			if (x) this->x = x->intValue();//Sets x
			if (y) this->y = y->intValue();//Sets y
			if (w) this->w = w->intValue();//Sets w
			if (h) this->h = h->intValue();//Sets h
			
			return true;//Returns true
		}
		
		return false;//Returns false if failed loading
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();//Base object data
		
		//Sets member variables
		result.set("x", x);
		result.set("y", y);
		result.set("w", w);
		result.set("h", h);
		
		return result;//Returns result
	}
};

//Base image class
//	represents a static image, via a SDL_Surface* variable and a rectangle
//	can be loaded from and saved to a script object, and can have paint scripts applied
//
//	paint script valid commands:
//		new [width] [height]: creates a new black surface of given size
//
//		fill [color]: fills surface with given color (opaque)
//
//		paint [primitive] [color] [primitive args]: paints given primitive with given color,
//			using given args. Supported primitives, with args:
//
//			rect, [topleft x] [topleft y] [bottomright x] [bottomright y]
//			circle, [centre x] [centre y] [radius]
//			roundedRect, [topleft x] [topleft y] [bottomright x] [bottomright y] [smooth radius]
//			line, [x1] [y1] [x2] [y2]
//			box, [topleft x] [topleft y] [bottomright x] [bottomright y]
//			filledCircle, [centre x] [centre y] [radius]
//			roundedBox, [topleft x] [topleft y] [bottomright x] [bottomright y] [smooth radius]
//
//		colorKey [color]: sets image's color key to given color
class image: public objectBased, public scriptable{
	private:
	bool cached;//Flag indicating if the image was taken from cache with no changes (used when painting, to avoid painting on cached surfaces)
	
	script paint;//Paint script
	
	public:
	SDL_Surface* surface;//The surface in which the image is contained
	string imageFilePath;//Image file path (used when saving to script object)
	
	rectangle rect;//Rectangle taken from the surface
	bool useRect;//Flag indicating if rectangle should be used
	
	int offsetX, offsetY;//Image offset (used when printing)
	
	//Constructor
	image(){
		id = "";
		type = OBJTYPE_IMAGE;
		
		imageFilePath = "";
		cached = false;
		
		surface = NULL;
		
		useRect = false;
		
		offsetX = 0;
		offsetY = 0;
	}
	
	//Creates the image from data
	image(string id, string path, int x, int y, int w, int h){
		this->id = id;
		type = OBJTYPE_IMAGE;
		
		imageFilePath = path;
		cached = true;
		
		surface = loadImg(path.c_str());
		
		useRect = true;
		
		offsetX = 0;
		offsetY = 0;
		
		rect.id = "rect";
		rect.x = x;
		rect.y = y;
		rect.w = w;
		rect.h = h;
		
		cout << "Done" << endl;
	}
	
	//Function to copy the current surface to a new one
	void copySurface(){
		surface = SDL_CreateRGBSurfaceFrom(surface->pixels, surface->w, surface->h, surface->format->BitsPerPixel, surface->pitch, surface->format->Rmask, surface->format->Gmask, surface->format->Bmask, surface->format->Amask);//Copies surface
		cached = false;//Surface is no more cached
	}
	
	//Print function
	//	offset is related to the topleft corner
	void print(SDL_Surface* target, int x, int y){
		SDL_Rect offset = {x + offsetX, y + offsetY};//Offset rectangle
		SDL_BlitSurface(surface, useRect ? &rect : NULL, target, &offset);//Blits surface
	}
	
	//Function to determine if image is valid (surface not null)
	bool valid() { return surface != NULL; }
	
	//Print function
	//	offset is related to the centre
	void print_centre (SDL_Surface* target, int x, int y){
		if (!surface){ print(target, x, y); return; }//If no surface is available, prints referring to topleft corner
		
		SDL_Rect offset = {x - (useRect ? rect.w / 2 : surface->w / 2) + offsetX, y - (useRect ? rect.h / 2 : surface->h / 2) + offsetY};//Offset rectangle
		SDL_BlitSurface(surface, useRect ? &rect : NULL, target, &offset);//Blits surface
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If can load an object of this kind from the given object
			var* imageFilePath = get <var, deque<var> > (&o.v, "imageFile");//Image file variable
			object* rect = get <object, deque<object> > (&o.o, "rect");//Rectangle object
			var* paintScript = get <var, deque<var> > (&o.v, "paintScript");//Paint script variable
			var* offsetX = get <var, deque<var> > (&o.v, "offsetX");//X offset variable
			var* offsetY = get <var, deque<var> > (&o.v, "offsetY");//Y offset variable
			
			if (imageFilePath){//If an image file path is available
				this->imageFilePath = imageFilePath->value;//Sets image file path member
				surface = CACHEDSURFACE(this->imageFilePath);//Loads surface from cache
				
				if (!surface) cerr << "Failed loading surface from " << this->imageFilePath << " - " << __FILE__ << ":" << __LINE__ - 2 << endl;//If failed loading, error message on standard error
				cached = true;//Sets cached flag
			}
			
			if (rect){//If rectangle was given
				this->rect.fromScriptObj(*rect);//Loads rectangle
				useRect = true;//Uses rectangle
			}
			
			if (paintScript){//If a paint script was given
				paint.fromString(paintScript->value);//Loads paint script
				applyScript(paint);//Applies the script
			}
			
			if (offsetX) this->offsetX = offsetX->intValue();//Gets x offset
			if (offsetY) this->offsetY = offsetY->intValue();//Gets y offset
			
			return true;//Returns true
		}
		
		return false;//Returns false if failed
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();//Base object data
		
		result.set("imageFile", imageFilePath);//Sets image file path variable
		result.o.push_back(rect.toScriptObj());//Adds rectangle object
		
		return result;//Returns result
	}
	
	//Function to apply a script command
	int applyScriptCommand(string commandName, deque<string> args){
		//Generic commands
		if (commandName == "new" && args.size() >= 2){//Command to create a new surface
			cached = false;//Surface is not cached anymore
			surface = SDL_CreateRGBSurface(SDL_SWSURFACE, atoi(args[0].c_str()), atoi(args[1].c_str()), 32, 0, 0, 0, 0);//Creates the new surface
			
			return CMD_RES_NORMAL;//Normal result
		}
		
		//Primitives painting commands
		if (commandName == "paint" && args.size() >= 1 && surface){//Command to paint
			if (cached) copySurface();//Copies the surface if it is cached
			
			Uint32 color = (strtol(args[1].c_str(), NULL, 0) << 8) + 0xFF;//Paint color
			
			if (args[0] == "line" && args.size() >= 6)//Line
				lineColor(surface, atoi(args[2].c_str()), atoi(args[3].c_str()), atoi(args[4].c_str()), atoi(args[5].c_str()), color);//Prints the line
				
			if (args[0] == "dot" && args.size() >= 4)//Dot
				pixelColor(surface, atoi(args[2].c_str()), atoi(args[3].c_str()), color);//Prints the dot
			
			if (args[0] == "rect" && args.size() >= 6)//Rectangle
				rectangleColor(surface, atoi(args[2].c_str()), atoi(args[3].c_str()), atoi(args[4].c_str()), atoi(args[5].c_str()), color);//Prints the rectangle
			
			if (args[0] == "circle" && args.size() >= 5)//Circle
				aacircleColor(surface, atoi(args[2].c_str()), atoi(args[3].c_str()), atoi(args[4].c_str()), color);//Prints the circle with antialiasing
			
			if (args[0] == "roundedRect" && args.size() >= 7)//Rounded rectangle
				roundedRectangleColor(surface, atoi(args[2].c_str()), atoi(args[3].c_str()), atoi(args[4].c_str()), atoi(args[5].c_str()), atoi(args[6].c_str()), color);//Prints the rectangle
				
			if (args[0] == "box" && args.size() >= 6)//Box
				boxColor(surface, atoi(args[2].c_str()), atoi(args[3].c_str()), atoi(args[4].c_str()), atoi(args[5].c_str()), color);//Prints the rectangle
			
			if (args[0] == "filledCircle" && args.size() >= 5){//Filled circle
				filledCircleColor(surface, atoi(args[2].c_str()), atoi(args[3].c_str()), atoi(args[4].c_str()), color);//Prints the circle fill
				aacircleColor(surface, atoi(args[2].c_str()), atoi(args[3].c_str()), atoi(args[4].c_str()), color);//Prints the circle with antialiasing
			}
			
			if (args[0] == "roundedBox" && args.size() >= 7)//Rounded box
				roundedBoxColor(surface, atoi(args[2].c_str()), atoi(args[3].c_str()), atoi(args[4].c_str()), atoi(args[5].c_str()), atoi(args[6].c_str()), color);//Prints the rectangle
			
			return CMD_RES_NORMAL;//Normal result
		}
		
		//Filling commands
		if (commandName == "fill" && args.size() >= 1){
			if (cached) copySurface();//Copies the surface if it is cached
			
			SDL_FillRect(surface, &surface->clip_rect, strtol(args[0].c_str(), NULL, 0));//Fills surface
			
			return CMD_RES_NORMAL;//Normal result
		}
		
		//Misc
		if (commandName == "colorKey" && args.size() >= 1){
			if (cached) copySurface();//Copies the surface if it is cached
			
			SDL_SetColorKey(surface, SDL_SRCCOLORKEY, strtol(args[0].c_str(), NULL, 0));//Sets color key
		}
	}
	
	//Function to get image width
	int w(){
		if (useRect || !surface) return rect.w;
		else return surface->w;
	}
	
	//Function to get image height
	int h(){
		if (useRect || !surface) return rect.h;
		else return surface->h;
	}
};

//Animation class
//	based on a deque of images used as frames, plus indication of the current frame and of the sequence frames will
//	appear in
//
//	note: maybe, in future script commands support should be added
class animation: public deque<image>, public objectBased {
	public:
	iterator current;//Current frame iterator
	
	deque<string> sequence;//Frames sequence (IDs of the frames)
	deque<string>::iterator currentId;//Current frame's ID iterator
	
	bool playOnce;//If true, animation is played once and stopped
	bool ended;//If true, animation has ended first loop
	
	//Constructor
	animation(){
		id = "";
		type = OBJTYPE_ANIMATION;
		
		current = begin();//Sets first frame
		currentId = sequence.begin();//Sets first frame id iterator
		
		playOnce = false;
		ended = false;
	}
	
	//Constructor from set of frames
	animation(deque<image> *frames){
		id = "";
		type = OBJTYPE_ANIMATION;
		
		current = begin();//Sets first frame
		currentId = sequence.begin();//Sets first frame id iterator
		
		playOnce = false;
		ended = false;
	
		deque<image>::iterator i;//Iterator
		
		for (i = frames->begin(); i != frames->end(); i++)//For each frame
			push_back(*i);//Adds to animation
	}
	
	//Function to restart
	void restart(){
		currentId = sequence.begin();//Sets first element of sequence
		if (currentId != sequence.end()) current = get_iterator <image, animation> (this, *currentId);//Sets new frame
		ended = false;//Doesn't end
	}
	
	//Function to move to next frame
	void next(){
		if (*currentId == sequence.back()){ currentId = sequence.begin(); ended = true; }//Restarts from beginning if reached end
		else currentId++;//Next frame in sequence
		
		if (currentId != sequence.end()) current = get_iterator <image, animation> (this, *currentId);//Sets new frame
	}
	
	//Print function
	//	offset is related to the toplefrt corner
	void print(SDL_Surface* target, int x, int y){
		if (current != end()) current->print(target, x, y);//Prints current image
	}
	
	//Print function
	//	offset is related to the centre
	void print_centre (SDL_Surface* target, int x, int y){
		if (current != end()) current->print_centre(target, x, y);//Prints current image with offset to centre
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If type is matching
			var* sequence = get <var, deque<var> > (&o.v, "sequence");//Sequence variable
			var* playOnce = get <var, deque<var> > (&o.v, "playOnce");//Play once variable
			
			if (sequence) this->sequence = tokenize < deque<string> > (sequence->value, " \t,");//Gets sequence
			if (playOnce) this->playOnce = playOnce->intValue();//Set play once flag
			
			deque<object>::iterator i;//Iterator for sub-objects
			for (i = o.o.begin(); i != o.o.end(); ++i){//For each sub-object in given object
				if (i->type == OBJTYPE_IMAGE){//If object is an image
					image newFrame;//New frame
					
					if (newFrame.fromScriptObj(*i))//If successfully loaded image from object
						push_back(newFrame);//Adds it to animation
				}
			}
			
			currentId = this->sequence.begin();//Sets current frame id
			if (currentId != this->sequence.end()) current = get_iterator <image, animation> (this, *currentId);//Sets new frame
		
			return true;//Returns true
		}
		
		return false;//Else returns false
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();//Sets base data
		
		result.set("sequence", join < deque<string> > (sequence, ","));//Sets sequence variable
		result.set("playOnce", playOnce);//Sets play once
		
		iterator i;//Iterator for frames
		for (i = begin(); i != end(); ++i) result.o.push_back(i->toScriptObj());//Adds each frame to result
		
		return result;//Returns result
	}
};

//Animation set class
//	based on a deque of animations, plus indication of the current animation
//
//	note: maybe, in future script commands support should be added
class animSet: public deque<animation>, public objectBased {
	public:
	deque<image> frames;//Common frames
	iterator current;//Iterator to current animation
	string queued;//Next animation id (for playOnce anims)
	
	//Constructor
	animSet(){
		id = "";
		type = OBJTYPE_ANIMSET;
		
		current = begin();
		queued = "";
	}
	
	//Function to set animation
	void setAnimation(string id, bool force = false){
		iterator next = get_iterator <animation> (this, id);//Next animation
		
		if (next == end() || next->id == current->id) return;//Exits function if same animation or is last (not found)
		
		if (force || !current->playOnce){//If has to set directly next animation
			next->restart();//Restarts animation
			current = next;//Sets current animation
		}
		
		else //Else
			queued = id;//Queues animation
	}
	
	//Print function
	//	offset is related to topleft corner
	void print(SDL_Surface* target, int x, int y){
		if (current != end()) current->print(target, x, y);//Prints current animation
	}
	
	//Print function
	//	offset is related to centre
	void print_centre(SDL_Surface* target, int x, int y){
		if (current != end()) current->print_centre(target, x, y);//Prints current animation
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succedeed loading the base object
			deque<object>::iterator i;//Iterator for sub-objects
			for (i = o.o.begin(); i != o.o.end(); ++i){//For each sub-object in given object
				if (i->type == OBJTYPE_IMAGE){//If object is an image
					image newImage;//New image
					
					if (newImage.fromScriptObj(*i))//If loaded image successfully
						frames.push_back(newImage);//Adds to common frames
				}
				
				if (i->type == OBJTYPE_ANIMATION){//If object is an animation
					animation newAnim;//New animation
					
					if (frames.size() > 0) newAnim = animation(&frames);//Adds common frames if any
					
					if (newAnim.fromScriptObj(*i))//If loaded animation successfully
						push_back(newAnim);//Adds animation to set
				}
			}
		
			current = begin();//Sets first animation
		
			return true;//Returns true
		}
	
		return false;//Returns false if failed
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();//Saves base content data
		
		iterator i;//Iterator
		for (i = begin(); i != end(); ++i) result.o.push_back(i->toScriptObj());//Adds each animation to result
		
		return result;//Returns result
	}
	
	//Function for next frame
	void next(){
		if (current->playOnce && current->ended && queued != ""){//If has to play once animation and has finished it
			setAnimation(queued, true);//Sets queued animation
		}
		
		else current->next();//Next frame for current animation
	}
};

#endif