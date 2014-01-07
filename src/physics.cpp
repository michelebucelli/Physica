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

#include "physics.h"

entity::entity (){
	eType = etype_generic;
	mass = 0;
	
	axesCount = 0;
	
	damping_tr = 1;
	damping_rot = 1;
	
	lockTranslation = false;
	lockX = false;
	lockY = false;
	
	lockRotation = false;
	torque = 0;
	theta = 0;
	omega = 0;
	alpha = 0;
	
	e = 0;
	
	color = 0xFFFFFF;
	
	parent = NULL;
	
	print = true;
	special = "";

	light = false;
	lightColor = 0xFFFFFF;
}

void entity::resetForces (){
	force = vect (0,0);
	torque = 0;
}

void entity::translate (vect s){
	position += s;
	
	for (deque<vect*>::iterator i = nodes.begin(); i != nodes.end(); i++) **i += s;
	for (deque<vect*>::iterator i = sensors.begin(); i != sensors.end(); i++) **i += s;
}

void entity::rotate (double a){
	theta += a;
	
	for (deque<vect*>::iterator i = nodes.begin(); i != nodes.end(); i++)	**i = position + (**i - position).rotate(a);		
	for (deque<vect*>::iterator i = sensors.begin(); i != sensors.end(); i++)	**i = position + (**i - position).rotate(a);
}

void entity::applyForce (vect p, vect force){
	this->force += force;
	torque += (position - p).cross(force);
}

void entity::applyImpulse (vect p, vect impulse){
	speed += impulse / mass;
	omega += (position - p).cross(impulse) / inertia();
}

vect entity::pointSpeed(vect point){ return speed + (point - position).perp() * omega; }

vect* entity::addNode(vect p){
	vect* v = new vect;
	*v = p;
	
	nodes.push_back(v);
	return v;
}

int entity::addSensor(vect p){
	vect* v = new vect;
	*v = p;
	
	sensors.push_back(v);
	return sensors.size() - 1;
}

bool entity::locked(){ return lockTranslation && lockRotation; }

void entity::step(double t, double trDamp, double angDamp){
	//Applies individual damping factors
	trDamp *= damping_tr;
	angDamp *= damping_rot;
	
	//Translation
	if (!lockTranslation){
		vect oldAccel = accel;//Old acceleration (stored for speed integration)
		accel = (force) / mass;//Sets new acceleration
	
		//If damping would stop entity, we must check it and stop it manually,
		//for entity could go on moving in opposite direction
		//To do so, we check if damping changes final direction, and if so we set speed to (0,0)
	
		vect damp = speed + ((oldAccel + accel) / 2 + speed * trDamp) * t;//New speed with damping
		vect noDamp = speed + ((oldAccel + accel) / 2) * t;//New speed without damping
		
		vect direction = speed.setModule(1);//Motion direction (normalized)
		
		if ((damp.dot(direction) >= 0) == (noDamp.dot(direction) >= 0))//If damping wouldn't change direction
			speed = damp;//Sets speed with damping
		else speed = vect (0, 0);//Else stops
		
		vect tr = speed * t + (accel + oldAccel) / 2 * 0.5 * t * t;//Translation vector
		
		//Checks individual axis locks
		//That's not physically correct, as far as I know, but it's fine for our purposes
		if (lockX) tr.x = 0;
		if (lockY) tr.y = 0;
		
		translate(tr);
	}
	
	else speed = vect(0, 0);

	//Rotation
	if (!lockRotation){
		double oldAlpha = alpha;//Old angular acceleration (stored for angular speed integration)		
		alpha = (torque) / inertia();//Sets new angular acceleration
		
		//Same as with translation above
		
		double damp = omega + ((oldAlpha + alpha) / 2 + omega * angDamp) * t;//New angular speed with damping
		double noDamp = omega + ((oldAlpha + alpha) / 2) * t;//New angular speed without damping
		
		if (damp > 0 == noDamp > 0)//If damping wouldn't invert rotation
			omega = damp;//Sets angular speed with damping
		else omega = 0;//Else stops rotation
			
		rotate(omega * t + (alpha + oldAlpha) / 2 * 0.5 * t * t);
	}
	
	else omega = 0;
}

