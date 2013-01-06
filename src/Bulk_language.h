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

//Function to load language database from a file
void loadLanguagesDB(string path){
	fileData source (path);//Source file, loaded and processed
	object database = source.objGen(path);//Object generated from the file
	
	deque<object>::iterator i;//Iterator
	for (i = database.o.begin(); i != database.o.end(); i++)//For each object in database
		if (i->type == OBJTYPE_LANGUAGE) languagesDB.push_back(*i);//Adds object to database if it is a language
}

//Function to get a text from current language
string getText(string id){
	if (currentLang){//If current language is set
		var* v = get <var, deque<var> > (&currentLang->v, id);//Requested variable
		if (v) return v->value;//Returns variable value if existing
	}
	
	return id;//Returns the id string if didn't find anything
}

#endif