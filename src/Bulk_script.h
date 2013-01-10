//Buch Library Kollection
//Script handling header

//This header provides a framework to load program content and settings
//from configuration files, following this syntax:
//		[OBJECT_TYPE] [OBJECT_NAME] {
//			[VARIABLE_NAME] = [VARIABLE_VALUE];
//		};

#ifndef _BULK_SCRIPT
#define _BULK_SCRIPT

class var;//Variable class prototype
class object;//Object class prototype
class fileData;//File data class prototype

//Keywords
#define PRESET_ID			"preset"//Preset object id

//Script command results
#define CMD_RES_NORMAL			0//Normal command result

//Searching function
//	returns a pointer to object with given id or null if not found
template <class type, class containerType> type* get (containerType *source, string id){
	typename containerType::iterator i;//Iterator
	
	for (i = source->begin(); i != source->end(); i++)//For each element in deque
		if (i->id == id) return &*i;//Returns the element if matching
	
	return NULL;//Returns null if not found
}

//Searching function
//	same as the above one, but with pointers
template <class type, class containerType> type* get_ptr (containerType *source, string id){
	typename containerType::iterator i;//Iterator
	
	for (i = source->begin(); i != source->end(); i++)//For each element in deque
		if ((*i)->id == id) return *i;//Returns the element if matching
	
	return NULL;//Returns null if not found
}

//Searching function
//	returns an iterator to object with given id or end iterator if not found
template <class type, class containerType> typename containerType::iterator get_iterator (containerType *source, string id){
	typename containerType::iterator i;//Iterator
	
	for (i = source->begin(); i != source->end(); i++)//For each element in deque
		if (i->id == id) return i;//Returns the iterator if matching
	
	return i;//Returns the iterator (end) if not found
}

//Variable class
//	used to store content variables; values are stored as strings
class var{
	public:
	string id;//Variable id
	string value;//Variable value
	
	//Constructor
	var(){
		id = "";
		value = "";
	}
	
	//Constructor
	template <class type> var(string id, type value){
		this->id = id;
		set(value);
	}
	
	//Function to set variable value
	template <class type> void set(type t){
		value = toString(t);//Sets value converted to string
	}
	
	//Function to return value as integer
	int intValue(){
		return atoi(value.c_str());
	}
	
	//Function to return value as double
	double doubleValue(){
		return atof(value.c_str());
	}
};

//Object class
//	like C structure, multiple variables stored in an only object
class object{
	public:
	string type;//Object type
	string id;//Object id
	
	deque<var> v;//Object variables
	deque<object> o;//Sub-objects
	
	object* parent;//Parent object
	
	//Constructor
	object(){
		type = "";
		id = "";
		parent = NULL;
	}
	
	//Constructor
	object(string type, string id){
		this->type = type;
		this->id = id;
		
		parent = NULL;
	}
	
	//Function to set the value of a variable
	//if the variable doesn't exist, the function creates it
	template <class type> void set(string id, type value){
		if (id.find(".") == id.npos){//If it is not a nested variable
			var* v = get <var, deque<var> > (&this->v, id);//Pointer to the variable
			
			if (v) v->set(value);//Sets the value if the variable exists

			else{//If not
				var newVar (id, value);//New variable
				this->v.push_back(newVar);//Adds the variable
			}
		}
		
		else {//Else
			object* child = get <object> (&this->o, id.substr(0, id.find(".")));//Child object containing the variable
			
			if (child)//If object exists
				child->set(id.substr(id.find(".") + 1), value);//Sets variable inside object
				
			else {//Else
				object newObj;//New object
				newObj.set(id.substr(id.find(".") + 1), value);//Sets variable inside new object
				o.push_back(newObj);//Adds object
			}
		}
	}
	
	//Function to create an empty object
	void createObj(string type, string id){
		if (id.find(".") == id.npos){//If not a nested object
			object newObj(type,id);//New object
			o.push_back(newObj);//Adds to 
		}
		
		else {//Else if sub-object exists
			object* child = get <object> (&o, id.substr(0, id.find(".")));//Child object
			
			if (child)//If child object exists
				child->createObj(type, id);//Creates the new object
				
			//Else does nothing, since no type for parent object has been specified and so it can't be created
		}
	}
	
	//Operator + to join two objects
	//If there are variables with the same id, they'll take the value of the second object (argument of the function)
	//If there are sub-objects with the same id, they'll be joined
	object operator + (object b){
		object result = *this;//Result
		
		deque<var>::iterator i;//Iterator for variables
		for (i = b.v.begin(); i != b.v.end(); i++)//For each variable of the other object
			result.set(i->id, i->value);//Sets result variable
		