bool entity::checkSensor(int id){
	if (parent && id >= 0 && id < sensors.size()){
		vect* sensor = sensors[id];//Requested sensor
		
		//Tile coordinates of the requested sensor
		int tileX = floor(sensor->x / (parent->w / DIVISIONS));
		int tileY = floor(sensor->y / (parent->h / DIVISIONS));
		
		int cellIndex = tileX + tileY * DIVISIONS;
		
		if (tileX >= 0 && tileX < DIVISIONS && tileY >= 0 && tileY < DIVISIONS){
			//Iterates through entities in cell and
			//returns true if sensor is inside entity
			
			for (list<entity*>::iterator i = parent->cells[cellIndex].begin(); i != parent->cells[cellIndex].end(); i++)
				if (*i != this && (*i)->isInside(*sensor)) return true;
		}
	}
	
	return false;
}

void entity::clone(entity* target){
	*target = *this;
		
	deque<vect*>::iterator i;
	target->sensors.clear();
	target->nodes.clear();
	
	for (i = sensors.begin(); i != sensors.end(); i++) target->addSensor(**i);
	for (i = nodes.begin(); i != nodes.end(); i++) target->addNode(**i);
}

void entity::load(xml_node source){
	content::load(source);
	
	if (xml_attribute a = source.attribute("mass")) mass = a.as_double();
	if (xml_attribute a = source.attribute("e")) e = a.as_double();
	if (xml_attribute a = source.attribute("damping_tr")) damping_tr = a.as_double();
	if (xml_attribute a = source.attribute("damping_rot")) damping_rot = a.as_double();
	
	if (xml_attribute a = source.attribute("position")) position.fromString(a.value());
	if (xml_attribute a = source.attribute("lockTranslation")) lockTranslation = a.as_bool();
	if (xml_attribute a = source.attribute("lockX")) lockX = a.as_bool();
	if (xml_attribute a = source.attribute("lockY")) lockY = a.as_bool();
	
	if (xml_attribute a = source.attribute("theta")) theta = a.as_double();
	if (xml_attribute a = source.attribute("lockRotation")) lockRotation = a.as_bool();
	
	if (xml_attribute a = source.attribute("print")) print = a.as_bool();
	if (xml_attribute a = source.attribute("color")) color = strtol(a.value(), NULL, 0);
	
	if (xml_attribute a = source.attribute("light")) light = a.as_bool();
	if (xml_attribute a = source.attribute("lightColor")) lightColor = strtol(a.value(), NULL, 0);
	
	if (xml_attribute a = source.attribute("special")) special = a.value();
	
	if (xml_attribute a = source.attribute("nodes")){
		string s = a.value();
		
		int n = 0, b = s.find(",");
		for ( ; b != s.npos; n = b, b = s.find(",", b + 1)) nodes.push_back(new vect (s.substr(n, b - n)));
	}
	
	if (xml_attribute a = source.attribute("sensors")){
		string s = a.value();
		
		int n = 0, b = s.find(";");
		for ( ; b != s.npos; n = b + 1, b = s.find(";", b + 1)) sensors.push_back(new vect (s.substr(n, b - n)));
		
		sensors.push_back(new vect (s.substr(n)));
	}
}

box::box(){
	eType = etype_box;
	axesCount = 2;
	
	w = 0;
	h = 0;
}

void box::resize(double w, double h){
	this->w = w;
	this->h = h;
	
	//Sets points
	point[0] = position + vect (w / 2, h / 2).rotate(theta);
	point[1] = position + vect (-w / 2, h / 2).rotate(theta);
	point[2] = position + vect (-w / 2, -h / 2).rotate(theta);
	point[3] = position + vect (w / 2, -h / 2).rotate(theta);
}

void box::translate(vect s){
	entity::translate(s);
	
	//Translates points
	point[0] += s;
	point[1] += s;
	point[2] += s;
	point[3] += s;
}

void box::rotate(double a){
	entity::rotate(a);
	
	//Rotates corners
	point[0] = position + (point[0] - position).rotate(a);
	point[1] = position + (point[1] - position).rotate(a);
	point[2] = position + (point[2] - position).rotate(a);
	point[3] = position + (point[3] - position).rotate(a);
}

double box::inertia(){
	return mass * (w * w + h * h) / 12;//Returns result
}

