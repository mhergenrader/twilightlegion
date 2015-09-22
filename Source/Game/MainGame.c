// Twilight Legion for TI-89(Titanium), TI-92(+), Voyage 200
// C Source File - MainGame.c
// Michael Hergenrader
// Compiled with TIGCC 0.96 Beta 8
// Please see README for license/disclaimer information. In short, please feel free to use code you see here,
// and any credit to me would be greatly appreciated! :-)
// Copyright 2005-2010 Michael Hergenrader

// This file contains all of the in-game logic during fight sequences, whether between linked calculators or during
// any mode (Arena, Episode, Tournament, etc.)

#include <tigcclib.h>
#include "headers.h"

static unsigned int mycounter = 0;
static unsigned int threshold = 0;
static unsigned int backIndex;
static unsigned int myEpisode;
static unsigned int numEnemiesDefeated = 0;

static volatile unsigned int counter = 0; // managed by the system clock, so marked volatile (for determining when to drop items)

static BOOL camera = FALSE;
static BOOL alreadyHitting = FALSE;

static char timerStr[8];

static Plane bgPlane;

static LINK_STRUCT* otherPlayerData; // for link games - the structure that is sent between calculators

static const unsigned int NO_WINNER_FOUND = 48;

// Internal linkage file function prototypes

static void mainGame(void);

static void setupP1(void); // need to combine these functions
static void setupP2(void);
static void setupP3(void);
static void setupP4(void);
static inline void setupPlayerInGame(PLAYER* player);

static void controlPlayers(void);
static void checkForPlayerCollisions(void);
static inline BOOL projectilePlayerCollision(PLAYER* p, PROJECTILE* j);
static LOCATION respawn(void);
static BOOL playerAlreadyThere(int scrx, int scry); // at screen X and Y coordinates
static inline BOOL checkForDeathEvent(PLAYER* me);

static unsigned int checkForWinnerTimed(void);
static unsigned int checkForWinnerStock(void);
static unsigned int checkForSuddenDeathWin(void);
static void declareWinner(TEAM theWinningTeam);

static int getXSpeed(PLAYER* p);
static int getDamageToHitPlayer(PLAYER* p);

static void renderMaps(void* dest);
static void drawHUD(void *dest); // Level Drawing Methods *
static void drawGameMessage(unsigned int x, unsigned int y, unsigned char* str, void *dest0);
static void drawDeathStuff(PLAYER* p);

static void saveBattle(void);

static inline void setupHands(void);
static void checkForPlayerHandCollision(void); // Boss Methods
static inline BOOL playerCollidedWith(HAND* h);
static BOOL checkForHandFightWin(void);
static BOOL canDropHand(HAND* h);
static void masterHandAI(void);
static void crazyHandAI(void);

static void fireSlam(HAND* h);
static void tryToGrabPlayer(HAND* h);
static void palmSlap(HAND* h);
static void punch(HAND* h);
static void swat(HAND* h);
static void teamClap(HAND* h);

static void (*handAttacks[6])(HAND* h) = { // different attacks that can be used by boss characters
	fireSlam,
	tryToGrabPlayer,
	palmSlap,
	punch,
	swat,
	teamClap
};

static void transferGameData(); // for link games


// Timer Interrupt during the game that does the clock and items - respond to system timer events
DEFINE_INT_HANDLER(timer_int) {
	register void* olda5 asm("%a4");
	asm volatile("move.l %%a5,%0" : "=a"(olda5));
	asm volatile("lea __ld_entry_point_plus_0x8000(%pc),%a5");

	if (!gameMatchType) { // if a timed match (as opposed to a stock/lives match)
		if (!timer->millis && !timer->seconds && !timer->minutes) { // time has expired
			timer->running = FALSE;
		} else {
			timer->millis--;
			if (timer->millis < 0) {
				timer->millis = 20; // number set based on system hardware
				timer->seconds--;
				
				if (timer->seconds < 0) {
					timer->seconds = 59;
					timer->minutes--;
				}
			}
		}
	}

	// Determine on every cycle of ITEM_PROBABILITY whether to add a new item (for battles, not race to the finish)
	if (!racing && !((++counter) & (ITEM_PROBABILITY - 1)) && !random(gameItemProb * 20)) {
		addItem(random(NUM_ITEMS));
	}
	
	asm("move.l %0,%%a5" : : "a"(olda5)); // restore a5 register
}

// Main Game launcher
void doGame(void) {
	setupP1(); // could include these as an array of pointers to functions and have a loop that operates to make this look cleaner
	if (numPlayers > 1) {
		setupP2();
	}
	if (numPlayers > 2) {
		setupP3();
	}
	if (numPlayers > 3) {
		setupP4();
	}	
	
	if (numHands) {	// if this is now a boss level (for Episodes, Story, etc.), initialize bosses
		masterHand->x = MASTER_HAND_X;
		masterHand->y = 24;
		masterHand->hitPoints = (currentProfile.difficulty == CLASSIC) ? BOSS_HP_CLASSIC : (currentProfile.difficulty == ADMIRAL ? BOSS_HP_ADMIRAL : (currentProfile.difficulty == PREMIERE ? BOSS_HP_PREMIERE : BOSS_HP_ELITE));
		
		masterHand->attackMarker = 0;
		masterHand->handCounter = 0;
		masterHand->attackIndex = 0;
		masterHand->frameIndex = IDLE; // all frames are in tl_extra - access using extraptr
	
		masterHand->hovering = FALSE;
		masterHand->holdingPlayer = FALSE;
		masterHand->dropping = FALSE;
		masterHand->spastic = FALSE; // if true, player has killed the boss AI!
		masterHand->dead = FALSE;
	  	
	 	crazyHand->x = CRAZY_HAND_X;
		crazyHand->y = 24;
		crazyHand->hitPoints = (currentProfile.difficulty == PREMIERE ? BOSS_HP_PREMIERE : BOSS_HP_ELITE);
		
		crazyHand->attackMarker = 0;
		crazyHand->handCounter = 0;
		crazyHand->attackIndex = 0;
		crazyHand->frameIndex = IDLE; // all frames are in tl_extra - access using extraptr
	
		crazyHand->hovering = FALSE;
		crazyHand->holdingPlayer = FALSE;
		crazyHand->dropping = FALSE;
		crazyHand->spastic = FALSE;
		crazyHand->dead = FALSE;		
	}
	
	// reset teams here for classic mode
	if (mode == STORY_MODE) {
		switch (battleCounter) { // check which story level is being played and assign whether p1 gets allies
			case 1:
			p2->team = WHITE_TEAM, p4->team = p3->team = BLACK_TEAM;
			break;
			case 4:
			p4->team = p3->team = p2->team = BLACK_TEAM;
			break;
			case 7:
			p3->team = p2->team = LIGHTGRAY_TEAM;
			break;
		}
	}
	
	if (mode != TOURNAMENT_MODE) {
		gameDifficulty = currentProfile.difficulty;
		gameMatchType = currentProfile.matchType;
		gameMatchMinutes = currentProfile.matchMinutes;
		gameCrowdPressure = currentProfile.crowdPressure;
		gameMatchLives = currentProfile.matchLives;
		gameItemProb = currentProfile.itemProb;
	} else {
		if (currentProfile.invitationalID > NONE) { // invitational mode - doesn't affect any settings!
			gameDifficulty = (extraptr->invitationalList[currentProfile.invitationalID]).difficultyLevel;
			gameMatchType = TIMED;
			gameMatchMinutes = 2;
			gameCrowdPressure = OFF;
			gameItemProb = 5;
			stageIndex = (extraptr->invitationalList[currentProfile.invitationalID]).stageIndex;
		} else { // just tournament mode - settings can be changed
			gameDifficulty = currentProfile.tDifficulty;
			gameMatchType = currentProfile.tType;
			gameMatchMinutes = currentProfile.tMatchMinutes;
			gameCrowdPressure = currentProfile.tCrowdPressure;
			gameMatchLives = currentProfile.tMatchLives;
			gameItemProb = currentProfile.tItemProb;
		}
	}
	
	backIndex = 0; // currently no crowd pressure enabled
	stageTemp = &stages[stageIndex];
	
	// Initialize the background and foreground plane based on the stage
	bgPlane = (Plane){(char*)dataptr->backgrounds[backIndex], 11, (short*)dataptr->bgtiles, NULL, 0, 0, 1};
	bgPlane.big_vscreen = block+LCD_SIZE+LCD_SIZE;
	stageTemp->fgPlane.big_vscreen = block+LCD_SIZE+LCD_SIZE+GRAY_BIG_VSCREEN_SIZE;
	
	x_fg = ((stageTemp->sw << 4) - 160) / 2; // center the stage horizontally
	y_fg = 0;
	x_bg = 4;
	y_bg = 4;
	
	suddenDeath = FALSE;
	timer->millis = 0;
	timer->seconds = 0;
	timer->minutes = gameMatchMinutes;
	timer->running = !gameMatchType;
	counter = 0;
	
	if (!numHands) { // if not a boss level, show the preview screen for the fighters involved
		VSScreen();
	}

	SetIntVec(AUTO_INT_5, timer_int); // redirect the timer interrupt to capture it from system clock
	mainGame();
}

// Just like setting up a normal battle, set up a particular episode before beginning
void doEpisode(unsigned int episodeIndex) {
	episodeSuccess = FALSE;
	numEnemiesDefeated = 0;
	myEpisode = episodeIndex;
	numPlayers = extraptr->episodes[episodeIndex].playerNumber;

	gameDifficulty = extraptr->episodes[episodeIndex].difficultyLevel;
	gameMatchType = (episodeIndex == 19 ? TIMED : STOCK);
	gameItemProb = currentProfile.itemProb;
	
	setupP1();
	p1->numLives = extraptr->episodes[episodeIndex].p1numLives;
	myPlayer = p1;
	if (numPlayers > 1) {
		p2->characterIndex = extraptr->episodes[episodeIndex].p2Index;
		setupP2();
		p2->numLives = extraptr->episodes[episodeIndex].p2numLives;
		p2->team = BLACK_TEAM;
	}
	if (numPlayers > 2) {
		p3->characterIndex = extraptr->episodes[episodeIndex].p3Index;
		setupP3();
		p3->numLives = extraptr->episodes[episodeIndex].p3numLives;
		p3->team = BLACK_TEAM;
	}
	if (numPlayers > 3)	{
		p4->characterIndex = extraptr->episodes[episodeIndex].p4Index;
		setupP4();
		p4->numLives = extraptr->episodes[episodeIndex].p4numLives;
		p4->team = BLACK_TEAM;
	}	

	if (episodeIndex == 3) { // all samuses are permanently metalized
		fightMetal = TRUE;
	}
	if (episodeIndex == 5) {
		p2->team = WHITE_TEAM; // ally zelda with p1
	}
	if (episodeIndex == 7 || episodeIndex == 18) {
		setContrast(20); // mark difficult to see (also disable contrast changes)
	}	
	if (episodeIndex == 12) {
		fightCloaked = TRUE;
	}	
	if (episodeIndex == 14) {
		p2->team = LIGHTGRAY_TEAM;
		p3->team = DARKGRAY_TEAM;
	}	
	if (episodeIndex == 22) {
		p2->team = DARKGRAY_TEAM;
	}	
	if (episodeIndex == 29) { // special level - master hand and crazy hand
		numHands = 2;
		setupHands();
	}
	
	backIndex = random(2); // randomly enable crowd pressure in the background for episodes
	bgPlane = (Plane){(char*)dataptr->backgrounds[backIndex], 11, (short*)dataptr->bgtiles, NULL, 0, 0, 1};
	bgPlane.big_vscreen = block + LCD_SIZE + LCD_SIZE;
	
	stageTemp = &stages[extraptr->episodes[episodeIndex].levelIndex];
	stageTemp->fgPlane.big_vscreen = block + LCD_SIZE + LCD_SIZE + GRAY_BIG_VSCREEN_SIZE;
	
	x_fg = ((stageTemp->sw << 4) - 160) / 2; // center the level to start
	y_fg = 0;
	x_bg = 4;
	y_bg = 4;
	
	suddenDeath = FALSE;
	timer->millis = 0;
	timer->seconds = 0;
	timer->minutes = 20;
	timer->running = !gameMatchType; // start the timer if the episode should be timed
	counter = 0;
	
	SetIntVec(AUTO_INT_5, timer_int); // start capturing the 
	mainGame();
	
	// Restore original program conditions from episode changes
	fightMetal = FALSE;
	fightCloaked = FALSE;
	numHands = 0;
	setContrast(TI89_CLASSIC?CON_CLASSIC:CON_TITANIUM);	
}

