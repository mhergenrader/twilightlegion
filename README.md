# twilightlegion
Twilight Legion for TI-89 (Titanium) calculators, written in C, 68k Assembly

&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

                  	                  TWILIGHT LEGION
                                    MICHAEL HERGENRADER
                                        VERSION: 1.9
                           COPYRIGHT � 2005-2010 MICHAEL HERGENRADER

&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&


Contents:
1.  Introduction, Requirements, and Features
2.  Controls (ingame)
3.  What to send to your calculator/list of files (why the game is archived memory-intensive)
4.  How to play/Modes of play/Points
5.  Profiles/Creating/Loading
6.  Options Menu and Settings
7.  Character Select Menu/Stage Select Menu/Team Select Screen/VS Screen
8.  Episode Mode Menu
9.  Starting Characters
10. Starting Stages Descriptions
11. Disclaimer/License/How to report bugs/How to contact me in general/Other Languages
12. Game Credits
13. Planned Features for later updates
14. Notes on the Game Guide

*************************************************************************************************

1. Introduction, Requirements, and Features

Introduction:
24 characters from across various games and platforms have been chosen to take part in a 
tournament of the best for the Twilight Legion Championship. Fight your way through as you pick
up new characters and stages as you put your strategy and coordination to the test! You may be
able to handle 1 minute timed matches of Super Smash Bros., but can you fight for 90 minutes
straight against the fiercest competitors in existence? Send your foes packing in four
different modes and prove that you are the fighting champion! Then, prove you are the master
by defeating the pure evil of Master Hand! Last year's champ is out to defend his title as well,
so you had better be ready for a scrap!


Requirements to run:

Calculator: TI-89 or TI-89 Titanium (92+ and Voyage 200 porting hopefully to come soon)
Emulator: I prefer TiEmu, but it doesn't matter; NOTE: the game speed is much faster on these!
Minimum AMS Version: 1.01
Free RAM: 
Free Archive:    bytes (to store the game)
(this game is entirely _nostub; no custom OS required)

If you see any dialog screen accompanied by a message, such as the following, please follow these
instructions:

*** ERROR: Unrecognized User! ***

This is just the profile creation screen, see section 5.

*** ERROR: PROGRAM NOT FOUND ***

accompanied by:
"tl_stage not installed"

Send tl_stage to your calculator.

accompanied by:
"tl_charx not installed"

Make sure to send all tl_char1.data, tl_char2.data, and tl_char3.data files to your calculator.

accompanied by:
"tl_extra not installed"

Send tl_extra.data to your calculator.

*** ERROR: MEMORY ***

Try to free up your RAM usage as much as possible, you may have to remove some files to make room.
If you have a lot of FlashApps, those take up the most space.


If not, you should see the profile loading screen, so play and enjoy!


Features:
- 24 unlockable characters, each with different heights and widths
- 26 unlockable various stages
- 10 different invitational tournaments to unlock, or create your own custom tournaments
- 31 complete unlockable episodes, with increasing difficulties (just try and beat #31)
- Familiar boss battles from Super Smash Bros. :-) (*spoiler at very bottom of file)
- timed matches anywhere from 1 to 90 minutes in duration (90 for the hardcore gamers ;-) )
- stock matches anywhere from 1 to 5 lives
- sudden death for timed matches if the game deems necessary
- up to 5 different saved user profiles to load and view
- save your arena battle and tournament and settings into your profile quickly - allows 1 of each per profile!
- Arena mode: freestyle fight based on the settings, choose anywhere from 2-4 players to fight
- Story mode: 10 consecutive, always different, random stages, with 2 Race to the finish
  minigames, a metal character fight, and, of course, the final battle (boss battle)
- complete trophy gallery with 5 pages to view 8 trophies
- "teacher" key to save your own custom battles and then instantly reload them on startup! (press ON)
- 4 different AI difficulties (Classic = easiest, Admiral, Premiere, Elite = hardest)
- crowd pressure enablement - fight like in a stadium with the crowd in the back with/against you
- 1 complete moving level enabled to put a twist into your battles (hopefully, more to come later)
- 26 different items that fall from the sky, including boosts, metal, invisibility, and projectiles
- 16 different types of terrain blocks inside levels (hopefully, more to come, as some were removed)
- works on hardware versions 1, 2, and 3 - no patch required
- full scrolling credits screen

Planned for later:
- Finishing linked battles (still in testing)
- TI-92(+)/Voyage 200 compatibility

*************************************************************************************************

2. Controls (ingame)