vect* box::getAxes(){
	vect *result = new vect[2];//Result
	
	result[0] = (point[1] - point[2]).setModule(1);//Adds first axis
	result[1] = (point[3] - point[2]).setModule(1);//Adds second axis
	
	return result;//Returns result
}

bool box::isInside(vect p){
	vect v = p - point[2];//Position relative to box
	
	v = v.rotate(-theta);//Rotates relative position
	
	if (v.x >= 0 && v.x <= w && v.y >= 0 && v.y <= h) return true;//Returns true if inside box
	else return false;//Else returns false
}

void box::clone(entity* target){
	entity::clone(target);
	if (target->eType == etype_box) ((box*) target)->resize(w, h);
}

void box::load(xml_node source){
	entity::load(source);
	
	if (xml_attribute a = source.attribute("w")) w = a.as_double();
	if (xml_attribute a = source.attribute("h")) h = a.as_double();
	
	resize(w, h);
}

phLink::phLink(){
	a = NULL;	
	b = NULL;
	
	a_point = vect(0,0);
	b_point = vect(0,0);
	
	parent = NULL;
}

void phLink::apply(){
	vect f = force();
	
	//Applies force to the entities
	if (a){
		vect ap = a->position + a_point.rotate(a->theta);
		a->applyForce(ap, f);
	}
	if (b){
		vect bp = b->position + b_point.rotate(b->theta);
		b->applyForce(bp, -f);
	}
}

double phLink::length() {
	if (!a || !b) return 0;
	
	vect ap = a->position + a_point.rotate(a->theta);
	vect bp = b->position + b_point.rotate(b->theta);
	
	return (ap - bp).module();
}

void phLink::load(xml_node source){
	content::load(source);
	
	xml_attribute a = source.attribute("a"),
				  a_point = source.attribute("a_point"),
				  b = source.attribute("b"),
				  b_point = source.attribute("b_point");
				  
	if (a && parent) this->a = parent->getEntity(a.value());
	if (b && parent) this->b = parent->getEntity(b.value());
	
	if (a_point) this->a_point = vect(a_point.value());
	if (b_point) this->b_point = vect(b_point.value());
}

spring::spring(){
	length_zero = 0;
	k = 0;
}

vect spring::force(){
	if (!a || !b) return vect(0, 0);
	
	vect ap = a->position + a_point.rotate(a->theta);
	vect bp = b->position + b_point.rotate(b->theta);
	
	return (bp - ap - (bp - ap).setModule(length_zero)) * k;//Returns force
}

void spring::load(xml_node source){
	phLink::load(source);
	
	if (xml_attribute a = source.attribute("k")) k = a.as_double();
	if (xml_attribute a = source.attribute("length_zero")) length_zero = a.as_double();
}

scene::scene(){
	damping_tr = 0;
	damping_rot = 0;
	
	w = 0;
	h = 0;
}

scene::~scene(){
	for (list<entity*>::iterator i = entities.begin(); i != entities.end(); i++) delete *i;//Deletes each entity
}

void scene::resetForces(){
	for (list <entity*> :: iterator i = entities.begin(); i != entities.end(); i++)
		(*i)->resetForces();//Resets forces on each entity
}

void scene::applyGlobalForce (vect force){
	for (list <entity*> :: iterator i = entities.begin(); i != entities.end(); i++)
		(*i)->applyForce((*i)->position, force);//Applies the force to each entity
}

void scene::applyGravity (vect g){
	for (list <entity*> :: iterator i = entities.begin(); i != entities.end(); i++)
		(*i)->applyForce((*i)->position, g * (*i)->mass);//Applies gravity
}

void scene::setCells(){
	for (int n = 0; n < DIVISIONS * DIVISIONS; n++) cells[n].clear();//Clears all cells
	unassigned.clear();//Clears unassigned
	
	for (list <entity*>::iterator i = entities.begin(); i != entities.end(); i++){//For each entity
	
		//Calculates projections on x and y axes
		
		double minX, maxX, minY, maxY;
		
		dProjection(*i, vect (1, 0), &minX, &maxX);
		dProjection(*i, vect (0, 1), &minY, &maxY);
		
		//Determines the tiles where the entity lies
		
		int minTileX = floor(minX / (w / DIVISIONS));//Left tile x
		int maxTileX = floor(maxX / (w / DIVISIONS));//Right tile x
		int minTileY = floor(minY / (h / DIVISIONS));//Top tile y
		int maxTileY = floor(maxY / (h / DIVISIONS));//Bottom tile y
		
		//Adds entity to the lists of those tiles
		
		for (int y = minTileY; y <= maxTileY; y++){
			for (int x = minTileX; x <= maxTileX; x++){
				if (x >= 0 && x < DIVISIONS && y >= 0 && y < DIVISIONS){
					cells[y * DIVISIONS + x].push_back(*i);
				}
			}
		}
		
		//If entity is out of the scene area, adds it to unassigned
		
		if (minX < 0 || minY < 0 || maxX >= w || maxY >= h) unassigned.push_back(*i);
	}	
}

