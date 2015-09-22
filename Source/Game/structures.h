// Twilight Legion for TI-89(Titanium), TI-92(+), Voyage 200
// Header File - structures.h
// Michael Hergenrader
// Compiled with TIGCC 0.96 Beta 8
// Please see README for license/disclaimer information. In short, please feel free to use code you see here,
// and any credit to me would be greatly appreciated! :-)
// Copyright 2005-2010 Michael Hergenrader

// This file contains all in-game proper structures as well as the external data file
// structures. The structure orderings and constants must match those of the TL Extra Data installer.

#ifndef STRUCTURES_H
#define STRUCTURES_H

#include "TileMap.h"

#define NUM_CHARS           24
#define NUM_STAGES          26
#define NUM_EPISODES        31 // "Episode Mode"
#define NUM_INVITATIONALS   10 // "Tournament Mode"
#define NUM_FRAMES          23 // total animation frames per playable character

typedef enum {
	ARENA_MODE,
	STORY_MODE,
	EPISODE_MODE,
	TOURNAMENT_MODE,
	NONE_SELECTED
} GAME_MODE; // All game modes ("Story" mode = "Classic" mode from Super Smash Bros)

typedef enum {
	CROWD_PRESSURE_SETT,
	MATCH_TYPE_SETT,
	MATCH_MINUTES_SETT,
	MATCH_LIVES_SETT,
	AI_DIFFICULTY_SETT,
	ITEM_PROBABILITY_SETT,
	STAGE_SELECT_SETT
} SETTINGS_TYPE; // Different types of settings in the game

typedef enum {
	HEIGHT = 100,
	WIDTH = 160
} SCREENDIMEN; // currently only for TI-89 dimensions

typedef enum {
	SMALL_MAPH = 7,
	SMALL_MAPW = 10,
	MED_MAPH = 8,
	MED_MAPW = 14,
	BIG_MAPH = 9,
	BIG_MAPW = 18
} MAPDIMEN; // sizes of each level size (small, medium, large)

typedef enum {
	HUMAN,
	CPU
} PLAYERTYPE; // whether character controlled by human or AI

typedef enum {
	LEFT = -1,
	RIGHT = 1
} DIRECTION; // directions for the player/everything else

typedef enum {
	UP = -1,
	DOWN = 1
} VERT_DIRECTION;

typedef enum {
	WHITE_TEAM = 1,
	LIGHTGRAY_TEAM,
	DARKGRAY_TEAM,
	BLACK_TEAM
} TEAM;



typedef enum {
	ARENA_MODE,
	STORY_MODE,
	EPISODE_MODE,
	TOURNAMENT_MODE,
	NONE_SELECTED
} GAME_MODE;

typedef enum {
	CROWD_PRESSURE_SETT,
	MATCH_TYPE_SETT,
	MATCH_MINUTES_SETT,
	MATCH_LIVES_SETT,
	AI_DIFFICULTY_SETT,
	ITEM_PROBABILITY_SETT,
	STAGE_SELECT_SETT
} SETTINGS_TYPE;

typedef enum {
	HEIGHT = 100,
	WIDTH = 160
} SCREENDIMEN; // currently only defined for TI-89/Titanium models; to be defined for TI-92+ and Voyage 200

typedef enum {
	SMALL_MAPH = 7,
	SMALL_MAPW = 10,
	MED_MAPH = 8,
	MED_MAPW = 14,
	BIG_MAPH = 9,
	BIG_MAPW = 18
} MAPDIMEN; // three different level/stage sizes

typedef enum {
	HUMAN,
	CPU
} PLAYERTYPE; // human controlled vs. AI

typedef enum {
	LEFT = -1,
	RIGHT = 1
} DIRECTION; // directions for the player/everything else

typedef enum {
	UP = -1,
	DOWN = 1
} VERT_DIRECTION;

typedef enum {
	WHITE_TEAM = 1,
	LIGHTGRAY_TEAM,
	DARKGRAY_TEAM,
	BLACK_TEAM
} TEAM; // four levels of grayscale - each team (4) receives one color

typedef enum {
	CLASSIC = 9,
	ADMIRAL = 7,
	PREMIERE = 5,
	ELITE = 3
} DIFFICULTY; // four levels of difficulty

