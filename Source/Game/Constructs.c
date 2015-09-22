// Twilight Legion for TI-89(Titanium), TI-92(+), Voyage 200
// C Source File - Constructs.c
// Michael Hergenrader
// Compiled with TIGCC 0.96 Beta 8
// Please see README for license/disclaimer information. In short, please feel free to use code you see here,
// and any credit to me would be greatly appreciated! :-)
// Copyright 2005-2010 Michael Hergenrader

// Useful container file for important in-game structures and variables (hence not stored as static
// here). Most of these helpful variables and structures are defined here then assigned during the game.
// Overall, this helps save space under the 64KB limit as opposed to placing these all static in the in-game file.
// Thanks to Lionel Debroux for recommending this optimization.

// A structure to organize these was initially used, but the filesize ended up growing substantially beyond the 64KB
// limit. Thus, these were broken out into individual fields. For the booleans below, a bit vector could be tried also;
// kept simple for now.

#include <tigcclib.h>
#include "constructs.h"

INT_HANDLER save_int_1;
INT_HANDLER save_int_5;
INT_HANDLER save_int_6;
void* block;
void* virtual; // game virtual screens

unsigned char* skylight;
unsigned char* skydark;

unsigned char* powbufferlight;
unsigned char* powbufferdark;

void* v2;
void* v3; // auxiliary virtual screens (out of game virtuals that are copied to the screen at each frame update)

PLAYER* p1;
PLAYER* p2;
PLAYER* p3;
PLAYER* p4;
PLAYER* myPlayer = NULL;

HAND* masterHand;
HAND* crazyHand;

TIMER* timer;

BOOL scrollL;
BOOL scrollR;
BOOL scrollU;
BOOL scrollD; // whether currently actively scrolling in a particular direction - to follow the player; will be false at edges

BOOL disabled; // in-game states for player 1 (human)
BOOL suddenDeath;
BOOL racing;
BOOL complete;
BOOL fightMetal = FALSE;
BOOL fightCloaked = FALSE;
BOOL cont = FALSE;
BOOL episodeSuccess;
BOOL goAheadAndSave = FALSE;
BOOL linked = FALSE; // current player items

int x_fg;
int y_fg;
int x_bg;
int y_bg; // coordinates of the foreground and background tile maps

unsigned int p1ClassicLives;
unsigned int collapse;
unsigned int scrollType = 0;
unsigned int stageIndex;
unsigned int numPlayers;
unsigned int numHands;
unsigned int battleCounter = 0;
unsigned int winningTeam;
GAME_MODE mode = NONE_SELECTED;

unsigned char calc = HOST_CALC; // ID of which calculator is being used in link play (master/host or slave/join/follower)

int gameDifficulty;
int gameMatchMinutes;
STATUS gameCrowdPressure;
MATCHTYPE gameMatchType;
unsigned int gameMatchLives;
unsigned int gameItemProb;

unsigned long points[8]; // classic mode point categories (8 different ways to earn points)

EXTERNAL* dataptr; // pointers for loading from external files
unsigned long* c1;
unsigned long* c2;
unsigned long* c3;
EXTRA_EXTERNAL* extraptr;

CHARACTER* characters;
ITEM* head;
PLAYER* pHead; // head of the player linked list
STAGE* stageTemp; // current stage being played in

unsigned char* profileNames[5];
PROFILE currentProfile;
char fileName[19]; // this is for profile name saving with the VAT (must keep in constructs)

// initial stage declaration structure (to be filled in when the external files are loaded after Main.c)
STAGE stages[26] = {
	{ 8,18,NULL,{NULL,18,NULL,NULL,0,0,1},FALSE}, // Antaira Village - my own stage
	{ 8,12,NULL,{NULL,12,NULL,NULL,0,0,1},FALSE}, // Battlefield
	{ 7,10,NULL,{NULL,10,NULL,NULL,0,0,1},FALSE}, // Big Blue
	{ 9,13,NULL,{NULL,13,NULL,NULL,0,0,1},FALSE}, // Brinstar
	{ 7,14,NULL,{NULL,14,NULL,NULL,0,0,1},FALSE}, // Caidru's Cathedral of Time - my own stage
	{ 7,14,NULL,{NULL,14,NULL,NULL,0,0,1},FALSE}, // Core Sanctuary Monument - my own stage
	{10,15,NULL,{NULL,15,NULL,NULL,0,0,1},FALSE}, // Corneria
	{ 8,14,NULL,{NULL,14,NULL,NULL,0,0,1},FALSE}, // Crasphone City - my own stage
	{ 8,14,NULL,{NULL,14,NULL,NULL,0,0,1},FALSE}, // Dreamland
	{ 7,10,NULL,{NULL,10,NULL,NULL,0,0,1},FALSE}, // Era Tower
	{ 6,14,NULL,{NULL,14,NULL,NULL,0,0,1},FALSE}, // Final Destination
	{ 9,14,NULL,{NULL,14,NULL,NULL,0,0,1},FALSE}, // Fourside
	{ 7,10,NULL,{NULL,11,NULL,NULL,0,0,1},FALSE}, // Flatzone     
	{ 7,14,NULL,{NULL,14,NULL,NULL,0,0,1},FALSE}, // Glass Submarine
	{ 9,15,NULL,{NULL,15,NULL,NULL,0,0,1},FALSE}, // Great Bay
	{ 9,14,NULL,{NULL,14,NULL,NULL,0,0,1},FALSE}, // Hethor Forest - my own stage
	{ 9,18,NULL,{NULL,18,NULL,NULL,0,0,1},FALSE}, // Hyrule
	{16,10,NULL,{NULL,10,NULL,NULL,0,0,1},TRUE }, // Ice Mountain
	{ 7,10,NULL,{NULL,10,NULL,NULL,0,0,1},FALSE}, // Jungle Japes
	{ 9,14,NULL,{NULL,14,NULL,NULL,0,0,1},FALSE}, // Lunor Stronghold - my own stage
	{ 8,14,NULL,{NULL,14,NULL,NULL,0,0,1},FALSE}, // Marked Mosque - my own stage
	{ 8,14,NULL,{NULL,14,NULL,NULL,0,0,1},FALSE}, // Reaten Base - my own stage
	{ 9,18,NULL,{NULL,18,NULL,NULL,0,0,1},FALSE}, // Red Sky Bay - my own stage
	{ 7,10,NULL,{NULL,10,NULL,NULL,0,0,1},FALSE}, // Soul Tower - my own stage
	{ 7,10,NULL,{NULL,10,NULL,NULL,0,0,1},FALSE}, // Terra Rock - my own stage
	{ 7,11,NULL,{NULL,11,NULL,NULL,0,0,1},FALSE}, // Yoshi's Island	
};

STAGE rtfstages[2] = { // race to the finish stages
	{20,20,NULL,{NULL,20,NULL,NULL,0,0,1},FALSE}, // layout 1
	{20,20,NULL,{NULL,20,NULL,NULL,0,0,1},FALSE}, // layout 2
};

// End of Source File