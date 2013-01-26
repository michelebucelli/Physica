//Bulk expression manager
//(requires Bulk_script.h

#ifndef _BULK_EXPR
#define _BULK_EXPR

#define RUNTIMEREFCODE					'$'//Code for runtime referencing values	
#define REFCODE							'\%'//Code for references

//Prototypes
template <class> class operand;
template <class> class op;
template <class> class expr;

//Operand type enumeration
typedef enum {
	operand_value,//Value type
	operand_reference,//Reference type
	operand_runtimeRef,//Runtime reference type (saved as string, searched upon calculation)
	operand_expr//Expression type
} operandTypeEnum;

//Operand class
//	can be a value, a reference or an expression
template <class operandType> class operand {
	private:
	operandTypeEnum opType;//Operand type
	
	operandType value;//Value
	operandType *reference;//Reference
	string runtimeRef;//Runtime reference (see enum)
	expr<operandType> *expression;//Expression
	
	public:
	
	friend class expr<operandType>;//Declares expression class as friend
	
	//Constructor
	operand(){
		opType = operand_value;
		
		reference = NULL;
		runtimeRef = "";
		expression = NULL;
	}
	
	//Function to set value
	void set(operandType value){
		opType = operand_value;//Sets operand type
		this->value = value;//Sets operand value
	}
	
	//Function to set reference
	void set(operandType* reference){
		opType = operand_reference;//Sets operand type
		this->reference = reference;//Sets operand reference
	}
	
	//Function to set runtime referecne
	void set(string runtimeRef){
		opType = operand_runtimeRef;//Sets runtime reference type
		this->runtimeRef = runtimeRef;//Sets runtime reference
	}
	
	//Function to set expression
	void set(expr <operandType> *expression){
		opType = operand_expr;//Sets expression type
		this->expression = expression;//Sets operand expression
	}
	
	//Function to get the value
	operandType getValue(operandType* (*)(string) = NULL);
	
	//Function to convert the operand to string
	string opToString();
};

//Function to load an operand value from a string
//	the default function simply returns a new operand value - no conversion
//	additional types have to be redefined
template <class operandType> operand<operandType> operandValueFromString(string source){
	operand<operandType> o;//New operand
	o.set(*new operandType);//Sets new operand type
	return o;//Returns operand
}

//Operator class
//	the template argument is the operand type
template <class operandType> class op {
	public:
	string sign;//Operator sign
	operandType (*operate)(operandType, operandType);//Operator function
	
	//Constructor
	op(){
		sign = "";
		operate = NULL;
	}
	
	//Operator execution operator
	operandType operator () (operandType, operandType);
};

