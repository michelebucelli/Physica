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

#include "game.h"

controls::controls(){
	up = SDL_SCANCODE_UP;
	left = SDL_SCANCODE_LEFT;
	right = SDL_SCANCODE_RIGHT;
}

void controls::load(xml_node source){
	content::load(source);
	
	if (xml_attribute a = source.attribute("up")) up = (SDL_Scancode) a.as_int();
	if (xml_attribute a = source.attribute("left")) left = (SDL_Scancode) a.as_int();
	if (xml_attribute a = source.attribute("right")) right = (SDL_Scancode) a.as_int();
}

void controls::save(xml_node *target){
	content::save(target);
	
	target->append_attribute("up") = up;
	target->append_attribute("left") = left;
	target->append_attribute("right") = right;
}

controls playerControls;

camera::camera(){
	position = vect (0, 0);
	speed = vect (0, 0);
	
	destination = NULL;
	
	range = 5;
	rangeFactor = 0.1;
	speedModule = 10;
	maxSpeed = 60;
}

void camera::move (double t){
	vect d = destination->position - position;//Distance vector
		
	double curS = speedModule + (d.module() - range) * rangeFactor;
	if (curS > maxSpeed) curS = maxSpeed;
	
	double dt = (destination->speed * t + destination->force / destination->mass / 2 * 0.5 * t * t).module();
	double cdt = curS * t;
	
	if (cdt >= d.module()) position = destination->position;
	else if (d.module() >= range){ speed = d.setModule(curS); position += speed * t; }//Moves
	else if (dt < range){ position = destination->position; speed = vect (0,0); }//Centers
	else speed = vect(0,0);//Stops if too close
}

_rect camera::viewport(int w, int h){
	return _rect (position.x - w / 2, position.y - w / 2, w, h);
}

_rules::_rules(){
	jumpImpulse = 30;
		
	groundSpeed = 30;
	groundForce = 15;
	groundDamping = 0.8;
	
	airSpeed = 15;
	airForce = 7;
	
	gravity = {0,10};
	
	jumpCount = 2;
	
	mask = 0;
}

rules::rules(){
}

void rules::load(xml_node source){
	content::load(source);
	
	mask  = 0;
	
	if (xml_attribute a = source.attribute("jumpImpulse")){ jumpImpulse = a.as_double(); mask  != 0b10000000; }
	if (xml_attribute a = source.attribute("groundSpeed")){ groundSpeed = a.as_double(); mask  != 0b01000000; }
	if (xml_attribute a = source.attribute("groundForce")){ groundForce = a.as_double(); mask  != 0b00100000; }
	if (xml_attribute a = source.attribute("groundDamping")){ groundDamping = a.as_double(); mask  != 0b00010000; }
	if (xml_attribute a = source.attribute("airSpeed")){ airSpeed = a.as_double(); mask  != 0b00001000; }
	if (xml_attribute a = source.attribute("airForce")){ airForce = a.as_double(); mask  != 0b00000100; }
	if (xml_attribute a = source.attribute("gravity")){ gravity.fromString(a.value()); mask  != 0b00000010; }
	if (xml_attribute a = source.attribute("jumpCount")){ jumpCount = a.as_int(); mask  != 0b00000001; }
}

_rules _rules::operator + (_rules r){
	_rules result = *this;
		
	if (r.mask  & 0b10000000) result.jumpImpulse = r.jumpImpulse;
	if (r.mask  & 0b01000000) result.groundSpeed = r.groundSpeed;
	if (r.mask  & 0b00100000) result.groundForce = r.groundForce;
	if (r.mask  & 0b00010000) result.groundDamping = r.groundDamping;
	if (r.mask  & 0b00001000) result.airSpeed = r.airSpeed;
	if (r.mask  & 0b00000100) result.airForce = r.airForce;
	if (r.mask  & 0b00000010) result.gravity = r.gravity;
	if (r.mask  & 0b00000001) result.jumpCount = r.jumpCount;
	
	result.mask  = result.mask  | r.mask ;
	
	return result;
}

void _rules::operator += (_rules r){
	*this = *this + r;
}

rules defaultRules, loadedRules;

area::area(){
}

void area::load(xml_node source){
	rect::load(source);
	
	if (xml_node n = source.child("rules")) areaRules.load(n);
}

level::level(){
	twoStarsTime = 0;
	threeStarsTime = 0;
	
	path = "";
}