<2nd>          smash attacks/pick up items
<DIAMOND>      special attacks for your player (stand still, fire projectile, or missile attack)
<SHIFT>        make player jump (up to two jumps - second is UP)
<ALPHA+ARROW>  dodge past enemy
<F1>           grab your enemy (+arrow key while grabbing = throw enemy)
<L,R ARROWS>   move the player/manuever attack direction/switch direction
<UP ARROW>     climb ladders (there are ladders, believe it or not, but only one looks like one),
               double jump, taunt enemies (when not doing anything)
<DOWN ARROW>   climb down ladders/crouch/sky attack if falling in the air (pretty effective attack)
<+>            increase the contrast
<->            decrease the contrast
<F3>           immediately pauses and exits the game - creates a copy of your battle inside your
               profile - this key is only allowed for Arena Mode, other pauses consist of <ON>
<ESC>          exit the battle (not allowed in a tournament battle)
<ON>           pause the game (turns off the calculator screen, pauses the game) - useful for any mode


NOTE: when you face one direction but then turn the other way and hold down that key, you will
run instead of walk. Then, if you stop and restart the same way, you will walk.

NOTE: the game automatically sets the contrast to what I believe is a good setting to see
everything based on the calculator model, but the + and - keys will still adjust it nonetheless

NOTE: a user is not allowed to alter the contrast in episode mode (at certain episodes, you will
see exactly why)


*************************************************************************************************

3. What to send to your calculator/list of files (why the game is archived memory-intensive)

Send to your calculator: (check to make sure that the right files are sent, there are different 
folders to make sure different calc models run at good speeds)

twilight.89z (ASM) - 1007 oncalc bytes
twippg.89y (ppg) - 31034 oncalc bytes
tl_char1.89y (data) - 61005 oncalc bytes
tl_char2.89y (data) - 54933 oncalc bytes
tl_char3.89y (data) - 50793 oncalc bytes
tl_extra.89y (data) - 29035 oncalc bytes
tl_stage.89y (data) - 45503 oncalc bytes

Other files created during the game's residence (no sending or manual creating necessary):
PROFILES folder
any user profiles (user) - 1159 bytes per profile (maximum profile amount is 5)

NOTE: I recommend you leave all of these files archived at all times so they will never be 
erased if you experience any sort of crash! (Do you see what I mean by archive-intensive ;-) )


*************************************************************************************************

4. How to play/Modes of play/Points

How to Play:

Your goal is to win the fight against your opponents in the level by whatever means necessary. Go
toward your enemies and attack them. When you attack them (and the attacks hit them), their percentage
will increase. As their percentage increases, they will become easier to hit out of the level. If they
fly out past certain bounds in the level, they will lose a life in stock mode, or will give you a point
and they will lose a point in timed matches. The same occurs with human players. Try to collect items
during a fight to help your chances for surviving for the win. The AI will become much faster and 
tougher in the harder settings... There are several different kinds of moves a player can do in a
fight, so glance at the controls to see what fits you for attack styles, as each character has varying
types of attacks and such. The game will combine fast reflexes


Modes of Play:

*** Arena Mode ***

Free-for-all, freestyle, whatever you want type fights. Go to settings and decide how the battle is
going to happen. Then select your character, your opponents, your stage, and go fight! (the default
mode of play) - it helps you practice for tournaments and those tougher episodes, or if you're just
feeling like smashing baddies.


*** Story Mode ***

Story mode is just like Super Smash Bros. Melee Classic Mode, a string of 10 consecutive
fights, all at the same difficulty set by the user before it starts (in settings). In the mode,
there are two Race to the Finish! minigames, a metal character duel level, and a final boss level.
Playing this mode can unlock some of the locked stuff in the game, as fighting consecutive matches
can be tough, especially at higher difficulty. You are allowed one continue per mode initiated,
and this will divide the total score the user has by two. You will be redirected to the same level,
but with a different set of enemies or enemy. Story Mode also features certain point categories:

+2000         per kill
-800          per time slain
+1000         if player grabs and throws enemy
+NUMBER       for completion of Race to the Finish!
+4000         if your HP is under 10 at the end of the stage
+20000-time   a time bonus for completing battles
+20000        for killing the boss at the end
+50000        completing story mode at all (with or without a continue)



*** Episode Mode ***

Just like Event mode from Super Smash Bros. Melee! There are 31 total episodes, progressing to
harder difficulties. Some episodes have different crazy themes and some are just hypothetical
(well, actually real) fights between sets of characters. The last one is ESPECIALLY tough to beat,
and Episode 30 is right under it. Episodes are just fights with set themes. To unlock new ones,
unlock some previous ones, and perhaps do a few other unlocking tasks. At the end, if you accomplished
the goal, you will see "Success", but if you don't or die, you will see "Failure".