typedef enum {
	TIMED,
	STOCK
} MATCHTYPE;

typedef enum { 
	OFF,
	ON
} STATUS;

typedef enum {
	STILL_ATTACK,
	PROJECTILE_ATTACK,
	MISSILE_ATTACK
} SPECIALATTACK_TYPE; // three types of "special attacks" in addition to general smash/direct hit attacks (defined uniquely for game characters)

typedef enum {
	SCROLL_DOWN,
	REST_AT_BOTTOM,
	SCROLL_UP,
	REST_AT_TOP
} MOVINGLEVEL_TYPE; // how the moving levels progress

typedef enum {
	NOT_COMPLETE,
	COMPLETE
} COMPLETE_STATUS; // to be more descriptive than true/false

typedef enum {
	STAND_RIGHT,
	BREATHE_RIGHT,
	RUN1_RIGHT,
	RUN2_RIGHT,
	CLIMB_RIGHT,
	JUMPUP_RIGHT,
	HURT_RIGHT,
	SMASH_RIGHT,
	SKY_RIGHT,
	SPECIAL_RIGHT,
	STAND_LEFT,
	BREATHE_LEFT,
	RUN1_LEFT,
	RUN2_LEFT,
	CLIMB_LEFT,
	JUMPUP_LEFT,
	HURT_LEFT,
	SMASH_LEFT,
	SKY_LEFT,
	SPECIAL_LEFT,
	CROUCH,
	TAUNT1,
	TAUNT2
} FRAME_ID; // 23 total animation frames (0-22 indices accessed at various actions)

typedef enum {
	PROXIMITY,
	FATIGUE,
	PLAYERONE_LAZINESS
} AI_ENEMY_DECISION; // for the final value, make sure to go after the human player if they are just resting on their laurels

typedef enum {
	IDLE,
	HOVER,
	HURT1,
	HURT2,
	FIRE_SLAM,
	PREP_GRAB,
	TRY_TO_CATCH,
	MISS,
	HOLDING_PLAYER,
	PALM_SLAP,
	PUNCH,
	SWAT_BACKSWING,
	SWAT_FRONTSWING,
	TEAMWORK_CLAP
} HAND_FRAME_ID; // boss battle ("Master Hand/Crazy Hand") animation sprite indices

typedef enum {
	ANTAIRA_VILLAGE,
	BATTLEFIELD,
	BIG_BLUE,
	BRINSTAR,
	CAIDRUS_CATHEDRAL,
	CORE_MONUMENT,
	CORNERIA,
	CRASPHONE_CITY,
	DREAMLAND,
	ERA_TOWER,
	FINAL_DESTINATION,
	FOURSIDE,
	FLATZONE,
	GLASS_SUBMARINE,
	GREAT_BAY,
	HETHOR_FOREST,
	HYRULE,
	INFINITE_GLACIER,
	JUNGLE_JAPES,
	LUNOR_STRONGHOLD,
	MARKED_MOSQUE,
	REATEN_BASE,
	RED_SKY_BAY,
	SOUL_TOWER,
	TERRA_ROCK, 
	YOSHIS_ISLAND
} STAGE_ID; // all 26 stages, some based off Super Smash Bros., others of my original design either for this game or from other games I brainstormed

typedef enum {
	AZZURRO,
	AXION,
	BOWSER,
	C_FALCON,
	DR_MARIO,
	DON_DORADO,
	FALCO,
	FOX,
	GANONDORF,
	KING_BOO,
	KIRBY,
	LINK,
	LUIGI,
	MARIO,
	MARTH,
	METAKNIGHT,
	MR_GAMENWATCH,
	PEACH,
	ROY,
	SAMUS,
	SONIC,
	WARIO,
	YOSHI,
	ZELDA,
	SELECT
} CHARACTER_ID; // 24 characters available, either based off Super Smash Bros., or from my own brainstormed games

typedef enum {
	RACE_POINTS,
	CLASSIC_COMPLETE,
	HAND_KILLER,
	SLAYER,
	SLAYED,
	INVULNERABLE,
	STRONG_GRIP,
	BONUS
} CLASSIC_POINTS; // types of points one can receive in "Classic Mode"