void level::load(xml_node source){
	scene::load(source);
	
	if (xml_attribute a = source.attribute("twoStarsTime")) twoStarsTime = a.as_int();
	if (xml_attribute a = source.attribute("threeStarsTime")) threeStarsTime = a.as_int();
}

void level::print (SDL_Renderer* target, int x, int y, bool hidden){
	//Prints areas
	for (list<area>::iterator a = areas.begin(); a != areas.end(); a++){
		SDL_Rect r = *a;
		
		r.x += x;
		r.y += y;
		
		SDL_RenderDrawRect(target, &r);
	}
	
	printScene(target, this, x, y, hidden);
}

void level::reload(){
	entities.clear();
	links.clear();
	
	load_file(path);
}

level* level::copy(){
	level* result = new level (*this);
	
	result->entities.clear();
	result->links.clear();
	
	for (list<entity*>::iterator e = entities.begin(); e != entities.end(); e++){
		entity* ne = NULL;
		
		if ((*e)->eType == etype_box)
			ne = new box(* (box*) (*e));
			
		if (ne) result->entities.push_back(ne);
	}
	
	
	//MISSING LINK COPYING
	
	return result;
}

achievement::achievement(){
	name = "";
	info = "";
	checkOnce = true;
	expression = "";
}

achievement::achievement (string id, string name, string info, string verify, image icon, bool checkOnce){
	this->id = id;
	
	this->name = name;
	this->info = info;
	
	this->expression = verify;
	
	this->icon = icon;
	this->icon.id = "icon";
	
	this->checkOnce = checkOnce;
}

void achievement::load(xml_node source){
	content::load(source);
	
	if (xml_attribute a = source.attribute("name")) name = a.value();
	if (xml_attribute a = source.attribute("info")) info = a.value();
	if (xml_attribute a = source.attribute("expression")) expression = a.value();
	if (xml_node n = source.child("icon")) icon.load(n);
	if (xml_attribute a = source.attribute("checkOnce")) checkOnce = a.as_bool();
}

////TO DO//////////////
bool achievement::verify(CTinyJS* js){
	CScriptVarLink result = js->evaluateComplex ( expression );
	return result.var->getBool();
}

list<achievement> globalAchievements;

achievement* getAchievement ( string id ) {
	for ( list<achievement>::iterator i = globalAchievements.begin(); i != globalAchievements.end(); i++ )
		if ( i->id == id ) return &*i;
		
	return NULL;
}

void loadAchievements () {
	xml_document d;
	d.load_file(FILE_ACHIEVEMENTS);
	
	xml_node n = d.child("achievements");
	
	for ( xml_node m = n.first_child(); m; m = m.next_sibling() ) {
		achievement a;
		a.load(m);
		
		globalAchievements.push_back(a);
	}
}

levelProgress::levelProgress(){
	time = -1;
	deaths = 0;
	rating = 0;
}

void levelProgress::load(xml_node source){
	content::load(source);
	
	if (xml_attribute a = source.attribute("unlocked")) unlocked = a.as_bool();
	if (xml_attribute a = source.attribute("time")) time = a.as_int();
	if (xml_attribute a = source.attribute("deaths")) deaths = a.as_int();
	if (xml_attribute a = source.attribute("rating")) rating = a.as_int();
}

void levelProgress::save(xml_node* target){
	target->append_attribute ( "time" ) = time;
	target->append_attribute ( "deaths" ) = deaths;
	target->append_attribute ( "rating") = rating;
}

string levelProgress::packId(){
	int dot = id.find(".");
	
	if (dot != id.npos) return id.substr(0, dot);
	else return "";
}

globalProgress::globalProgress(){
	globalDeaths = 0;
	globalTime = 0;
}

void globalProgress::load(xml_node source){
	if (xml_node n = source.child("levels")){
		for (xml_node l = n.first_child(); l; l = l.next_sibling()){
			levelProgress p;
			p.load(l);
			push_back(p);
		}
	}
	
	if (xml_attribute a = source.attribute("achievements")){
		string s = a.value();
		int b = s.find(",");
		int c = 0;
		
		unlockedAch.clear();
		
		while ( b != s.npos ) {
			unlockedAch.push_back(s.substr(c, b - c));
			
			c = b + 1;
			b = s.find(",", b + 1);
		}
			
		unlockedAch.push_back(s.substr(c));
	}
	
	if (xml_attribute a = source.attribute("deaths")) globalDeaths = a.as_int();
	if (xml_attribute a = source.attribute("time")) globalTime = a.as_double();
}