list <collision> scene::collisions(){
	list<collision> result;
	
	//Iterates through cells and check collisions for those cells
	
	for (int n = 0; n < DIVISIONS * DIVISIONS; n++){//For each cell
		if (cells[n].size() < 2) continue;//Next loop if less than two entities in cell (no need to check...)
		result = handleCollisions(&cells[n], &result);
	}
	
	//We handle also collisions between unassigned entities
	result = handleCollisions(&unassigned, &result);
	
	return result;//Returns result
}

list <collision> scene::step(double t){
	//Applies links
	for (list <phLink*> :: iterator l = links.begin(); l != links.end(); l++) (*l)->apply();

	//Moves entities
	for (list <entity*> :: iterator i = entities.begin(); i != entities.end(); i++)
		(*i)->step(t, damping_tr, damping_rot);
	
	setCells();
	return collisions();
}

entity* scene::getEntity(string id){
	for (list<entity*>::iterator i = entities.begin(); i != entities.end(); i++)
		if ((*i)->id == id) return *i;
		
	return NULL;
}

void scene::load(xml_node source){
	content::load(source);
	
	if (xml_attribute a = source.attribute("damping_tr")) damping_tr = a.as_double();
	if (xml_attribute a = source.attribute("damping_rot")) damping_rot = a.as_double();
	if (xml_attribute a = source.attribute("w")) w = a.as_double();
	if (xml_attribute a = source.attribute("h")) h = a.as_double();
	
	entities.clear();
	if (xml_node n = source.child("entities")){
		for (xml_node e = n.first_child(); e; e = e.next_sibling()){
			content c; c.load(e);
			entity* newEntity = NULL;
			
			if (c.type == "box") newEntity = new box;
			
			if (newEntity){
				newEntity->parent = this;
				newEntity->load(e);
				entities.push_back(newEntity);
			}
		}
	}
	
	links.clear();
	if (xml_node n = source.child("links")){
		for (xml_node l = n.first_child(); l; l = l.next_sibling()){
			content c; c.load(l);
			phLink* newLink = NULL;
			
			if (c.type == "spring") newLink = new spring;
			
			if (newLink){
				newLink->parent = this;
				newLink->load(l);
				links.push_back(newLink);
			}
		}
	}
}

vect projection(entity* a, vect b, vect* application){
	if (a->eType == etype_box){
		box* bx = (box*) a;
		
		//Iterates through vertices and picks the ones whose projections are
		//minimum and maximum, then returns the difference
		
		vect pMin = projection(bx->point[0], b);
		vect pMax = projection(bx->point[0], b);
		
		for (int i = 1; i < 4; i++){//For each vertex
			vect p = projection(bx->point[i], b);//Projection
			
			if (p.module() < pMin.module()) pMin = p;//Sets minimum
			if (p.module() > pMax.module()) pMax = p;//Sets maximum
		}
		
		if (application) *application = pMin;
		return pMax - pMin;
	}
	
	else return vect (0,0);
}

void dProjection (entity* a, vect b, double* min, double* max){
	if (a->eType == etype_box){
		box* bx = (box*) a;
		
		//Same as function above, but we use doubles instead of vectors
		
		if (min) *min = bx->point[0].dot(b);
		if (max) *max = bx->point[0].dot(b);
		
		for (int i = 1; i < 4; i++){
			double d = bx->point[i].dot(b);
			
			if (min && d < *min) *min = d;
			if (max && d > *max) *max = d;
		}
		
		if (min && max && *min > *max){
			int tmp = *min;
			*min = *max;
			*max = tmp;
		}
	}
}

