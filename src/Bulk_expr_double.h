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
double doubleDiff (double a, double b) { return a != b; }
double doubleAnd (double a, double b) { return a && b; }
double doubleOr (double a, double b) { return a || b; }

list< op<double> > doubleOps;//Double operators list

//Initialization function
void Bulk_doubleExpr_init(){
	op <double> sum, subt, mult, div, greater, less, equal, diff, andOp, orOp;
	
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
	diff.sign = "!=";
	diff.operate = doubleDiff;
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
	doubleOps.push_back(diff);
	doubleOps.push_back(andOp);
	doubleOps.push_back(orOp);
}

#endif