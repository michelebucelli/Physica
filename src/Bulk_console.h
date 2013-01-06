//Buch's Library Kollection
//Console handling header

//This header provides a framework to quickly setup prompt-based consoles
//through any input-output type.

#ifndef _BULK_CONSOLE
#define _BULK_CONSOLE

#define CMD_ECHO			"echo"//Default echo command

//Console command class
class console_command {
	public:
	string keyword;//Command keyword
	void (*func)(string);//Command function
	
	//Constructor
	console_command(){
		keyword = "";
		func = NULL;
	}
};

//Console class
class console {
	public:
	void (*out)(string);//Output function
	deque<console_command> commands;//Known commands
	
	//Constructor
	console(){
		out = NULL;
	}
	
	//Function to input to the prompt
	//	the argument is the whole command string; the first token is taken as keyword, the others as arguments
	void input(string command){
		string keyword = getFirstToken(command, " \t");//Gets command keyword
		int i;//Counter
		
		for (i = 0; i < commands.size(); i++){//For each command
			if (commands[i].keyword == keyword && commands[i].func){//If command has the same keyword and a valid function
				commands[i].func(keyword.size() < command.size() ? command.substr(keyword.size() + 1) : "");//Calls command function
				return;//Exits function
			}
		}
		
		if (keyword == CMD_ECHO){//If command is echo and no other command was found
			if (out) out(command.substr(keyword.size() + 1) + "\n");//Echoes the argument if there's an output function
		}
	}
	
	//Function to add a command to the console
	//	c is the added command
	void addCommand(console_command c){
		commands.push_back(c);//Adds new command
	}
	
	//Function to add a command to the console
	//	keyword is the keyword of the new command
	//	func is the function of the new command
	void addCommand(string keyword, void (*func)(string)){
		console_command newCommand;//New command
		
		newCommand.keyword = keyword;//Sets command keyword
		newCommand.func = func;//Sets command function
		
		commands.push_back(newCommand);//Adds command to known commands
	}
};

#endif