vect collisionPoint (entity* a, entity* b, vect n){
	deque<vect> points;//Colliding points found
	vect result (0,0);
		
	//We still want to check if entities are a boxes, for we might want
	//to add new entity types in future
		
	//Checks vertices
	if (a->eType == etype_box){
		box* bx = (box*) a;
		
		for (int i = 0; i < 4; i++)
			if (b->isInside(bx->point[i])){
				points.push_back(bx->point[i]);
			}
	}
	
	//Checks vertices
	if (b->eType == etype_box){
		box* bx = (box*) b;
		
		for (int i = 0; i < 4; i++)
			if (a->isInside(bx->point[i])){
				points.push_back(bx->point[i]);
			}
	}
	
	//Now we calculate the average point
	//All points are summed and then devided by the sum of point speeds relatives to collision normal
	
	double total = 0;
	for (int i = 0; i < points.size(); i++){
		double vp = (a->pointSpeed(points[i]) - b->pointSpeed(points[i])).dot(n);
		
		if (vp <= 0){
			total += vp;
			result += points[i] * vp;
		}
	};
	
	result = result / total;
	
	return result;
}

collision* collide (entity* a, entity* b){
	//We use the separating axis theorem to check if entities are colliding
	//To do so, we need the relevant axes of each entity
	
	vect *axes_a = a->getAxes();
	vect *axes_b = b->getAxes();
	
	bool mtvSet = false;//If true, mtv has been set
	vect mtv;
	
	//Checks axes of first entity
	for (int i = 0; i < a->axesCount; i++){
		double minA, maxA, minB, maxB;
		dProjection(a, axes_a[i], &minA, &maxA);
		dProjection(b, axes_a[i], &minB, &maxB);
		
		//If projections don't overlap, there's no collision and we return null
		if (!((minA <= minB && maxA >= minB) || (minB < minA && maxB >= minA))){
			//Frees axes
			free(axes_a);
			free(axes_b);
			
			return NULL;
		}
			
		//If they overlap, instead, we want to know something more about the collision
		else {
			//We keep track of the shortest translation vector to separate entities for each axis
			
			vect v;
			
			if (minA > minB) v = axes_a[i].setModule(maxB - minA);
			else v = axes_a[i].setModule(minB - maxA);
			
			if (!mtvSet || v.module() <= mtv.module()){
				mtvSet = true;
				mtv = v;
			}
		}
	}
	
	//Does the same with axes of second entity
	for (int i = 0; i < b->axesCount; i++){
		double minA, maxA, minB, maxB;
		dProjection(a, axes_b[i], &minA, &maxA);
		dProjection(b, axes_b[i], &minB, &maxB);
		
		if (!((minA <= minB && maxA >= minB) || (minB < minA && maxB >= minA))){
			free(axes_a);
			free(axes_b);
			
			return NULL;
		}
			
		else {
			vect v;
			
			if (minA > minB) v = axes_b[i].setModule(maxB - minA);
			else v = axes_b[i].setModule(minB - maxA);
			
			if (!mtvSet || v.module() <= mtv.module()){
				mtvSet = true;
				mtv = v;
			}
		}
	}
	
	//Finally, we fill in the collision data and return it
	
	collision *result = new collision;
	
	result->mtv = mtv;
	result->n = mtv.setModule(1);
	result->p = collisionPoint(a, b, result->n);
	result->e = a->e * b->e;
	result->a = a;
	result->b = b;
	
	//Frees axes
	free(axes_a);
	free(axes_b);
	
	return result;
}