		deque<object>::iterator j;//Iterator for objects
		for (j = b.o.begin(); j != b.o.end(); j++){//For each object of the other object
			object* ob = get <object, deque<object> > (&result.o, j->id);//Pointer to the required object
			
			if (ob) *ob += *j;//If the object already exists, joins the two objects
			else result.o.push_back(*j);//Else adds the object to the result
		}
		
		return result;//Returns the result
	}
	
	//Operator +=
	void operator += (object b){
		*this = *this + b;
	}
	
	//Function to read the object from script strings (only prototype; function defined below class fileData)
	void fromStrings(string type, string id, deque<string> lines);
	
	//Function to print the object onto a string
	string toString(int indent = 0){
		string result = "";//Result
		int i;//Counter to indent
		
		deque<object>::iterator l;//Iterator
		for (l = o.begin(); l != o.end(); l++){//For each object
			if (l->o.size() + l->v.size() == 0) continue;//Next object if this is empty
			
			for (i = 0; i < indent; i++) result += " ";//Indent
			result += l->type + " " + l->id + " {\n";//Object declaration
			result += l->toString(indent + 1);//Object content
			for (i = 0; i < indent; i++) result += " ";//Indent
			result += "};\n";//Close bracket
			
			if (l != o.end() - 1 || !v.empty()) result += "\n";
		}
		
		deque<var>::iterator j;//Iterator
		for (j = v.begin(); j != v.end(); j++){//For each variable
			if (j->value == "") continue;//Skips if empty
			
			for (i = 0; i < indent; i++) result += " ";//Indent
			result += j->id + " = " + j->value + ";\n";//Print variable
		}
		
		return result;//Returns the result
	}
} presetObjects;//Preset objects (can be recalled with inherit line in object descriptions)

//Specialized template for get function with objects
template <class containerType> object* get (containerType* source, string id){
	if (id.find(".") == id.npos)//If it is not a nested object
		return get <object, containerType> (source, id);//Returns object normally
		
	else {//Else
		object* child = get <containerType> (source, id.substr(0, id.find(".")));//Child object
		
		if (child)//If child exists
			return get <containerType> (&child->o, id.substr(id.find(".") + 1));//Returns object from child
		else return NULL;//Else returns null
	}
}

//File data class
//	used to read files and make content from them
class fileData{
	public:
	string fileName;//File id
	
	string readContent;//Content directly read from the file, all in one line
	deque<string> procContent;//Processed content, obtained by splitting readContent in semicolon-divided lines
	
	//Constructor
	fileData(){
		fileName = "";
		readContent = "";
	}
	
	//Constructor
	fileData(string path){
		read(path);//Reads file
		proc();//Processes the content
	}
	
	//File reading function
	//Reads the content of the file and stores it on a single line in readContent
	void read(string path){
		ifstream file(path.c_str());//Opens file for reading
		string tmpString;//Temporary string
		
		fileName = path;//Sets file id
		readContent = "";//Resets content
		
		#ifdef _ERROR//If the error handling file has been included
			if (!file.good() && errFunc)//If the file isn't good and there is an error function
				errFunc(SCRIPT_ERROR_FILENOTFOUND, "[script.h] File not found " + path);//Calls error function
		#endif
		
		while (file.good()){//While the file is good for reading
			getline(file, tmpString);//Reads line from file
			tmpString = tmpString.substr(0, tmpString.find("//"));//Removes comments
			readContent += " " + tmpString;//Adds the line to the content
		}
		
		file.close();//Closes file
	}
	
	//File processing function
	//Generates procContent
	void proc(){
		char* tok = strtok((char*) readContent.c_str(), ";");//First token
			
		while (tok){//While there are still tokens left
			procContent.push_back(string(tok));//Adds the token to the content
			tok = strtok(NULL, ";");//Next token
		}
	}
	
	//Function to generate file object; the parent object contains the child objects declared in the file
	object objGen(string id){
		object o;//New object
		o.fromStrings("file", id, procContent);//Generates the object from the content
		return o;//Returns the object
	}
};

//Function to load preset objects
void loadPresetObjects(string source){
	fileData sourceFile (source);//Source file
	presetObjects += sourceFile.objGen(PRESET_ID);//Generates object
}

