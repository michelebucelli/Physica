//Buch's Library Kollection
//Localization handling header

#ifndef BULK_LOCALE
#define BULK_LOCALE

//Script object types
#define OBJTYPE_LANGUAGE		"language"

//Macros
#define SETLANG(ID)				currentLang = get <language, list<language> > (&languagesDB, ID)//Macro to set current language

typedef object language;//Redefines object class as language

list <language> languagesDB;//Languages database
language *currentLang = NULL;//Current language

//Function to add language object to database
void addLang(language l, bool namespaced = false, string namespaceId = ""){
	language *o = get <language> (&languagesDB, l.id);//Requested language
	
	if (namespaced){//If language is namespaced
		if (o){//If existing
			l.id = namespaceId;//Sets namespace id
			o->o.push_back(l);//Adds to language
		}
		
		else {//If not existing
			language ln;//New language
			ln.id = l.id;//Sets id
			
			l.id = namespaceId;//Sets namespace id
			ln.o.push_back(l);//Adds sub-language
			
			languagesDB.push_back(ln);//Adds new language
		};
	}
	
	else {//If not namespaced
		if (o) *o += l;//If existing, adds to language
		else languagesDB.push_back(l);//Else adds to database
	}
}

//Function to load language database from a file
object* loadLanguagesDB(string path){
	if (!ifstream(path.c_str())) return NULL;//If failed opening, quits
	
	fileData source (path);//Source file, loaded and processed
	object *database = new object(source.objGen(path));//Object generated from the file
	
	deque<object>::iterator i;//Iterator
	for (i = database->o.begin(); i != database->o.end(); i++)//For each object in database
		if (i->type == OBJTYPE_LANGUAGE) addLang(*i);//Adds object to database if it is a language
		
	return database;
}

//Function to get a text from current language
string getText(string id, ...){
	va_list l;//Argument list
	va_start (l, id);//Stores args
	
	if (currentLang){//If current language is set
		var* v = currentLang->getVar(id);//Requested variable
		
		if (v){//If variable was found
			string s = v->value;//Retrieved string
			int n = 0;//Counter of total parameters
			int i;//Counter
			
			while (true){//Loops
				if (s.find("[" + toString(n) + "]") == s.npos) break;//Exits loop if token wasn't found
				n++;//Increases counter
			}
			
			for (i = 0; i < n; i++){//For each parameter
				string tok = "[" + toString(i) + "]";//Token to replace
				s.replace(s.find(tok), tok.size(), va_arg(l, const char*));//Replaces string
			}
			
			return s;//Returns string
		}
	}
	
	return id;//Returns the id string if didn't find anything
}

#endif