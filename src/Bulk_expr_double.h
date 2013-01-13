//Bulk expression header extension
//numeric expressions

#ifndef _BULK_EXPR_DOUBLE
#define _BULK_EXPR_DOUBLE

//String conversion function
template <> operand<double> operandValueFromString(string source){
	operand<double> result;//Result operand
	result.set(atof(source.c_str()));//Sets result operand
	return result;//Returns result
}

//Operators functions
double doubleSum(double a, double b) { return a + b; }
double doubleSubtract(double a, double b) { return a - b; }
double doubleMultiply(double a, double b) { return a * b; }
double doubleDivide(double a, double b) { return a / b; }
double doubleGreater (double a, double b) { return a > b; }
double doubleLess (double a, double b) { return a < b; }
double doubleEqual (double a, double b) { return a == b; }
double doubleAnd (double a, double b) { return a && b; }
double doubleOr (double a, double b) { return a || b; }

list< op<double> > doubleOps;//Double operators list

//Initialization function
void Bulk_doubleExpr_init(){
	op <double> sum, subt, mult, div, greater, less, equal, andOp, orOp;
	
	sum.sign = "+";
	sum.operate = doubleSum;
	subt.sign = "-";
	subt.operate = doubleSubtract;
	mult.sign = "*";
	mult.operate = doubleMultiply;
	div.sign = "/";
	div.operate = doubleDivide;
	greater.sign = ">";
	greater.operate = doubleGreater;
	less.sign = "<";
	less.operate = doubleLess;
	equal.sign = "=";
	equal.operate = doubleEqual;
	andOp.sign = "&";
	andOp.operate = doubleAnd;
	orOp.sign = "|";
	orOp.operate = doubleOr;
	
	doubleOps.push_back(sum);
	doubleOps.push_back(subt);
	doubleOps.push_back(mult);
	doubleOps.push_back(div);
	doubleOps.push_back(greater);
	doubleOps.push_back(less);
	doubleOps.push_back(equal);
	doubleOps.push_back(andOp);
	doubleOps.push_back(orOp);
}

#endif