*** Tournament Mode ***

Host a tournament with a bracket of 16 players where 2 players fight each other, complete with an
auto-scrolling bracket, transition effects, and ability to save the tournaments. Create your own
or start one of 10 unlockable invitationals included with the game. A player can either watch the AI
fight to see who he/she will fight or skip to the next one. When the preparation screen appears to
initiate a battle, press 2nd to play your battle/watch an AI battle or press ESC - if you are in the
battle, it will go to the main menu and save the tournament, if you are not in the battle, it will
just skip the AI battle if you do not wish to see it at all. Set the tournament settings in the
tournament menu.


*************************************************************************************************

5. Profiles/Creating/Loading

The game has enough tolerance for up to 5 profiles. Profiles store all the player's data and the
initials are drawn above their character during the game. It stores everything that a player has
unlocked, a quick-save battle if the user hits a teacher key (F3) and a saved tournament a user
has started or created.

On the first run of the program, a dialog box will popup if either there exists no profiles
folder (don't worry about adding it, the game does so automatically) or the profiles folder is
empty. Here, enter your name (8 characters maximum) and your battle ID (initials drawn above your
character in the game) and then press enter. The profile will be created. This will be the current
profile set throughout until you exit the game.

If one or more profiles were found (of type .user in the profiles folder), a popup
menu will ask you which one to load. Select the correct profile and enjoy. I recommend always 
leaving these archived at all times to prevent losing any data.

Everytime a match occurs and is FINISHED, a win or loss will be updated to the profile. An 
incomplete match will not be added to a profile, so you must actually play all the matches
to add to the record and/or unlock some parts of the game.


*************************************************************************************************

6. Options Menu and Settings

Settings: Adjust game settings
Profiles: View the current profile and your accomplishments
Credits: View the credits screen


Settings: some settings affect only Arena Mode fights

*** Crowd Pressure ***
Turning on crowd pressure will heighten the intensity of the fight with a shaking, earthquake
background, that will go BERZERK when you are smashed out. It is automatically turned on in
Tournament mode, otherwise, if it is turned off, there will either be a white, day background
or a night, dark background.

AFFECTS MODES: ARENA, STORY, TOURNAMENT (non-invitational)


*** Match Type ***
Stock matches include a set number of lives, and the last one (or team) standing wins the match.

Timed matches include unlimited lives and players will fight for the duration of the battle to
see who kills the most (smashes the players out) and gets killed the most. The player with the
most kills and the least number of times killed wins the match. If there is a tie between two or
more characters, sudden death occurs, in which all players have 999% (max) hit percent. The one
who survives that fight (like a stock match with 1 life) wins overall. There are no teams (at
least, they are all different) in a timed match.

AFFECTS MODES: ARENA, TOURNAMENT (non-invitational)


*** Match Length ***
This field is for timed matches only. The time can be set from 1 to 90 minutes. HINT: some
parts of the game can be unlocked for longer matches. 90 minute straight matches are pretty brutal
(I've tested with one), so make sure you are ready and have time to do it if you are ready (they
are certainly not for beginners if you want to win). Otherwise, you can break down the match by
pressing [ON] to pause, or exit with F3 to save it for later.

AFFECTS MODES: ARENA, TOURNAMENT (non-invitational)


*** Life Count ***
This field is for stock matches only. The number of lives can be set from 1 to 5 for stock matches.
In Story Mode, a user receives this many lives and must endure all of them for the whole time.

AFFECTS MODES: ARENA, STORY, TOURNAMENT (non-invitational)


*** Difficulty ***
There are 4 different difficulties for the AI in the game: Classic (basic), Admiral, Premiere, and,
of course, the toughest one, Elite. Episodes advance in this order as you progress, and Episode 31
is considered a "Super Elite" difficulty, the hardest fight in the game (it is not a real
difficulty, it is just Elite beefed up with better and harder AI). Set the difficulty of the AI
in this setting.

AFFECTS MODES: ARENA, STORY, TOURNAMENT (non-invitational)


*** Item Probability ***
This is for either mode, just set how often items will occur. Do you want a survival fight with
none? Do you want a replenishment every second? It's up to you. There are different kinds of items:
ones that heal (the heart is the best and they are all food), ones that give you a quick projectile,
ones that turn you invisible for a time, ones that make you metallic for a time, and ones that
increase your stats for a short time.

AFFECTS MODES: ALL


Profiles:

At any time outside a match, a user can view certain stats in their profile, such as wins, losses,
and how many objects/characters they have unlocked.

Credits:
Press <+> key to speed up the scrolling and <-> to slow it down, or hit <ESC> to return to the
options menu, it's pretty self-explanatory.


*************************************************************************************************

7. Character Select Menu/Stage Select Menu/Team Select Screen/VS Screen

Character Select Menu:
A game can be played with 2-4 characters at one time. To manipulate how many characters can
play, use the <+> and <-> keys. The number in the upper righthand corner will tell you. Your
character is always displayed as the first one on the left. To select a character, press <2nd>. 
In Story mode when selecting a character, if you press (and hold) <DIAMOND>, you will be
able to see the high score for that character and the highest difficulty they have completed.


Team Selection Screen:
This screen will only popup if you choose Stock Fights and there are more than 2 players. Here,
use only the arrow keys, with vertical to choose the character and horizontal to change the teams.
Press 2nd to confirm.


Stage Select Menu:
Use the arrow keys to move the cursor around the menu to highlight a stage. The name will be drawn
in the middle. Hit <2nd> to select the highlighted stage, or hit <DIAMOND> to select a random one.

NOTE: if any of the pictures have a question mark on them, they are locked and not selectable yet
until you unlock them.


VS Screen:
All four characters are drawn with their respective names at the bottom of the screen. The numbers
on the side of the characters represent their TEAM number. If you or any character(s) has the 
same number, you are on the same team, so try to work WITH them.


*************************************************************************************************

8. Episode Mode Menu

There are 31 episodes. To access the next one or the next set, you must complete the previous ones
and sometimes you must also complete other tasks to unlock them.

Just use the up and down arrow keys and hit <2nd> to access the episode. There are various ones
to play, you will have to see more to know more. In the guide, all 31 episodes are highlighted.

The menu displays the name of the character, a description of what the user will face, and also
tells by an icon to the left if the episode has previously been completed (they are replayable).

NOTE: some episodes take more time to complete, and hitting ESC is an automatic failure for that
episode. During testing, one took about 8-10 minutes, while another took 20!


*************************************************************************************************

9. Starting Characters (10/24)

NOTE: no matter how big or small the characters are, I have evened out the playing field to base
everything on their stats and your skill..., not just on their size. In fact, with terrain
collisions with players, there is a "center of gravity" where collisions affect Mario in the same
way they affect Bowser such as falling, moving, etc.

Bowser
Captain Falcon
Fox
Kirby
Link
Mario
Peach
Samus
Yoshi
Zelda

The other 14 are for you to unlock and use later!


*************************************************************************************************

10. Starting Stages Descriptions (14)

F-Zero Grand Prix: Big Blue
From Super Smash Bros. Melee now put onto the calc, Captain Falcon's home turf. Just like the 
real game, stay off the road.

Zebes: Brinstar
Again, taken from the original SSBM, but be careful, I have modified a few things to make the 
game more interesting like all the blocks are clouds (don't press down) and the lava below will
kill anyone who falls in it.

Corneria:
Modeled to look like the Great Fox like SSBM. What else is there to say besides it has been
seen in all the Starfox Games. Fox's and his longtime friend's home turf - all solid, a small 
hill, etc.

Dreamland:
Can be a little harder to see with the night background, but nonetheless is a pretty
straightforward stage hosting Kirby and one of his nemeses - all the top floors are clouds.

Era Tower:
A location from what was supposed to be a game I would do: an RPG called Black Eagle 68k. It is
full of puzzles, and in this game, it has a few hills, and some collapsing tiles.

Fourside:
The classic city fight with some redecorated buildings and possibilities to fall through the large
holes. Just because you have a big character doesn't mean they can't fall through.

Glass Submarine:
A stage taken from another game I hoped to do. I started it, but didn't have the motivation to
actually continue it: LUXA I: Tiger's Eye. Maybe in the future I will. The name is a misnomer.

Great Bay:
The classic from SSBM with the stage floors just above the water. Careful about any water tiles
in this game - you will fall down twice as fast, so keep your reflexes ready.

Hyrule:
The largest stage from the original Super Smash Bros. was Hyrule, and here, it is also one of the
largest here. Most of it is solid, a few clouds, pretty straightforward - Link, Zelda, and the
other Zelda series character's home turf.

Ice Mountain:
The one moving level of the current version (I hope in the future to add more) that scrolls up
and down, keeps the same type of concentration as the real game.

Jungle Japes:
The classic Donkey Kong stage where all floors are clouds and players can be smashed out fairly
quickly.

Lunor Stronghold:
A base of operations for the Lunor Forces for Black Eagle 68k before I decided not to do that
project when I was learning C. The bottom is an endless pit, so don't fall in.

Soul Tower:
The Sun Forces guarded this tower in Black Eagle 68k and again, it was also full of puzzles, this
one is pretty small like the Era Tower, so smashing should not be a problem here. Lots of places
to hang on before you fall off.

Yoshi's Island:
This stage, I tried to make it look a lot like the original from SSBM, hence the hills and the 
smiling cloud blocks.

The other 12 stages are left for you to unlock and discover (and they are in the guide).


*************************************************************************************************

11. Disclaimer/License/How to report bugs/How to contact me in general/Other Languages

I release the source code with this game with the hope that it may teach others the way that
creating this game over five+ years has taught me. If you use code from my game in your own,
I would greatly appreciate some form of credit :-).

Regarding the game and source, neither is not to be distributed for profit of any sort, as I, Michael
Hergenrader, as the game's sole creator, will not ever charge for access to it.

Twilight Legion is Copyright (c) 2005 - 2010 Michael Hergenrader.

Disclaimer:
As programmer and graphics artist of the project, I hold no responsibility for any
calculator malfunctions that may or may not have been caused by my game. By playing the game,
you agree to this condition. I have spent countless hours searching for any sort of bug, and if
for some reason I have missed one (hopefully unlikely), please contact me at the below
information.

I have released the game at ticalc.org. If you would like to distribute the game at another site,
PLEASE ASK ME FIRST, as I hope to make subsequent updates to the game (see updates below).
And again, this program is to be distributed freely, never for profit.

I am confident that I have found the bugs in the program, so there's no need to worry, but if 
for some reason, you find a bug, please contact me immediately so I can fix it.

Please report as many details as possible, thank you, or if you need a question about something
else relevant to me or the game, please also contact me if you wish.

If you would like something of the game to be in another language, please also ask me. I am 
fluent in English and Spanish, learning Mandarin, and I will try to acquire help for others
not listed. (Mandarin would be tough ;-} )

