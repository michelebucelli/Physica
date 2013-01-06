//Buch's library Kollection for games
//Resources handling header

#ifndef _BULK_GAMES_RESOURCES
#define _BULK_GAMES_RESOURCES

#define OBJTYPE_RESOURCES		"res"//Resources type

//Resource class
class res: public objectBased {
	public:
	int amount;//Current resource amount
	
	//Constructor
	res(){
		amount = 0;
	}
	
	//Function to add to resource
	void add(int amount){
		this->amount += amount;//Increases resource amount
	}
	
	//Function to pay resource (returns false if couldn't pay)
	bool pay(int amount){
		if (this->amount >= amount){//If can pay
			this->amount -= amount;//Subtracts paid amount
			return true;//Returns true
		}
		
		return false;//Returns false
	}
};

//Resource collection class
class resources: public deque<res>, public objectBased {
	public:
	
	//Constructor
	resources(){
		id = "";
		type = OBJTYPE_RESOURCES;
	}
	
	//Function to set given resource
	void set(string id, int amount){
		iterator i;//Iterator
		
		for (i = begin(); i != end(); i++){//For each resource
			if (i->id == id){//If id matches
				i->amount = amount;//Sets resource amount
				return;//Exits function
			}
		}
		
		res newRes;//New resource
		newRes.id = id;//Sets resource id
		newRes.amount = amount;//Sets resource samount
		push_back(newRes);//Adds resource
	}
	
	//Function to add given amount to given resource
	//	if the resource doesn't exist, creates it
	void add(string id, int amount){
		iterator i;//Iterator
		
		for (i = begin(); i != end(); i++){//For each resource
			if (i->id == id){//If id matches
				i->add(amount);//Increases resource amount
				return;//Exits function
			}
		}
		
		res newRes;//New resource
		newRes.id = id;//Sets resource id
		newRes.amount = amount;//Sets resource samount
		push_back(newRes);//Adds resource
	}
	
	//Function to pay given amount of given resource
	bool pay(string id, int amount){
		iterator i;//Iterator
		
		for (i = begin(); i != end(); i++)//For each resource
			if (i->id == id) return i->pay(amount);//If id matches pays resource
			
		return false;//Returns false if didn't find resource
	}
	
	//Function to get amount of given resource
	int amount(string id){
		iterator i;//Iterator
		
		for (i = begin(); i != end(); i++)//For each resource
			if (i->id == id) return i->amount;//If id matches returns resource amount
		
		return 0;//Returns 0 if resource wasn't found
	}
	
	//Function to add resources
	void operator += (resources r){
		iterator i;//Iterator
		
		for (i = r.begin(); i != r.end(); i++)//For each resource
			add(i->id, i->amount);//Adds resource
	}
	
	//Function to load from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succedeed loading base data
			deque<var>::iterator i;//Iterator
			
			for (i = o.v.begin(); i != o.v.end(); i++)//For each variable
				add(i->id, i->intValue());//Adds corresponding resource
		}
		
		return false;//Returns false
	}
	
	//Function to save to script object
	object toScriptObj(){
		object result = objectBased::toScriptObj();//Base object
		
		iterator i;//Iterator
		for (i = begin(); i != end(); i++)//For each resource
			result.set(i->id, i->amount);//Sets variable
			
		return result;//Returns result
	}
};

#endif