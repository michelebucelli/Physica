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

//Buch's Library Kollection
//Physics handling header

#ifndef _BULK_PHYS
#define _BULK_PHYS

#define DIVISIONS										10//Scene divisions for space partitioning

#define OBJTYPE_ENTITY									"entity"//Entity objects
#define OBJTYPE_BOX										"box"//Box objects
#define OBJTYPE_BALL									"ball"//Ball objects
#define OBJTYPE_LINK									"link"//Link objects
#define OBJTYPE_SPRING									"spring"//Spring objects
#define OBJTYPE_SCENE									"scene"//Scene objects

class scene;//Class scene prototype
class gBox;//Graphic box class

//Entity type enumeration
enum entityType {
	etype_generic,//Generic entity (default)
	etype_ball,//Ball
	etype_box,//Box
};

//Entity base class
class entity: public objectBased {
	public:
	//General
	entityType eType;//Entity type
	int axesCount;//Number of axes of the entity
	
	double mass;//Entity mass
	
	double e;//Coefficient of restitution
	
	deque<vector*> nodes;//Entity nodes, rototranslated with entity
	deque<vector*> sensors;//Contact sensors, rototranslated with entity
	
	//Individual damping factors
	//Scene factors are multiplied by those. They can be used to model different entity
	//behaviour - 0 to 1 recommended
	double damping_tr;//Translation damping factor
	double damping_rot;//Rotation damping factor
	
	double curLifetime;//Current lifetime
	double lifetime;//Lifetime (used for particles, 0 for endless)
	
	//Relatives
	scene* parent;//Parent scene
	
	//Translation
	bool lockTranslation;//If true, entity doesn't move
	bool lockX, lockY;//If true, locks on X or Y axis separately (lockTranslation will have most priority)
	
	vector force;//Force vector
	
	vector position;//Position
	vector speed;//Speed
	vector accel;//Acceleration
	
	//Rotation
	bool lockRotation;//If true, entity doesn't rotate
	
	double torque;//Torque
	
	double theta;//Angle
	double omega;//Angular speed
	double alpha;//Angular acceleration
	
	//Graphics
	int color;//Entity color
	
	bool useAnim;//If true, uses animation
	animSet anims;//Animations
	
	bool print;//If true, entity is printed
	
	//Misc
	string special;//Special stuff
	
	//Constructor
	entity(){
		id = "";
		type = OBJTYPE_ENTITY;
	
		eType = etype_generic;
		mass = 0;
		
		axesCount = 0;
		
		damping_tr = 1;
		damping_rot = 1;
		
		curLifetime = 0;
		lifetime = 0;
		
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
		
		useAnim = false;
		
		print = true;
		special = "";	
	}
	
	//Function for inertia momentum around entity centre
	virtual double inertia() = 0;
	
	//Function to reset the force
	void resetForces(){
		force = {0,0};//Resets force
		torque = 0;//Resets torque
	}
	
	//Function to translate the entity
	virtual void translate(vector s){
		position += s;//Increments position
		
		deque<vector*>::iterator i;//Iterator
		for (i = nodes.begin(); i != nodes.end(); i++)//For each node
			**i += s;//Translates node
			
		for (i = sensors.begin(); i != sensors.end(); i++)//For each sensor
			**i += s;//Translates sensor
	}
	
	//Function to rotate the entity
	virtual void rotate(double a){
		theta += a;//Increments angle
		
		deque<vector*>::iterator i;//Iterator
		for (i = nodes.begin(); i != nodes.end(); i++)//For each node
			**i = position + (**i - position).rotate(a);//Rotates node
			
		for (i = sensors.begin(); i != sensors.end(); i++)//For each sensor
			**i = position + (**i - position).rotate(a);//Rotates sensor
	}
	
	//Function to get entity axes for collision detection
	virtual vector* getAxes(entity*) = 0;
	
	//Function to set the force applied to the entity
	//  p is the point where the force is applied, force is the force itself
	void applyForce (vector p, vector force){
		this->force += force;//Adds force
		torque += (position - p).cross(force);//Adds torque
	}
	
	//Function to apply an impulse to the entity
	void applyImpulse (vector p, vector impulse){
		speed += impulse / mass;
		omega += (position - p).cross(impulse) / inertia();//Adds angular impulse
	}
	
	//Function to move entity
	void step(double, double, double);
	
	//Function to determine if a point is inside the entity
	virtual bool isInside(vector p) = 0;
	
	//Function to calculate a point's speed
	vector pointSpeed(vector point){
		return speed + (point - position).perp() * omega;
	}
	
	//Function to add a node to the entity (vector variable rototranslated with the entity)
	vector* addNode(vector p){
		vector* v = new vector;//New node
		*v = p;//Sets node
		
		nodes.push_back(v);//Adds to nodes
		return v;//Returns node
	}
	
	//Function to add a sensor to the entity (returns the sensor id for testing)
	int addSensor(vector p){
		vector* v = new vector;//New sensor
		*v = p;//Sets sensor
		
		sensors.push_back(v);//Adds to sensors
		return sensors.size() - 1;//Returns sensor id
	}
	
	//Total lock
	bool locked(){
		return lockTranslation && lockRotation;
	}
	
	//Function to check contact sensors
	bool checkSensor(int);
	