Email: mike.hergenrader@gmail.com
TI-Freakware Forum: abstract
TICT Message Board: mhergenrader


*************************************************************************************************

12. Game Credits (other than myself for making it ;-), of course)

NOTE: Most of the graphics of the game were designed and drawn by myself, but some I received
from various ripped sprites from public domain sources. I have sent emails to each of these other
designers and received their explicit permission. Thank you to all those designers, you know who you are.

I would like to thank Thomas Nussbaumer for getting me started programming for the 68k series,
and also directing me to the TICT Board, where brilliant people have helped me overcome issues
during development of the game.

*** Special Thanks *** (in alphabetical order)

Ben Cherry - emails for certain features of my program, such as better exiting methods and animation
Lionel Debroux - maintainer of extgraph, and amazing optimizer who helped shave off tons of kilobytes
Martial Demolins - helped to save Lionel countless hours when debugging to help me optimize a lot
Kevin Kofler - current maintainer of fantastic TIGCC IDE and TiEmu, for shorter testing times
David Randall - help at TI-Freakware with the terrain engine, profile stuff, and motivation
Samuel Stearley - help at the TICT board with the character data files and other annoying bugs
Julien Richard-Foy - author of the fantastic TileMap engine
MathStuf - giving me a lot of help with the vat.h functions and data file saving/loading
Lachprog - helping me a lot with compression techniques, especially RLE

