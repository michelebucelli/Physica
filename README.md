<h1> PHYSICA </h1>
<h2> 1.GET IT </h2>
<h4> WINDOWS USERS: </h4>
<p>Just head to https://sourceforge.net/projects/physica/ , download and extract the .zip file, then run physica.exe</p>

<h4> LINUX USERS: </h4>
<ul>
	<li>if you're on Debian, you should check to have these packages installed:
			<ul>
				<li>libsdl1.2-dev</li>
				<li>libsdl-image1.2-dev</li>
				<li>libsdl-ttf2.0-dev</li>
				<li>libsdl-mixer1.2-dev</li>
				<li>libsdl-gfx1.2-def</li>
			</ul>
	</li>
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

Click the big play button to start playing and pick your level.
Use the arrow keys to move the green cube, reach the yellow cube to complete the level. Oh, and did I tell you to avoid red? It hurts.

<h2> 3. ADD NEW LEVELS </h2>

Just use the editor bundled with the game to make new awesome levels! You can then find the levels you made inside the data/cfg/levels folder, along with core levels. They're automatically queued to the Editor level set. Switch on debug mode (in the options menu) to have them unlocked immediately for testing.
Take a look at the <a href="https://github.com/buch415/Physica/wiki">Wiki</a> for some instructions on how to use the editor.
