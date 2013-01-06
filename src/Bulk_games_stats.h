//Buch's Library Kollection for games
//Flexible game statistics handling header

//(requires Bulk to be included)

#ifndef _BULK_GAMES_STATS
#define _BURP_GAMES_STATS

#define OBJTYPE_ATTR_PRIMARY		"primaryAttribute"//Primary attribute object type
#define OBJTYPE_ATTR_SECONDARY		"secondaryAttribute"//Secondary attribute object type
#define OBJTYPE_ATTRIBUTES			"attributeList"//Attribute list object type

//Prototypes and global variables
class attribute;//Base attribute class
class primaryAttribute;//Primary attribute class
class secondaryAttribute;//Secondary attribute class

class attributes;//Attribute collection class

attributes* attr_searching = NULL;//Attribute collection currently being searched by function below
primaryAttribute* getAttr(string);//Attribute searching function

//Base attribute class
//	Does not define the way attribute value is stored, but gives an interface
//	to handle both primary and secondary attributes
class attribute: public objectBased {
	public:	
	attributes* parent;//Parent attribute collection
	
	//Constructor
	attribute(){
		id = "";
		type = OBJTYPE_ATTR_PRIMARY;
		
		parent = NULL;
	}
	
	//Virtual function to get attribute value
	virtual double value(){ return 0; }
};

//Primary attribute class
//	a primary attribute has its own value
class primaryAttribute: public attribute {
	public:
	double amount;//Attribute amount
	
	//Constructor
	primaryAttribute(){
		objectBased::id = "";
		objectBased::type = OBJTYPE_ATTR_PRIMARY;
		
		amount = 0;
	}
	
	//Virtual function to get attribute value
	double value(){ return amount; }
};

//Function to get a primary attribute value from string
template <> operand<primaryAttribute> operandValueFromString(string source){
	operand<primaryAttribute> result;//Result
	
	primaryAttribute constAttribute;//Constant integer attribute
	constAttribute.amount = atof(source.c_str());//Converts source string
	
	result.set(constAttribute);//Sets result value
	return result;//Returns result
}

//Function for attribute sum
primaryAttribute attr_sum (primaryAttribute a, primaryAttribute b){
	primaryAttribute constAttribute;//Constant attribute
	constAttribute.amount = a.amount + b.amount;//Sets amount
	return constAttribute;//Returns attribute
}

//Function for attribute mult
primaryAttribute attr_multiply (primaryAttribute a, primaryAttribute b){
	primaryAttribute constAttribute;//Constant attribute
	constAttribute.amount = a.amount * b.amount;//Sets amount
	return constAttribute;//Returns attribute
}

//Function for attribute subtraction
primaryAttribute attr_subtract (primaryAttribute a, primaryAttribute b){
	primaryAttribute constAttribute;//Constant attribute
	constAttribute.amount = a.amount - b.amount;//Sets amount
	return constAttribute;//Returns attribute
}

//Function for attribute division
primaryAttribute attr_divide (primaryAttribute a, primaryAttribute b){
	primaryAttribute constAttribute;//Constant attribute
	constAttribute.amount = a.amount / b.amount;//Sets amount
	return constAttribute;//Returns attribute
}

list< op<primaryAttribute> > attrOps;//Attributes operators

//Secondary attribute class
class secondaryAttribute: public attribute{
	public:
	expr <primaryAttribute> amount;//Attribute amount expression
	
	//Constructor
	secondaryAttribute(){
		id = "";
		type = OBJTYPE_ATTR_SECONDARY;
	}
	
	//Function to get value
	double value() {
		attributes* old = attr_searching;//Backs up the attribute searching pointer
		attr_searching = parent;//Sets the attribute collection being searched
		
		double result = amount.calculate(getAttr).value();//Result
		
		attr_searching = old;//Resets old attr_searching value
		
		return result;//Returns result
	}
};

//Attribute collection class
class attributes: public list<attribute*>, public objectBased {
	public:
	