// Reset all hand (boss) statuses
static inline void setupHands() {
	masterHand->x = MASTER_HAND_X,
	masterHand->y = 24,
	masterHand->hitPoints = BOSS_HP_ELITE,
	
	masterHand->attackMarker = 0,
	masterHand->handCounter = 0,
	masterHand->attackIndex = 0,
	masterHand->frameIndex = IDLE,

	masterHand->hovering = FALSE,
	masterHand->holdingPlayer = FALSE,
	masterHand->dropping = FALSE,
	masterHand->spastic = FALSE,
	masterHand->dead = FALSE,
  	
  	crazyHand->x = CRAZY_HAND_X,
	crazyHand->y = 24,
	crazyHand->hitPoints = BOSS_HP_ELITE,
	
	crazyHand->attackMarker = 0,
	crazyHand->handCounter = 0,
	crazyHand->attackIndex = 0,
	crazyHand->frameIndex = IDLE, // all frames are in tl_extra - access using extraptr

	crazyHand->hovering = FALSE,
	crazyHand->holdingPlayer = FALSE,
	crazyHand->dropping = FALSE,
	crazyHand->spastic = FALSE,
	crazyHand->dead = FALSE;	
}

// If a player has quit the game in the middle of a battle, this will load the game right where it was left off (Arena mode only)
void setupLoadedGame(void) {
	mode = ARENA_MODE;

	gameDifficulty = currentProfile.difficulty;
	gameMatchType = currentProfile.matchType;
	gameMatchMinutes = currentProfile.matchMinutes;
	gameCrowdPressure = currentProfile.crowdPressure;
	gameMatchLives = currentProfile.matchLives;
	gameItemProb = currentProfile.itemProb;
	
	backIndex = currentProfile.savedBackground; // crowd pressure or not
	bgPlane = (Plane){(char*)dataptr->backgrounds[backIndex],11,(short*)dataptr->bgtiles,NULL,0,0,1};
	bgPlane.big_vscreen = block + LCD_SIZE + LCD_SIZE;
	
	stageIndex = currentProfile.savedStageIndex;
	stageTemp = &stages[stageIndex];
	stageTemp->fgPlane.big_vscreen = block + LCD_SIZE + LCD_SIZE + GRAY_BIG_VSCREEN_SIZE;

	suddenDeath = FALSE; // need to take this into consideration
	
	x_fg = currentProfile.savedXFG;
	y_fg = currentProfile.savedYFG;
	x_bg = currentProfile.savedXBG;
	y_bg = currentProfile.savedYBG;
	
	// loop to set up all the players
	unsigned int i = 0;
	pHead = p1, p1->next = p2, p2->next = p3, p3->next = p4, p4->next = NULL;
	
	PLAYER* p = pHead;
	while (i < currentProfile.savedNumPlayers) {
		p->x = currentProfile.savedPlayers[i].x;
		p->y = currentProfile.savedPlayers[i].y;
		p->jumpValue = currentProfile.savedPlayers[i].jumpValue;
		p->numKills = currentProfile.savedPlayers[i].numKills;
		p->numTimesKilled = currentProfile.savedPlayers[i].numTimesKilled;
		p->moveSpeed = currentProfile.savedPlayers[i].moveSpeed;
		p->xspeed = currentProfile.savedPlayers[i].xspeed;
		p->yspeed = currentProfile.savedPlayers[i].yspeed;
		p->numLives = currentProfile.savedPlayers[i].numLives;
		p->power = currentProfile.savedPlayers[i].power;
		p->size = currentProfile.savedPlayers[i].size;
		p->characterIndex = currentProfile.savedPlayers[i].characterIndex;
		p->percent = currentProfile.savedPlayers[i].percent;
		p->numJumps = currentProfile.savedPlayers[i].numJumps;
		p->playerCounter = currentProfile.savedPlayers[i].playerCounter;
		p->attackMarker = currentProfile.savedPlayers[i].attackMarker;
		
		memmove(&p->pointsHolder, &currentProfile.savedPlayers[i].pointsHolder, 3 * sizeof(unsigned int));
		
		p->breathing = currentProfile.savedPlayers[i].breathing;
		p->running = currentProfile.savedPlayers[i].running;
		p->taunting = currentProfile.savedPlayers[i].taunting;
		p->falling = currentProfile.savedPlayers[i].falling;
		p->climbing = currentProfile.savedPlayers[i].climbing;
		p->crouching = currentProfile.savedPlayers[i].crouching;
		p->hanging = currentProfile.savedPlayers[i].hanging;
		p->smashAttacking = currentProfile.savedPlayers[i].smashAttacking;
		p->specialAttacking = currentProfile.savedPlayers[i].specialAttacking;
		p->skyAttacking = currentProfile.savedPlayers[i].skyAttacking;
		p->beingHeld = currentProfile.savedPlayers[i].beingHeld;
		p->grabbing = currentProfile.savedPlayers[i].grabbing;
		p->invincible = currentProfile.savedPlayers[i].invincible;
		p->dead = currentProfile.savedPlayers[i].dead;
		p->paralyzed = currentProfile.savedPlayers[i].paralyzed;
		p->onStage = currentProfile.savedPlayers[i].onStage;
		p->onHillL = currentProfile.savedPlayers[i].onHillL;
		p->onHillR = currentProfile.savedPlayers[i].onHillR;
		p->canFire = currentProfile.savedPlayers[i].canFire;
		p->cloaked = currentProfile.savedPlayers[i].cloaked;
		p->metal = currentProfile.savedPlayers[i].metal;
		
		p->direction = currentProfile.savedPlayers[i].direction;
		p->team = currentProfile.savedPlayers[i].team;
		p->type = currentProfile.savedPlayers[i].type;
		
		p->currentItem = NULL;
		
		p->rightCurrent = &characters[p->characterIndex].frames[STAND_RIGHT];
		p->leftCurrent = &characters[p->characterIndex].frames[STAND_LEFT];
		p->enemy = NULL;
				
		p = p->next;
		i++;
	}
	
	numPlayers = currentProfile.savedNumPlayers;
	
	pHead = p1;
	p1->next = p2;
	p2->next = NULL;
	if (numPlayers > 2) {
		p2->next = p3;
		p3->next = NULL;
	}
	if (numPlayers > 3) {
		p3->next = p4;
		p4->next = NULL;
  	}
	  
	myPlayer = p1;
		
	timer->millis = currentProfile.savedMilliseconds;
	timer->seconds = currentProfile.savedSeconds;
	timer->minutes = currentProfile.savedMinutes;
	timer->running = !gameMatchType; // if timed, then track the clock and capture timer interrupt events
	counter = 0;	
	
	SetIntVec(AUTO_INT_5,timer_int);
	mainGame();
	currentProfile.iHaveBattle = FALSE; // no more loaded game
}

// Credit to Fisch2 for Sumo 68k for help with this function
static void saveBattle(void) {
	freeItemList(&head); // don't save the current items in play (doing so would involve too much at the moment)
	
	currentProfile.iHaveBattle = TRUE,	
	currentProfile.savedMinutes = timer->minutes;
	currentProfile.savedSeconds = timer->seconds;
	currentProfile.savedMilliseconds = timer->millis;
	currentProfile.savedXBG = x_bg;
	currentProfile.savedXFG = x_fg;
	currentProfile.savedYBG = y_bg;
	currentProfile.savedYFG = y_fg;
	
	currentProfile.savedBackground = backIndex;
	currentProfile.savedNumPlayers = numPlayers;
	currentProfile.savedStageIndex = stageIndex;
		
	PLAYER* p = pHead;
	unsigned int i = 0;
	while (p != NULL) {
		currentProfile.savedPlayers[i].x = p->x;
		currentProfile.savedPlayers[i].y = p->y;
		currentProfile.savedPlayers[i].jumpValue = p->jumpValue;
		currentProfile.savedPlayers[i].numKills = p->numKills;
		currentProfile.savedPlayers[i].numTimesKilled = p->numTimesKilled;
		currentProfile.savedPlayers[i].moveSpeed = p->moveSpeed;
		currentProfile.savedPlayers[i].xspeed = p->xspeed;
		currentProfile.savedPlayers[i].yspeed = p->yspeed;
		currentProfile.savedPlayers[i].numLives = p->numLives;
		currentProfile.savedPlayers[i].power = p->power;
		currentProfile.savedPlayers[i].size = p->size;
		currentProfile.savedPlayers[i].characterIndex = p->characterIndex;
		currentProfile.savedPlayers[i].percent = p->percent;
		currentProfile.savedPlayers[i].numJumps = p->numJumps;
		currentProfile.savedPlayers[i].playerCounter = p->playerCounter;
		currentProfile.savedPlayers[i].attackMarker = p->attackMarker;
		
		memmove(&currentProfile.savedPlayers[i].pointsHolder, &p->pointsHolder, 3 * sizeof(unsigned int));
		
		currentProfile.savedPlayers[i].breathing = p->breathing;
		currentProfile.savedPlayers[i].running = p->running;
		currentProfile.savedPlayers[i].taunting = p->taunting;
		currentProfile.savedPlayers[i].falling = p->falling;
		currentProfile.savedPlayers[i].climbing = p->climbing;
		currentProfile.savedPlayers[i].crouching = p->crouching;
		currentProfile.savedPlayers[i].hanging = p->hanging;
		currentProfile.savedPlayers[i].smashAttacking = p->smashAttacking;
		currentProfile.savedPlayers[i].specialAttacking = p->specialAttacking;
		currentProfile.savedPlayers[i].skyAttacking = p->skyAttacking;
		currentProfile.savedPlayers[i].beingHeld = p->beingHeld;
		currentProfile.savedPlayers[i].grabbing = p->grabbing;
		currentProfile.savedPlayers[i].invincible = p->invincible;
		currentProfile.savedPlayers[i].dead = p->dead;
		currentProfile.savedPlayers[i].paralyzed = p->paralyzed;
		currentProfile.savedPlayers[i].onStage = p->onStage;
		currentProfile.savedPlayers[i].onHillL = p->onHillL;
		currentProfile.savedPlayers[i].onHillR = p->onHillR;
		currentProfile.savedPlayers[i].canFire = p->canFire;
		currentProfile.savedPlayers[i].cloaked = p->cloaked;
		currentProfile.savedPlayers[i].metal = p->metal;
		currentProfile.savedPlayers[i].direction = p->direction;
		currentProfile.savedPlayers[i].team = p->team;
		currentProfile.savedPlayers[i].type = p->type;
		
		currentProfile.savedPlayers[i].currentItem = NULL;
		currentProfile.savedPlayers[i].myProjectile = NULL;
		currentProfile.savedPlayers[i].rightCurrent = NULL;
		currentProfile.savedPlayers[i].leftCurrent = NULL;
		currentProfile.savedPlayers[i].enemy = NULL;
		currentProfile.savedPlayers[i].next = NULL; // these pointers will be set again during the course of the match
		
		i++;
		p = p->next;		
	}	
}

