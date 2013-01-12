PHYSICA
============

1.GET IT
============

WINDOWS USERS:
---------------
just head to https://sourceforge.net/projects/physica/ , download and extract the .zip file, then run physica.exe

LINUX USERS:
-------------
* download and extract the repository content (button above)
* from inside the downloaded folder, run the command
    g++ -enable-auto-import -w -obin/physica src/physica.cpp -lSDLmain -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx
* move into the bin folder and run the command
    physica
				
2. PLAY IT
============
Click the big play button to start playing and pick your level (currently just two, but I'm working on the core level set).
WASD to move the green cube, reach the yellow cube to complete the level. Oh, and did I tell you to avoid red stuff?

3. ADD NEW LEVELS
===============
Just use the editor bundled with the game to make new awesome levels! You can then find the levels you made inside the data/cfg/levels folder, along with core levels. They're automatically queued to the core level set. Switch on debug mode (in the options menu) to have them unlocked immediately for testing.
