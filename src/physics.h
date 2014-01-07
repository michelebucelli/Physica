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
PHYSICS
*/

#ifndef _PHYSICS
#define _PHYSICS

#include "content.h"
#include "vect.h"
#include "log.h"
#include <deque>
#include <list>

#define DIVISIONS						10//Scene divisions for space partitioning collision detection

//NOTE:
//Still need to define load function for all the classes.
//Also, for the editor, save functions will be needed
//It would also be interesting to add javascript support to
//those classes

class scene;

//Entity type enumeration
enum entityType {
	etype_generic,
	etype_box
};

//Entity base class
class entity: public content {
	public:
	
	//General data
	entityType eType;
	int axesCount;//Number of axes of the entity
	
	double mass;
	
	double e;//Coefficient of restitution (used to calculate bouncing)
	
	deque<vect*> nodes;//Entity nodes (points, rotated and traslated with entity)
	deque<vect*> sensors;//Contact sensors (rotated and traslated with entity)
	
	//Individual damping factors
	//Effective damping is calculated: scene damping * entity damping
	double damping_tr;//Translation damping factor
	double damping_rot;//Rotation damping factor
	
	//Relatives
	scene* parent;
	
	//Translation
	
	//Translation locks
	//If lockTranslation is true, others are ignored
	bool lockTranslation;
	bool lockX, lockY;
	
	vect force;
	vect accel;
	vect speed;
	vect position;
	
	//Rotation
	bool lockRotation;
	
	double torque;
	double alpha;//Angular acceleration
	double omega;//Angular speed
	double theta;//Effective angle
	
	//Graphics
	int color;//Entity color
	bool print;//Print flag (print entity only if true)
	
	bool light;//Light flag
	int lightColor;//Light color
	
	//Misc
	string special;//Special additional data
	
	entity();
	
	//Returns the inertia momentum around centre
	//Virtual because depends on entity type
	virtual double inertia() = 0;
	
	//Resets forces
	void resetForces();
	
	//Translates entity by given vector
	virtual void translate (vect);
	
	//Rotates the entity by given angle (in radiants)
	virtual void rotate (double);
	
	//Returns entity axes for collision detection
	virtual vect* getAxes () = 0;
	
	//Applies a force on given point
	void applyForce (vect, vect);
	
	//Applies an impulse on given point 
	void applyImpulse (vect, vect);
	
	//Moves entity by given time
	void step(double, double, double);
	
	//Returns true if a point is inside the entity
	virtual bool isInside(vect) = 0;
	
	//Calculates the speed of a point moving with the entity
	vect pointSpeed (vect);
	
	//Adds a new node to the entity (returns handle to node)
	vect* addNode (vect);
	
	//Adds a sensor to the entity (returns sensor id)
	int addSensor (vect);
	
	//Checks total lock
	bool locked();
	
	//Checks contact sensors
	bool checkSensor(int);
	
	//Loads entity
	void load(xml_node);
	
	//Clones entity (at the end, argument contains copy of entity)
	virtual void clone (entity*);
};

//Box class
class box: public entity {
	public:
	double w, h;
	vect point[4];//Vertices
	
	box();
	
	void resize (double, double);
	void translate(vect);
	void rotate (double);
	double inertia();
	vect* getAxes ();
	bool isInside(vect);
	void clone(entity*);
	
	void load(xml_node);
};

//Collision structure
struct collision { 
	vect mtv;//Minimum translation vector
	vect n;//Collision normal
	vect p;//Collision point
	
	double e;//Coefficient of restitution
	
	entity* a;//First entity
	entity* b;//Second entity
};

//Link class
//Reprensents a force interaction between two entities
class phLink: public content {
	public:
	entity* a;//First entity
	vect a_point;//First entity application point (relative to entity position and orientation)
	
	entity* b;//Second entity
	vect b_point;//Second entity application point (relative to entity position and orientation)
	
	scene* parent;
	
	phLink();
	void load(xml_node);
	
	//Calculates the force of the link
	virtual vect force() = 0;
	
	//Applies the force to the two entities
	void apply();
	
	//Calculates link length
	double length();	
};

//Spring class
//Derived from link, represents a spring interaction
class spring: public phLink {
	public:
	double length_zero;//Base length (when length is equal to this, force is 0)
	double k;//Spring coefficient
	
	spring();
	vect force();
	void load(xml_node);
};

//Scene class
class scene: public content {
	public:
	list <entity*> entities;//Entities within the scene
	list <phLink*> links;//Links between entities
	
	double damping_tr;//Translation damping factor
	double damping_rot;//Rotation damping factor
	
	int w, h;//Scene size
	
	//Space partitioning
	//Collisions are checked only between entities belonging to the same list
	list <entity*> cells [DIVISIONS * DIVISIONS];//Cells
	list <entity*> unassigned;
	
	scene();
	~scene();
	void load(xml_node);
	
	//Resets forces on all entities in scene
	void resetForces();
	
	//Applies a force on all entities on scene (applied on entity centre)
	void applyGlobalForce(vect);
	
	//Applies gravity
	//The effective force depends on the entity mass, so that all entities
	//have the same resulting acceleration (applied on entity centre)
	void applyGravity(vect);
	
	//Assigns entities to space partitioning cells
	void setCells();
	
	//Checks and handles collisions, and returns a list of collisions found
	list<collision> collisions();
	
	//Time step (returns the list of the collisions)
	list<collision> step(double);
	
	//Returns requested entity
	entity* getEntity(string);
};

//Returns the projection of an entity onto a vector
//The third parameter is set to the beginning point of the projection
//so that the result summed with that vector corresponds
//to the ending point of the projection
vect projection (entity*, vect, vect*);

//Calculates the projection of an entity onto a vector
//Sets the two double pointers parameters respectively
//to the beginning and ending points of the projection
//(the value set is the distance between the origin and the
//projection's beginning and ending point)
void dProjection (entity*, vect, double*, double*);

//Calculates the collision point between two entities
//Does not check if entities are effectively colliding
vect collisionPoint (entity*, entity*, vect);

//Determines if two entities are colliding
//If true, returns the collision data; else returns NULL
collision* collide (entity*, entity*);

//Handles collision between two entities
void handleCollision (collision);

//Handles collisions between entities in a list
list<collision> handleCollisions (list <entity*> * , list <collision> *);

#endif