	//Function to load from script object
	virtual bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succeeded loading base data
			//Gets variables
			var* mass = get <var> (&o.v, "mass");
			var* e = get <var> (&o.v, "e");
			var* nodes = get <var> (&o.v, "nodes");
			var* sensors = get <var> (&o.v, "sensors");
			
			var* damping_tr = get <var> (&o.v, "damping_tr");
			var* damping_rot = get <var> (&o.v, "damping_rot");
			
			var* position = get <var> (&o.v, "position");
			var* lockTranslation = get <var> (&o.v, "lockTranslation");
			var* lockX = get <var> (&o.v, "lockX");
			var* lockY = get <var> (&o.v, "lockY");
			
			var* theta = get <var> (&o.v, "theta");
			var* lockRotation = get <var> (&o.v, "lockRotation");
			
			var* color = get <var> (&o.v, "color");
			object* anims = get <object> (&o.o, "anims");
			var* print = get <var> (&o.v, "print");
			
			var* special = get <var> (&o.v, "special");
			
			//Sets values
			if (mass) this->mass = mass->doubleValue();
			if (e) this->e = e->doubleValue();
			
			if (damping_tr) this->damping_tr = damping_tr->doubleValue();
			if (damping_rot) this->damping_rot = damping_rot->doubleValue();
			
			if (lockTranslation) this->lockTranslation = lockTranslation->intValue();
			if (lockX) this->lockX = lockX->intValue();
			if (lockY) this->lockY = lockY->intValue();
			if (position) this->position.fromString(position->value);
			
			if (lockRotation) this->lockRotation = lockRotation->intValue();
			if (theta) this->theta = theta->doubleValue() * PI / 180;
			
			if (nodes){
				deque<string> toks = tokenize <deque<string> > (nodes->value, ",");
				deque<string>::iterator i;
				
				for (i = toks.begin(); i != toks.end(); i++)
					addNode(this->position + vector(*i).rotate(this->theta));
			}
			
			if (sensors){
				deque<string> toks = tokenize <deque<string> > (sensors->value, ",");
				deque<string>::iterator i;
				
				for (i = toks.begin(); i != toks.end(); i++)
					addSensor(this->position + vector(*i).rotate(this->theta));
			}
			
			if (anims) {
				this->anims.fromScriptObj(*anims);
				useAnim = true;
			}
			else if (color) this->color = strtol(color->value.c_str(), NULL, 0);
			
			if (print) this->print = print->intValue();
			
			if (special) this->special = special->value;
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	virtual object toScriptObj(){
		object result = objectBased::toScriptObj();//Result object
		
		//Sets members
		result.set("mass", mass);
		result.set("e", e);
		result.set("damping_tr", damping_tr);
		result.set("damping_rot", damping_rot);
		result.set("position", toString(position.x) + " " + toString(position.y));
		result.set("lockTranslation", lockTranslation);
		result.set("lockX", lockX);
		result.set("lockY", lockY);
		result.set("theta", theta);
		result.set("lockRotation", lockRotation);
		result.set("color", color);
		result.set("print", print);
		result.set("special", special);
		
		result.o.push_back(anims.toScriptObj());
		
		deque<vector*>::iterator i;
		string s = "";
		
		for (i = nodes.begin(); i != nodes.end(); i++)
			s += ", " + toString((*i)->x - position.x) + " " + toString((*i)->y - position.y);
		
		s.erase(0, 2);
		result.set("nodes", s);
		s = "";
		
		for (i = sensors.begin(); i != sensors.end(); i++)
			s += ", " + toString((*i)->x - position.x) + " " + toString((*i)->y - position.y);
		
		s.erase(0, 2);
		result.set("sensors", s);
		
		return result;//Returns result
	}
	
	//Function to clone entity
	virtual void clone(entity* target){
		*target = *this;
		
		deque<vector*>::iterator i;
		target->sensors.clear();
		target->nodes.clear();
		
		for (i = sensors.begin(); i != sensors.end(); i++) target->addSensor(**i);
		for (i = nodes.begin(); i != nodes.end(); i++) target->addNode(**i);
	}
};

//Box entity class
class box: public entity {
	public:
	//General
	double w, h;//Size
	
	vector point[4];//Box corners
	
	//Constructor
	box() {
		id = "";
		type = OBJTYPE_BOX;
	
		eType = etype_box;
		mass = 0;
		
		axesCount = 2;
		
		lockTranslation = false;
		
		lockRotation = false;
		torque = 0;
		theta = 0;
		omega = 0;
		alpha = 0;
		
		e = 0;
		
		color = 0xFFFFFF;
		
		parent = NULL;
		
		w = 0;
		h = 0;
		
		useAnim = false;
	}
	
	//Function to resize the box
	void resize(double w, double h){
		this->w = w;//Sets width
		this->h = h;//Sets height
		
		//Sets points
		point[0] = position + vector {w / 2, h / 2}.rotate(theta);
		point[1] = position + vector {-w / 2, h / 2}.rotate(theta);
		point[2] = position + vector {-w / 2, -h / 2}.rotate(theta);
		point[3] = position + vector {w / 2, -h / 2}.rotate(theta);
	}

	//Function to translate
	void translate(vector s){
		entity::translate(s);//Default translation
		
		//Translates points
		point[0] += s;
		point[1] += s;
		point[2] += s;
		point[3] += s;
	}
	