void globalProgress::save (xml_node *target){
	xml_node levels = target->append_child("levels");
	
	for ( iterator i = begin(); i != end(); i++ ) {
		xml_node n = levels.append_child(i->id.c_str());
		i->save(&n);
	}
	
	string achString = "";
	for ( deque<string>::iterator i = unlockedAch.begin(); i != unlockedAch.end(); i++ )
		achString += "," + *i;
	
	achString.erase(achString.begin());
	
	target->append_attribute ("achievements") = achString.c_str();
	target->append_attribute ("deaths") = globalDeaths;
	target->append_attribute ("time") = globalTime;
}

void globalProgress::fillProgress(string id, int time, int deaths, int rating){
	iterator i = find (begin(), end(), content(id));
	levelProgress* p = i == end() ? NULL : &*i;
		
	if (!p){
		levelProgress pr;
				
		//Sets members
		pr.id = id;
		pr.unlocked = true;
		pr.time = time;
		pr.deaths = deaths;
		pr.rating = rating;
		
		push_back(pr);
	}
	
	else {
		bool set = p->time > 0;//True if level has already been set
		
		//Picks best
		if (p->time > time || !set) p->time = time;
		if (p->deaths > deaths || !set) p->deaths = deaths;
		if (p->rating < rating || !set) p->rating = rating;
	}
}

int globalProgress::getRating(string id){
	iterator i = find (begin(), end(), content(id));
	levelProgress* p = i == end() ? NULL : &*i;
		
	if (p) return p->rating;
	else return 0;
}

bool globalProgress::canPlay(string id){
	iterator i = find (begin(), end(), content(id));
	levelProgress* p = i == end() ? NULL : &*i;
	
	if (p && p->unlocked) return true;
	else return false;
}

void globalProgress::unlock(string id){
	if (!canPlay(id)){
		iterator i = find (begin(), end(), content(id));
		levelProgress* p = i == end() ? NULL : &*i;
		
		if (p) p->unlocked = true;//Unlocks
			
		else {
			levelProgress pr;//New progress
			
			//Sets members
			pr.id = id;
			pr.unlocked = true;
			
			push_back(pr);//Adds to data
		}
	}
}

void globalProgress::verifyAchs(){
	CTinyJS* js = new CTinyJS();
	currentGame.storeVariables(js);
	
	//Verifies global achievements
	for ( list<achievement>::iterator i = globalAchievements.begin(); i != globalAchievements.end(); i++ ) {
		if ( i->checkOnce && find ( unlockedAch.begin(), unlockedAch.end(), i->id ) != unlockedAch.end() ){
			continue;
		}
		
		else if ( i->verify ( js ) ) {
			LOG("Unlocked achievement '" << i->id << "'");
			showUnlockedAchievement ( &*i );
			unlockedAch.push_back(i->id);
		}
	}
	
	delete js;
}

////TO DO//////////////
int globalProgress::completed(string id){
}

////TO DO//////////////
int globalProgress::percent(string id){
}

////TO DO//////////////
void globalProgress::check(){
	iterator i;
	
	for (i = begin(); i != end(); i++){
		string packId = "";//Pack id

		packId = i->id.substr(0, i->id.find("."));//Pack id

		levelPack* s = getPack (packId);//Gets level pack

		if (!s){//If pack is invalid
			i = erase(i);//Erases element
			i--;//Goes back
		}
	}
	
	for ( deque<string>::iterator i = unlockedAch.begin(); i != unlockedAch.end(); i++ ) {
		if ( !getAchievement(*i) ) {
			i = unlockedAch.erase(i);
			i--;
		}
	}
}

//Current progress
globalProgress progress;

game::game(){
	player = NULL;
	goal = NULL;
	
	releasedUp = true;
	playerJumps = 0;
	
	paused = false;
	lastFrameTime = 0;
	
	currentLevelPack = NULL;
	currentLevel = NULL;
	levelIndex = 0;
	
	completed = false;
	
	gameRules.mask = 0b11111111;
}