// All in-game logic for all modes
static void mainGame(void) {
	do {	
		unsigned int a; // checking for sudden death
		
		if (!scrollL && !scrollR && !scrollD && !scrollU && !stageTemp->movingLevel && !myPlayer->onStage && !myPlayer->dead) { 
			camera = TRUE; // adjusting the camera
			if (myPlayer->x < 16 && x_fg > 0) {
				scrollL = TRUE;
				myPlayer->x += 2;
			}
			if (myPlayer->x+characters[myPlayer->characterIndex].w > 144 && x_fg < stageTemp->sw * 16 - 160) {
				scrollR = TRUE;
				myPlayer->x -= 2;
			}
			if (myPlayer->y < 16 && y_fg > 0) {
				scrollU = TRUE;
				myPlayer->y += 2;
			}
			if (myPlayer->y + characters[myPlayer->characterIndex].h > 84 && y_fg < stageTemp->sh * 16 - 100) {
				scrollD = TRUE;
				myPlayer->y -= 2;
			}
	  	}
  	
  		if (camera) { // same as above - normal: +2 for all, no ?'s asked
  			if (scrollL) {
				if (myPlayer == p1) {
	  				p2->x += 2;
				} else {
	  				p1->x += 2;
				}
				p3->x += 2;
				p4->x += 2;
	  		}
	  		if (scrollR) {
		  		if (myPlayer == p1) {
		  			p2->x -= 2;
				} else {
		  			p1->x -= 2;	
				}
	  			p3->x -= 2;
				p4->x -= 2;
		  	}
	 		if (scrollU) {
	  			if (myPlayer == p1) {
		  			p2->y += 2;
				} else {
		  			p1->y += 2;
				}
		  		p3->y += 2;
				p4->y += 2;
	  		}
		  	if (scrollD) {
		  		if (myPlayer == p1) {
		  			p2->y -= 2;
				} else {
		  			p1->y -= 2;
				}
		  		p3->y -= 2;
				p4->y -= 2;
		  	}
  		}

		renderMaps(virtual); // draw the background and foregrounds at the new offsets from scrolling
		
		// Timed Matches - check for winner
		if (!gameMatchType) {
			if (!timer->running) {
				GraySprite32_SMASK_R(32, 34, 24, extraptr->timesign, extraptr->timesign + 24, extraptr->signmasks[2], GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE));
				GraySprite32_SMASK_R(64, 34, 24, extraptr->timesign + 48, extraptr->timesign + 72, extraptr->signmasks[3], GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE));
				GraySprite32_SMASK_R(96, 34, 24, extraptr->timesign + 96, extraptr->timesign + 120, extraptr->signmasks[4], GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE));
				
				if ((a = checkForWinnerTimed()) < NO_WINNER_FOUND) {
					declareWinner((TEAM)a);
					episodeSuccess = TRUE; // do I need to do special code for episode mode here?
					break;
				} else {
					PLAYER *p = pHead, *bestPlayer = NULL;
					int best = -30;
					while (p != NULL) {
						if (p->numKills - p->numTimesKilled > best) { // player has more points, make that player the comparative now
							if (bestPlayer != NULL) {
								bestPlayer->dead = TRUE; // found a new champ, send the other packing
							}
							bestPlayer = p;
							best = p->numKills-p->numTimesKilled;
						} else if (p->numKills-p->numTimesKilled < best) {
							p->dead = TRUE;
						}
						p = p->next;
					}
					
					p = pHead; // need to rescan, unfortunately, to pick up the real winners and put them back in
					while (p != NULL) {
						if (!p->dead) {
							p->percent = 300;
							p->numLives = 1;
							p->onStage = TRUE;
						}
						p = p->next;
					}
					
					WaitForMillis(2000);
					suddenDeath = TRUE;
					mycounter = 0;
					counter = 0;
					timer->millis = 0;
					timer->seconds = 0;
					timer->minutes = 1;
					timer->running = TRUE;
					continue; // start it up again! - sudden death time, baby, yeah!
				}
			}
		} else { // Stock Matches - check for winner 
			if (!numHands && (a = checkForWinnerStock()) < NO_WINNER_FOUND) {
				GraySprite32_SMASK_R(32, 34, 24, extraptr->gamesign, extraptr->gamesign + 24, extraptr->signmasks[5], GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE));
				GraySprite32_SMASK_R(64, 34, 24, extraptr->gamesign + 48, extraptr->gamesign + 72, extraptr->signmasks[6], GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE));
				GraySprite32_SMASK_R(96, 34, 24, extraptr->gamesign + 96, extraptr->gamesign + 120, extraptr->signmasks[7], GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE));
				
				if (mode != EPISODE_MODE) {
					declareWinner((TEAM)a);
				} else {
					if (a == myPlayer->team) {
						episodeSuccess = TRUE;
					}
				}
				break;
			}
		}
		
		if (suddenDeath && ((a = checkForSuddenDeathWin()) < NO_WINNER_FOUND)) { // NO_WINNER_FOUND is maximum value; if winner found, the check for win function will return a smaller value
			GraySprite32_SMASK_R(32, 34, 24, extraptr->gamesign, extraptr->gamesign + 24, extraptr->signmasks[5], GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE));
			GraySprite32_SMASK_R(64, 34, 24, extraptr->gamesign + 48, extraptr->gamesign + 72, extraptr->signmasks[6], GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE));
			GraySprite32_SMASK_R(96, 34, 24, extraptr->gamesign + 96, extraptr->gamesign + 120, extraptr->signmasks[7], GrayGetPlane(LIGHT_PLANE), GrayGetPlane(DARK_PLANE));
			declareWinner((TEAM)a);
			
			mycounter = 0;
			counter = 0;
			scrollType = 0;
			suddenDeath = FALSE;
			bgPlane.force_update++;
			stageTemp->fgPlane.force_update++;
			freeItemList(&head);
			
			if (mode != TOURNAMENT_MODE) { // required, else tournament will exit
				mainMenu();
			} else {
				return;
			}
		}
		
		controlPlayers(); // handle all players and AI BEFORE testing for collisions and attacks between them (gives equal opportunity)
		
		if (numHands > 0 && !masterHand->dead) {
			masterHandAI();
		}
		if (numHands > 1 && !crazyHand->dead) {
			crazyHandAI();
		}

		if (!numHands) {
			checkForPlayerCollisions(); // between two players
		} else {
			checkForPlayerHandCollision(); // between player and boss
		}
		
		if (fightMetal) {
			p2->metal = TRUE;
			p3->metal = TRUE;
			p4->metal = TRUE;
		}
		if (fightCloaked)
			p2->cloaked = TRUE;
			p3->cloaked = TRUE;
			p4->cloaked = TRUE;
		}		
		
		if (mode == EPISODE_MODE) {
			if (currentProfile.matchType && p1->dead) {
				break;
			}
			if ((myEpisode == 4 && mycounter > 1000) || (myEpisode == 23 && mycounter > 400)) {
				break;
			}
			if (myEpisode == 30 && (mycounter & 255) == 0) {
				fightCloaked = !fightCloaked;
			}
			if (myEpisode == 8) {
				if (p2->dead) {
					p2->numLives = 1, p2->dead = FALSE, p2->characterIndex = MARIO, p2->x = ((respawn()).x)-x_fg, p2->y = ((respawn()).y)-4-y_fg,numEnemiesDefeated++;
				}
				if (p3->dead) {
					p3->numLives = 1, p3->dead = FALSE, p3->characterIndex = MARIO, p3->x = ((respawn()).x)-x_fg, p3->y = ((respawn()).y)-4-y_fg,numEnemiesDefeated++;
				}
				if (p4->dead) {
					p4->numLives = 1, p4->dead = FALSE, p4->characterIndex = MARIO, p4->x = ((respawn()).x)-x_fg, p4->y = ((respawn()).y)-4-y_fg,numEnemiesDefeated++;
				}					
				if (numEnemiesDefeated > 127) {
					episodeSuccess = TRUE;
					break;
				}
			}
			if (myEpisode == 16)
				{
				if (p2->dead) {
					p2->numLives = 1;
					p2->dead = FALSE;
					p2->x = ((respawn()).x)+8-((characters[p2->characterIndex].w)/2)-x_fg;
					p2->y = ((respawn()).y)-(characters[p2->characterIndex].h)-((characters[p2->characterIndex].h)&1)+16-y_fg;
					numEnemiesDefeated++;
				}				
				if (numEnemiesDefeated > 4) {
					episodeSuccess = TRUE;
					break;
				}
				
				switch (numEnemiesDefeated) {
					case 1:
					p2->characterIndex = KIRBY;
					break;
					case 2:
					p2->characterIndex = FOX;
					break;
					case 3:
					p2->characterIndex = C_FALCON;
					break;
					case 4:
					p2->characterIndex = FALCO;
					break;
				}				
			}		
		}
		
		if (numHands) { // boss battles
			if (p1->dead) {
				break;
			}			
			if (!masterHand->spastic && !crazyHand->spastic && (a = checkForHandFightWin())) {			
				points[HAND_KILLER] = 20000 + (numHands - 1) * 20000;
				GraySprite32_SMASK_R(32,34,24,extraptr->gamesign,extraptr->gamesign+24,extraptr->signmasks[5],GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
				GraySprite32_SMASK_R(64,34,24,extraptr->gamesign+48,extraptr->gamesign+72,extraptr->signmasks[6],GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
				GraySprite32_SMASK_R(96,34,24,extraptr->gamesign+96,extraptr->gamesign+120,extraptr->signmasks[7],GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
				declareWinner((TEAM)a);
				episodeSuccess = TRUE;
				break;
			}
		}

		if (_keytest(RR_PLUS) && mode != EPISODE_MODE) {
			OSContrastUp();
		}
		if (_keytest(RR_MINUS) && mode != EPISODE_MODE) {
			OSContrastDn();
		}
		if (_keytest(RR_ESC) && mode != TOURNAMENT_MODE) {
			break;
		}		
		if (_keytest(RR_F3) && mode == ARENA_MODE && !linked && !suddenDeath) { // quick save battle in Arena mode
			saveBattle();
			ER_success();
			exit(0);
		}
		
		pokeIO(0x600005, 0x17);
	} while (TRUE);
	
	if (mode == STORY_MODE) {
		points[BONUS] = 20000 - mycounter;
	}
	mycounter = 0;
	counter = 0;
	scrollType = 0;
	bgPlane.force_update++;
	stageTemp->fgPlane.force_update++;
	
	freeItemList(&head);
	SetIntVec(AUTO_INT_5, DUMMY_HANDLER); // remove the timer handler from interrupt 5 (stop capturing it)
}

static void setupP1(void) {
	p1->x = 16, p1->y = 16;
	p1->jumpValue = 0;
	p1->numKills = 0, p1->numTimesKilled = 0;
	p1->moveSpeed = WALKSPEED;
	p1->xspeed = 0;
	p1->yspeed = 0;
	p1->numLives = (mode == ARENA_MODE) ? currentProfile.matchLives : ((mode == TOURNAMENT_MODE) ? currentProfile.tMatchLives : (cont ? currentProfile.matchLives : p1ClassicLives));
	
	setupPlayerInGame(p1);
	
	if (mode != TOURNAMENT_MODE) {
		p1->type = HUMAN; // human in all other modes guaranteed; tournament mode can be human watching a battle between 2 AI players
	}
	
	p1->currentItem = NULL;	
	p1->rightCurrent = &characters[p1->characterIndex].frames[STAND_RIGHT];
	p1->leftCurrent = &characters[p1->characterIndex].frames[STAND_LEFT];
	
	p1->enemy = NULL;
	p1->next = NULL;
	pHead = p1;
}

static void setupP2(void) {
	p2->x = 112, p2->y = 16;
	p2->jumpValue = 0;
	p2->numKills = 0, p2->numTimesKilled = 0;
	p2->moveSpeed = WALKSPEED;
	p2->xspeed = 0;
	p2->yspeed = 0;
	p2->numLives = (mode == ARENA_MODE) ? currentProfile.matchLives : ((mode == TOURNAMENT_MODE) ? currentProfile.tMatchLives : 1);
		
	setupPlayerInGame(p2);
	
	p2->type = (linked) ? HUMAN : CPU;
	p2->currentItem = NULL;
	p2->rightCurrent = &characters[p2->characterIndex].frames[STAND_RIGHT];
	p2->leftCurrent = &characters[p2->characterIndex].frames[STAND_LEFT];
	
	p2->enemy = NULL;
	p2->next = NULL;
	p1->next = p2;
}

static void setupP3(void) {
	p3->x = 80, p3->y = 16;
	p3->jumpValue = 0;
	p3->numKills = 0, p3->numTimesKilled = 0;
	p3->moveSpeed = WALKSPEED;
	p3->xspeed = 0;
	p3->yspeed = 0;
	p3->numLives = (mode==ARENA_MODE)?currentProfile.matchLives:1;
	
	setupPlayerInGame(p3);
	
	p3->type = CPU;		
	p3->currentItem = NULL;
	p3->rightCurrent = &characters[p3->characterIndex].frames[STAND_RIGHT];
	p3->leftCurrent = &characters[p3->characterIndex].frames[STAND_LEFT];
	
	p3->enemy = NULL;
	p3->next = NULL;
	p2->next = p3;
}

static void setupP4(void) {
	p4->x = 48, p4->y = 16,
	p4->jumpValue = 0, // needed?
	p4->numKills = 0, p4->numTimesKilled = 0,
	p4->moveSpeed = WALKSPEED,
	p4->xspeed = 0,
	p4->yspeed = 0,
	p4->numLives = (mode==ARENA_MODE)?currentProfile.matchLives:1;
	
	setupPlayerInGame(p4); // this separate method saves 438 bytes!
	
	p4->type = CPU,
	p4->currentItem = NULL,
	p4->rightCurrent = &characters[p4->characterIndex].frames[STAND_RIGHT],
	p4->leftCurrent = &characters[p4->characterIndex].frames[STAND_LEFT],
	
	p4->enemy = NULL,
	p4->next = NULL,
	p3->next = p4;
}

static void checkForPlayerCollisions(void) {
	PLAYER* t = pHead; // testing for collisions AFTER players have been handled to ensure fair chances
	while (t != NULL) {
		if (t->enemy != NULL && !t->paralyzed && !t->enemy->paralyzed && !t->onStage && !t->enemy->onStage && playersCollided(t,t->enemy)) {
			if (t->grabbing) {
				if (t->enemy->grabbing) {
					if (!random(gameDifficulty)) {
						if (!t->invincible) {
							t->beingHeld = TRUE, t->grabbing = FALSE;
						}
					} else { // I grab first
						if (!t->enemy->invincible) {
							t->enemy->beingHeld = TRUE, t->enemy->grabbing = FALSE;
						}
					}
				} else { // only I am grabbing
					if (!t->enemy->invincible) {
						t->enemy->beingHeld = TRUE;
					}
				}
			} else if (t->enemy->grabbing) { // only enemy grabbing
				if (!t->invincible) {
					t->beingHeld = TRUE, t->grabbing = FALSE;
				}
			} else if (t->smashAttacking || t->specialAttacking || t->skyAttacking) { // no one is grabbing, so onto attacking
				if (t->enemy->smashAttacking || t->enemy->specialAttacking || t->enemy->skyAttacking) {
					if (!random(gameDifficulty)) {
						if (((t->x+(characters[t->characterIndex].w/2)-8 <= t->enemy->x+(characters[t->enemy->characterIndex].w/2)-8 && t->enemy->direction < 0) || (t->x+(characters[t->characterIndex].w/2)+7 > t->enemy->x+(characters[t->enemy->characterIndex].w/2)+7 && t->enemy->direction > 0)) && !t->invincible) {
							t->percent += getDamageToHitPlayer(t),	t->xspeed = getXSpeed(t), t->yspeed = (((t->percent/18)*2)+2), t->paralyzed = TRUE;
						}
					} else {// I hit first
						if (((t->x+(characters[t->characterIndex].w/2)-8 > t->enemy->x+(characters[t->enemy->characterIndex].w/2)-8 && t->direction < 0) || (t->x+(characters[t->characterIndex].w/2)+7 <= t->enemy->x+(characters[t->enemy->characterIndex].w/2)+7 && t->direction > 0)) && !t->enemy->invincible) {
							t->enemy->percent += getDamageToHitPlayer(t->enemy), t->enemy->xspeed = getXSpeed(t->enemy), t->enemy->yspeed = (((t->enemy->percent/18)*2)+2), t->enemy->paralyzed = TRUE;
						}
					}
				} else { // only me attacking
					if (((t->x+(characters[t->characterIndex].w/2)-8 > t->enemy->x+(characters[t->enemy->characterIndex].w/2)-8 && t->direction < 0) || (t->x+(characters[t->characterIndex].w/2)+7 <= t->enemy->x+(characters[t->enemy->characterIndex].w/2)+7 && t->direction > 0)) && !t->enemy->invincible) {
						t->enemy->percent += getDamageToHitPlayer(t->enemy), t->enemy->xspeed = getXSpeed(t->enemy), t->enemy->yspeed = (((t->enemy->percent/18)*2)+2), t->enemy->paralyzed = TRUE;
					}
				}
			} else if (t->enemy->smashAttacking || t->enemy->specialAttacking || t->enemy->skyAttacking) { // only enemy attacking
				if (((t->x+(characters[t->characterIndex].w/2)-8 <= t->enemy->x+(characters[t->enemy->characterIndex].w/2)-8 && t->enemy->direction < 0) || (t->x+(characters[t->characterIndex].w/2)+7 > t->enemy->x+(characters[t->enemy->characterIndex].w/2)+7 && t->enemy->direction > 0)) && !t->invincible) {
					t->percent += getDamageToHitPlayer(t),	t->xspeed = getXSpeed(t), t->yspeed = (((t->percent/18)*2)+2), t->paralyzed = TRUE;
				}
			}
		}
	t = t->next;
	}
}

static void checkForPlayerHandCollision(void) {// only use p1, master hand, and crazy hand
	BOOL set = FALSE;
	if (!p1->invincible && !p1->paralyzed && !p1->onStage) {
		if (playerCollidedWith(masterHand) && !masterHand->dead) {
			set = TRUE;
			
			if (p1->smashAttacking || p1->specialAttacking || p1->skyAttacking) {
				if (!alreadyHitting) {
					masterHand->hitPoints -= 9+random(4)+(p1->skyAttacking*8), alreadyHitting = TRUE;
				}
			} else {
				alreadyHitting = FALSE;
			}
			
			if (masterHand->attackIndex > 0 && masterHand->attackIndex != 1 && !masterHand->spastic) {
				p1->percent += 16+((masterHand->attackIndex>5)*8),	p1->xspeed = -(random(3)*2), p1->yspeed = (((p1->percent/18)*2)+2), p1->paralyzed = TRUE;
			}
		}
		
		if (numHands > 0 && playerCollidedWith(crazyHand) && !set && !crazyHand->dead) {
			if (p1->smashAttacking || p1->specialAttacking || p1->skyAttacking) {
				if (!alreadyHitting) {
					crazyHand->hitPoints -= 9+random(4)+(p1->skyAttacking*8), alreadyHitting = TRUE;
				}
			} else {
				alreadyHitting = FALSE;
			}
			
			if (crazyHand->attackIndex > 0 && crazyHand->attackIndex != 1 && !crazyHand->spastic) {
				p1->percent += 16+((crazyHand->attackIndex>5)*8),	p1->xspeed = random(3)*2, p1->yspeed = (((p1->percent/18)*2)+2), p1->paralyzed = TRUE;
			}
		}
	}	
}

static void masterHandAI(void) {	
	if (scrollL) {
		masterHand->x += p1->moveSpeed;
	}
	if (scrollR) {
		masterHand->x -= p1->moveSpeed;
	}
	if (scrollD) {
		masterHand->y -= 2;
	}
	if (scrollU) {
		masterHand->y += 2;
	}
	
	if (masterHand->attackIndex > 0 && !masterHand->spastic) {
		handAttacks[masterHand->attackIndex - 1](masterHand);
	} else {// not attacking, so see if I want to
		BOOL adjusting = FALSE;
		
		// adjust master hand back to normal position
		if (masterHand->x + x_fg < MASTER_HAND_X + 32) {
			masterHand->x += 4, adjusting = TRUE;
		}
		if (masterHand->y + y_fg > 20) {
			masterHand->y -= 4, adjusting = TRUE;
		}
				
		if ((masterHand->handCounter & 63) == 0 && !adjusting) {
			masterHand->attackIndex = random((numHands>1)?6:5)+1; // don't allow clapping if only one hand
		}
	}
	
	if ((masterHand->handCounter & 15) == 0) { // could add a fatigue factor - breathe faster if the p has larger HP (between 32 and 64)
		masterHand->hovering = !masterHand->hovering;
	}
	
	// update hand frames
	
	if (!masterHand->attackIndex) { // if not attacking, hover
		masterHand->frameIndex = masterHand->hovering;
	}
	
	if (masterHand->holdingPlayer) {
		BOOL a = playerCollidedWith(masterHand);
		if (!a) {
			masterHand->x -= 4;
		}
		if (masterHand->handCounter-masterHand->attackMarker > 8) {
			masterHand->frameIndex = TRY_TO_CATCH;
		}
		if (masterHand->frameIndex == TRY_TO_CATCH) {
			if (a) {
				masterHand->frameIndex = HOLDING_PLAYER, p1->beingHeld = TRUE;
			} else {
				masterHand->frameIndex = MISS, masterHand->attackIndex = 0, masterHand->holdingPlayer = FALSE;
			}
		}
	}
	
	if (p1->beingHeld && masterHand->handCounter-masterHand->attackMarker > 25) {
		p1->beingHeld = FALSE, masterHand->attackIndex = 0, masterHand->holdingPlayer = FALSE, p1->percent+=10;
	}
	
	if (!masterHand->hitPoints && !masterHand->spastic) {
		masterHand->spastic = TRUE, masterHand->attackMarker = masterHand->handCounter;
	}
	
	if (masterHand->spastic) { // do the convulsing animation for a dying boss
		if ((masterHand->handCounter & 7) == 0) {
			masterHand->frameIndex = (!masterHand->hovering)+2;
		}
		if (masterHand->handCounter - masterHand->attackMarker > 128) {
			masterHand->spastic = FALSE, masterHand->dead = TRUE;
		}
	}
	
	masterHand->handCounter++;
}

static void crazyHandAI(void) { // bases its clapping on masterhand
	if (scrollL) {
		crazyHand->x += p1->moveSpeed;
	}
	if (scrollR) {
		crazyHand->x -= p1->moveSpeed;
	}
	if (scrollD) {
		crazyHand->y -= 2;
	}
	if (scrollU) {
		crazyHand->y += 2;
	}
	
	if (crazyHand->attackIndex > 0 && !crazyHand->spastic) {
		handAttacks[crazyHand->attackIndex - 1](crazyHand);
	} else { // not attacking, so see if I (crazy hand) want to (var is unsigned)
		BOOL adjusting = FALSE;
		// adjust crazy hand back to normal position
		if (crazyHand->x + x_fg > CRAZY_HAND_X + 32) {
			crazyHand->x -= 4, adjusting = TRUE;
		}
		if (crazyHand->y + y_fg > 20) {
			crazyHand->y -= 4, adjusting = TRUE;
		}
				
		if ((crazyHand->handCounter & 63) == 0 && !adjusting) {// use handCounter instead of random with &
			crazyHand->attackIndex = (masterHand->attackIndex > 5) ? 6 : ((masterHand->attackIndex == 4) ? 4 : random(5) + 1); // don't allow clapping if only one hand
		}
	}
	
	if ((crazyHand->handCounter & 15) == 0) { // could add a fatigue factor - breathe faster if the p has larger HP
		crazyHand->hovering = !crazyHand->hovering;
	}
	
	// update frames
	
	if (!crazyHand->attackIndex) { // if not attacking, hover
		crazyHand->frameIndex = crazyHand->hovering;
	}
	
	if (crazyHand->holdingPlayer) {
		BOOL a = playerCollidedWith(crazyHand);
		if (!a) {
			crazyHand->x += 4;
		}
		if (crazyHand->handCounter - crazyHand->attackMarker > 8) {
			crazyHand->frameIndex = TRY_TO_CATCH;
		}
		if (crazyHand->frameIndex == TRY_TO_CATCH) {
			if (a) {
				crazyHand->frameIndex = HOLDING_PLAYER;
				p1->beingHeld = TRUE;
			} else {
				crazyHand->frameIndex = MISS;
				crazyHand->attackIndex = 0;
				crazyHand->holdingPlayer = FALSE;
			}
		}
	}
	
	if (p1->beingHeld && crazyHand->handCounter-crazyHand->attackMarker > 25) {
		p1->beingHeld = FALSE;
		crazyHand->attackIndex = 0;
		crazyHand->holdingPlayer = FALSE;
		p1->percent += 10;
	}
	
	if (!crazyHand->hitPoints && !crazyHand->spastic) {
		crazyHand->spastic = TRUE;
		crazyHand->attackMarker = crazyHand->handCounter;
	}
	
	if (crazyHand->spastic) {
		if ((crazyHand->handCounter & 7) == 0) { // if on an 8px group boundary
			crazyHand->frameIndex = (!crazyHand->hovering) + 2;
		}
		if (crazyHand->handCounter - crazyHand->attackMarker > 128) {
			crazyHand->spastic = FALSE;
			crazyHand->dead = TRUE;
		}
	}
	
	crazyHand->handCounter++; // keep a running timer for crazy hand at which to make decisions at different intervals
}

// special attack: fire slam for master/crazy hand
static void fireSlam(HAND* h) {
	if (!h->dropping) {
		if (h->y > -32) {
			h->y -= 4;
		}
		if (h == masterHand) {
			if (h->x > p1->x) {
				h->x -= 2;
				h->dropping = FALSE;
			} else {
				h->dropping = TRUE;
			}
		} else {
			if (h->x < p1->x) {
				h->x += 2, h->dropping = FALSE;
			} else {
				h->dropping = TRUE;
			}
		}		
	} else {
		h->frameIndex = FIRE_SLAM;
		h->dropping = TRUE;
		
		if (canDropHand(h)) { // drop and smash them!
			h->y += 8;
		} else {
			h->attackIndex = 0, h->dropping = FALSE;
		}
	}
}

// boss move: grab a human player to temporarily disable their moves and hit them
static void tryToGrabPlayer(HAND* h) {
	if (h == masterHand || (h == crazyHand && !masterHand->holdingPlayer)) {
		if (!h->holdingPlayer) {
			h->attackMarker = h->handCounter;
			h->frameIndex = PREP_GRAB;
		}
		h->holdingPlayer = TRUE;
	}
}

// boss move: slap a player
static void palmSlap(HAND* h) {
	h->frameIndex = PALM_SLAP;
	if (canDropHand(h)) { // drop and smash them!
		h->y += 4;
	} else {
		h->x += (h == masterHand) ? -4 : 4;
	}
	
	if ((h == crazyHand && h->x + x_fg > 160) || (h == masterHand && h->x + x_fg < 32)) {
		h->attackIndex = 0;
	}
}

// simple punch attack (boss move)
static void punch(HAND* h) {
	h->frameIndex = PUNCH;
			
	if (canDropHand(h)) { // drop and smash them!
		h->y += 4;
	} else {
		h->x += (h == masterHand) ? -4 : 4;
	}
	
	if (h == masterHand) {
		if (h->x + x_fg < 36 || (crazyHand->attackIndex == 4 && crazyHand->x + 32 > h->x)) {
			h->attackIndex = 0;
		}
	} else {
		if (h->x + 32 > masterHand->x) {
			h->attackIndex = 0;
		}
	}
}

// hand attack: simple swat
static void swat(HAND* h) {
	if (canDropHand(h)) {// drop and smash them!
		h->y += 4;
	} else {
		h->x += (h == masterHand) ? -4 : 4;
	}
	
	if (((h->x + x_fg) & 31) == 0) {
		h->frameIndex = SWAT_FRONTSWING;
	} else if (((h->x + x_fg) & 15) == 0) {
		h->frameIndex = SWAT_BACKSWING;
	}
	
	if ((h == crazyHand && h->x + x_fg > 160) || (h == masterHand && h->x + x_fg < 32)) {
		h->attackIndex = 0;
	}
}

// coordinate clap attack between the two bosses
static void teamClap(HAND* h) {
	h->frameIndex = TEAMWORK_CLAP;
	if (canDropHand(h)) {// drop and smash them!
		h->y += 4;
	} else {
		h->x += (h == masterHand) ? -4 : 4;
	}
	
	if (h == masterHand) {
		if (crazyHand->x + 32 > h->x) {
			h->attackIndex = 0;
		}
	} else {
		if (h->x + 32 > masterHand->x) {
			h->attackIndex = 0;
		}
	}	
}

// return whether the boss can drop any further from hovering to the stage
static BOOL canDropHand(HAND* h) {
	if ((stageTemp->tileInfo[getTile(h->x + x_fg + 16,h->y + y_fg + 32)]) & SOLID) { // if a solid tile, cannot go any lower
		return FALSE;
	}
	return TRUE;
}

static inline BOOL playerCollidedWith(HAND* h) {// same as playersCollided, but modified for hands
	if (p1->x + characters[p1->characterIndex].w - 1 < h->x || p1->x > h->x + 31 || p1->y > h->y + 31 || p1->y+characters[p1->characterIndex].h - 1 < h->y) {
		return FALSE;
  	}
	return TRUE;
}

// determine whether the boss battle has ended
static BOOL checkForHandFightWin(void) {
	if (numHands < 2 && masterHand->dead) {
		return TRUE;
	}
	if (masterHand->dead && crazyHand->dead) {
		return TRUE;
	}
	return FALSE;
}

// determine whether a projectile that is active has hit a player (can only fire one projectile at once: once off screen or exploded, then replenish)
static inline BOOL projectilePlayerCollision(PLAYER* p, PROJECTILE* j) {
	if (j->x + 8 < p->x + (characters[p->characterIndex].w / 2) || j->x > p->x + (characters[p->characterIndex].w / 2) || j->y > p->y + characters[p->characterIndex].h - 1 || j->y + 8 < p->y) {
		return FALSE;
  	}
	return TRUE;
}

// find a suitable location in the current scrolling area of the tilemap to reintroduce a player w/ remaining lives (works for characters of any size)
static LOCATION respawn(void) {
	// gets the first topmost leftmost tile based on the current coordinates
	int tileStartX = (x_fg + (16 - (x_fg & 15))), tileStartY = (y_fg + (16 - (y_fg & 15)));
	int loopX = 0, loopY = 0, columnY;
	
	do {		
		do {
			if (DEREF_SMALL(stageTemp->tileInfo, getTile(tileStartX + (loopX * 16), tileStartY + (loopY * 16))) == 0x00000000 && !playerAlreadyThere(tileStartX + (loopX * 16), tileStartY + (loopY * 16))) {		
				columnY = tileStartY + ((loopY + 1) * 16); // scan the tile starting below, so add the +1
				do {
					unsigned long info = DEREF_SMALL(stageTemp->tileInfo, (getTile(tileStartX + (loopX * 16), columnY))); // get the current tile type
					if (info == SOLID || info == (SOLID | CLOUD)) {
						return (LOCATION){tileStartX + (loopX * 16), tileStartY + (loopY * 16)};
					}
					columnY += 16;				
				} while (columnY < 96);
			}			
			loopX++;
		} while (loopX < 8);
		loopX = 0, loopY++;
	} while (loopY < 5);
	
	return (LOCATION){0,0}; // default, so no compiler warning
}

// prevent respawning players on top of each other with this helper function
static BOOL playerAlreadyThere(int scrx, int scry) {
	PLAYER* temp = pHead;
	while (temp != NULL) { // must be respawning also, else, doesn't matter anyway
		if (temp->onStage && ((temp->x + x_fg + ((characters[temp->characterIndex].w) / 2) - 8) == scrx && (temp->y + y_fg + ((characters[temp->characterIndex].h) - 16)) == scry)) {
			return TRUE;
		}
		temp = temp->next;
	}
	return FALSE;
}

// the main control loop across all players: handle all player interactions and input
static void controlPlayers(void) {
	PLAYER* t = pHead;
	while (t != NULL) {
		if (!t->dead) {
			if (t != myPlayer) {
				if (scrollL)	{
					t->x += myPlayer->moveSpeed;
				}
				if (scrollR)	{
					t->x -= myPlayer->moveSpeed;
				}
				if (scrollD)	{
					t->y -= 2;
				}
				if (scrollU)	{
					t->y += 2;
				}
			} // in link play, the master/head calc manages all AI and p1 players; join calc is p2
			
			if (!t->canFire) { // can't fire due to disablement or already having the projectile in motion
	  			if (!moveProjectile(t->myProjectile, t->direction) || t->myProjectile->distance >= 90 || t->myProjectile->x < 0 || t->myProjectile->x > 156) {
					if ((t->myProjectile->x < 152 && t->myProjectile->x > 0) && (t->myProjectile->y > 0 && t->myProjectile->y < 92)) {
						t->myProjectile->exploding = TRUE; // if is on the screen, explode the projectile and display if something is hit
					}
					t->myProjectile->distance = 0, t->canFire = TRUE;
				} else {
					t->myProjectile->x += (t->myProjectile->dir * 4); // move the projectile and drop it ever so slightly
					t->myProjectile->distance++;
					
					if ((t->myProjectile->distance & 9) == 0) {
						t->myProjectile->y++;
					}
				}
			}
	  		
	  		PLAYER* projTemp = pHead; // projectile collisions are tested here now - now with both players and objects/boundaries
	  		while (projTemp != NULL) {
	  			if (projTemp != t && !projTemp->invincible && !projTemp->onStage && projectilePlayerCollision(projTemp, t->myProjectile)) {
	  				t->myProjectile->exploding = TRUE;
					projTemp->percent += random(6) + 6;
					t->myProjectile->distance = 0;
					t->canFire = TRUE;
	  				break;
	  			}
	  			projTemp = projTemp->next;
	  		}
				  
			if (t->myProjectile->exploding) { // start the animation for an exploding projectile
		  		explode(t->myProjectile);
			}
			
			if (!t->onStage) {
		  		if (checkForDeathEvent(t)) { // if a player has fallen from some "death event," need to account for that - ending the game, scores, etc.
					if (mode == STORY_MODE) {
						if (t == p1) {
							points[SLAYED] += 50;
						} else if (t->enemy == p1) {
							points[SLAYER] += 125;
						}
					}
					
					static unsigned int d = 0;
					if (++d < 16) {
						drawDeathStuff(t);
					} else {
						if (t->enemy != NULL) {
							t->enemy->numKills++;
						}
						
						if (gameMatchType) { // stock matches: number of remaining lives
							t->numLives--;
						} else { // timed matches accounting for scores
							t->numTimesKilled++;
						}
						
						t->percent = 0;
						if (t->numLives == 0 || suddenDeath) {// no more lives left
							t->dead = TRUE;
						} else {
						  	t->paralyzed = FALSE; // reset all player statuses
							t->xspeed = 0;
							t->yspeed = 0;
							t->rightCurrent = &characters[t->characterIndex].frames[STAND_RIGHT];
							t->leftCurrent = &characters[t->characterIndex].frames[STAND_LEFT];
							t->onStage = TRUE;
							t->cloaked = FALSE;
							t->metal = FALSE;
							t->onHillL = FALSE;
							t->onHillR = FALSE;
							t->hanging = FALSE;
							t->jumpValue = 0;
							t->numJumps = 0;
							
							if (stageTemp->movingLevel) { // determine where to place the player to respawn
								t->x = 72, t->y = 42;
							} else {
								t->x = ((respawn()).x) + 8 - ((characters[t->characterIndex].w) / 2) - x_fg;
								t->y = ((respawn()).y) - (characters[t->characterIndex].h) - ((characters[t->characterIndex].h) & 1) + 16 - y_fg;
							}
						}
					d = 0;
					}
				} else {
					if (!linked || (linked && ((calc == HOST_CALC && t != p2) || (calc == JOIN_CALC && t == p2)))) {
						playerFuncs[t->type](t);
					}
				}
			} else {
				t->invincible = TRUE; // currently (and temporarily) invincible
				static unsigned int countP = 0;
		  	
				if ((!t->type && _rowread(0)) || countP > 30) {
		  			t->onStage = FALSE;
					t->invincible = FALSE;
					t->jumpValue = 0;
					countP = 0;
				}
				countP++;
		  	}				  		  
		}
		t = t->next;
	}
}

static inline BOOL checkForDeathEvent(PLAYER* me) { // check for out of bounds - not a DEATH space
	if (stageTemp->movingLevel) { // if moving level and outside the screen boundary
		if (me->x < -20 || me->x > 160 || me->y + characters[me->characterIndex].h < -16 || me->y > 116) {
			return TRUE;
		}
		return FALSE;
	}
	
	if (me->x + x_fg < -36 || me->x + x_fg + characters[me->characterIndex].w > (stageTemp->sw * 16) + 36 || me->y + y_fg < -40 || me->y + y_fg > (stageTemp->sh << 4)) {
		return TRUE;
	}
	return FALSE;
}

// Render the Map function - draws everything and copies over to real screen from virtual (dest is the place to draw)
static void renderMaps(void* dest) {
	PLAYER* pTemp = pHead;
  
  	if (stageTemp->movingLevel) {
 		switch (scrollType) {
 			case SCROLL_DOWN: // scroll downward
 			if (y_fg < 156) {
				scrollD = TRUE;
			} else {
				scrollType++;
				threshold = mycounter;
				scrollD = FALSE;
			}
 			break;
 			
 			case REST_AT_BOTTOM: // stay at the bottom
 			if (mycounter-threshold > MOVING_PAUSE) {
				scrollType++;
			}
 			break;
 			
 			case SCROLL_UP: // scroll upward
 			if (y_fg > 0) {
				scrollU = TRUE;
			} else {
				scrollType++;
				threshold = mycounter;
				scrollU = FALSE;
			}
 			break;
 			
 			case REST_AT_TOP: // stay at the top 			
 			if (mycounter - threshold > MOVING_PAUSE) {
				scrollType = 0;
			}
			break;
 		}
  	}
	
	if (scrollL) { // adjust the foreground by how fast the P1 is moving
		x_fg -= myPlayer->moveSpeed;
	}
	if (scrollR)	{
		x_fg += myPlayer->moveSpeed;
	}
	if (scrollU)	{
		y_fg -= 2;
	}
	if (scrollD) {
		y_fg += 2;
	}
	
	if (x_fg < 0) {
		x_fg = 0;
	}
	if (x_fg > (stageTemp->sw << 4) - 160) {
		x_fg = (stageTemp->sw << 4) - 160;
	}
	if (y_fg < 0) {
		y_fg = 0;
	}
	if (x_bg < 0) {
		x_bg = 0;
	}
	if (y_bg < 0) {
		y_bg = 0;
	}

	DrawGrayPlane(x_bg, y_bg, &bgPlane, dest, dest + LCD_SIZE, TM_GRPLC89, TM_G16B);
  	DrawGrayPlane(x_fg, y_fg, &stageTemp->fgPlane, dest, dest + LCD_SIZE, TM_GTRANW89, TM_G16B); // copy the buffers to the real screen

  	if (gameCrowdPressure) { // keep shaking the background layer to give a notion of crowd pressure
		if (!(mycounter & 1)) {
			x_bg -= 2;
		} else {
			x_bg += 2;
		}
		
		if (!(mycounter & 3)) {
			y_bg -= 2;
		} else if (!(mycounter & 5)) {
			y_bg += 2;
		}
	}

	if (head != NULL) {
		drawAllItems(dest); // render all items on top now
	}
	
  	while (pTemp != NULL) {
		if (!pTemp->dead) {
			if (pTemp->cloaked) {
  				static unsigned int c = 0;
				if (++c > 80) {
					pTemp->cloaked = FALSE;
					c = 0; // invisibility counter
				}
  			} else if (pTemp->metal) {
					static unsigned int m = 0;
					if (++m > 300) {
		  				pTemp->size -= 5;
						pTemp->metal = FALSE;
						m = 0; // temporarily metal with higher "size"/defense stat
					}
				}
		  		
		  		if (pTemp->direction > 0) { // draw the player rendered as metal
		  			GrayClipSprite32_XOR_R(pTemp->x, pTemp->y, characters[pTemp->characterIndex].h, pTemp->rightCurrent->data, pTemp->rightCurrent->data + characters[pTemp->characterIndex].h, dest, dest + LCD_SIZE);
		  		} else {
		  			GrayClipSprite32_XOR_R(pTemp->x, pTemp->y, characters[pTemp->characterIndex].h, pTemp->leftCurrent->data, pTemp->leftCurrent->data + characters[pTemp->characterIndex].h, dest, dest + LCD_SIZE);
		  		}
			} else { // draw the player normally rendered, with clipping for sides of the screen
				if (!numHands || !pTemp->beingHeld) {
					if (pTemp->direction > 0) {
						GrayClipSprite32_SMASK_R(pTemp->x, pTemp->y, characters[pTemp->characterIndex].h, (const unsigned long*)pTemp->rightCurrent->data, (const unsigned long*)(pTemp->rightCurrent->data + (characters[pTemp->characterIndex].h)), (const unsigned long*)(pTemp->rightCurrent->data + (2 * characters[pTemp->characterIndex].h)), dest, dest + LCD_SIZE);
				  	} else {
						GrayClipSprite32_SMASK_R(pTemp->x, pTemp->y, characters[pTemp->characterIndex].h, (const unsigned long*)pTemp->leftCurrent->data, (const unsigned long*)(pTemp->leftCurrent->data + (characters[pTemp->characterIndex].h)), (const unsigned long*)(pTemp->leftCurrent->data + (2 * characters[pTemp->characterIndex].h)), dest, dest + LCD_SIZE);
					}
				}
			}
		}
		
  		if (pTemp->onStage) {
			GrayClipISprite16_XOR_R(pTemp->x, pTemp->y + characters[pTemp->characterIndex].h, 9, entrystage, dest, dest + LCD_SIZE);
		}
  		if (!pTemp->canFire) {
			GrayClipSprite8_SMASK_R(pTemp->myProjectile->x, pTemp->myProjectile->y, 8, bullet, bullet + 8, bulletmask, dest, dest + LCD_SIZE);
		}
		if (pTemp != myPlayer && pTemp->team == myPlayer->team) {// draw ally heart sprite above all allies
			GrayClipISprite8_XOR_R(pTemp->x + 4, pTemp->y - 8, 8, allysprt, dest, dest + LCD_SIZE);
		}
		
		pTemp = pTemp->next;
	}
	
	// for boss battles, render the hands
	if (numHands > 0 && !masterHand->dead) {
  		GrayClipSprite32_SMASK_R(masterHand->x, masterHand->y, 32, (const unsigned long*)extraptr->masterHandSprites[masterHand->frameIndex], (const unsigned long*)extraptr->masterHandSprites[masterHand->frameIndex] + 32, (const unsigned long*)extraptr->masterHandSprites[masterHand->frameIndex] + 64, dest, dest + LCD_SIZE);
	}
	if (numHands > 1 && !crazyHand->dead) {
		GrayClipSprite32_SMASK_R(crazyHand->x, crazyHand->y, 32, (const unsigned long*)extraptr->crazyHandSprites[crazyHand->frameIndex], (const unsigned long*)extraptr->crazyHandSprites[crazyHand->frameIndex] + 32, (const unsigned long*)extraptr->crazyHandSprites[crazyHand->frameIndex] + 64, dest, dest + LCD_SIZE);
	}
  	
	// draw initials above each player from the current profile
	if (myPlayer->x > -6 && myPlayer->x < 150 && myPlayer->y > 6 && myPlayer->y < 100 && !myPlayer->type && !myPlayer->dead) {
  		GrayDrawStrExt2B(myPlayer->x + 4, myPlayer->y - 6, currentProfile.initials, A_XOR, F_4x6, dest, dest + LCD_SIZE);
	}
	
  	// Draws the current time remaining centered near the top of the screen (white on black rectangle)
  	if (!gameMatchType && timer->running) {
		GrayFastFillRect_R(dest, dest + LCD_SIZE, 66, 9, 88, 15, COLOR_BLACK);
		sprintf(timerStr, ((timer->seconds < 10) ? "%d : 0%d" : "%d : %d"), timer->minutes, timer->seconds);
		GrayDrawStrExt2B(HCENTER(timerStr, 4), 10, timerStr, A_REVERSE, F_4x6, dest, dest + LCD_SIZE);
		
		if (!timer->minutes && timer->seconds < 6) {
			unsigned char str[2];
			sprintf(str, "%d", timer->seconds);
			drawGameMessage(76, 45, str, dest);
		}
	}
	
	// if player is off screen, then draw the arrow on the screen to determine where he/she is
	if (!myPlayer->dead && !myPlayer->type) {
		if (myPlayer->x <= -characters[myPlayer->characterIndex].w) {
	  		GrayClipISprite16_XOR_R(0, myPlayer->y, 12, playerarrowL, dest, dest + LCD_SIZE);
		} else if (myPlayer->x >= 160) {
	 	   	GrayClipISprite16_XOR_R(144, myPlayer->y, 12, playerarrowR, dest, dest + LCD_SIZE);
		} else if (myPlayer->y <= -characters[myPlayer->characterIndex].h) {
	  		GrayClipISprite16_XOR_R(myPlayer->x, 0, 14, playerarrowU, dest, dest + LCD_SIZE);
		} else if (myPlayer->y >= 100) {
	  		GrayClipISprite16_XOR_R(myPlayer->x, 86, 14, playerarrowD, dest, dest + LCD_SIZE);
		}
	}

	// draw the announcement screens during the game
	if (mycounter < 10 && !currentProfile.iHaveBattle) {
		drawGameMessage(suddenDeath ? 32 : 60, 45, (unsigned char*)(suddenDeath ? "SUDDEN DEATH" : "READY"), dest);
	} else if (mycounter < 16 && !currentProfile.iHaveBattle) {
		GraySprite32_SMASK_R(48, 34, 24, extraptr->gosign1, extraptr->gosign1 + 24, extraptr->signmasks[0], dest, dest + LCD_SIZE);
		GraySprite32_SMASK_R(80, 34, 24, extraptr->gosign2, extraptr->gosign2 + 24, extraptr->signmasks[1], dest, dest + LCD_SIZE);
	}
	
	drawHUD(dest); // render the heads-up display
	
	if (mode == EPISODE_MODE) { // special episode mode logic/rendering
		char str[5];
		if (myEpisode == 4 || myEpisode == 23) {			
			sprintf(str, "%u", mycounter);
			GrayDrawStrExt2B(10, 10, str, A_XOR, F_4x6, dest, dest + LCD_SIZE);
		}
		if (myEpisode == 8) {
			sprintf(str, "x%u", 128 - numEnemiesDefeated);
			GrayDrawStrExt2B(10, 10, str, A_XOR, F_4x6, dest, dest + LCD_SIZE);
		}
	}
	
  	mycounter++;
	scrollL = FALSE; // ensure that the game doesn't continuously scroll after keypress done
	scrollR = FALSE;
	scrollU = FALSE;
	scrollD = FALSE;
	camera = FALSE;
	
	memcpy(GrayGetPlane(LIGHT_PLANE), dest, LCD_SIZE);
  	memcpy(GrayGetPlane(DARK_PLANE), dest + LCD_SIZE, LCD_SIZE); // copy from virtual buffers to full screen buffer
}

