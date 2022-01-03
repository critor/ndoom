nDoom
by Critor


Credits to:
- Mrakoplaz for the initial Nspire port
- Id Software for creating the game

Thanks to:
- the TI-Planet community	-> http://tiplanet.org (french)
- ExtendeD for Ndless



Installation:
************
- Install Ndless on your calculator
- send "ndoom.tns" to your calculator
- take Doom/DoomII compatible IWAD files and change their extensions from ".wad" to ".wad.tns"
- send the ".wad.tns" files to your calculator
- if necessary, add a "ext.wad=ndoom" line to your "ndless.cfg.tns" file



Supported games:
***************
There exists two types of WAD files:
- IWAD files which include complete game data
- PWAD files which are patches with partial data, designed to be loaded over an IWAD

nDoom mainly targets IWAD files and has partial support for PWAD files.
Note that most 3rd party WAD files available on the Internet are PWAD files and won't work.

Only IWAD/PWAD files compatible with the Doom or Doom II original engine are supported. IWAD/PWAD files from other Id Software games (Heretic, Hexen...) won't work.

The Id Software supported IWAD files are:
- Doom ShareWare 	(usually "doom1.wad")		(Doom IWAD format)
- Doom registered	(usually "doom.wad")		(Doom IWAD format)
- Ultimate Doom		(usually "doomu.wad")		(Doom IWAD format)
- Doom II		(usually "doom2.wad")		(Doom II IWAD format)
- Final Doom - Plutonia	(usually "plutonia.wad")	(Doom II IWAD format)
- Final Doom - TNT	(usually "tnt.wad")		(Doom II IWAD format)



Use:
***
- Launch nDoom
- Choose your IWAD game file
- Choose the episode (for Doom Registered and Ultimate Doom compatible IWAD files)
- Choose the skill level
- Enjoy!


Controls:
********
+ in file browser:

  up/down arrow			: select the file/folder
  left arrow			: move to parent folder
  click / right arrow / enter	: open file/folder
  esc				: cancel

+ in setup:

  + -	: to increase/decrease skill level
  ( )	: to select episode for Doom Registered and Ultimate Doom compatible IWAD files
  enter	: run game
  esc	: cancel

+ in game:

Action			| 
------------------------+---------------------------
Forward			| 8 Up arrow
Backward		| 2 Down arrow
Turn left		| 4 Left arrow
Turn right		| 6 Right arrow
Forward + turn left	| 7 Up-Left arrow
Forward + turn right	| 9 Up-Right arrow
Backward + turn left	| 1 Down-Left arrow
Backward + turn right	| 3 Down-Right arrow
Strafe left		| Scratchpad Tab x² /
Strafe right		| Doc Menu x ^
Shoot			| Ctrl del 5 . (-)
Use			| Shift Space 0 Enter Return
Weapons 1/2/3/4/5/6/7	| A B C D E F G
Map			| Home ? ?!>
Run			| Click Trig = |
Contrast / Brightness	| + -
Quicksave		| Var
Quickload		| Library
Quit			| Esc



New in version 2.0 (19/01/2012):
*******************************
- polyvalent version working on all Nspire models: TI-Nspire ClickPad, TI-Nspire TouchPad & TI-Nspire CX
- support for the TI-Nspire CX color screen
- support for the touchpad of the TI-Nspire TouchPad & TI-Nspire CX
- support for all Doom/Doom II compatible IWAD files
- file browser to select your IWAD file
- menu to select the episode (in Doom Registered and Ultimate Doom)
- menu to select the skill level



New in version 2.1 (28/01/2012):
*******************************
- fixed a memory leak bug in the file browser
- menu to select the map
- support for the touchpad of the TI-Nspire TouchPad & TI-Nspire CX
- support for hot-patching games with Doom/Doom II compatible PWAD files


New in version 2.11 (28/01/2012):
*******************************
- turned some WAD fatal errors into warning messages into warning messages
in order to be able to open more PWAD files (with some graphic glitches)


New in version 2.12 (06/02/2012):
********************************
- added support to directly open wad files from the OS browser (needs Ndless 3.1 r350 at least, and a correctly set up ndless.cfg.tns)
- partially fixed the wad close bug: if the game is exited correctly, you should be able to delete the used wad file
if the game does crash, that's another problem...

New in version 2.2 (30/07/2012):
********************************
- patches ndless.cfg.tns if necessary on 1st run in order to directly open wad files from the Nspire doc browser
- fixed screen memory address alignment on 8 bytes, which was randomly shifting the screen horizontally
- now correct support of the CX 2nd timer, needed for screen fading

New in version 2.3 (27/03/2016):
********************************
- TI-Nspire CX CR4 new screen support

New in version 2.4 (19/09/2020):
********************************
- TI-Nspire CX II screen support



Todo list:
*********
- Need to fix the error dialog boxes bug once the game has started (which should not happen) - freeze after the dialog box instead of quit.
- Need to fix end of episode/game animations which seem to freeze sometimes



Support:
*******
http://tiplanet.org
