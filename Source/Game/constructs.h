// Twilight Legion for TI-89(Titanium), TI-92(+), Voyage 200
// Header File - constructs.h
// Michael Hergenrader
// Compiled with TIGCC 0.96 Beta 8
// Please see README for license/disclaimer information. In short, please feel free to use code you see here,
// and any credit to me would be greatly appreciated! :-)
// Copyright 2005-2010 Michael Hergenrader

// This file includes game definitions and handy function prototypes for setup, as well
// as the definitions for malloc'd constants.

#ifndef CONSTRUCTS_H
#define CONSTRUCTS_H

#include "structures.h"

extern INT_HANDLER save_int_1; // device interrupt handlers
extern INT_HANDLER save_int_5;
extern INT_HANDLER save_int_6;
extern void* block;
extern void* virtual; // game virtual screens - tilemap and tournament bracket (buffers written to)

// Error messages for lacking memory or not having installed external files
#define NOT_ENOUGH_MEM       "Not enough memory for TL"
#define STAGE_NOT_INSTALLED  "tl_stage not installed"
#define CHARS_NOT_INSTALLED  "tl_charx files not installed"
#define EXTRA_NOT_INSTALLED  "tl_extra not installed"

// External data file names and extensions
#define STAGE_FILENAME       "tl_stage"
#define CHAR1_FILENAME       "tl_char1"
#define CHAR2_FILENAME       "tl_char2"
#define CHAR3_FILENAME       "tl_char3"
#define EXTRA_FILENAME       "tl_extra"
#define PROFILE_FOLDERNAME   "profiles"
#define PROFILE_EXTENSION    "user"

#define SENTINEL_VALUE   0x91 // splashscreen compression marker value

extern unsigned char* skylight;
extern unsigned char* skydark; // background screen for menus, splashscreen buffers (light and dark buffer to enable 4-level grayscale)

extern unsigned char* powbufferlight; // for the "Pow!" screen (Tournament round introduction screen)
extern unsigned char* powbufferdark;

// MCARD allocation (primary game buffers)

extern char* mainBlock;
extern char* tournamentBlock;
extern unsigned char* profileNameBlock;

extern void *v2;
extern void *v3; // auxiliary virtual screens (out of game virtuals)

extern PLAYER* p1;
extern PLAYER* p2;
extern PLAYER* p3;
extern PLAYER* p4; // player in-game structures

extern HAND* masterHand;
extern HAND* crazyHand; // structures for bosses

extern TIMER* timer;

#define CON_CLASSIC       20
#define CON_TITANIUM      15 // optimal contrast levels for device
#define MAX_PLAYERS        4
#define MAX_PROJECTILES    4

// 2001 * 4 = 8004 (four total planes to write to: onscreen and background grayscale buffers)
#define MCARD (LCD_SIZE + LCD_SIZE + sizeof(PLAYER) * MAX_PLAYERS + sizeof(TIMER) + sizeof(PROJECTILE) * MAX_PROJECTILES + 2 * sizeof(HAND) + 8004)
#define TCARD (GRAY_BIG_VSCREEN_SIZE * 2 + LCD_SIZE * 2) // tilemap allocation

#define MOVING_PAUSE        200
#define AI_DIFFERENCE         2 // speed of AI players to react

#define MIN_HP				  0
#define MAX_HP              999 // each player's max hit points
#define NUM_ITEMS            26
#define ITEM_PROBABILITY      2 // increase this to decrease chance of items in level
#define ITEM_OFFSET           5 // used to help add small items

#define ATTACK_ANIM_DELAY     8

#define JUMPVALUE            36
#define JUMPSPEED             2 // faster jumps = 4

#define WALKSPEED             2
#define RUNSPEED              4

#define MASTER_HAND_X       104 // boss values: hit points and starting horizontal locations
#define CRAZY_HAND_X         16
#define BOSS_HP_CLASSIC     200
#define BOSS_HP_ADMIRAL     300
#define BOSS_HP_PREMIERE    400
#define BOSS_HP_ELITE       500

#define NUM_STORY_STAGES      6
#define RACE_SECS_CLASSIC    13 // seconds to compete a race to the finish level under each difficulty setting
#define RACE_SECS_ADMIRAL    12
#define RACE_SECS_PREMIERE   11
#define RACE_SECS_ELITE       9

#define RETURN_TO_MAIN       80

// Used to establish the connection only
#define HOST_ID				 22 // a.k.a. "master" calc
#define JOIN_ID              24 // a.k.a. "slave" calc
#define LINK_ID              28 // magic numbers for establishing a connection between two calculators through serial link port (choose a value)

#define SETTINGS_COMPLETE    10
#define TEAMS_COMPLETE       12
#define STAGES_COMPLETE      14 // just marker values