// determine the winner of a timed match
static unsigned int checkForWinnerTimed(void) {
	PLAYER *temp = pHead;
	PLAYER *greatest = NULL;
	int best = -30, a;
	
	while (temp != NULL) {
		a = temp->numKills - temp->numTimesKilled; // difference in kills vs deaths
		if (a > best) {
			greatest = temp;
			best = a;
		} else if (a == best) { // we have a sudden death situation
			return NO_WINNER_FOUND;
		}
		temp = temp->next;
	}
	return (int)greatest->team;
}

// check for a winning player/team in stock/lives match
static unsigned int checkForWinnerStock(void) {
	PLAYER *temp = pHead;
	PLAYER *checker;
	
	while (temp != NULL) {
		if (!temp->dead) {
			break;
		}
		temp = temp->next;
	} // find the first not dead player in the list
	
	if (temp->next == NULL) {
		return (int)temp->team;
	}
	
	checker = temp->next;
	while (checker != NULL) {
		if (!checker->dead && temp->team != checker->team) {
			return NO_WINNER_FOUND;
		}
		checker = checker->next;
	}
	return (int)temp->team; // did not return early so a team won!	
}

// check for winning player/team in sudden death situation
static unsigned int checkForSuddenDeathWin(void) {
	PLAYER *temp = pHead;
	PLAYER *winner = NULL;
	
	while (temp != NULL) {
		if (!temp->dead) {
			if (winner == NULL) {
				winner = temp;
			} else {
				return NO_WINNER_FOUND;
			}
		}
		temp = temp->next;
	}
	
	return (int)winner->team;
}