//Expression class
template <class operandType> class expr {
	public:
	deque< operand <operandType> > operands;//List of all the operands
	deque< op <operandType> > operators;//List of all the operators
	
	//Function to calculate the expression
	operandType calculate(operandType* (*getFunc)(string) = NULL){
		operandType current = operands.front().getValue(getFunc);//First value
		
		int i;//Counter
		for (i = 0; i < operators.size() && i + 1 < operands.size(); i++){//For each operator
			current = operators[i](current, operands[i + 1].getValue(getFunc));//Calculates result
		}

		return current;//Returns value
	}
	
	//Function to load from string
	void fromString(string source, list< op<operandType> > *ops, operandType* (*getFunc)(string) = NULL){
		operands.clear();//Clears operands
		operators.clear();//Clears operators
		
		deque<string> toks = tokenize < deque<string> > (source, " \t");//Splits the string into tokens
		
		deque<string>::iterator i;//Iterator
		bool expectOp = false;//Indicates whether to expect an operator or not
		
		for (i = toks.begin(); i != toks.end(); i++){//For each token
			if (expectOp) {//If operator is expected
				typename list< op<operandType> >::iterator j;//Operators iterator
				
				for (j = ops->begin(); j != ops->end(); j++){//For each operator in given list
					if (j->sign == *i){//If operator sign is matching
						operators.push_back(*j);//Adds it to operators
						break;//Exits loop
					}
				}
				
				if (j == ops->end())//If didn't find operator
					cerr << "EXPR ERROR: expected known operator, found " << *i << endl;//Error message
			}
			
			else if ((*i)[0] != RUNTIMEREFCODE && (*i)[0] != REFCODE && *i != "(" && *i != ")")//If token is not a reference or a parenthesis
				operands.push_back(operandValueFromString <operandType> (*i));//Adds operand
				
			else if (*i == "("){//If token is an open parenthesis
				int openCount = 1;//Open parenthesis count
				string innerExpr = "";//Inner expression
				
				for (i++; i != toks.end() && openCount > 0; i++){//For the following tokens
					if (*i == "(") openCount++;//Increases open parenthesis count
					if (*i == ")") openCount--;//Reduces open parenthesis count
					
					if (openCount > 0)//If didn't close the parenthesis
						innerExpr += *i + " ";//Adds token to inner expression
				}
				
				expr <operandType> *e = new expr <operandType>;//New expression
				e->fromString(innerExpr, ops, getFunc);//Sets new expression
				
				operand <operandType> newOperand;//New operand
				newOperand.set(e);//Sets operand
				operands.push_back(newOperand);//Adds operand
				
				expectOp = false;//Doesn't expect operator (so when toggles, expectOp is set to true)
				
				i--;//Goes back to previous token
			}
			
			else if ((*i)[0] == REFCODE && getFunc){//If token is a direct reference and getFunc was specified
				operand <operandType> newOperand;//New operand
				newOperand.set(getFunc(i->substr(1)));//Sets operand
				operands.push_back(newOperand);//Adds the operand to expression
			}
				
			else if ((*i)[0] == RUNTIMEREFCODE) {//If token is a runtime reference
				operand <operandType> newOperand;//New operand
				newOperand.set(i->substr(1));//Sets operand value
				operands.push_back(newOperand);//Adds the operand to expression
			}
			
			else cerr << "EXPR ERROR: unknown token " << *i << endl;//Error message
			
			expectOp = !expectOp;//Toggles expecting operators
		}
	}
	
	//Function to determine if the expression is constant (always results the same)
	//	an expression is constant when all its operands are values or constant expressions
	bool constant(){
		typename deque< operand<operandType> >::iterator i;//Iterator
		
		for (i = operands.begin(); i != operands.end(); i++)//For each operand
			if (!(i->opType == operand_value || (i->opType == operand_expr && i->expression->constant()))) return false;//If operand is not constant, returns false
		
		return true;//Returns true
	}
	
	//Function to convert the expression to string
	string exprToString(){
		int i;//Iterator
		string result = "";//Result
		
		for (i = 0; i < operands.size() - 1; i++)//For each operand
			result += operands[i].opToString() + " " + operators[i].sign + " ";//Adds to string
			
		result += operands.back().opToString();//Adds last operand
		
		return result;//Returns result
	}
};

//Function to get the value of an operand
template <class operandType> operandType operand<operandType>::getValue(operandType* (*getFunc)(string)){
	if (opType == operand_value) return value;//Returns value
	else if (opType == operand_reference) return *reference;//Returns de-referenced value
	else if (opType == operand_expr) return expression->calculate(getFunc);//Returns calculated expression
	else if (opType == operand_runtimeRef && getFunc) return *getFunc(runtimeRef);//Returns de-referenced value from runtime reference
}

//() operator
//	executes the operator if its function was given
template <class operandType> operandType op<operandType>::operator () (operandType a, operandType b){
	if (operate)//If operation function was given
		return operate(a, b);//Returns calculated value
	
	else return *new operandType;//Else returns the default operand
}

//Function to convert the operand to string
template <class operandType> string operand<operandType>::opToString(){
	if (opType == operand_value) return toString(value);
	else if (opType == operand_runtimeRef) return "$" + runtimeRef;
	else if (opType == operand_expr) return "( " + expression->exprToString() + " )";
}

#endif