void handleCollision(collision c){
	double vap = c.a->pointSpeed(c.p).dot(c.n);//Point speed on a
	double vbp = c.b->pointSpeed(c.p).dot(c.n);//Point speed on b
	
	double vab = vap - vbp;//Relative point speed
	
	double rap = (c.p - c.a->position).perp().dot(c.n);
	double rbp = (c.p - c.b->position).perp().dot(c.n);
	
	//Entities are colliding only if relative point speed is negative
	//We also need to check for locked entities: if an entity is locked, impulse is
	//applied totally to the other one
	
	if (vab <= 0){
		if (!c.a->locked() && !c.b->locked()){
			double j = -(1 + c.e) * vab / (c.n.dot(c.n * (1 / c.a->mass + 1 / c.b->mass)) + (c.a->lockRotation ? 0 : pow(rap, 2) / c.a->inertia()) + (c.b->lockRotation ? 0 : pow(rbp, 2) / c.b->inertia()));
			vect impulse = c.n.setModule(j);
			
			c.a->applyImpulse(c.p, impulse);
			c.b->applyImpulse(c.p, -impulse);
		}
		
		else if (c.a->locked()){
			double j = -(1 + c.e) * vab / (c.n.dot(c.n * 1 / c.b->mass) + (c.b->lockRotation ? 0 : pow(rbp, 2) / c.b->inertia()));
			vect impulse = c.n.setModule(j);
			
			c.b->applyImpulse(c.p, -impulse);
		}
		
		else if (c.b->locked()){
			double j = -(1 + c.e) * vab / (c.n.dot(c.n * 1 / c.a->mass) + (c.a->lockRotation ? 0 : pow(rap, 2) / c.a->inertia()));
			vect impulse = c.n.setModule(j);
		
			c.a->applyImpulse(c.p, impulse);
		}
	}
}

list<collision> handleCollisions (list <entity*> * entities, list <collision> * colls){
	list<collision> result;
	
	if (colls) for (list<collision>::iterator i = colls->begin(); i != colls->end(); i++) result.push_back(*i);
	
	//Iterates through pair of entities in given list checking for collisions
	for (list<entity*>::iterator i = entities->begin(); i != --entities->end(); i++){
		for (list<entity*>::iterator j = i; j != entities->end(); j++){
			if (j == i) continue;
			
			collision* c = collide(*i, *j);//Checks collision between entities
			
			//We have to see if collision hasn't been checked yet
			//Iterates through collisions found and ignores the current if it has already been handled
			
			if (c && colls){
				for (list<collision>::iterator n = result.begin(); n != result.end(); n++){
					if ((n->a == *i && n->b == *j) || (n->a == *j && n->b == *i)){//If colliding entities are the same
						c = NULL;
					}
				}
			}
			
			if (c){//If collision is still valid						
				result.push_back(*c);
				
				//First we handle collision (this means applying impulse to colliding entities)
				//then we translate the entities accoding to minimum translation vector (MTV)
				//To do so, we must check if one of the entities is locked on one of the axes and act consequently
				
				handleCollision(*c);
				
				//Vector components of the MTV
				vect x (c->mtv.x, 0);
				vect y (0, c->mtv.y);
				
				//If none of the entity is completely locked, we must take care of axes separately
				if (!(*i)->lockTranslation && !(*j)->lockTranslation){
				
					//If both entities are completely free, we can translate them by MTV and -MTV respectively
					if (!(*i)->lockX && !(*i)->lockY && !(*j)->lockX && !(*j)->lockY){
						(*i)->translate(c->mtv * (*j)->mass / ((*i)->mass + (*j)->mass));
						(*j)->translate(-c->mtv * (*i)->mass / ((*i)->mass + (*j)->mass));
					}
					
					//If at least one entity is locked on at least one axis, we check all possible situations
					//considering the two components of MTV separately
					
					else {
					
						//Checks X axis
						if ((*i)->lockX && !(*j)->lockX)
							(*j)->translate(-x);
						else if ((*j)->lockX && !(*i)->lockX)
							(*i)->translate(x);
						else if (!(*i)->lockX && !(*j)->lockX){
							(*i)->translate(x * (*j)->mass / ((*i)->mass + (*j)->mass));
							(*j)->translate(-x * (*i)->mass / ((*i)->mass + (*j)->mass));
						}
							
						//Checks Y axis
						if ((*i)->lockY && !(*j)->lockY)
							(*j)->translate(-y);
						else if ((*j)->lockY && !(*i)->lockY)
							(*i)->translate(y);
						else if (!(*i)->lockY && !(*j)->lockY){
							(*i)->translate(y * (*j)->mass / ((*i)->mass + (*j)->mass));
							(*j)->translate(-y * (*i)->mass / ((*i)->mass + (*j)->mass));
						}
					}
				}
				
				//If one of the entity is completely locked, we translate only the non-locked one, if any
				
				else {
					if (!(*i)->lockTranslation) (*i)->translate(c->mtv);
					if (!(*j)->lockTranslation) (*j)->translate(-c->mtv);
				}
			}
		}			
	}
	
	return result;
}