//Function to read an object from script strings
void object::fromStrings(string type, string id, deque<string> lines){
	deque<string>::iterator i;//Iterator
	int lCount = 1;//Line counter (for error-finding purposes)
	
	//Sets type and id
	this->type = type;
	this->id = id;
	
	for (i = lines.begin(); i != lines.end(); i++, lCount++){//For each line
		deque<string> tokens = tokenize < deque<string> >(*i, " \t");//Splits line in tokens
		
		if (tokens.size() >= 3 && tokens[1] == "="){//Variable assignment
			set(tokens[0], i->substr(i->find(tokens[2], i->find("="))));//Sets the variable
		}
		
		else if (tokens.size() >= 3 && tokens[2] == "{"){//Object declaration
			object newObj;//New object
			newObj.parent = this;//Sets object parent
			
			deque<string> subLines;//Lines of the sub-object
			subLines.push_back(i->substr(i->find("{") + 1));//Adds first line
			
			int open = 1;//Open brackets counter
			deque<string>::iterator j;//Iterator for object end
			
			open += howMany(subLines[0], '{');//Counts open brackets of the first line
			open -= howMany(subLines[0], '}');//Counts close brackets of the first line
		
			for (j = i + 1; j != lines.end(); j++){//For each of the following lines
				open += howMany(*j, '{');//Open brackets
				open -= howMany(*j, '}');//Close brackets
				
				if (open <= 0) break;//Exit loop if all brackets have been closed
				if (open > 0) subLines.push_back(*j);//Else adds line to sub-object
			}
			
			newObj.fromStrings(tokens[0],tokens[1],subLines);//Creates object
			
			object* ob = get <object> (&o, tokens[1]);//Existing object
				
			if (!ob) o.push_back(newObj);//Adds object if not existing
			else *ob += newObj;//Else joins the two objects
			
			i = j;//Next line
			i--;//Goes a line backwards
		}
		
		else if (tokens.size() >= 2 && tokens[0] == "file"){//File insertion
			fileData f (i->substr(i->find(tokens[1])));//New file
			*this += f.objGen(tokens[1]);//Adds the content of the file to the object
		}
		
		else if (tokens.size() >= 2 && tokens[0] == "inherit"){//Inherit command: copies preset object
			object *inherited = get <object> (&presetObjects.o, tokens[1]);//Object to be inherited
			
			object* searching = parent;//Object in which inherited has to be searched
			while (!inherited && searching){//While object wasn't found and there's a searching object
				inherited = get <object> (&searching->o, tokens[1]);//Looks for object in parent object
				searching = searching->parent;//Goes to parent
			}
			
			if (inherited) *this += *inherited;//If matching, joins object
		}
	}
}

//Object-based class
//	used to represent something that can be loaded from or saved to a script object
//	ideally, should be inherited by a class which re-defines the member functions
class objectBased {
	protected:
	string type;//Object type
	
	public:
	string id;//Object id
	
	//Constructor
	objectBased(){
		id = "";
		type = "";
	}
	
	//Function to load from script object
	virtual bool fromScriptObj(object o){
		if (o.type == type){//If object type is matching
			id = o.id;//Sets id
			return true;//Returns true
		}
		
		return false;//Returns false if type didn't match
	}
	
	//Function to convert to script object
	virtual object toScriptObj(){
		object result (type, id);//Result object
		return result;//Returns result
	}
	
	//Function to get object type
	string getType() { return type; }
};

//Script class
//	based on a string deque, adds functions to save and load from strings
class script: public deque<string> {
	public:
	
	//Function to load from a string
	//	basically, splits the string into its lines
	void fromString(string source){
		*this = tokenize <script> (source, ",");//Tokenizes the string
	}
	
	//Function to save to a string
	string toString(){
		return join <script> (*this, ",");//Joins all lines into an only string and returns it
	}
};

//Scriptable object class
//	represents an object that can have scripts applied to it
//	ideally, should be inherited by a class which re-defines the member functions
class scriptable {
	public:
	
	//Function to apply a script command to the object
	virtual int applyScriptCommand (string commandName, deque<string> args){
		return CMD_RES_NORMAL;//Returns normal result
	}
	
	//Function to apply a script command to the object
	int applyScriptCommandLine (string command){
		deque<string> args = tokenize <deque<string> > (command, " \t");//Gets args
		string commandName = args[0];//Gets command name
		args.pop_front();//Removes first token
		
		return applyScriptCommand(commandName, args);//Applies script command
	}
	
	//Function to apply a script to the object
	void applyScript (script s){
		script::iterator i;//Iterator
		
		for (i = s.begin(); i != s.end(); i++){//For each line of script
			deque<string> tokens = tokenize < deque<string> > (*i, " \t");//Splits line in tokens
			
			string commandName = tokens[0];//Command name
			tokens.erase(tokens.begin());//Removes first token (command name) from arguments
			
			applyScriptCommand(commandName, tokens);//Applies script command
		}
	}
};

#endif