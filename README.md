<h1> PHYSICA </h1>
<h2> 1.GET IT </h2>
<h4> WINDOWS USERS: </h4>
<p>Just head to https://sourceforge.net/projects/physica/ , download and extract the .zip file, then run physica.exe</p>

<h4> LINUX USERS: </h4>
<ul>
	<li>download and extract the repository content (button above)</li>

	<li>
	    from inside the downloaded folder, run the command<br>
	    <code>g++ -enable-auto-import -w -obin/physica src/physica.cpp -lSDLmain -lSDL -lSDL_image -lSDL_ttf -lSDL_mixer -lSDL_gfx</code>
	</li>

	<li>
	    move into the bin folder and run the command<br>
	    <code>physica</code>
	</li>
</ul>
				
<h2> 2. PLAY IT </h2>

Click the big play button to start playing and pick your level (currently just two, but I'm working on the core level set).
WASD to move the green cube, reach the yellow cube to complete the level. Oh, and did I tell you to avoid red blocks?

<h2> 3. ADD NEW LEVELS </h2>

Just use the editor bundled with the game to make new awesome levels! You can then find the levels you made inside the data/cfg/levels folder, along with core levels. They're automatically queued to the core level set. Switch on debug mode (in the options menu) to have them unlocked immediately for testing.