// declares a winner if one team is still left in the playing field
static void declareWinner(TEAM theWinningTeam) {
	while (!_rowread(0)); // wait for a keypress before proceeding
	
	winningTeam = theWinningTeam;
	
	if (mode == ARENA_MODE) { // this now only counts for arena mode - critical update for unlocking stuff
		if (myPlayer->team == winningTeam) {
			currentProfile.numWins++;
			if (suddenDeath) {
				currentProfile.numSuddenDeathWins++;
			}
		} else {
			currentProfile.numLosses++;
			if (suddenDeath) {
				currentProfile.numSuddenDeathLosses++;
			}
		}
	} else if (mode == TOURNAMENT_MODE && suddenDeath) { // track the number of tourny wins and losses for the player's profile
		if (p1->team == winningTeam) {
			currentProfile.numTournamentWins++;
			currentProfile.numSuddenDeathWins++;
		} else {
			currentProfile.numTournamentLosses++;
			currentProfile.numSuddenDeathLosses++;
		}
	}
	
	// update profile with a win or loss depending on if white team won or lost
	if (!mode || mode == TOURNAMENT_MODE || (mode == EPISODE_MODE && myEpisode == 19)) {
		memset(v2, 0xFF, LCD_SIZE);
		memset(v3, 0xFF, LCD_SIZE);
		
		char title[21];
		char extra[13];
		char winners[80];
		
		sprintf(title, "%s Match Complete", matchTypeText[gameMatchType]);
		GrayDrawStrExt2B(20, 2, title, A_XOR, F_6x8, v2, v3);
		
		GrayFastDrawHLine2B_R(v2,v3,0,159,12,COLOR_WHITE);
		GrayFastDrawHLine2B_R(v2,v3,0,159,22,COLOR_WHITE);
		GrayFastDrawHLine2B_R(v2,v3,0,159,90,COLOR_WHITE);
		
		GrayFastDrawLine2B_R(v2,v3,28,22,28,90,COLOR_WHITE);
		GrayFastDrawLine2B_R(v2,v3,61,22,61,90,COLOR_WHITE);
		GrayFastDrawLine2B_R(v2,v3,94,22,94,90,COLOR_WHITE);
		GrayFastDrawLine2B_R(v2,v3,127,22,127,90,COLOR_WHITE);
			
		FontSetSys(F_4x6);
		GrayDrawStr2B(4,16,"Stats",A_XOR,v2,v3);
		
		if (!p1->type) {
			GrayDrawStr2B(38,16,currentProfile.initials,A_XOR,v2,v3);
		} else {
			GrayDrawStr2B(40,16,"P1",A_XOR,v2,v3);
		}
		
		if (!p2->type && calc == JOIN_CALC) {
			GrayDrawStr2B(73,16,currentProfile.initials,A_XOR,v2,v3);
		} else {
			GrayDrawStr2B(73,16,"P2",A_XOR,v2,v3);
		}
		
		GrayDrawStr2B(106,16,"P3",A_XOR,v2,v3);
		GrayDrawStr2B(139,16,"P4",A_XOR,v2,v3);
		
		// Sidebar stuff
		if (!gameMatchType) { // timed match
			GrayDrawStr2B(2,28,"Kills",A_XOR,v2,v3);
			GrayDrawStr2B(2,36,"Falls",A_XOR,v2,v3);
			GrayDrawStr2B(2,44,"Total",A_XOR,v2,v3);
			
			unsigned int x = 36, n = 0;
			char stat[4];
			PLAYER* temp = pHead;
			
			while (temp != NULL) {
				if (numPlayers > n) {
					sprintf(stat,"%d",temp->numKills);
					GrayDrawStr2B(x,28,stat,A_XOR,v2,v3);
				}
				temp = temp->next,n++,x+=33;
			}
			
			x = 36, n = 0, temp = pHead;
			while (temp != NULL) {
				if (numPlayers > n) {
					sprintf(stat,"-%d",temp->numTimesKilled);
					GrayDrawStr2B(x,36,stat,A_XOR,v2,v3);
				}
				temp = temp->next,n++,x+=33;
			}
			
			x = 36, n = 0, temp = pHead;
			while (temp != NULL) {
				if (numPlayers > n) {
					sprintf(stat,"%d",temp->numKills-temp->numTimesKilled);
					GrayDrawStr2B(x,44,stat,A_XOR,v2,v3);
				}
				temp = temp->next,n++,x+=33;
			}
			
			// display winner strings
			temp = pHead,n = 0;
			while (temp != NULL) {
				if (numPlayers > n) {
					if (temp->team == winningTeam) {
						sprintf(extra,"%s,",characterNames[temp->characterIndex]);
						strcat(winners,extra);
					}
				}
				temp = temp->next,n++;
			}	
		} else { // stock matches
			GrayDrawStr2B(2,28,"Kills",A_XOR,v2,v3);
			GrayDrawStr2B(2,36,"Lives",A_XOR,v2,v3);
			GrayDrawStr2B(2,44,"Status",A_XOR,v2,v3);
			
			unsigned int x = 36, n = 0;
			char stat[4];
			PLAYER* temp = pHead;			
			
			while (temp != NULL) {
				if (numPlayers > n) {
					sprintf(stat,"%d",temp->numKills);
					GrayDrawStr2B(x,28,stat,A_XOR,v2,v3);
				}
				temp = temp->next,n++,x+=33;
			}
			
			x = 36, n = 0, temp = pHead;
			while (temp != NULL) {
				if (numPlayers > n) {
					sprintf(stat,"%d",temp->numLives);
					GrayDrawStr2B(x,36,stat,A_XOR,v2,v3);
				}
				temp = temp->next,n++,x+=33;
			}
			
			x = 36, n = 0, temp = pHead;
			while (temp != NULL) {
				if (numPlayers > n) {
					sprintf(stat,"%s",(temp->team!=winningTeam)?"DEAD":"WIN!"); // partner could have won the game
					GrayDrawStr2B(x,44,stat,A_XOR,v2,v3);
				}
				temp = temp->next,n++,x+=33;
			}
			
			temp = pHead, n = 0;
			while (temp != NULL) {
				if (numPlayers > n) {
					if (temp->team == winningTeam) { // the partner(s) could have still won
						sprintf(extra,"%s,",characterNames[temp->characterIndex]);
						strcat(winners,extra);
					}
				}
				temp = temp->next,n++;
			}	
		}
		
		PLAYER* temp = pHead;
		unsigned int n = 0, x = 29;
		while (temp != NULL) {
			if (numPlayers > n) {
				if (!gameMatchType) {
					GraySprite32_SMASK_R(x,56,characters[temp->characterIndex].h,characters[temp->characterIndex].frames[(temp->team==winningTeam)?TAUNT1:HURT_RIGHT].data,characters[temp->characterIndex].frames[(temp->team==winningTeam)?TAUNT1:HURT_RIGHT].data+characterHeights[temp->characterIndex],characters[temp->characterIndex].frames[(temp->team==winningTeam)?TAUNT1:HURT_RIGHT].data+2*characterHeights[temp->characterIndex],v2,v3);
				} else {
					GraySprite32_SMASK_R(x,56,characters[temp->characterIndex].h,characters[temp->characterIndex].frames[(temp->numLives>0)?TAUNT1:HURT_RIGHT].data,characters[temp->characterIndex].frames[(temp->numLives>0)?TAUNT1:HURT_RIGHT].data+characterHeights[temp->characterIndex],characters[temp->characterIndex].frames[(temp->numLives>0)?TAUNT1:HURT_RIGHT].data+2*characterHeights[temp->characterIndex],v2,v3);
				}
			}
			temp = temp->next,x+=33;
		}
	
		GrayDrawStrExt2B(2,92,"Winner(s):",A_XOR,F_4x6,v2,v3);
		GrayDrawStrExt2B(38,92,winners,A_XOR,F_4x6,v2,v3);
		pokeIO(0x600005, 0x17); // wait for any key
		
		memcpy(GrayGetPlane(LIGHT_PLANE),v2,LCD_SIZE);
		memcpy(GrayGetPlane(DARK_PLANE),v3,LCD_SIZE);
		
		while (_rowread(0)); // flush keys
		while (!_rowread(0));
	}
}

