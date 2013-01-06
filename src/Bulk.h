//Buch's Library Kollection
//Main file - includes other headers and defines common methods

#ifndef _BULK
#define _BULK

#include <iostream>//Includes standard I/O streams
#include <string>//Includes strings
#include <sstream>//Includes stringstreams
#include <fstream>//Includes filestreams
#include <deque>//Includes deques
#include <list>//Includes lists
#include <cstdlib>//Includes standard C library
#include <cstring>//Includes standard C string library
#include <ctime>//Includes C time header
#include <cmath>//Includes standard C math library

using namespace std;//Uses standard namespace

//Conversion functions
template <class type> string toString (type);//Function to convert anything to string

//String splitting functions
template <class containerType> containerType tokenize(string, string); //Function to split a string into tokens (delimiters are not included in result)
string getFirstToken(string, string); //Function to get first token of a string

//String joining function
template <class containerType> string join(containerType, string); //Function to join all strings of a deque of strings into an only string

//Misc
int howMany(string, char);//Function to count the number of repetitions of a character in a string

#include "Bulk_cache.h"//Includes the cache handling header
#include "Bulk_console.h"//Includes the console handling header
#include "Bulk_script.h"//Includes the script handling header
#include "Bulk_language.h"//Includes the localization handling header

#include "Bulk_expr.h"//Includes the expression handling header
#include "Bulk_expr_double.h"//Includes the double expressions handling header

//Function to get a random element from a container
template <class type, class containerType> type random(containerType container){
	int result = rand() % container.size();//Result index
	typename containerType::iterator i;//Iterator
	int counter;//Counter
	
	for (counter = 0, i = container.begin(); counter < result; counter++, i++);//Moves to requested element
	
	return *i;//Returns random element
}

//Function to split a string into tokens (delimiters are not included in result)
//	first argument is the string to split
//	second argument is a string containing all the delimiters
template <class containerType> containerType tokenize(string source, string delimiters){
	containerType result;//Result
	string current = "";//Current token
	int i;//Counter
	
	for (i = 0; i < source.size(); i++){//For each character in source string
		if (delimiters.find(source[i]) != delimiters.npos){//If character is part of delimiters string
			if (current != ""){//If current token is not empty
				result.push_back(current);//Adds token to result
				current = "";//Resets the token
			}
		}
		
		else {//If character is not part of delimiter string
			current += source[i];//Adds character to current token
		}
	}
	
	if (current != "") result.push_back(current);//Adds last token if it's not empty
	
	return result;//Returns result
}

//Function to get first token of a string (delimiters not included in result)
//	first argument is the source string
//	second argument is a string containing all the delimiters
string getFirstToken(string source, string delimiters){
	string result = "";//Result
	int i;//Counter
	
	for (i = 0; i < source.size(); i++){//For each character in source string
		if (delimiters.find(source[i]) != delimiters.npos){//If character is a delimiter
			if (result != "") return result;//Returns result if not empty
		}
		
		else result += source[i];//Else adds character to result
	}
	
	return result;//Returns result
}

//Function to join all strings of a deque of strings into an only string
//	first argument is the deque of strings to join
//	second argument is the string placed between each string
template <class containerType> string join (containerType source, string delimiters){
	typename containerType::iterator i;//Iterator
	string result = "";//Result
	
	for (i = source.begin(); i != source.end() - 1; i++)//For each element except the last
		result += *i + delimiters;//Adds element to result
		
	result += *i;//Adds last element
	
	return result;//Returns result
}

//Function to convert anything to string
//	first argument is the obejct to convert
template <class type> string toString (type source){
	stringstream s;//Stringstream used for conversion
	s << source;//Outputs source on stream
	return s.str();//Returns string content of stream
}

//Function to count the repetition of chars in a string
int howMany(string s, char c){
	int result = 0;//Result
	int pos = s.find(c);//Position of char to find
	
	while (pos != s.npos){//While other occurrencies of the char are found
		result++;
		pos = s.find(c, pos + 1);//Next char
	}
	
	return result;//Returns the result
}

//Function to swap content
template <class type> void swap (type* a, type* b){
	type c = *b;
	
	*b = *a;
	*a = c;
}

#endif