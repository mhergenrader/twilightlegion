// Twilight Legion Extra Data Installer
// Header File - header.h
// Michael Hergenrader

// This defines all constants and data structures that are stored in the external data files (sprites, strings, constants).
// The main file initializes the data and writes them to an external data file. Due to the large amount of data for the game 
// and a 64KB limit on the 68k TI calculators for any executable, this file must first be run and created on the calculator
// first before running the game. The benefit is that this space saved allows for more game features/code :-)
// Users do not need to worry about this process, as the data files will be distributed with the main game executables.
// These created files are "tlextra.*" for 89z, 9xz, and v2z (for each calculator model family).

#define EMPTY            0x00000000
#define CLOUD            0x00000010
#define SLOPELEFT        0x00000020
#define SLOPERIGHT       0x00000040
#define LADDER           0x00000080
#define COLLAPSING       0x00000800
#define HOTTILE          0x00004000
#define WATERTILE        0x00008000
#define DOOR             0x01000000 // changed this bit since caused testing errors
#define SOLID            0x20000000
#define PARTIAL          0x00000001 // solid block with the bottom 8 part solid

#define NUM_CHARS           24
#define NUM_STAGES          26
#define NUM_EPISODES        31 // "Episode Mode"
#define NUM_INVITATIONALS   10 // "Tournament Mode"
#define NUM_FRAMES          23 // total animation frames per playable character

#define BLACK_TILES
//#define WHITE_TILES // only one can be defined at a time - which shade of tile sprites to include

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

// Game Data Structures

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

typedef struct stageString {
	unsigned int length;
	unsigned char string[19];
} STAGE_STRING;

typedef struct ext {
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

// total data file size (with all added): 25000 bytes (does not include trophy gallery yet)

// End of Header File