// Profile constants
#define MAX_LEN               9
#define MAX_LEN2              3

// Whether scrolling needed in each 2-D direction
extern BOOL scrollL;
extern BOOL scrollR;
extern BOOL scrollU;
extern BOOL scrollD;

// In-game state trackers (in future iterations, could place under a Game structure)
extern BOOL disabled;
extern BOOL suddenDeath;
extern BOOL racing;
extern BOOL complete;
extern BOOL fightMetal;
extern BOOL fightCloaked;
extern BOOL cont;
extern BOOL episodeSuccess;
extern BOOL goAheadAndSave;
extern BOOL linked;

// Tilemap coordinates (where to render)
extern int x_fg;
extern int y_fg;
extern int x_bg;
extern int y_bg;

extern int numProfiles;
extern unsigned int p1ClassicLives;
extern unsigned int collapse;
extern unsigned int scrollType; // could move last one into GameEngine as static, but might be used for AI
extern unsigned int stageIndex;
extern unsigned int numPlayers;
extern unsigned int numHands;
extern unsigned int battleCounter; // used for classic
extern unsigned int winningTeam;
extern GAME_MODE mode;

extern unsigned char calc; // which calculator in the link process (master or slave)

extern int gameDifficulty;
extern int gameMatchMinutes;
extern STATUS gameCrowdPressure;
extern MATCHTYPE gameMatchType; // stock/lives or timed
extern unsigned int gameMatchLives;
extern unsigned int gameItemProb;

extern unsigned long points[8]; // the points array for completing a classic mode level

// References to external data files: tl_stage,tl_chars1,tl_chars2,tl_chars3,tl_extra
extern EXTERNAL* dataptr;
extern unsigned long* c1;
extern unsigned long* c2;
extern unsigned long* c3;
extern EXTRA_EXTERNAL* extraptr;

// In-game structures/linked lists
extern CHARACTER* characters;
extern ITEM* head;
extern PLAYER* pHead;
extern STAGE* stageTemp;
extern PLAYER* myPlayer;

extern PROFILE currentProfile;
extern char fileName[19];

extern STAGE stages[26];
extern STAGE rtfstages[2]; // race to the finish stages

extern unsigned char* profileNames[5];


// Utility macros

// optimization for array accessing p[i] - used for stage arrays (smaller size instructions than normal faster dereference operator)
#define DEREF_SMALL(__p,__i) \
 (*(typeof(&*(__p)))((unsigned char*)(__p) + (long)(short)((short)(__i) * sizeof(*(__p)))))


// Function prototypes

// Main.c:
void initialize(void);
void WaitForMillis(unsigned short);
unsigned char* ActiveContrastAddr(void);
void setContrast(unsigned int con);
void unarchiveFile(const char* file);
atexit_t exitGame(void);

// Menus.c:
//void introduction(void); // cool little intro
void mainMenu(void);
void VSScreen(void);
void RLE_Decompress(unsigned char* src, unsigned char* dest, short size);
void doProfileLoadingOrCreating(void);

// MainGame.c:
void doGame(void);
void doEpisode(unsigned int episodeIndex);
void setupLoadedGame(void);
void raceToTheFinish(unsigned int index);

// Players.c:
void handlePlayer(PLAYER* player); // used to handle the actual user based on key inputs and interaction w/ environment
inline unsigned int getTile(int txx, int tyy) __attribute__ ((pure));
inline BOOL playersCollided(PLAYER* playerA, PLAYER* playerB);
void executeNewAI(PLAYER* cpu);

// pointers to functions that handle each player, based on type (human or AI)
void (*playerFuncs[2])(PLAYER* p) = {
	handlePlayer,
	executeNewAI
};

// Projectile methods
PROJECTILE* setupProj(PLAYER* player);
BOOL moveProjectile(PROJECTILE* projectile, DIRECTION direction);
void explode(PROJECTILE* projectile);

// Items.c:
// Item handling methods for linked lists
inline void addItem(unsigned int index);
void drawAllItems(void *dest);
void freeItemList(ITEM** head);
ITEM* myItem(PLAYER* p);
BOOL existsItemInRegion(PLAYER* p);
PLAYER *throw(PLAYER* p);
PLAYER *smashBoost(PLAYER* p);
PLAYER *metalInit(PLAYER* p);
PLAYER *cloaking(PLAYER* p);
PLAYER *makeInvincible(PLAYER* p);

// functions for different items (what type of functionality they have)
PLAYER* (*itemFuncs[5])(PLAYER* p) = {
	throw,
	makeInvincible,
	smashBoost,
	cloaking,
	metalInit
};

// Extras.c:
// horizontally center text
inline unsigned int HCENTER(const char* const str, int width) __attribute__ ((pure));

// Link.c: TBD

// End of Header File

#endif