typedef enum {
	ONE_RANDOM,
	ALL_RANDOM,
	ONE_SELECT,
	ALL_SELECT
} STAGE_SELECT_SETTING; // for modes of multiple stage fights, how to select them (choose one, either deliberately or randomly; choose multiple)

typedef enum {
	NONE = -1,
	HYRULE_INV = 0,
	PAPAYA_BAY_INV = 1,
	FUSION_STAR_INV = 2,
	MANHATTAN_INV = 3,
	TITAN_INV = 4,
	HEROS_MOUNTAIN_INV = 5,
	INFERNO_REGION_INV = 6,
	COSMO_GALAXY_INV = 7,
	MANGO_ISLAND_INV = 8,
	ORANGE_INV = 9
} INVITATIONAL_ID; // different tournament types

typedef enum {
	HOST_CALC,
	JOIN_CALC,
	AI
} CALC_LINK_SETTING; // ID's for players during link play

typedef enum {
	EXIT = 32,
	NO_SIGNAL = 34,
	SUCCESSFUL = 36
} SIGNAL_STATUS; // link play connection statuses

typedef enum { 
	RACE1 = 2,
	RACE2 = 6,
	METAL_CHAR_FIGHT = 8,
	BOSS_FIGHT = 9
} STORY_MODE_SPECIAL_STAGE; // index of when each of these special missions must be completed in Story mode

typedef enum {
	NAME,
	INITIALS
} PROFILE_ENTRY;


typedef struct item { // item structure
	int x;
	int y;
	int h;
	BOOL beingHeld;
	BOOL beenUsed;
	unsigned int replenish;
	unsigned int index;
	const void *data;
	struct item* next; // for use in the linked lists (all items in a level stored in a linked list for quick appends)
} ITEM;

typedef struct timer {
	volatile int millis;
	volatile int seconds;
	volatile int minutes;
	volatile BOOL running;
} TIMER;

typedef struct frame { // sprite frame structure
	unsigned long* data;
	struct frame* next;
} FRAME;

typedef struct proj {
	int x;
	int y;
	int dir;
	unsigned int distance;
	unsigned int e; // counter for explosions
	BOOL exploding;
	const void* data;
} PROJECTILE;

typedef struct character { // stores character attributes for all 30 available (see playerdata.h)
	int w;
	int h; // physical attributes
	unsigned int specialType; // type of special attack
	FRAME frames[NUM_FRAMES];
} CHARACTER;

typedef struct linkstruct {
	int x;
	int y;
	int numKills;
	int numTimesKilled;
	int moveSpeed;
	int xspeed;
	int yspeed;
	
	unsigned int numLives;
	unsigned int power;
	unsigned int size;
	unsigned int characterIndex;
	unsigned int percent;
	unsigned int numJumps;
	
	BOOL breathing; // these basically correspond to every sense of animation - could use a bit array also
	BOOL running;
	BOOL taunting;
	BOOL falling;
	BOOL climbing;
	BOOL crouching;
	BOOL hanging;
	BOOL smashAttacking;
	BOOL specialAttacking;
	BOOL skyAttacking;
	BOOL beingHeld;
	BOOL grabbing;
	BOOL invincible;
	BOOL dead;
	BOOL paralyzed;
	BOOL onStage;
	BOOL onHillL;
	BOOL onHillR;
	BOOL canFire;
	BOOL cloaked;
	BOOL metal;
	
	DIRECTION direction;
	TEAM team;
	PLAYERTYPE type;
	
	unsigned int enemyIndex;
} LINK_STRUCT; // in-game structure sent across the link cable; has many similar fields but with fewer pointers to save space in transit