// determine current player speed for handling horizontal scrolling/player flying up when hit
static int getXSpeed(PLAYER* p) {
	p->xspeed = 0;
	
	if (abs(p->x - p->enemy->x) < 8) {
		p->xspeed = 2; // assuming player is attacked
	} else if (abs(p->x - p->enemy->x) < 33) {
		p->xspeed = 4;
	}
	
	if (p->xspeed > 0 && p->x < p->enemy->x) {
		p->xspeed = -p->xspeed;
	}
	
	return p->xspeed;
}

// get the incremental damage to a player just hit by an attack; based on attack and hit points
static int getDamageToHitPlayer(PLAYER* p) {
	if (p->enemy == NULL) {
		return 0;
	}
	return (p->enemy->power - p->size + random(7) + random(6) + ((p->percent) / 128) * 13);
}

// render the HUD (heads up display)
static void drawHUD(void* dest) {
	GrayFastDrawHLine2B_R(dest,dest+LCD_SIZE,0,159,91,COLOR_BLACK);
	GrayFastFillRect_R(dest,dest+LCD_SIZE,0,92,159,99,COLOR_LIGHTGRAY);
	
	if (!numHands) {
		PLAYER* temp = pHead;
		unsigned int x = 30;
		while (temp != NULL) {
			GraySprite8_TRANB_R(x+10,92,8,hudcharsprts[hudIndexes[temp->characterIndex]],hudcharsprts[hudIndexes[temp->characterIndex]]+8,dest,dest+LCD_SIZE);
			
			if (!temp->dead) {			
				char str[6];
				if (gameMatchType) { // if a stock match, draw the number of lives remaining - might need to check tournament mode here
					GrayClipISprite8_XOR_R(x,92,8,hudlives[temp->numLives],dest,dest+LCD_SIZE);
				}
				if (temp->percent > MAX_HP) {
					temp->percent = MAX_HP;
				}
				
				sprintf(str,"%.3u",temp->percent);
				strcat(str,"%");
				GrayDrawStrExt2B(x+10,93,str,A_NORMAL,F_4x6,dest,dest+LCD_SIZE);
			}
			x+=30, temp = temp->next;
		}
	} else {
		char str[6];
		GrayClipISprite8_XOR_R(30,92,8,hudlives[p1->numLives],dest,dest+LCD_SIZE);
		if (p1->percent > MAX_HP) {
			p1->percent = MAX_HP;
		}
		sprintf(str,"%.3u",p1->percent);
		strcat(str,"%");
		GrayDrawStrExt2B(40,93,str,A_NORMAL,F_4x6,dest,dest+LCD_SIZE);
		
		if (masterHand->hitPoints < 0) {
			masterHand->hitPoints = 0;
		}
		sprintf(str,"%.3uHP",masterHand->hitPoints);
		GrayDrawStrExt2B(110,93,str,A_NORMAL,F_4x6,dest,dest+LCD_SIZE);
		
		if (numHands > 1) { // print out crazy hand info
			if (crazyHand->hitPoints < 0) {
				crazyHand->hitPoints = 0;
			}
			sprintf(str,"%.3uHP",crazyHand->hitPoints);
			GrayDrawStrExt2B(70,93,str,A_NORMAL,F_4x6,dest,dest+LCD_SIZE);
		}
	}
}

