# Stellar Eagle

## OVERVIEW
This is a game development project intended to pay an homage to the old school shmup arcade games of the past.
It utilizes/simulates techniques of old to get that nice pixelated look.
A custom 2D sprite engine is used, instead of Unity or UnrealEngine 4, for learning and fun purposes.


If you want to help with the development, donations are welcomed:

Donations on Patreon:
<br>[[Patreon]](http://www.patreon.com/7thfactor)

Donations on PayPal:
<br>[![PayPal](https://www.paypalobjects.com/en_US/i/btn/btn_donate_LG.gif)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=D2J3J2A766KXY)

## PREREQUISITES
**Windows:**
	Microsoft Visual Studio 2019+

**Linux:**
g++

GLU/GLUT dev, needed by GLEW, use:
		```sudo apt-get install libglu1-mesa-dev freeglut3-dev mesa-common-dev```
	
## BUILDING
**Windows:**
- Execute: ```generate.bat```
- Open and compile: ```build_win/game.sln```
- Run generated files from ```./bin``` folder

**Linux:**
- Execute: ```sh ./generate.sh``` to generate makefiles
- Execute: ```sh ./build.sh``` to compile and generate the lib and executables for the examples
- Run generated files from the ```./bin``` folder
