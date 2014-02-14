#!/bin/bash

clear

if [ "$1" != "runonly" ]
then

echo Building...
g++ -w src/*.cpp src/PugiXML/*.cpp src/TinyJS/*.cpp -obin/test -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_gfx -lSDL2_ttf

if [ $? -ne 0 ]
then
	echo Build failed.
	exit
else
	echo Build successful.
fi

fi

if [ $# -eq 0 ]
then
	exit
fi

if [ "$1" == "run" ] || [ "$1" == "runonly" ]
then
	echo Now running...
	
	cd ./bin
	./test
	cd ..
	
	echo $?
	
	echo LOG
	
	cat ./bin/output.log
fi

if [ "$1" == "debug" ]
then
	echo Debugging...
	
	cd ./bin
	gdb test --eval-command run --eval-command quit
	cd ..
	
	echo LOG
	cat ./bin/output.log
fi