typedef struct player {
	int x;
	int y;
	int jumpValue;
	int numKills;
	int numTimesKilled;
	int moveSpeed;
	int xspeed;
	int yspeed;
	
	unsigned int numLives;	
	unsigned int power;
	unsigned int size;	
	unsigned int characterIndex;
	unsigned int percent;
	unsigned int numJumps;
	unsigned int playerCounter;
	unsigned int attackMarker;
	
	unsigned int pointsHolder[3]; // points for which player to attack (each player receives a system of points from criteria to determine whom to attack)
	
	BOOL breathing; // these basically correspond to every sense of animation - could use a bit array
	BOOL running;
	BOOL taunting;
	BOOL falling;
	BOOL climbing;
	BOOL crouching;
	BOOL hanging;
	BOOL smashAttacking;
	BOOL specialAttacking;
	BOOL skyAttacking;
	BOOL beingHeld;
	BOOL grabbing;
	BOOL invincible;
	BOOL dead;
	BOOL paralyzed;
	BOOL onStage;
	BOOL onHillL;
	BOOL onHillR;
	BOOL canFire;
	BOOL cloaked;
	BOOL metal;
	
	DIRECTION direction;
	TEAM team;
	PLAYERTYPE type;

	ITEM* currentItem;
	PROJECTILE* myProjectile;
	FRAME* rightCurrent;
	FRAME* leftCurrent;

	struct player* enemy; // scanned enemy is assigned here - even for human players (who players target)
	struct player* next;  // keeps the linked list of player structures
} PLAYER; // human and AI ingame player structure

typedef struct hand {
	int x;
	int y;
	int hitPoints;
	
	unsigned int attackMarker;
	unsigned int handCounter;
	unsigned int attackIndex;
	unsigned int frameIndex;

	BOOL hovering; // like above, could also use a bit array
	BOOL holdingPlayer;
	BOOL dropping;
	BOOL spastic; // player has killed the AI! (dying now - use those frames)
	BOOL dead;
} HAND; // used for master hand and crazy hand statuses

typedef struct stage {
	int sh;
	int sw;
	unsigned long* tileInfo; // the tile layout of a stage
	Plane fgPlane; // which plane to draw the stage to
	BOOL movingLevel;
} STAGE;

typedef struct episode {
	unsigned int p1numLives;
	unsigned int p2numLives;
	unsigned int p3numLives;
	unsigned int p4numLives;

	unsigned int playerNumber;
	unsigned int p1Index;
	unsigned int p2Index;
	unsigned int p3Index;
	unsigned int p4Index;	
	unsigned int levelIndex;
	int difficultyLevel;
} EPISODE; // for "Episode Mode": each episode has a custom definition/objective

typedef struct invitational {
	int difficultyLevel;
	char stageIndex;
	unsigned int opponents[15];
} INVITATIONAL; // aka Tournament

typedef struct settings {
	int difficulty;
	int matchMinutes;
	STATUS crowdPressure;
	MATCHTYPE matchType;
	unsigned int matchLives;
	unsigned int itemProb;
	unsigned int numPlayers;
} LINK_SETTINGS; // settings to establish when first linking calculators before game: sent from master to slave and then confirmed

typedef struct location {
	unsigned int x;
	unsigned int y;
} LOCATION;

typedef struct cursor {
	unsigned int x;
	unsigned int y;
} CURSOR;

typedef struct movingcursor {
	unsigned int x;
	int direction;
} MOVING_CURSOR;


// Saved file data

typedef struct profile {
	unsigned char name[9];
	unsigned char initials[4];
	
	// Player Record
	unsigned int numWins;
	unsigned int numLosses;
	unsigned int numTournamentWins;
	unsigned int numTournamentLosses;
	unsigned int numSuddenDeathWins;
	unsigned int numSuddenDeathLosses;
	
	// Settings for both Tournament and General (interweaved)	
	INVITATIONAL_ID invitationalID;	
	
	// can set these as chars? (still unsigned or signed)
	int difficulty;
	int tDifficulty;
	int matchMinutes; // minutes played in all matches (Arena and Tournament)
	int tMatchMinutes; // minutes played in tournament matches
	
	int tStageSelect; // current tournament setting for selecting stages
	
	// In-game settings
	STATUS crowdPressure; // rocks the background, making it more difficult for human players
	STATUS tCrowdPressure;
	MATCHTYPE matchType;
	MATCHTYPE tType;	
	unsigned int matchLives; // number of stock lives to set (1-5)
	unsigned int tMatchLives;
	unsigned int itemProb;
	unsigned int tItemProb;
	unsigned int roundNum;
	unsigned int savedStage;
	
	BOOL iHaveTournament; // current tournament exists
	BOOL iHaveBattle;
		
	char tourBracketLayout[35][13]; // all the current players in the tournament
	int characterIndexes[16]; // saves what players are still alive in the tournament
	
	// Quicksave Battle Stuff	- Arena mode only, non-linked
	char savedMinutes;
	char savedSeconds;
	char savedMilliseconds;
	int savedXBG;
	int savedXFG;
	int savedYBG;
	int savedYFG;	
	unsigned char savedBackground;
	unsigned char savedNumPlayers;
	unsigned int savedStageIndex;
	PLAYER savedPlayers[4];
	
	// Story Mode arrays - rename them to story mode
	int classicCharacterDifficultiesDone[NUM_CHARS]; // highest difficulty a player has completed - print out as a string - make this a char value
	unsigned long classicCharacterHighScores[NUM_CHARS];
	
	// Unlocking characters' arrays - store the defaults in tl_extra.data; this holds which items in the game have been unlocked
	#ifdef UNLOCK_CHARS
	unsigned char playersUnlocked[NUM_CHARS];
	unsigned char stagesUnlocked[NUM_STAGES];
	unsigned char episodesUnlocked[NUM_EPISODES];
	#endif
	
} PROFILE;

