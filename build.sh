#!/bin/bash

clear

g++ -w src/*.cpp src/PugiXML/*.cpp src/TinyJS/*.cpp -obin/test -L/usr/local/lib -lSDL2main -lSDL2 -lSDL2_image -lSDL2_gfx -lSDL2_ttf

if [ $? = 0 ]
then
	cd ./bin
	./test
	cd ..
	
	cat ./bin/output.log
fi