	//Constructor
	attributes(){
		id = "";
		type = OBJTYPE_ATTRIBUTES;
	}
	
	//Function to add an attribute, given its value
	attribute* add(string id, double value){
		primaryAttribute *newAtt = new primaryAttribute;//New attribute
		
		newAtt->id = id;//Sets attribute id
		newAtt->amount = value;//Sets attribute amount
		newAtt->parent = this;//Sets attribute parent
		
		push_back(newAtt);//Adds attribute to list
		
		return newAtt;//Returns newly added attribute
	}
	
	//Function to add an attribute, given a string expression which defines it
	attribute* add(string id, string expression){
		expr<primaryAttribute> e;//Expression
		e.fromString(expression, &attrOps);//Loads expression
		
		if (e.constant())//If expression is constant
			return add(id, e.calculate().value());//Normally adds constant attribute*/
		
		else {
			secondaryAttribute* newAttr = new secondaryAttribute;//New secondary attribute
			
			newAttr->id = id;//Sets attribute id
			newAttr->amount = e;//Sets attribute expression
			newAttr->parent = this;//Sets attribute parent
			
			push_back(newAttr);//Adds attribute to list
			
			return newAttr;//Returns newly created attribute
		}
	}
	
	//Function to get the value of a specific attribute
	//	if the attribute is not found, this function returns 0
	double get(string id){
		attribute* a = get_ptr <attribute> (this, id);//Gets the attribute
		
		if (a) return a->value();//Returns the attribute value if the attribute was found
		else return 0;//Else returns 0
	}
	
	//Function to set the value of a specific primary attribute
	//	if the attribute is not found, a new primary attribute will be created with that id and value
	void set(string id, double value){
		attribute* a = get_ptr <attribute> (this, id);//Gets the attribute
		
		if (a && a->getType() == OBJTYPE_ATTR_PRIMARY)//If the attribute was found and it is primary
			((primaryAttribute*) a)->amount = value;//Sets attribute value
			
		else if (!a)//Else if the attribute was not found
			add(id, value);//Adds the attribute
	}
	
	//Print function (temporary)
	void print(){
		iterator i;
		for (i = begin(); i != end(); i++)
			cout << (*i)->getType() << "\t" << (*i)->id << "\t" << (*i)->value() << endl;
			
		cout << endl;
	}
	
	//Function to load attribute list from script object
	bool fromScriptObj(object o){
		if (objectBased::fromScriptObj(o)){//If succedeed in loading base data
			deque<var>::iterator i;//Iterator for variables
			
			for (i = o.v.begin(); i != o.v.end(); i++)//For each variable
				add(i->id, i->value);//Adds corresponding attribute to the list
				
			return true;//Returns true
		}
		
		return false;//Returns false
	}
};

//Attribute searching function
primaryAttribute *getAttr(string id){
	if (attr_searching){//If there's a searching field
		attribute* a = get_ptr <attribute> (attr_searching, id);//Gets attribute
		
		if (a->getType() == OBJTYPE_ATTR_PRIMARY) return (primaryAttribute*) a;//Returns attribute if it is primary
	}
	
	return NULL;//Returns null
}

//Initialization function
bool Bulk_games_stats_init(){
	op<primaryAttribute> doubleAttr_plus, doubleAttr_minus, doubleAttr_mult, doubleAttr_div;//Double attributes operators
	
	//Sets operators
	doubleAttr_plus.sign = "+";
	doubleAttr_plus.operate = attr_sum;
	
	doubleAttr_minus.sign = "-";
	doubleAttr_minus.operate = attr_subtract;
	
	doubleAttr_mult.sign = "*";
	doubleAttr_mult.operate = attr_multiply;
	
	doubleAttr_div.sign = "/";
	doubleAttr_div.operate = attr_divide;
	
	//Adds operators to list
	attrOps.push_back(doubleAttr_plus);
	attrOps.push_back(doubleAttr_minus);
	attrOps.push_back(doubleAttr_mult);
	attrOps.push_back(doubleAttr_div);
	
	return true;//Returns true
}

#endif