// External data file structures

typedef struct stageexternal { // tl_stage.data
	char antairavillage[8][18];
	char battlefield[8][12];
	char bigblue[7][10];
	char brinstar[9][13];
	char caidruscathedraloftime[7][14];
	char coresanctuarymonument[7][14];
	char corneria[10][15];
	char crasphonecity[8][14];
	char dreamland[8][14];
	char eratower[7][10];
	char finaldestination[6][14];
	char fourside[9][14];
	char flatzone[7][11];
	char glasssubmarine[7][14];
	char greatbay[9][15];
	char hethorforest[9][14];
	char hyrule[9][18];
	char icemountain[16][10];
	char junglejapes[7][10];
	char lunorstronghold[9][14];
	char markedmosque[8][14];
	char reatenbase[8][14];
	char redskybay[9][18];
	char soultower[7][10];
	char terrarock[7][10];
	char yoshisisland[7][11]; // all level layouts*
	
	short tiles1[104][32];
	short tiles2[118][32];
	short tiles3[100][32];
	short tiles4[89][32];
	short tiles5[112][32]; // all tile data*
	
	unsigned long tileinfo1[104];
	unsigned long tileinfo2[118];
	unsigned long tileinfo3[100]; 
	unsigned long tileinfo4[89]; 
	unsigned long tileinfo5[112]; // all tile info
	
	char backgrounds[3][7][11];
	short bgtiles[71][32];
	
	unsigned char smallitems[21][16];
	unsigned short bigitems[5][32]; // item data
	
	unsigned short datasprts[26][32]; // moved from old DATA_EXTERNAL
} EXTERNAL;

typedef struct ext { // tl_extra.data
	unsigned char coatlightsplash[820];
	unsigned char coatdarksplash[24]; // compressed - 844 bytes (21% of original); decompressed on launch
	unsigned char powlightsplash[540];
	unsigned char powdarksplash[432];
	
	const unsigned char charslightsplash[1040]; // level loading splashscreen (shows the characters)
	const unsigned char charsdarksplash[1040];
	
	unsigned long masterHandSprites[14][96]; // boss character sprites
	unsigned long crazyHandSprites[14][96];

	char raceLayouts[2][20][20];
	char raceToFinishBG[7][11];
	short raceToTheFinishSprites[10][32];
	unsigned long raceTileInfo[10];
	
	unsigned long gosign1[48]; // all signs shown at beginning, during, and end of match
	unsigned long gosign2[48];	
	unsigned long completesign[320];
	unsigned long completemask[160];
	unsigned long gamesign[144];
	unsigned long timesign[144];	
	unsigned long signmasks[8][24];
	
	unsigned char oncialFont[36][16]; // custom font defined by me
	unsigned char episodeTitlesText[31][31]; // Episode mode titles and descriptions
	unsigned char episodeDescriptions[62][27];
		
	unsigned short vsscreendata[11][32];
	unsigned short banner[96];
	
	short tournamentTiles[31][32];	
	char bracketLayout[35][13];
	
	EPISODE episodes[31];
	INVITATIONAL invitationalList[10];
	unsigned char credits[38][27];
} EXTRA_EXTERNAL;


// End of Header File

#endif