	//Function to rotate
	void rotate(double a){
		entity::rotate(a);//Default rotation
		
		//Rotates corners
		point[0] = position + (point[0] - position).rotate(a);
		point[1] = position + (point[1] - position).rotate(a);
		point[2] = position + (point[2] - position).rotate(a);
		point[3] = position + (point[3] - position).rotate(a);
	}
	
	//Function for inertia momentum
	double inertia(){
		return mass * (w * w + h * h) / 12;//Returns result
	}
	
	//Function to get axes for collision detection
	vector* getAxes(entity* e){
		vector *result = new vector[2];//Result
		
		result[0] = (point[1] - point[2]).setModule(1);//Adds first axis
		result[1] = (point[3] - point[2]).setModule(1);//Adds second axis
		
		return result;//Returns result
	}
	
	//Function to determine if a point is inside the entity
	bool isInside(vector p){
		vector v = p - point[2];//Position relative to box
		
		v = v.rotate(-theta);//Rotates relative position
		
		if (v.x >= 0 && v.x <= w && v.y >= 0 && v.y <= h) return true;//Returns true if inside box
		else return false;//Else returns false
	}
	
	//Function to load from script obj
	virtual bool fromScriptObj(object o){
		if (entity::fromScriptObj(o)){//If succeeded loading base data
			var* w = get <var> (&o.v, "w");//Width
			var* h = get <var> (&o.v, "h");//Height
			
			if (w) this->w = w->doubleValue();//Sets width
			if (h) this->h = h->doubleValue();//Sets height
			
			resize(this->w, this->h);//Resizes
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script obj
	virtual object toScriptObj(){
		object result = entity::toScriptObj();//Result object
		
		result.set("w", w);
		result.set("h", h);
		
		return result;//Returns result
	}
	
	//Clone function
	void clone(box* target){
		entity::clone(target);
		target->resize(w, h);
	}
};

//Ball entity class [INCOMPLETE]
class ball: public entity {
	public:
	//General
	double radius;//Ball radius
	
	//Constructor
	ball(){
		id = "";
		type = OBJTYPE_BALL;
	
		eType = etype_ball;
		mass = 0;
		
		axesCount = 1;
		
		lockTranslation = false;
		
		lockRotation = false;
		torque = 0;
		theta = 0;
		omega = 0;
		alpha = 0;
		
		e = 0;
		
		color = 0xFFFFFF;
		
		parent = NULL;
		
		radius = 0;
		
		useAnim = false;
	}
	
	//Function for inertia momentum
	virtual double inertia(){
		return mass * radius * radius / 2;//Returns result
	}
	
	//Function to get axes for collision detection
	virtual vector* getAxes(entity* e){
		vector *result = new vector[1];//Result
		
		if (e->eType == etype_ball)//If other entity is a ball
			result[0] = e->position - position;//Adds vector between centres
			
		else if (e->eType == etype_box){//Else if other entity is a box
			int i;//Counter
			int closest = 0;//Chosen vertex of the box
			
			box* b = (box*) e;//Converts to box
			
			for (i = 1; i < 4; i++)//For each box vertex
				if ((position - b->point[i]).module() < (position - b->point[closest]).module()) closest = i;//Picks closest vertex
				
			result[0] = b->point[closest] - position;//Adds vector from centre to closest vertex
		}
		
		return result;//Returns result
	}
	
	//Function to determine if a point is inside the entity
	virtual bool isInside(vector p){
		if ((p - position).module() <= radius) return true;
		else return false;
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (entity::fromScriptObj(o)){//If succeeded loading base data
			var* radius = get <var> (&o.v, "radius");//Radius
			if (radius) this->radius = radius->doubleValue();//Sets radius
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
};

//Function to project an entity onto a vector
vector projection(entity* a, vector b, vector* application){
	if (a->eType == etype_ball){//If entity is a ball
		ball* bl = (ball*) a;//Converts to ball
		
		//HOW??? APPLICATION POINT...
		return b.setModule(bl->radius * 2);//Returns result
	}
	
	if (a->eType == etype_box){//If entity is a box
		box* bx = (box*) a;//Converts to box
		int i;//Counter
		
		vector pMin = projection(bx->point[0], b);//Minimum projection
		vector pMax = projection(bx->point[0], b);//Maximum projection
		
		for (i = 1; i < 4; i++){//For each vertex
			vector p = projection(bx->point[i], b);//Projection
			
			if (p.module() < pMin.module()) pMin = p;//Sets minimum
			if (p.module() > pMax.module()) pMax = p;//Sets maximum
		}
		
		if (application) *application = pMin;//Sets application point
		return pMax - pMin;//Returns projection
	}
}

//Function to calculate projection of an entity onto a vector (doubles)
void dProjection(entity* a, vector b, double* min, double* max){
	if (a->eType == etype_ball){//If entity is a ball
		ball* bl = (ball*) bl;//Converts to ball
		
		if (min) *min = a->position.dot(b) - bl->radius;//Gets minimum
		if (max) *max = a->position.dot(b) + bl->radius;//Gets maximum
		
		if (min && max && *min > *max) swap(min, max);//Sorts minimum and maximum
	}
	
	if (a->eType == etype_box){//If entity is a box
		box* bx = (box*) a;//Converts to box
		int i;//Counter
		
		if (min) *min = bx->point[0].dot(b);//Minimum projection
		if (max) *max = bx->point[0].dot(b);//Maximum projection
		
		for (i = 1; i < 4; i++){//For each point
			double d = bx->point[i].dot(b);//Projects point
			
			if (min && d < *min) *min = d;//Sets minimum
			if (max && d > *max) *max = d;//Sets maximum
		}
		
		if (min && max && *min > *max) swap(min, max);//Sorts minimum and maximum
	}
}

typedef struct { vector mtv; vector n; vector p; double e; entity* a; entity* b;} collision;//Collision structure

//Function to get the collision point of two entities
vector collisionPoint(entity* a, entity* b, vector n){
	deque<vector> points;//Points found by now
	vector result (0,0);//Result
	int i;//Counter
	
	if (a->eType == etype_box){//If first entity is a box
		box* bx = (box*) a;//Converts to box
		
		for (i = 0; i < 4; i++)//For each point
			if (b->isInside(bx->point[i])){//If point is inside
				points.push_back(bx->point[i]);//Adds point if inside and colliding
			}
	}
	
	if (b->eType == etype_box){//If second entity is a box
		box* bx = (box*) b;//Converts to box
		
		for (i = 0; i < 4; i++)//For each point
			if (a->isInside(bx->point[i])){//If point is inside
				points.push_back(bx->point[i]);//Adds point if inside and colliding
			}
	}
	
	double total = 0;//Sum of total point speeds
	for (i = 0; i < points.size(); i++){//For each point
		double vp = (a->pointSpeed(points[i]) - b->pointSpeed(points[i])).dot(n);//Relative point speed
		
		if (vp <= 0){
			total += vp;//Adds to total speed
			result += points[i] * vp;//Adds point for average
		}
	};
	
	result = result / total;//Devides by total
	
	return result;//Returns result
}

//Function to determine if two entities are colliding
collision* collide(entity* a, entity* b){
	vector *axes_a = a->getAxes(b);//First entity axes
	vector *axes_b = b->getAxes(a);//Second entity axes
	
	int i;//Counter
	bool mtvSet = false;//If true, mtv has been set
	vector mtv;
	
	for (i = 0; i < a->axesCount; i++){//For each axis of first entity
		double minA, maxA, minB, maxB;
		dProjection(a, axes_a[i], &minA, &maxA);
		dProjection(b, axes_a[i], &minB, &maxB);
		
		if (!((minA <= minB && maxA >= minB) || (minB < minA && maxB >= minA))){//If not overlapping
			//Frees axes
			free(axes_a);
			free(axes_b);
			
			return NULL;//Returns null
		}
			
		else {//If overlapping		
			vector v;//Translation vector on this axis
			
			//Sets translation vector
			if (minA > minB) v = axes_a[i].setModule(maxB - minA);
			else v = axes_a[i].setModule(minB - maxA);
			
			if (!mtvSet || v.module() <= mtv.module()){//If lower than mtv
				mtvSet = true;//Mtv was set
				mtv = v;//Sets minimum translation vector
			}
		}
	}
	
	for (i = 0; i < b->axesCount; i++){//For each axis of second entity
		double minA, maxA, minB, maxB;
		dProjection(a, axes_b[i], &minA, &maxA);
		dProjection(b, axes_b[i], &minB, &maxB);
		
		if (!((minA <= minB && maxA >= minB) || (minB < minA && maxB >= minA))){//If not overlapping
			//Frees axes
			free(axes_a);
			free(axes_b);
			
			return NULL;//Returns null
		}
			
		else {//If overlapping
			vector v;//Translation vector on this axis
			
			//Sets translation vector
			if (minA > minB) v = axes_b[i].setModule(maxB - minA);
			else v = axes_b[i].setModule(minB - maxA);
			
			if (!mtvSet || v.module() <= mtv.module()){//If lower than mtv
				mtvSet = true;//Mtv was set
				mtv = v;//Sets minimum translation vector
			}
		}
	}
	
	collision *result = new collision;//Result collision
	
	//Sets collision data
	result->mtv = mtv;
	result->n = mtv.setModule(1);
	result->p = collisionPoint(a, b, result->n);
	result->e = a->e * b->e;
	result->a = a;
	result->b = b;
	
	//Frees axes
	free(axes_a);
	free(axes_b);
	
	return result;//Returns result if no gap was found
}

//Function to handle collision between two entities
void handleCollision(entity* a, entity* b, collision c){
	double vap = a->pointSpeed(c.p).dot(c.n);//Point speed on a
	double vbp = b->pointSpeed(c.p).dot(c.n);//Point speed on b
	
	double vab = vap - vbp;//Relative point speed
	
	double rap = (c.p - a->position).perp().dot(c.n);
	double rbp = (c.p - b->position).perp().dot(c.n);
	
	if (vab <= 0){//If entities are bumping
		if (!a->locked() && !b->locked()){//If none of the entities is locked
			double j = -(1 + c.e) * vab / (c.n.dot(c.n * (1 / a->mass + 1 / b->mass)) + (a->lockRotation ? 0 : pow(rap, 2) / a->inertia()) + (b->lockRotation ? 0 : pow(rbp, 2) / b->inertia()));//Impulse module
			vector impulse = c.n.setModule(j);//Impulse vector
			
			a->applyImpulse(c.p, impulse);//Applies impulse
			b->applyImpulse(c.p, -impulse);//Applies impulse
		}
		
		else if (a->locked()){//If a is locked
			double j = -(1 + c.e) * vab / (c.n.dot(c.n * 1 / b->mass) + (b->lockRotation ? 0 : pow(rbp, 2) / b->inertia()));//Impulse module
			vector impulse = c.n.setModule(j);//Impulse vector
			
			b->applyImpulse(c.p, -impulse);//Applies all impulse to b
		}
		
		else if (b->locked()){//If b is locked
			double j = -(1 + c.e) * vab / (c.n.dot(c.n * 1 / a->mass) + (a->lockRotation ? 0 : pow(rap, 2) / a->inertia()));//Impulse module
			vector impulse = c.n.setModule(j);//Impulse vector
		
			a->applyImpulse(c.p, impulse);//Applies all impulse to a
		}
	}
}

//Link base class
//	represents a force connecting two points-vectors of two entities
class phLink: public objectBased {
	public:
	entity* a;//First entity
	vector* a_point;//First entity application point
	int aPointIndex;//A point index
	
	entity* b;//Second entity
	vector* b_point;//Second entity application point
	int bPointIndex;//B point index
	
	scene* parent;//Parent scene
	
	//Constructor
	phLink(){
		id = "";
		type = OBJTYPE_LINK;
		
		a = NULL;
		a_point = NULL;
		
		b = NULL;
		b_point = NULL;
		
		parent = NULL;
	}
	
	//Function to calculate the force of the link
	virtual vector force() = 0;
	
	//Function to apply the force to the two entities
	void apply(){
		vector f = force();//Force vector
		
		if (a) a->applyForce(*a_point, f);//Applies force to first entity, if not null
		if (b) b->applyForce(*b_point, -f);//Applies force to second entity, if not null
	}
	
	//Function to calculate link length
	double length(){
		return (*b_point - *a_point).module();
	}
	
	//Function to load from script object
	virtual bool fromScriptObj(object);
	
	//Function to save to script object
	virtual object toScriptObj(){
		object result = objectBased::toScriptObj();//Result object
		
		//Sets members
		if (a){//If A entity was given
			result.set ("a", a->id);//Sets A entity
			result.set ("a_point", aPointIndex);//Sets a point
		}
		
		else result.set ("a_point", toString(a_point->x) + " " + toString(a_point->y));//Else sets point only
		
		if (b){//If B entity was given
			result.set ("b", b->id);//Sets B entity
			result.set ("b_point", bPointIndex);//Sets b point
		}
		
		else result.set ("b_point", toString(b_point->x) + " " + toString(b_point->y));//Else sets point only
		
		return result;//Returns result
	}
};

//Spring class
//	force is caluclated multiplying distance between points by a factor
class spring: public phLink {
	public:
	double length_zero;//Base length
	double k;//Spring coefficient
	
	//Constructor
	spring(){
		id = "";
		type = OBJTYPE_SPRING;
		
		a = NULL;
		a_point = NULL;
		
		b = NULL;
		b_point = NULL;
		
		length_zero = 0;
		k = 0;
	}
	
	//Function to calculate the force of the link
	virtual vector force() {
		if (a_point && b_point){//If both points aren't null
			vector lZero = (*b_point - *a_point).setModule(length_zero);//Vector of the link at base length
			return (*b_point - *a_point - lZero) * k;//Returns force
		}
		
		else return {0, 0};//Else returns a null vector
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (phLink::fromScriptObj(o)){//If succeeded loading base data
			var* length_zero = get <var> (&o.v, "length_zero");//Base length
			var* k = get <var> (&o.v, "k");//Coefficient
			
			if (length_zero) this->length_zero = length_zero->doubleValue();//Sets base length
			if (k) this->k = k->doubleValue();//Sets coefficient
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = phLink::toScriptObj();//Result object
		
		//Sets members
		result.set("length_zero", length_zero);
		result.set("k", k);
		
		return result;//Returns result
	}
};

//Scene class
class scene: public objectBased {
	public:
	list <entity*> entities;//Entities in scene
	list <entity*> particles;//Particles in scene
	deque <phLink*> links;//Links in scene
	
	list <entity*> toRemove;//Particles to remove from scene
	
	double damping_tr;//Translation damping factor
	double damping_rot;//Angular damping factor
	
	int w, h;//Scene size
	
	list <entity*> cells [DIVISIONS * DIVISIONS];//Cells
	list <entity*> unassigned;//Unassigned cells
	
	//Constructor
	scene(){
		id = "";
		type = OBJTYPE_SCENE;
		
		damping_tr = 0;
		damping_rot = 0;
		
		w = 0;
		h = 0;
	}
	
	//Destructor
	~scene(){
		list<entity*>::iterator i;//Iterator for entities
		
		for (i = entities.begin(); i != entities.end(); i++) delete *i;//Deletes each entity
	}
	
	//Function to reset forces on entities
	void resetForces(){
		list <entity*> :: iterator i;//Iterator for entities
		
		for (i = entities.begin(); i != entities.end(); i++)//For each entity
			(*i)->resetForces();//Resets forces on entity
			
		for (i = particles.begin(); i != particles.end(); i++)//For each particle
			(*i)->resetForces();//Resets forces on particle
	}
	
	//Function to apply global force
	//	the force is applied to entity centres
	void applyGlobalForce(vector force){
		list <entity*> :: iterator i;//Iterator for entities
		
		for (i = entities.begin(); i != entities.end(); i++)//For each entity
			(*i)->applyForce((*i)->position, force);//Applies the force to each entity
			
		for (i = particles.begin(); i != particles.end(); i++)//For each particle
			(*i)->applyForce((*i)->position, force);//Applies the force to each particle
	}
	
	//Function to apply gravity
	void applyGravity(vector g){
		list <entity*> :: iterator i;//Iterator for entities
		
		for (i = entities.begin(); i != entities.end(); i++)//For each entity
			(*i)->applyForce((*i)->position, g * (*i)->mass);//Applies gravity
			
		for (i = particles.begin(); i != particles.end(); i++)//For each particle
			(*i)->applyForce((*i)->position, g * (*i)->mass);//Applies gravity
	}
	
	//Function to set cells
	void setCells(){
		list <entity*>::iterator i;//Iterator
		
		int n;//Counter
		for (n = 0; n < DIVISIONS * DIVISIONS; n++) cells[n].clear();//Clears all cells
		unassigned.clear();//Clears unassigned
		
		for (i = entities.begin(); i != entities.end(); i++){//For each entity
			double minX, maxX, minY, maxY;//Projections on coordinated axes
			
			dProjection(*i, vector {1, 0}, &minX, &maxX);//Projects on x
			dProjection(*i, vector {0, 1}, &minY, &maxY);//Projects on y
			
			int minTileX = floor(minX / (w / DIVISIONS));//Left tile x
			int maxTileX = floor(maxX / (w / DIVISIONS));//Right tile x
			int minTileY = floor(minY / (h / DIVISIONS));//Top tile y
			int maxTileY = floor(maxY / (h / DIVISIONS));//Bottom tile y
			
			int x, y;//Counters
			
			for (y = minTileY; y <= maxTileY; y++){//For each tile row
				for (x = minTileX; x <= maxTileX; x++){//For each tile column
					if (x >= 0 && x < DIVISIONS && y >= 0 && y < DIVISIONS){//If coords are valid
						cells[y * DIVISIONS + x].push_back(*i);//Adds entity to cell
					}
				}
			}
			
			double bX, eX, bY, eY;//Projections on axes
			dProjection(*i, vector (w, 0), &bX, &eX);//Projects on x
			dProjection(*i, vector (0, h), &bY, &eY);//Projects on y
			
			if (bX < 0 || bY < 0 || eX >= w || eY >= h) unassigned.push_back(*i);//Adds to unassigned if out of scene
		}
	}
	
	//Function to check and handle collisions
	list<collision> collisions(){
		list <entity*> :: iterator i, j;//Entity iterators
		int n;//Counter
		
		list<collision> result;//Result
		
		for (n = 0; n < DIVISIONS * DIVISIONS; n++){//For each cell
			if (cells[n].size() < 2) continue;//Next loop if less than two entities in cell
			
			for (i = cells[n].begin(); i != cells[n].end(); i++){//For each entity
				for (j = i, j++; j != cells[n].end(); j++){//For each other entity
					collision* c = collide(*i, *j);//Checks collision between entities
					
					if (c){//If collision happened
						list<collision>::iterator n;//Iterator for collisions
						for (n = result.begin(); n != result.end(); n++){//For each collision found up to now
							if ((n->a == *i && n->b == *j) || (n->a == *j && n->b == *i)){//If colliding entities are the same
								c = NULL;//Deletes collision
							}
						}
					}
					
					if (c){//If collision is still valid						
						result.push_back(*c);//Adds collision to result
						handleCollision(*i, *j, *c);//Handles collision
						
						//Vector components
						vector x (c->mtv.x, 0);
						vector y (0, c->mtv.y);
						
						if (!(*i)->lockTranslation && !(*j)->lockTranslation){//If no entity is locked
							if (!(*i)->lockX && !(*i)->lockY && !(*j)->lockX && !(*j)->lockY){//If entities are totally free
								(*i)->translate(c->mtv * (*j)->mass / ((*i)->mass + (*j)->mass));//Translates first
								(*j)->translate(-c->mtv * (*i)->mass / ((*i)->mass + (*j)->mass));//Translates second
							}
							
							else {//If at least one entity is locked on at least one axis
								if ((*i)->lockX && !(*j)->lockX)//If first is locked on x and second isn't
									(*j)->translate(-x);//Translates second on x
								else if ((*j)->lockX && !(*i)->lockX)//If second is locked on x and first isn't
									(*i)->translate(x);//Translates first on x
								else if (!(*i)->lockX && !(*j)->lockX){//If none is locked on x
									(*i)->translate(x * (*j)->mass / ((*i)->mass + (*j)->mass));//Translates first
									(*j)->translate(-x * (*i)->mass / ((*i)->mass + (*j)->mass));//Translates second
								}
									
								if ((*i)->lockY && !(*j)->lockY)//If first is locked on y and second isn't
									(*j)->translate(-y);//Translates second on y
								else if ((*j)->lockY && !(*i)->lockY)//If second is locked on y and first isn't
									(*i)->translate(y);//Translates first on y
								else if (!(*i)->lockY && !(*j)->lockY){//If none is locked on y
									(*i)->translate(y * (*j)->mass / ((*i)->mass + (*j)->mass));//Translates first
									(*j)->translate(-y * (*i)->mass / ((*i)->mass + (*j)->mass));//Translates second
								}
							}
						}
						
						else {
							if (!(*i)->lockTranslation) (*i)->translate(c->mtv);//Translates first if not locked
							if (!(*j)->lockTranslation) (*j)->translate(-c->mtv);//Translates second if not locked
						}
						
						free(c);//Frees collision
					}
				}			
			}
		}
		
		for (i = unassigned.begin(); i != unassigned.end(); i++){//For each unassigned cell
			for (j = i, j++; j != unassigned.end(); j++){//For each other entity
				collision* c = collide(*i, *j);//Checks collision between entities
				
				if (c){//If collision happened
					list<collision>::iterator n;//Iterator for collisions
					for (n = result.begin(); n != result.end(); n++){//For each collision found up to now
						if ((n->a == *i && n->b == *j) || (n->a == *j && n->b == *i)){//If colliding entities are the same
							c = NULL;//Deletes collision
						}
					}
				}
				
				if (c){//If collision is still valid						
					result.push_back(*c);//Adds collision to result
					handleCollision(*i, *j, *c);//Handles collision
					
					if (!(*i)->lockTranslation && !(*j)->lockTranslation){//If no entity is locked
						(*i)->translate(c->mtv * (*j)->mass / ((*i)->mass + (*j)->mass));//Translates first
						(*j)->translate(-c->mtv * (*i)->mass / ((*i)->mass + (*j)->mass));//Translates second
					}
					
					else {
						if (!(*i)->lockTranslation) (*i)->translate(c->mtv);//Translates first if not locked
						if (!(*j)->lockTranslation) (*j)->translate(-c->mtv);//Translates second if not locked
					}
					
					free(c);//Frees collision
				}
			}
		}
		
		return result;//Returns result
	}
	
	//Function for time step
	list<collision> step(double t){
		list <entity*> :: iterator i;//Iterator for entities
		deque <phLink*> :: iterator l;//Iterator for links
		
		for (l = links.begin(); l != links.end(); l++)//For each link
			(*l)->apply();//Applies link

		for (i = entities.begin(); i != entities.end(); i++)//For each entity
			(*i)->step(t, damping_tr, damping_rot);//Steps entity
			
		for (i = particles.begin(); i != particles.end(); i++)//For each particle
			(*i)->step(t, 0, 0);//Steps particle
		
		setCells();
		list<collision> result = collisions();
		
		for (i = toRemove.begin(); i != toRemove.end(); i++) particles.remove(*i);//Removes each particle
		toRemove.clear();//Clears removal list
		
		return result;
	}
	
	//Function to load from script obj
	virtual bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succeeded loading base data
			var* damping_tr = get <var> (&o.v, "damping_tr");//Translation damping
			var* damping_rot = get <var> (&o.v, "damping_rot");//Rotation damping
			var* w = get <var> (&o.v, "w");//Width
			var* h = get <var> (&o.v, "h");//Height
			
			if (damping_tr) this->damping_tr = damping_tr->doubleValue();//Sets translation damping
			if (damping_rot) this->damping_rot = damping_rot->doubleValue();//Sets rotation damping
			if (w) this->w = w->intValue();//Sets width
			if (h) this->h = h->intValue();//Sets height
			
			deque<object>::iterator i;//Sub object iterator
			for (i = o.o.begin(); i != o.o.end(); i++){//For each sub object
				objectBased* b = NULL;//Object to be loaded
				
				//Allocates object
				if (i->type == OBJTYPE_BOX) b = new box;
				else if (i->type == OBJTYPE_BALL) b = new ball;
				else if (i->type == OBJTYPE_SPRING) b = new spring;
				
				if (b && (i->type == OBJTYPE_BOX || i->type == OBJTYPE_BALL)){//If object is an entity
					entity* e = (entity*) b;//Converts to entity
					e->parent = this;//Sets parent
					e->fromScriptObj(*i);//Loads entity
					entities.push_back(e);//Adds to entities
				}
				
				else if (b){//Else (if object is a link)
					phLink* l = (phLink*) b;//Converts to link
					l->parent = this;//Sets parent
					l->fromScriptObj(*i);//Loads link
					links.push_back(l);//Adds to links
				}
			}
			
			return true;//Returns true
		}
		
		return false;//Returns false
	}
	
	//Function to save to script obj
	virtual object toScriptObj(){
		object result = objectBased::toScriptObj();//Result object
		
		//Sets variables
		result.set("damping_tr", damping_tr);
		result.set("damping_rot", damping_rot);
		result.set("w", w);
		result.set("h", h);
		
		list<entity*>::iterator i;//Entity iterator
		deque<phLink*>::iterator l;//Link iterator
		
		for (i = entities.begin(); i != entities.end(); i++) result.o.push_back((*i)->toScriptObj());//Adds all entities
		for (l = links.begin(); l != links.end(); l++) result.o.push_back((*l)->toScriptObj());//Adds all links
		
		return result;//Returns result
	}
	
	//Function for animation step in animated entities
	void animStep(){
		list <entity*>::iterator i;//Iterator
		
		for (i = entities.begin(); i != entities.end(); i++)//For each entity
			if ((*i)->useAnim) (*i)->anims.next();//Next frame of animation
	}
	
	//Function to create a particle entity
	entity* createParticle(vector position, vector speed, int mass, int radius, int color, int lifetime){
		ball* e = new ball;//New entity
		
		//Sets members
		e->position = position;
		e->speed = speed;
		e->mass = mass;
		e->radius = radius;
		e->color = color;
		e->lifetime = lifetime;
		
		e->parent = this;//Sets parent
		particles.push_back(e);//Adds to particles
		
		return e;//Returns ball
	}
};

//Entity function to check contact sensors
bool entity::checkSensor(int id){
	if (parent && id >= 0 && id < sensors.size()){//If parent was given and sensor id is valid
		vector* sensor = sensors[id];//Requested sensor
		
		int tileX = floor(sensor->x / (parent->w / DIVISIONS));//Sensor x tile
		int tileY = floor(sensor->y / (parent->h / DIVISIONS));//Sensor y tile
		
		int cellIndex = tileX + tileY * DIVISIONS;//Cell index
		
		if (tileX >= 0 && tileX < DIVISIONS && tileY >= 0 && tileY < DIVISIONS){//If tile coords are valid
			list<entity*>::iterator i;//Iterator
			
			for (i = parent->cells[cellIndex].begin(); i != parent->cells[cellIndex].end(); i++)//For each entity in cell
				if (*i != this && (*i)->isInside(*sensor)) return true;//Returns true if sensor is inside entity
		}
	}
	
	return false;//Returns false
}

//Function to move entity
void entity::step(double t, double trDamp, double angDamp){
	//Applies individual damping factors
	trDamp *= damping_tr;
	angDamp *= damping_rot;
	
	curLifetime += t;//Increases lifetime

	//Translation
	if (!lockTranslation){//If entity can move
		vector oldAccel = accel;//Old acceleration
		
		accel = (force) / mass;//Sets acceleration
	
		vector damp = speed + ((oldAccel + accel) / 2 + speed * trDamp) * t;//New speed with damping
		vector noDamp = speed + ((oldAccel + accel) / 2) * t;//New speed without damping
		
		vector direction = speed.setModule(1);//Motion direction (normalized)
		
		if ((damp.dot(direction) >= 0) == (noDamp.dot(direction) >= 0))//If didn't change direction
			speed = damp;//Sets speed with damping
		else speed = {0, 0};//Else stops
		
		vector tr = speed * t + (accel + oldAccel) / 2 * 0.5 * t * t;//Translation vector
		
		if (lockX) tr.x = 0;//Removes X component
		if (lockY) tr.y = 0;//Removes Y component
		
		translate(tr);//Translates
	}
	
	else speed = {0, 0};//Else stops speed

	//Rotation
	if (!lockRotation){//If entity can rotate
		double oldAlpha = alpha;//Old angular acceleration
		
		alpha = (torque) / inertia();//Calculates new angular acceleration
		
		double damp = omega + ((oldAlpha + alpha) / 2 + omega * angDamp) * t;//New angular speed with damping
		double noDamp = omega + ((oldAlpha + alpha) / 2) * t;//New angular speed without damping
		
		if (damp > 0 == noDamp > 0)//If damping wouldn't invert rotation
			omega = damp;//Sets angular speed with damping
		else //Else
			omega = 0;//Stops rotation
			
		rotate(omega * t + (alpha + oldAlpha) / 2 * 0.5 * t * t);//Rotates
	}
	
	else omega = 0;//Else stops rotation
	
	if (lifetime > 0 && curLifetime >= lifetime && parent)//If reached max lifetime
		parent->toRemove.push_back(this);//Erases entity from parent
}

//Function to load link from script object
bool phLink::fromScriptObj(object o){
	if (objectBased::fromScriptObj(o)){//If succeeded loading base data
		var* a = get <var> (&o.v, "a");//A entity
		var* b = get <var> (&o.v, "b");//B entity
		var* a_point = get <var> (&o.v, "a_point");//A point
		var* b_point = get <var> (&o.v, "b_point");//B point
		
		if (a && parent) this->a = get_ptr <entity> (&parent->entities, a->value);//Gets a entity
		if (b && parent) this->b = get_ptr <entity> (&parent->entities, b->value);//Gets b entity
		
		if (this->a && a_point->intValue() < this->a->nodes.size()){//If a entity set successfully
			this->a_point = this->a->nodes[a_point->intValue()];//Gets a point
			this->aPointIndex = a_point->intValue();//Gets a point index
		}
		else this->a_point = new vector (a_point->value);//Else loads from string
		
		if (this->b && b_point->intValue() < this->b->nodes.size()){//If b entity set successfully
			this->b_point = this->b->nodes[b_point->intValue()];//Gets b point
			this->bPointIndex = b_point->intValue();//Gets b point index
		}
		else this->b_point = new vector (b_point->value);//Else loads from string
	
		return true;//Returns true
	}
	
	return false;//Returns false
}

//Function to load a scene from a file
scene* loadScene(string path){
	fileData source (path);//Loads and processes file
	
	object s = source.objGen(path);//Scene object
	s.type = OBJTYPE_SCENE;//Sets object typ
	
	scene *result = new scene;//Result scene
	result->fromScriptObj(s);//Loads result
	
	return result;//Returns scene
}

#endif