void game::handleControls(Uint8* keys){
	_rules currentRules = getAreaRules();//Gets rules
		
	if (keys && player){//If keys array is valid and there's a player
		int lowestSensor = 0;//Lowest player sensor
		int i;//Counter
		
		bool up = int(keys[playerControls.up]) == 1;
		bool right = int(keys[playerControls.right]) == 1;
		bool left = int(keys[playerControls.left]) == 1;
		
		for (i = 0; i < player->sensors.size(); i++)//For each sensor in player
			if ((currentRules.gravity.y > 0 && player->sensors[i]->y > player->sensors[lowestSensor]->y) || (currentRules.gravity.y < 0 && player->sensors[i]->y < player->sensors[lowestSensor]->y)) lowestSensor = i;//Sets lowest sensor
		
		bool ground = player->checkSensor(lowestSensor);//True if touching the ground
		
		list<collision>::iterator c;//Collision iterator
		for (c = this->c.begin(); c != this->c.end(); c++){//For each collision
			if ((c->a == player || c->b == player) && ((currentRules.gravity.y > 0 && c->p.y > player->position.y) || (currentRules.gravity.y < 0 && c->p.y < player->position.y)) && abs(c->p.x - player->position.x) < ((box*) player)->w / 3) ground = true;//Sets ground flag if touching lower entity
		}
		
		if (!up) releasedUp = true;//Resets released jump flag if jump is not pressed
		if (ground) playerJumps = 0;//Resets jump count
		
		if (keys[playerControls.up] && releasedUp && (playerJumps < currentRules.jumpCount || currentRules.jumpCount < 0)){//If pressed up and can jump
			player->speed.y = 0;//Stops on y
			player->applyImpulse(player->position, {0, -currentRules.jumpImpulse * currentRules.gravity.y / abs(currentRules.gravity.y)});//Applies impulse
			
			playerJumps++;//Increases jump count
			releasedUp = false;//Not released jump
		}
		
		if (right){//If pressing right
			if (ground && player->speed.x < currentRules.groundSpeed)//If on ground and moving slower
				player->applyForce(player->position, {currentRules.groundForce, 0});//Applies force
				
			else if (!ground && player->speed.x < currentRules.airSpeed)//If on air and moving slower
				player->applyForce(player->position, {currentRules.airForce, 0});//Applies force
		}
		
		if (left){//If pressing left
			if (ground && player->speed.x > -currentRules.groundSpeed)//If on ground and moving slower
				player->applyForce(player->position, {-currentRules.groundForce, 0});//Applies force
				
			else if (!ground && player->speed.x > -currentRules.airSpeed)//If on air and moving slower
				player->applyForce(player->position, {-currentRules.airForce, 0});//Applies force
		}
		
		if (!(right ^ left) && ground) player->applyForce(player->position, {-player->speed.x * currentRules.groundDamping, 0});//Applies damping on ground
	}
}

_rules game::getAreaRules(){
	_rules result = defaultRules + loadedRules + currentLevelPack->lpRules + currentLevel->lvlRules;//Stacks rules without considering areas
	
	for (list<area>::iterator a = currentLevel->areas.begin(); a != currentLevel->areas.end(); a++)
		if (a->isInside(player->position.x, player->position.y)) result += a->areaRules;
		
	return result;
}

_rules game::getAreaRules(vect p){
	_rules result = defaultRules + loadedRules + currentLevelPack->lpRules + currentLevel->lvlRules;//Stacks rules without considering areas
	
	for (list<area>::iterator a = currentLevel->areas.begin(); a != currentLevel->areas.end(); a++)
		if (a->isInside(p.x, p.y)) result += a->areaRules;
		
	return result;
}

void game::print(SDL_Renderer* target, int x, int y){
	if (currentLevel){
		currentLevel->print(target, x - cam.position.x, y - cam.position.y);
	}
}

void game::setup(int levelIndex, bool death, bool moveCam){
	currentLevel = currentLevelPack->levels[levelIndex];
	this->levelIndex = levelIndex;
	
	setup(death, moveCam);
	
	if (!death){
		lastFrameTime = SDL_GetTicks();
		time = 0;
		deaths = 0;
	}
}

void game::setup(bool death, bool moveCam){
	currentLevel->reload();
	
	player = currentLevel->getEntity("player");
	goal = currentLevel->getEntity("goal");
	
	cam.destination = player;
	if (moveCam) cam.position = player->position;
	
	releasedUp = true;
	playerJumps = 0;
	
	paused = false;
	completed = false;
}

void game::reset(){
	setup(levelIndex, true, false);
	deaths++;
	progress.globalDeaths++;
}

bool game::next(){
	if (levelIndex < currentLevelPack->levels.size() - 1){ setup(levelIndex + 1); return true; }
	else { setup(levelIndex, false, true); return false; }
}