// function for drawing a text message in the game
// Thanks for Fisch2 for helping develop this function
static void drawGameMessage(unsigned int x, unsigned int y, unsigned char* str, void *dest0) {
	unsigned int pos = 0, len = strlen(str); // length of string 
	int offset;
	
	do { // loop through each character of the string
		if (str[pos] >= 65 && str[pos] <= 90) {// opposite of CAPS lock
			str[pos] += 32;
		}
		offset = -1;
		
		if (str[pos] >= 97 && str[pos] <= 122) { // a letter
			offset = (str[pos] - 97);
		}
		else if (str[pos] >= 47 && str[pos] <= 57) { // a number
			offset = 25+(str[pos] - 47);
		}
				
		if (offset > -1) {
			GrayClipISprite8_XOR_R(x,y,8,extraptr->oncialFont[offset],dest0,dest0+LCD_SIZE);
		}

		x += 8;
		if (x > LCD_WIDTH) { // if past the screen boundaries, don't continue drawing letters
			return;
		}
		pos++;
	} while (pos < len);	
}

// draw the explosions for when a player flies off the screen (for different sides)
static void drawDeathStuff(PLAYER* p) {
	unsigned int i = 0;
	
	if (p->x < 0) {
		unsigned int x = 0;
		do {
			GrayClipISprite16_XOR_R(x,p->y,16,blast,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
			x+=16;
			i++;
		} while (i < 5);
		return;
	}
	
	if (p->y < 0) {
		unsigned int y = 0;
		do {
			GrayClipISprite16_XOR_R(p->x,y,16,blast,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
			y+=16, i++;
		} while (i < 5);
		return;
	}
	if (p->y > (stageTemp->sh << 4) - 32) {
		unsigned int y = 84;
		do {
			GrayClipISprite16_XOR_R(p->x,y,16,blast,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
			y-=16, i++;
		} while (i < 5);
		return;
	}
	
	unsigned int x = 144;
	do {
		GrayClipISprite16_XOR_R(x,p->y,16,blast,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
		x-=16, i++;
	} while (i < 5);
}

// set the initial status for each player before going into a game
static inline void setupPlayerInGame(PLAYER* player) {
	player->power = 10,
	player->size = 2,
	
	player->percent = 0,
	player->numJumps = 0,
	player->playerCounter = 0, 
	player->attackMarker = 0,
	
	player->breathing = FALSE, 
	player->running = FALSE, 
	player->taunting = FALSE,
	player->falling = FALSE,
	player->climbing = FALSE,
	player->crouching = FALSE,
	player->hanging = FALSE,
	player->smashAttacking = FALSE, 
	player->specialAttacking = FALSE, 
	player->skyAttacking = FALSE,
	player->beingHeld = FALSE, 
	player->grabbing = FALSE,
	player->invincible = FALSE,
	player->dead = FALSE,
	player->paralyzed = FALSE,
	player->onStage = TRUE,
	player->onHillL = FALSE, 
	player->onHillR = FALSE,
	player->canFire = TRUE,
	player->cloaked = FALSE, 
	player->metal = FALSE, // need to set this for classic mode
	player->direction = RIGHT;
}

// run the custom level/minigame for race to the finish in Story/Classic mode
void raceToTheFinish(unsigned int index) {
	BOOL savePressure = currentProfile.crowdPressure;
	currentProfile.crowdPressure = FALSE;
	
	p1->x = 0, p1->y = 32;
	p1->jumpValue = 0;
	p1->numJumps = 0;
	p1->numKills = 0, 
	p1->numTimesKilled = 0;
	p1->moveSpeed = WALKSPEED;
	p1->xspeed = 0;
	p1->yspeed = 0;
	
	setupPlayerInGame(p1);
	p1->onStage = FALSE;	
	p1->type = HUMAN;
	
	p1->currentItem = NULL;	
	p1->rightCurrent = &characters[p1->characterIndex].frames[STAND_RIGHT];
	p1->leftCurrent = &characters[p1->characterIndex].frames[STAND_LEFT];
	p1->enemy = NULL;
	p1->next = NULL;
	pHead = p1;
	
	numPlayers = 1, numHands = 0, racing = TRUE;
	
	stageTemp = &rtfstages[index];
	bgPlane = (Plane){(char*)extraptr->raceToFinishBG,11,(short*)extraptr->raceToTheFinishSprites,NULL,0,0,1};
	bgPlane.big_vscreen = block+LCD_SIZE+LCD_SIZE;
	stageTemp->fgPlane.big_vscreen = block+LCD_SIZE+LCD_SIZE+GRAY_BIG_VSCREEN_SIZE;

	x_fg = 0, y_fg = (index<<5)+64, x_bg = 4, y_bg = 4; // taking away conditional saves 4 bytes
	
	gameMatchType = TIMED;
	timer->millis = 0;
	timer->seconds = (currentProfile.difficulty == CLASSIC) ? RACE_SECS_CLASSIC : (currentProfile.difficulty == ADMIRAL ? RACE_SECS_ADMIRAL : (currentProfile.difficulty == PREMIERE ? RACE_SECS_PREMIERE : RACE_SECS_ELITE));
	timer->minutes = 0;
	timer->running = TRUE;

	SetIntVec(AUTO_INT_5, timer_int);

	do {
		if (!scrollL && !scrollR && !scrollD && !scrollU && !p1->onStage && !p1->dead) { // camera adjustments
			camera = TRUE;
		  	if (p1->x < 16 && x_fg > 0) {
				scrollL = TRUE, p1->x+=2;
			}
			if (p1->x+characters[p1->characterIndex].w > 144 && x_fg < stageTemp->sw*16-160) {
				scrollR = TRUE, p1->x-=2;
			}
			if (p1->y < 16 && y_fg > 0) {
				scrollU = TRUE, p1->y+=2;
			}
			if (p1->y+characters[p1->characterIndex].h > 84 && y_fg < stageTemp->sh*16-100) {
				scrollD = TRUE, p1->y-=2;
		  	}

			renderMaps(virtual);		
			handlePlayer(p1);
		
			if (checkForDeathEvent(p1)) {
				break;
			}
		
			if (!timer->running) {
				complete = FALSE;
				points[RACE_POINTS] += 0;
				break;
			}
		
			if (complete) { // draw the complete sign - only for Race to the Finish! (classic mode only)
				GraySprite32_SMASK_R(0,34,32,extraptr->completesign,extraptr->completesign+32,extraptr->completemask,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
				GraySprite32_SMASK_R(32,34,32,extraptr->completesign+64,extraptr->completesign+96,extraptr->completemask+32,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
				GraySprite32_SMASK_R(64,34,32,extraptr->completesign+128,extraptr->completesign+160,extraptr->completemask+64,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
				GraySprite32_SMASK_R(96,34,32,extraptr->completesign+192,extraptr->completesign+224,extraptr->completemask+96,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
				GraySprite32_SMASK_R(128,34,32,extraptr->completesign+256,extraptr->completesign+288,extraptr->completemask+128,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
			
				points[RACE_POINTS] += (unsigned long)(((unsigned int)p1->x+(unsigned int)x_fg)*((unsigned int)p1->x+(unsigned int)x_fg)+((unsigned int)p1->y-64)*((unsigned int)p1->y-64)+(random(2)*5000));

				while (_rowread(0)); // wait for single keypress
				while (!_rowread(0));
				break;
			}
	
			if (_keytest(RR_PLUS)) { // allow user to adjust the contrast if needed
				OSContrastUp();
			}
			if (_keytest(RR_MINUS)) {
				OSContrastDn();
			}
		
			pokeIO(0x600005,0x17); // allow ON-button pause interrupts
		}
	} while (TRUE);
	
	mycounter = 0;
	bgPlane.force_update++;
	stageTemp->fgPlane.force_update++;
	freeItemList(&head); // delete all existing items
	racing = FALSE;
	currentProfile.crowdPressure = savePressure;
}

// End of Source File