Thank you to all those who encouraged and helped me and to the authors and contributors of the
TileMap engine and the extgraph library. Without all of them, none of this would be possible.

If for some reason, I have not included you and you assisted in the development of the project,
please contact me ASAP so I can.


*************************************************************************************************

13. Planned features for later updates (if space and time allow)

* TI-92+ and Voyage 200 porting capabilities
* Link play between calcs (I want to include the regular I/O port, but I also want to test out 
  Kevin Kofler's USB methods to use them as well)
* other different kinds of attacks than the ones here
* a bonus mode besides the ones here, like an All-Star mode like in SSBM
* perhaps a sequel? not really likely unless people want one - email me if you support the idea
  because at this point, I'm not really sure... I am continuing with another project after this
  one for which I have already developed some of the engine

NOTE: This game will not include a level editor, as the sprites are not generic enough, and I
would like to make different projects after I add the updates above.


*************************************************************************************************

14. Notes on the Game Guide

I have also written a short game guide for Twilight Legion on how to unlock everything in the
game. If you would like to look at the guide, feel free to do so, but I guarantee that if you 
figure out how to unlock everything yourself, the game will be incredibly more fun. I will release
the guide shortly, either in an updated version in the game, on my website, or on ticalc.org in 
the texts section (if I am allowed to).


*************************************************************************************************


Enjoy the game and thank you for downloading!
-- Michael Hergenrader --






* Spoiler: Master and Crazy Hand both make appearances in the game!