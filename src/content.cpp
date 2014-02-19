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

#include "content.h"

list<cachedFile> cachedFiles;

xml_document* getCachedFile ( string path ) {
	for (list<cachedFile>::iterator i = cachedFiles.begin(); i != cachedFiles.end(); i++)
		if (i->path == path) return i->file;
		
	if (!ifstream(path.c_str())) { LOG_ERR ("invalid file path " << path); return NULL; }
		
	cachedFile newFile;
	newFile.path = path;
	newFile.file = new xml_document;
	newFile.file->load_file(path.c_str());
	
	cachedFiles.push_back(newFile);
	
	return newFile.file;
}

//Content constructor
content::content(){
	id = "";
	type = "";
	
	userDefined = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
	userDefined->ref();
	userDefined->setString("userDefined");
}

//Content constructor
content::content (string id){
	this->id = id;
	type = "";
	
	userDefined = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
	userDefined->ref();
	userDefined->setString("userDefined");
}

content::content ( const content &c ) {
	this->id = c.id;
	this->type = c.type;
	
	userDefined = new CScriptVar(TINYJS_BLANK_DATA, SCRIPTVAR_OBJECT);
	userDefined->ref();
	userDefined->setString("userDefined");
}

//Content destructor
content :: ~content(){
	userDefined->unref();
}

//Content function to load from XML node
void content::load(xml_node source){
	if (!source) return;
	
	//Extern file attribute check
	//If the externFile attribute is specified, the object will be loaded from the
	//specified file. If any data is specified within the source node, it will overwrite
	//the loaded data. The node to pick is accessed via the XPath expression specified within
	//the attribute importedNode
	xml_attribute externFile = source.attribute("externFile");//Gets optional extern file attribute
	
	//Imported node check
	//If an imported node is specified without an extern file, it will be searched inside the source
	//node passed as argument to the load function
	xml_attribute importedNode = source.attribute("importedNode");//Gets imported node attribute
	
	if (importedNode){//If a node is imported
		xpath_node n;//Node to import
		
		if (externFile){//If both extern file and imported node are specified
			string path = preprocessFilePath(externFile.value());
			if ( xml_document* file = getCachedFile ( preprocessFilePath(path.c_str()) ) ) {
				n = file->select_single_node(importedNode.value());//Gets required node
			}
			
			else LOG_ERR("Couldn't get file " << path);
		}
		
		else {//If only imported node is specified
			n = source.parent().select_single_node(importedNode.value());//Gets required node
		}
		
		if (n){
			load(n.node());//Loads from node
		}
	}	
	
	id = source.name();
	if (xml_attribute a = source.attribute("type")) type = a.value();
}

//Content save function
void content::save(xml_node *target){
	target->set_name(id.c_str());
	if (type != "") target->append_attribute("type") = type.c_str();
}

//Content function to load from file
void content::load_file(string source){
	source = preprocessFilePath(source);
	if (!ifstream(source.c_str())) LOG_ERR("invalid file path " << source);
	
	xml_document doc;//Document to load
	doc.load_file(source.c_str());//Loads document
	
	if (xml_node n = doc.first_child()) load(n);//Loads from first child
}

//Function to store into a javascript variable
void content::toJSVar(CScriptVar* c){
	c->findChildOrCreate("id")->var->setString(id);
	c->findChildOrCreate("type")->var->setString(type);
	
	if (userDefined) c->addChildNoDup("userDefined", userDefined);
}

//Function to load from javascript variable
void content::fromJSVar(CScriptVar* c){
	if (CScriptVarLink *v = c->findChild("id")) id = v->var->getString();
	if (CScriptVarLink *v = c->findChild("type")) type = v->var->getString();
	
	//NOTE:
	//User defined variable NOT LOADED, since somehow it would cause errors
	//Still, any change to that variable within a script is applied to the
	//userDefined member if the variable is added via content::toJSVar(CScriptVar*)
	//for the variable is added by reference
}

//Operator []
CScriptVar* content :: operator [] (string varName){
	return userDefined->findChildOrCreate(varName, SCRIPTVAR_INTEGER)->var;
}

//Operator ==
bool content :: operator == (content c){ return id == c.id; }

//Function to get last child of xml_node with requested name
xml_node lastChild(xml_node source, const char* name){
	for (xml_node n = source.child(name); n; ){
		xml_node m = n.next_sibling(name);
		
		if (!m) return n;
		else n = n.next_sibling(name);
	}
	
	return xml_node();
}
