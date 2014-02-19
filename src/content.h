/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
GENERAL CONTENT HEADER
This header defines a basic content class, that includes some members
referrable to any kind of content (identifier, content type), as well
as member functions that load the object data from XML (basing upon
pugixml library)

The class is meant to be inherited by any XML loaded class
*/

#ifndef _CONTENT
#define _CONTENT

#include "PugiXML/pugixml.hpp"
#include "TinyJS/TinyJS.h"
#include "files.h"
#include "log.h"
#include <fstream>
#include <string>
#include <list>

using namespace std;
using namespace pugi;

class cachedFile {
	public:
	string path;
	xml_document *file;
};

extern list <cachedFile> cachedFiles;

xml_document* getCachedFile(string);

class content {
	public:
	
	//Content type: a string to identify the type of the content
	string type;
	
	//Content identifier (the object name)
	string id;
	
	//User defined variables
	//These can be declared/assigned within javascript scripts and are stored inside the *.userDefined object variable
	CScriptVar* userDefined;
	
	content();
	content(const content&);
	content(string);
	~content();
	
	//Function to load from XML
	//To be redefined in any implementation and called by the function within the derived class
	virtual void load(xml_node);
	
	//Function to save to XML
	virtual void save(xml_node*);
	
	//Function to store into a javascript variable
	virtual void toJSVar(CScriptVar*);
	
	//Function to load from javascript variable
	void fromJSVar(CScriptVar*);
	
	//Function to load from file
	//The first node of the file will be used to load this control
	void load_file(string);
	
	//Operator [] to get user defined variable
	CScriptVar* operator [] (string);
	
	//Operator == (matching id)
	bool operator == (content);
};

xml_node lastChild(xml_node, const char*);

#endif