void game::checkRelevant(){
	if (!player) return;
	
	for (list<collision>::iterator i = c.begin(); i != c.end(); i++){
		if ((i->a == player && i->b->special == "hazard") || (i->a->special == "hazard" && i->b == player)){
			reset();
			break;
		}
		
		if (goal && ((i->a == player && i->b == goal) || (i->a == goal && i->b == player))){
			if (!completed) {
				progress.fillProgress ( currentLevelPack->id + "." + currentLevel->id, time, deaths, rating() );
			}
			
			completed = true;
			break;
		}
	}
}

void game::step(double t){
	if (currentLevel){
		for (list<entity*>::iterator i = currentLevel->entities.begin(); i != currentLevel->entities.end(); i++){
			(*i)->applyForce((*i)->position, getAreaRules((*i)->position).gravity * (*i)->mass);
		}
		
		c = currentLevel->step(t);
		checkRelevant();
	}
}

void game::frame(double t, Uint8* keys){
	if (paused || completed){ lastFrameTime = SDL_GetTicks(); return; }//Exits function if paused
	
	cam.move(t);
	step(t);
	
	currentLevel->resetForces();
	handleControls(keys);
	
	time += SDL_GetTicks() - lastFrameTime;
	progress.globalTime += double(SDL_GetTicks() - lastFrameTime) / 60000;//Increases global time counter
	
	lastFrameTime = SDL_GetTicks();
}

int game::rating(){
	if (!currentLevel) return 0;
	
	if (time / 1000 + deaths < currentLevel->threeStarsTime) return 3;
	else if (time / 1000 + deaths < currentLevel->twoStarsTime) return 2;
	else return 1;
}

void game::storeVariables ( CTinyJS* js ) {
	CScriptVarLink* global = js->root->addChild("global");
	
	global->var->findChildOrCreate("deaths")->var->setInt(progress.globalDeaths);
	global->var->findChildOrCreate("time")->var->setInt(progress.globalTime);
}

game currentGame;

levelPack::levelPack(){
	order = 0;
}

void levelPack::load(xml_node source){
	content::load(source);
	
	if (xml_attribute a = source.attribute("order")) order = a.as_int();
	if (xml_attribute a = source.attribute("name")) name = a.value();
	if (xml_node n = source.child("icon")) icon.load(n);
	
	for (xml_node n = source.child("level"); n; n = n.next_sibling("level"))
		if (xml_attribute a = n.attribute("file")) levelFiles.push_back(preprocessFilePath("(@levels)/" + id + "/" + a.value()));
}

void levelPack::loadLevels(){
	//We remove all levels first, because if one of the levels has been changed after
	//loading (maybe within the editor) this will allow to play the latest version
	for (deque<level*>::iterator i = levels.begin(); i != levels.end(); i++){
		delete *i;
	}
	
	levels.clear();
	
	for (deque<string>::iterator i = levelFiles.begin(); i != levelFiles.end(); i++){
		if (!ifstream(i->c_str())){
			LOG_ERR("file " << *i << " is not a valid level file");
			continue;
		}
		
		level* l = new level;
		l->load_file(*i);
		l->path = *i;
		levels.push_back(l);
	}
}

bool levelPack_compare(levelPack* a, levelPack* b){
	return a->order < b->order;
}

list <levelPack*> levelPacks;//Loaded level packs

levelPack* getPack ( string id ) {
	for ( list<levelPack*>::iterator i = levelPacks.begin(); i != levelPacks.end(); i++ )
		if ((*i)->id == id) return *i;
		
	return NULL;
}

void loadLevelPacks(){
	list<string> packs = listFiles(FOLDER_LEVELS);
	
	for (list<string>::iterator i = packs.begin(); i != packs.end(); i++){
		if (i->substr(i->size() - 4) == ".xml") continue;
		
		if (ifstream((string(FOLDER_LEVELS) + "/" + *i + "/packInfo.xml").c_str())){
			levelPack* p = new levelPack;
			p->load_file(string(FOLDER_LEVELS) + "/" + *i + "/packInfo.xml");
			levelPacks.push_back(p);
		}
		
		else LOG_WARN("packInfo.xml not found for pack " << *i);
	}
	
	levelPacks.sort(levelPack_compare);
}

void scSetupLevel(CScriptVar* v, void* userdata){
	int index = v->getParameter("index")->getInt();
	
	REPORT_TIME(currentGame.setup(index, false, true), "Opening level " << index);
}

void registerGameFuncs(CTinyJS* js){
	js->addNative("function setupLevel(index)", scSetupLevel, NULL);
}
