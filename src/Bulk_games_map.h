//Buch's Library Kollection for games
//Map handling header

#ifndef _BULK_GAMES_MAP
#define _BULK_GAMES_MAP

typedef enum {dir_none, dir_up, dir_down, dir_right, dir_left} direction;//Direction enumeration

//Prototypes
class pathSegment;//Path segment
class path;//Path
template <class> class baseMap;//Map

#define COORDINDEX(X,Y,W)		((X) + (Y) * (W))//Coordinates index macro

//Function for inverse direction
direction invert(direction d){
	switch (d){	
		case dir_up: return dir_down;
		case dir_right: return dir_left;
		case dir_down: return dir_up;
		case dir_left: return dir_right;
		case dir_none: return dir_none;
	}
}

//Path class
class path: public deque<direction> {
	public:
	
	//Operator to add path
	path operator + (path p){
		path result = *this;//Result
		
		int i;//Counter
		for (i = 0; i < p.size(); i++) result.push_back(p[i]);//Adds direction
		
		return result;//Returns result
	}
	
	//Increment function
	template <class type> void operator += (type p){
		*this = *this + p;//Adds to this
	}
};

//Map base class
//	the template argument is the tile class
template <class tile> class baseMap {
	public://Public members
	int w, h;//Map size
	tile* tiles;//Tiles
	
	deque<int> *values;//Distance values for tiles
	
	//Constructor
	baseMap(){
		w = 1;
		h = 1;
		tiles = NULL;
		values = NULL;
	}
	
	//Function to resize the map
	void resize(int w, int h){
		this->w = w;//Sets new width
		this->h = h;//Sets new height
		
		if (tiles) delete tiles;//Deletes tiles if they exist
		tiles = new tile [w * h];//Creates new tiles
		
		if (values) delete values;//Deletes values if they exist
		values = new deque<int> [w * h];//Creates new values
	}
	
	//Function to evaluate single tile
	void evaluate(int x, int y){
		deque<int>* d = &values[x + y * w];//Values deque for requested tile
		
		d->clear();//Clears current values
		
		int i;//Counter
		for (i = 0; i < w * h; i++){//For each tile
			d->push_back(-1);//Adds un-evaluated code to the values deque
		}
		
		if (!isFree(x, y)) return;//Exits function if requested tile is not free
		
		(*d)[x + y * w] = 0;//Sets source tile value
		
		int curValue = 1;//Value currently being set
		bool looping = true;//Flag indicating if has to loop (set to false when all tiles have been evaluated)
		while (looping){
			int tX, tY;//Tiles counters
			
			looping = false;//Sets flag to false (if sets a value, looping is set to true)
			
			for (tY = 0; tY < h; tY++){//For each row
				for (tX = 0; tX < w; tX++){//For each column
					if (!isFree(tX, tY) || (*d)[tX + tY * w] >= 0) continue;//Next loop if tile is not free
					
					else if (tY > 0 && (*d)[COORDINDEX(tX, tY - 1, w)] == curValue - 1){//If tile above is set to the previous value
						looping = true;//Sets looping to true
						(*d)[tX + tY * w] = curValue;//Sets this tile's value
					}
					
					else if (tX > 0 && (*d)[COORDINDEX(tX - 1, tY, w)] == curValue - 1){//If tile on the left is set to the previous value
						looping = true;//Sets looping to true
						(*d)[tX + tY * w] = curValue;//Sets this tile's value
					}
					
					else if (tY < h - 1 && (*d)[COORDINDEX(tX, tY + 1, w)] == curValue - 1){//If tile below is set to the previous value
						looping = true;//Sets looping to true
						(*d)[tX + tY * w] = curValue;//Sets this tile's value
					}
					
					else if (tX < w - 1 && (*d)[COORDINDEX(tX + 1, tY, w)] == curValue - 1){//If tile on the right is set to the previous value
						looping = true;//Sets looping to true
						(*d)[tX + tY * w] = curValue;//Sets this tile's value
					}
				}
			}
			
			curValue++;//Increments current value
		}
	}
	
	//Function to evaluate tiles
	void evaluate(){
		int x, y;//Coord counters
		
		for (y = 0; y < h; y++)//For each row
			for (x = 0; x < w; x++)//For each column
				evaluate(x, y);//Evaluates tile
	}
	
	//Function to get a pointer to a tile
	tile* getTile(int x, int y){
		if (x >= 0 && x < w && y >= 0 && y < h)//If coords are valid
			return &tiles[y * w + x];//Returns tile
			
		else return NULL;//Else returns NULL
	}
	
	//Function to determine if a tile is free or not
	//	to be overriden in derived class
	virtual bool isFree(int x, int y) = 0;
	
	//Function to get the best path from a point to another
	path* bestPath(int startX, int startY, int endX, int endY){
		if (startX < 0 || startX >= w || startY < 0 || startY >= h || endX < 0 || endX >= w || endY < 0 || endY >= h) return NULL;//Returns null if one of the points is not valid
		
		deque<int>* d = &values[startX + startY * w];//Values grid for start tile
		path* result = new path;//Result path
		
		int curValue = (*d)[endX + endY * w];//Current tile value, set to end tile
		if (curValue <= 0) return NULL;//Returns null if current value is lower than 0 (tile cannot be reached) or 0 (tiles are the same)
		
		int x = endX, y = endY;//Current coords
		
		while (curValue > 0){//While current value is positive (didn't reach start tile)
			if (y > 0 && (*d)[COORDINDEX(x, y - 1, w)] == curValue - 1){//If next tile is up
				y--;//Moves up
				result->push_front(dir_down);//Adds inverse direction to path
			}
			
			else if (y < h - 1 && (*d)[COORDINDEX(x, y + 1, w)] == curValue - 1){//If next tile is down
				y++;//Moves down
				result->push_front(dir_up);//Adds inverse direction to path
			}
			
			else if (x > 0 && (*d)[COORDINDEX(x - 1, y, w)] == curValue - 1){//If next tile is left
				x--;//Moves left
				result->push_front(dir_right);//Adds inverse direction to path
			}
			
			else if (x < w - 1 && (*d)[COORDINDEX(x + 1, y, w)] == curValue - 1){//If next tile is right
				x++;//Moves right
				result->push_front(dir_left);//Adds inverse direction to path
			}
			
			curValue--;//Decreases current value
		}
		
		return result;//Returns result
	}
	
	//Function to load values from string
	void valuesFromString(string source, int x, int y){
		deque<string> t = tokenize<deque<string> >(source, ",");//Splits source in tokens
		
		deque<int> *d = &values[x + y * w];//Values grid
		d->clear();//Clears grid
		
		int i;//Counter
		for (i = 0; i < w * h && i < t.size(); i++)//For each tile
			d->push_back(atoi(t[i].c_str()));//Adds value to values grid
	}
	
	//Function to convert values into string
	string valuesToString(int x, int y){
		string result = "";//Result
		
		deque<int>::iterator i;//Iterator
		for (i = values[x + y * w].begin(); i != values[x + y * w].end(); i++)//For each value of the grid
			result += "," + toString(*i);//Adds value to result string
			
		result.erase(0,1);//Removes first comma
		
		return result;//Returns result
	}
};

#endif