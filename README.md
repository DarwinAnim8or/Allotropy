# Allotropy
A slightly modified version of [QuakeSpasm](http://quakespasm.sourceforge.net/) (a modernized version of the Quake 1 engine by the legendary Id Software).

## Goal
The goal of this project is to make slight modifications to the engine and then use it to make a game with it.
Not to make the newest, next-gen game, but just for fun. 
Newer FPS games just don't play as well as Quake did, and that's sad. 
So I want to make a game that's just fun to play, with a retro look. And Quake's engine is the perfect fit for that. :)
(Of course, none of the original Quake assets will be used as those are not GPL'd by Id.)

## Compiling on Linux (Ubuntu)
(This assumes you've already set up the common build tools, make, etc on your system.)
You will need to install the following libraries:
* libmpg123-dev
* libogg-dev
* libvorbis-dev
* (libmad)

Which one you end up using is configurable through the Makefile(s).
Please see the Quakespasm.txt file for more details regarding this.
To install the libraries, just use your distrobution's provided repositories. 
(So on Ubuntu, just use "sudo apt install <package name>".)

Soundtrack should be placed inside of a "music" folder inside of the same folder as the pak files. 
Tracks should be named "track02.ogg", "track03.ogg", and so forth. (There is no "track01.ogg" at the time of writing.)

After this, you can either open up the Code::Blocks project in the "Linux" folder, or go into the "Quake" folder using the Terminal and typing "make -j5" (with the number after the "j" being the number of cores your processor has + 1.)
