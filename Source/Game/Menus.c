// Twilight Legion for TI-89(Titanium), TI-92(+), Voyage 200
// C Source File - Menus.c
// Michael Hergenrader
// Compiled with TIGCC 0.96 Beta 8
// Please see README for license/disclaimer information. In short, please feel free to use code you see here,
// and any credit to me would be greatly appreciated! :-)
// Copyright 2005-2010 Michael Hergenrader

// This file handles all in-game menus, including automatic cursors, vertical cursor logic,
// menu hierachy, and custom move cursors like settings menu, select stage menu, and character select menu.
// This also includes logic for tournament and invitationals.

#include <tigcclib.h>
#include "headers.h"

// General variables
unsigned int savePlayers = 0;
unsigned long* charsSelected[4] = { NULL, NULL, NULL, NULL };
unsigned int charHeights[4];

// General menu variables
static MOVING_CURSOR* mc; // main menus cursor
static CURSOR* c; // stage select cursor

// Custom menu variables
static unsigned int col, row; // stage select menu
BOOL drawStats = FALSE; // character select menu

// Link play
char settingsHostPress; // settings menu in linked mode
char teamsHostPress;
char stagesHostPress;

// story mode variables
unsigned long totalPoints; // story mode
unsigned int storyModeStages[NUM_STORY_STAGES];
static const unsigned int storyStageNumPlayers[7] = { 2, 4, 2, 3, 2, 3, 2 };

static unsigned int option = 1, cursor = 1; // episode mode

// Tournament mode variables
unsigned int numBattles = 0; // tournament mode
unsigned int fighter1, fighter2;
BOOL playerLost, playerWon;
Plane bracketPlane;
static const unsigned int maxRoundBattles[4] = { 8, 4, 2, 1 };

// Profile Variables
int numProfiles = 0;


/////////////////////////////////////////////////////////////////   Menu Text   /////////////////////////////////////////////////////////////////////////

static const unsigned char* stageNamesText[26] = { // UP TO 20 CHARS APIECE
	"ANTAIRA VILLAGE", // keep this until tested on Ti-89T
	"BATTLEFIELD",
	"BIG BLUE",
	"BRINSTAR",
	"CAIDRUS CATHEDRAL", //5
	"CORE MONUMENT",
	"CORNERIA",
	"CRASPHONE CITY",
	"DREAMLAND",
	"ERA TOWER", //10
	"FINAL DESTINATION",
	"FOURSIDE",
	"FLATZONE",
	"GLASS SUBMARINE",
	"GREAT BAY", //15
	"HETHOR FOREST",
	"HYRULE",
	"INFINITE GLACIER",
	"JUNGLE JAPES",
	"LUNOR STRONGHOLD", //20
	"MARKED MOSQUE",
	"REATEN BASE",
	"RED SKY BAY",
	"SOUL TOWER",
	"TERRA ROCK", //25
	"YOSHIS ISLAND",
};

static const unsigned char* settingsText[] = {
	"Crowd Pressure",
	"Match Type",
	"Match Length",
	"Life Count",
	"Difficulty",
	"Item Caliber", // how often items appear
	"Stage Select",
};

static const unsigned char* difficultyText[] = { // difficulty settings
	"Classic",
	"Admiral",
	"Premiere",
	"Elite",
};

static const unsigned char* onOrOffText[] = { // matches boolean values
	"Off",
	"On",
};

static const unsigned char* stageSelectText[] = { // how stages are chosen in tournament mode
	"One Random",
	"All Random",
	"One Select",
	"All Select",
};

static const unsigned char* invitationalsText[NUM_INVITATIONALS] = {
	"Hyrule",
	"Papaya Bay",
	"Fusion Star",
	"Manhattan",
	"Titan",
	"Hero\'s Mountain",
	"Inferno Region",
	"Cosmo Galaxy",
	"Mango Island",
	"Orange",
};

// Menu sprites
static const unsigned short handcursor[36] = {
	0xC000,0xA000,0x9700,0x4A80,0x2280,0x1540,0x08C0,0x6860,0x5810,0x2030,0x1F60,0x00A0, // light
	0xC000,0xA000,0xB700,0x5D80,0x2D80,0x10C0,0x1840,0x7820,0x7C30,0x3060,0x1F40,0x00C0, // dark
	0xFFFF,0xBFFF,0x9FFF,0xC8FF,0xE0FF,0xF07F,0xF87F,0xF83F,0xD81F,0xE03F,0xFF7F,0xFFFF  // mask 
};

static const unsigned short fist[24] = {
	0x03FC,0x0C02,0x1072,0x207D,0x2171,0x20FE,0x1804,0x07F8,
	0x03FC,0x0C02,0x1062,0x205D,0x2021,0x201E,0x1804,0x07F8,
	0xFFFF,0xFC03,0xF073,0xE07D,0xE171,0xE0FF,0xF807,0xFFFF,
};

static const unsigned char arrow[12] = {
	0x04,0x06,0xFF,0xFF,0x06,0x04,
	0x00,0x04,0xFE,0xFF,0x06,0x04,
};

static const unsigned char menuarrows[][16] = {
	{0x00,0x7E,0x3C,0x18,0x00,0x00,0x00,0x00,0x00,0x7C,0x38,0x10,0x00,0x00,0x00,0x00}, // down arrow
	{0x00,0x00,0x00,0x00,0x18,0x3C,0x7E,0x00,0x00,0x00,0x00,0x00,0x10,0x38,0x7C,0x00}, // up arrow
};

static const unsigned char episodeMarker[2][8] = { // if an episode is unlocked or not - draw TRANB (GraySprite8)
	{0x9F,0x0F,0x0F,0x9F,0x9F,0x0F,0x0F,0x9F},
	{0xFF,0xFF,0xFF,0xFF,0x9F,0x0F,0x0F,0x9F},
};

static const unsigned short characterSelectBox[16] = { // character select cursor box
	0xFFFF,0x8001,0x8001,0x8001,0x8001,0x8001,0x8001,0x8001,0x8001,0x8001,0x8001,0x8001,0x8001,0x8001,0x8001,0xFFFF
};

static const unsigned short lockedsprite[32] = { // ? block - if level not unlocked yet, draw this (not interlaced)
	0xFFFF,0x8FF1,0x9C19,0xB80D,0xF9CF,0xFF8F,0xFF1F,0xFE3F,0xFE7F,0xFE7F,0xFE7F,0xFFFF,0xBE7D,0x9E79,0x8FF1,0xFFFF,
	0xFFFF,0xD00B,0xA3E5,0xC7F3,0x8631,0x8071,0x80E1,0x81C1,0x8181,0x8181,0x8181,0x8001,0xC183,0xA185,0xD00B,0xFFFF
};

// Menu method prototypes
static inline void waitForKeyReleased(void);
static void drawMainMenu(void);
static void drawScreen(const unsigned char *scr0);
static void copyScreens(void);
static void drawCustomFontString(unsigned int x, unsigned int y, unsigned char* str); // for Oncial fonts and such

// Stage Select Menu
static BOOL stageSelectMenu(void);
static inline void drawStageMenu(void);

// Character Select Menu
static BOOL characterSelectMenu(void);
static void handleLinkportKeypressCharMenu(void);
static void drawCharSelectScreen(int c, int r);

// Team Select Screen/Menu
static void teamSelectScreen(void);
static void drawTeamSelectScreen(int co);

// Arena Mode Menu - 1 or 2 players?
static void howManyUsersMenu(void);
static void drawAskUsersMenu(void);

// Story Mode Functions
static void storyMode(void);
static void generateStoryModeList(void);
static void doPointsStuff(void);
static BOOL askToContinue(void);

// Episode Mode - Menu, other functions
static void episodeMenu(void);
static void drawEpisodeMenu(void);
static inline void drawStrings(void);
static void evaluationScreen(void);

// Tournament Menus and Overall Mode
static void tournamentMenu(void);
static void drawTournamentMenu(void);
static void drawRoundScreen(void);
static BOOL prepScreen(void);
static void renderBracket(void);
static BOOL invitationalMenu(void);
static void drawInvitationalMenu(unsigned int choice);
static unsigned int round0LayoutReturn(void);
static unsigned int round1LayoutReturn(void);
static unsigned int round2LayoutReturn(void);
static unsigned int (*roundReturnMethods[3])(void) = {
	round0LayoutReturn,
	round1LayoutReturn,
	round2LayoutReturn
};

// Options Menu
static void optionsMenu(void);
static void drawOptionsMenu(void);
static void drawOptionsCursor(int yc);

// Settings Menu
static BOOL settingsMenu(void);
static void drawSettingsMenu(int choice);

// Credits Screen
static void creditsScroller(void);

// Profile Functions - Creating, Loading, Drawing
static void createProfileScreen(void);
static void drawCreateProfileScreen(char* buffer, short width, PROFILE_ENTRY whatToEnter);
static void drawLoadProfileScreen(void);
static void loadProfileScreen(void);
static void profileScreen(void);

// Link Play Menu Functions
static char establishConnection(void);


static inline void waitForKeyReleased(void) {
	while (_rowread(0)); // don't need more than one test - this covers the whole keyboard! (no separate test for esc)
}

// Main menu handling loop
void mainMenu(void) {
	waitForKeyReleased();
	
	mode = NONE_SELECTED;
	mc = &(MOVING_CURSOR){16,1};
	int choice = 0;
	
	do {
		drawMainMenu();
		drawOptionsCursor(choice);
					
		if (_keytest(RR_DOWN)) {// move the cursor down (calculate)
			if (++choice > 4) {
				choice = 0;
			}
			waitForKeyReleased();
			continue;
		}
		if (_keytest(RR_UP)) { // move the cursor up (calculate)
			if (--choice < 0) {
				choice = 4;
			}
			waitForKeyReleased();
			continue;
		}
		if (_keytest(RR_2ND)) { // select the current option
			// Tests what position the current cursor is pointing to and acts through the array			
			switch (choice) {
				case ARENA_MODE: // Arena Mode
				mode = ARENA_MODE;
				numHands = 0;				
				howManyUsersMenu(); // determine whether single player or linked battles - redirect to that menu
				break;
				
				case STORY_MODE: // Story/Classic Mode
				mode = STORY_MODE;
				if (!characterSelectMenu()) {// if exit from character select menu, then don't bother with the mode, just stay in control in this menu
					break;
				}				
				storyMode();
				break;
				
				case EPISODE_MODE: // Episodes mode
				mode = EPISODE_MODE;				
				episodeMenu();		
				break;

				case TOURNAMENT_MODE: // Tournament mode
				mode = TOURNAMENT_MODE;
				tournamentMenu();
				break;
				
				case 4: // Options menu
				optionsMenu();
				break;
			}

			waitForKeyReleased();
		}
			
		if (_keytest(RR_ESC)) { // exit the program
			ER_success();
			exit(0);
		}
		
		pokeIO(0x600005,0x17); // allow ON button pausing
	} while (TRUE);
}

// Draws everything for the main menu
static void drawMainMenu(void) {
	drawScreen(skylight); // this method is the main drawer method of the cursor in v2 and v3
	drawCustomFontString(20,6,(unsigned char*)"TWILIGHT LEGION");

	drawCustomFontString(36,30,(unsigned char*)"ARENA");
	drawCustomFontString(36,40,(unsigned char*)"STORY");
	drawCustomFontString(36,50,(unsigned char*)"EPISODES");
	drawCustomFontString(36,60,(unsigned char*)"TOURNAMENT");
	drawCustomFontString(36,70,(unsigned char*)"OPTIONS");
}

// Draws a specified splashscreen on the background
static void drawScreen(const unsigned char *scr0) {
  	unsigned int loop = 0;
  	unsigned char *dest1 = v2, *dest2 = v3;
  	const unsigned char *src1 = scr0, *src2 = scr0+2001;
  
	do {
  		memcpy(dest1,src1,20);
		memcpy(dest2,src2,20);
		loop++, src1 += 20, src2 += 20, dest1 += 30, dest2 += 30;
	} while (loop < 100);
}

// Copies the data from the virtual screen to the real screen (used for any virtual screens)
static void copyScreens(void) {
	memcpy(GrayGetPlane(LIGHT_PLANE),v2,LCD_SIZE);
	memcpy(GrayGetPlane(DARK_PLANE),v3,LCD_SIZE);
}

// Custom Font Drawer (sprite strings) for Oncial font
static void drawCustomFontString(unsigned int x, unsigned int y, unsigned char* str) {
	unsigned int pos = 0, len = strlen(str); 
	int offset;
	
	do {  // loop through each character of the string
		if (str[pos] >= 65 && str[pos] <= 90) { // opposite of CAPS lock
			str[pos] += 32;
		}
		offset = -1; // space character
		
		if (str[pos] >= 97 && str[pos] <= 122) { // a letter
			offset = (str[pos] - 97);
		} else if (str[pos] >= 47 && str[pos] <= 57) { // a number
			offset = 25+(str[pos] - 47);
		}
		
		if (offset > -1) {
			GrayClipISprite8_TRANW_R(x,y,8,DEREF_SMALL(extraptr->oncialFont,offset),v2,v3);
		}

		x += 8,	pos++;
	} while (pos < len);	
}


// Stage Select Menu

static BOOL stageSelectMenu(void) { // hand-cursor-moving-type menu
	waitForKeyReleased();
	c = &(CURSOR){20,20};
		
	do {
		drawStageMenu();
		col = ((c->x-24)>>4);
		row = ((c->y>80)?3:(c->y-10)>>4);
		
		if (_keytest(RR_RIGHT) && c->x < 148) {
			c->x++;
		}
		if (_keytest(RR_LEFT) && c->x > 0) {
			c->x--;
		}
		if (_keytest(RR_UP) && c->y > 0) {
			c->y--;
		}
		if (_keytest(RR_DOWN) && c->y < 88) {
			c->y++;
		}
		
		if (_keytest(RR_2ND) && ((c->x > 24 && c->x < 136 && c->y > 10 && c->y < 58) || (c->x > 40 && c->x < 120 && c->y > 80 && c->y < 96))) {
			stageIndex = (char)((row<3)?((row*7)+col):col+20);
			if (linked) {
				stagesHostPress = STAGES_COMPLETE;
			}
			return TRUE;
		}
		if (_keytest(RR_DIAMOND)) {
			stageIndex = (char)random(NUM_STAGES);
			if (linked) {
				stagesHostPress = STAGES_COMPLETE;
			}
			return TRUE;
		}
		if (_keytest(RR_ESC) && mode != TOURNAMENT_MODE) { // make sure user cannot go back in a tournament - have to pick
			if (linked) {
				stagesHostPress = EXIT;
			}
			return FALSE;
		}
		
		pokeIO(0x600005,0x17);
	} while (TRUE);
}

// render the stage select menu
static inline void drawStageMenu(void) {	
	memset(v2,0x00,LCD_SIZE);
	memset(v3,0x00,LCD_SIZE);
	drawCustomFontString(32,1,(unsigned char*)"SELECT STAGE");
	
	unsigned int i = 0, x = 24, y = 10;
	do {
		GraySprite16_XOR_R(x,y,16,DEREF_SMALL(dataptr->datasprts,i)+16,DEREF_SMALL(dataptr->datasprts,i),v2,v3);
		
		x+=16;
		if (x > 120) {
			y+=16, x = 24;
		}
		i++;
	} while (i < 21);
	
	x = 40, y = 80;
	
	do {
		GraySprite16_XOR_R(x,y,16,DEREF_SMALL(dataptr->datasprts,i)+16,DEREF_SMALL(dataptr->datasprts,i),v2,v3);
		x+=16,i++;
	} while (i < 26);
	
	GrayClipSprite16_SMASK_R(c->x,c->y,12,handcursor,handcursor+12,handcursor+24,v2,v3);

	if (c->y < 58 && c->x < 136 && c->x > 24 && c->y > 10) {
		drawCustomFontString(HCENTER(stageNamesText[((row*7)+col)],8),64,(unsigned char*)stageNamesText[((row*7)+col)]);
	} else if (c->x < 120 && c->x > 40 && c->y > 80 && c->y < 96) {
		drawCustomFontString(HCENTER(stageNamesText[col+20],8),64,(unsigned char*)stageNamesText[col+20]);
	}

	copyScreens();
}


// Character Select Screen

char hostSelected = 0, joinSelected = 0;
char p3Index = -1, p4Index = -1;

BOOL confirmScreen = FALSE;
BOOL characterSelected = FALSE;

static BOOL characterSelectMenu(void) {
	waitForKeyReleased();
	static int c = 1, r = 0;	
	char linkJoinInfo[4], receiveValues[2];
	
	hostSelected = 0, joinSelected = 0; // if either of these are -1, then the other exits
	p3Index = -1, p4Index = -1;
	
	characterSelected = FALSE;
	confirmScreen = FALSE;
	
	do {		
		drawCharSelectScreen(c,r);
		
		if (!linked && characterSelected) {
			WaitForMillis(2000);
			return TRUE;
		}
		
		if (!confirmScreen) {
			if (_keytest(RR_RIGHT)) {
				if (++c > 8) {
					c = 1;
				}
				continue;
			}
			if (_keytest(RR_LEFT)) {
				if (--c < 1) {
					c = 8;
				}
				continue;
			}
			if (_keytest(RR_DOWN)) {
				if (++r > 2) {
					r = 0;
				}
				continue;
			}
			if (_keytest(RR_UP)) {
				if (--r < 0) {
					r = 2;
				}
				continue;	
			}	
			if (_keytest(RR_2ND)) { // need to add 1-player stuff here and return TRUE, so other modes besides linked can work
				if (!linked || (linked && calc == HOST_CALC)) {
					p1->characterIndex = (r<<3)+c-1;
				} else if (linked && calc == JOIN_CALC) {
					p2->characterIndex = (r<<3)+c-1;
				}			
				characterSelected = TRUE;
				
				if (linked) {
					handleLinkportKeypressCharMenu();
				}	
				waitForKeyReleased();
			}			
			if (_keytest(RR_DIAMOND)) { // select random character
				if (!linked || (linked && calc == HOST_CALC)) {
					p1->characterIndex = random(NUM_CHARS);
				} else if (linked && calc == JOIN_CALC) {
					p2->characterIndex = random(NUM_CHARS);
				}
				characterSelected = TRUE;
				
				if (linked) {
					handleLinkportKeypressCharMenu();
				}				
				waitForKeyReleased();
			}
			
			// toggle stats screen for each character
			if (_keytest(RR_F1) && mode == STORY_MODE && currentProfile.classicCharacterDifficultiesDone[(r << 3) + c - 1] < 11 && currentProfile.classicCharacterHighScores[(r << 3) + c - 1] > 0) {
				drawStats = TRUE;
			} else {
				drawStats = FALSE;
			}
			
			if (_keytest(RR_PLUS) && mode == ARENA_MODE && !linked) {
				waitForKeyReleased();
				if (++numPlayers > 4) {
					numPlayers = 2;
				}
				continue;
			}
			if (_keytest(RR_MINUS) && mode == ARENA_MODE && !linked) {
				waitForKeyReleased();
				if (--numPlayers < 2) {
					numPlayers = 4;
				}
				continue;
			}			
			if (_keytest(RR_ESC)) {
				if (linked) {
					if (calc == HOST_CALC) {
						hostSelected = -1; // notifies other calc through a negative value that this one is exiting
						
						char hostLinkInfo[4] = { hostSelected, 0, -1, -1 };
						OSWriteLinkBlock((char*)&hostLinkInfo,4);
					} else if (calc == JOIN_CALC) {
						joinSelected = -1;
						
						char sendValues[2] = { joinSelected, 0 };
						OSWriteLinkBlock((char*)&sendValues,2);
					}
				}
				return FALSE;
			}			
		} else { // linked play
			if (calc == HOST_CALC) {
				if (_keytest(RR_2ND)) { // send the true to other calc and return TRUE
					char send = 1;
					OSWriteLinkBlock(&send,1);
					return TRUE;
				}
				if (_keytest(RR_ESC)) { // turn off the confirm screen here and return to char select menu
					char send = -1;
					OSWriteLinkBlock(&send,1);
					hostSelected = 0, joinSelected = 0, confirmScreen = FALSE;
					waitForKeyReleased();
				}
			} else if (calc == JOIN_CALC) { // listen for host actions
				char receiveValue;
				if (OSReadLinkBlock(&receiveValue,1) == 1) {
					if (receiveValue > 0) {
						return TRUE;
					} else if (receiveValue < 0) {
						hostSelected = 0, joinSelected = 0, confirmScreen = FALSE;
					}
				}
			}
		}
				
		if (linked) {
			if (calc == JOIN_CALC) {
				if (OSReadLinkBlock((char*)&linkJoinInfo,4) == 4) {
					if (linkJoinInfo[0] < 0) {
						return FALSE;
					}
					
					hostSelected = 1;
					p1->characterIndex = linkJoinInfo[1];
					
					if (linkJoinInfo[2] > -1) {
						p3->characterIndex = linkJoinInfo[2];
					}
					if (linkJoinInfo[3] > -1) {
						p4->characterIndex = linkJoinInfo[3];
					}
				}
			} else if (calc == HOST_CALC) {
				if (OSReadLinkBlock((char*)&receiveValues,2) == 2) {
					if (receiveValues[0] < 0) {
						return FALSE;
					}
					
					joinSelected = 1;
					p2->characterIndex = receiveValues[1];
				}
			}
		}
			
		if (!linked && characterSelected) { // select the AI characters
			if (numPlayers > 1) {
				p2->characterIndex = random(NUM_CHARS), charsSelected[1] = characters[p2->characterIndex].frames[TAUNT1].data, charHeights[1] = characters[p2->characterIndex].h;
			}
			if (numPlayers > 2) {
				p3->characterIndex = random(NUM_CHARS),charsSelected[2] = characters[p3->characterIndex].frames[TAUNT1].data,charHeights[2] = characters[p3->characterIndex].h;
			}
			if (numPlayers > 3) {
				p4->characterIndex = random(NUM_CHARS),charsSelected[3] = characters[p4->characterIndex].frames[TAUNT1].data,charHeights[3] = characters[p4->characterIndex].h;
			}
		}
		
		if (!linked || (linked && hostSelected > 0)) {
	  		charsSelected[0] = characters[p1->characterIndex].frames[TAUNT1].data; // set up Player 1
			charHeights[0] = characters[p1->characterIndex].h;
	 	}
	 	if (linked && joinSelected > 0) {
	 		charsSelected[1] = characters[p2->characterIndex].frames[TAUNT1].data; // set up Player 2
			charHeights[1] = characters[p2->characterIndex].h;
	 	}
	 	if (linked && hostSelected > 0 && numPlayers > 2) {
	 		charsSelected[2] = characters[p3->characterIndex].frames[TAUNT1].data; // set up Player 3
			charHeights[2] = characters[p3->characterIndex].h;
	 	}
	 	if (linked && hostSelected > 0 && numPlayers > 3) {
	 		charsSelected[3] = characters[p4->characterIndex].frames[TAUNT1].data; // set up Player 4
			charHeights[3] = characters[p4->characterIndex].h;
	 	}
		
		if (linked && hostSelected > 0 && joinSelected > 0) {// both characters are now done, so set up the confirmation screen
			confirmScreen = TRUE;
		}
		
		pokeIO(0x600005,0x17); // allow ON keypress to pause
	} while (TRUE);	
}

// work on the character select menu for linked play
static void handleLinkportKeypressCharMenu(void) {
	if (calc == HOST_CALC) {
		hostSelected = 1;
		
		if (numPlayers > 2) {
			p3->characterIndex = p3Index = random(NUM_CHARS);
		}
		if (numPlayers > 3) {
			p4->characterIndex = p4Index = random(NUM_CHARS);
		}
		
		char hostLinkInfo[4] = { hostSelected, p1->characterIndex, p3Index, p4Index };
		OSWriteLinkBlock((char*)&hostLinkInfo,4);
	} else if (calc == JOIN_CALC) {
		joinSelected = 1;
		
		char sendValues[2] = { joinSelected, p2->characterIndex };
		OSWriteLinkBlock((char*)&sendValues,2);
	}
}

// render the custom character select screen
static void drawCharSelectScreen(int c, int r) {
	memset(v2,0x00,LCD_SIZE);
	memset(v3,0x00,LCD_SIZE);

	unsigned int loop = 0;
  	unsigned char *dest1 = v2, *dest2 = v3;
  	const unsigned char *src1 = extraptr->charsdarksplash, *src2 = extraptr->charslightsplash;
  
  	do { // draw title section
		memcpy(dest1,src1,20);
		memcpy(dest2,src2,20);
		loop++, src1 += 20, src2 += 20, dest1 += 30, dest2 += 30;
	} while (loop < 52);

	if (!drawStats)	{
		unsigned int i = 0, x = 16, y = 52;
		do {
			GrayClipISprite16_XOR_R(x,y,16,DEREF_SMALL(extraptr->tournamentTiles,i),v2,v3);
			x+=16;
			if (x > 129)
				y+=16, x=16;
			i++;						
		} while (i < 24);
		
		GraySprite16_XOR_R(0,52,48,extraptr->banner,extraptr->banner+48,v2,v3);
		GraySprite16_XOR_R(144,52,48,extraptr->banner,extraptr->banner+48,v2,v3);
		GraySprite16_XOR_R((c<<4),(r<<4)+52,16,characterSelectBox,characterSelectBox,v2,v3);
	} else { // draws the classic high score for players that have completed it, also draws difficulty level
		memset(v2+1560,0x00,1440);
		memset(v3+1560,0x00,1440);
		FontSetSys(F_6x8);
		
		char str[15];

		sprintf(str,"High: %lu",currentProfile.classicCharacterHighScores[(r<<3)+c-1]);
		GrayDrawStr2B(10,60,str,A_NORMAL,v2,v3);
		
		int a = currentProfile.classicCharacterDifficultiesDone[(r<<3)+c-1];
		sprintf(str,"Diff: %s",(a==CLASSIC)?"CLASSIC":(a==ADMIRAL?"ADMIRAL":(a==PREMIERE?"PREMIERE":"ELITE")));
		GrayDrawStr2B(10,70,str,A_NORMAL,v2,v3);
	}
	
	if (charsSelected[0] != NULL) {
		if (!linked || (linked && hostSelected > 0)) {
			GraySprite32_SMASK_R(7,15,charHeights[0],charsSelected[0],charsSelected[0]+charHeights[0],charsSelected[0]+2*charHeights[0],v2,v3);
		}
	}	
	if (mode != EPISODE_MODE) {
		if (charsSelected[1] != NULL) {
			if (!linked || (linked && joinSelected > 0)) {
			 	GraySprite32_SMASK_R(45,15,charHeights[1],charsSelected[1],charsSelected[1]+charHeights[1],charsSelected[1]+2*charHeights[1],v2,v3);
			}
		}
		if (charsSelected[2] != NULL) {
			GraySprite32_SMASK_R(83,15,charHeights[2],charsSelected[2],charsSelected[2]+charHeights[2],charsSelected[2]+2*charHeights[2],v2,v3);
		}
		if (charsSelected[3] != NULL) {
			GraySprite32_SMASK_R(121,15,charHeights[3],charsSelected[3],charsSelected[3]+charHeights[3],charsSelected[3]+2*charHeights[3],v2,v3);
		}
	}
	
	char num[2];
	sprintf(num,"%u",numPlayers);
	GrayDrawStrExt2B(154,2,num,A_XOR,F_4x6,v2,v3);
	
	if (linked && confirmScreen) {
		GrayDrawRect2B(0,44,160,56,COLOR_LIGHTGRAY,RECT_FILLED,v2,v3);
		drawCustomFontString(0+52*(calc==JOIN_CALC),46,(calc==HOST_CALC)?((unsigned char*)"2ND   READY TO FIGHT"):((unsigned char*)"WAITING"));
	}
			
	copyScreens();
}

// Team Select Screen - allows user to select what teams players will be placed on (1-4)
// returns success if everything looks good (user hit 2nd) or failure if user hit esc to change something
// this function is called directly after the character select screen

static void teamSelectScreen(void) {
	waitForKeyReleased();
	unsigned int currentOption = 0;
	drawTeamSelectScreen(currentOption);
	
	do {
		if (_keytest(RR_DOWN)) {
			waitForKeyReleased();
			if (++currentOption > numPlayers-1) {
				currentOption = 0;
			}
			drawTeamSelectScreen(currentOption);
		}
		if (_keytest(RR_UP)) {
			waitForKeyReleased();
			if (--currentOption > 1000) {
				currentOption = numPlayers-1;
			}
			drawTeamSelectScreen(currentOption);
		}
		if (_keytest(RR_LEFT)) { // toggle team to the left
			waitForKeyReleased();
			switch (currentOption) {
				case 0:
				if (--p1->team < 1) {
					p1->team = 4;
				}
				break;
				case 1:
				if (--p2->team < 1) {
					p2->team = 4;
				}
				break;
				case 2:
				if (--p3->team < 1) {
					p3->team = 4;
				}
				break;
				case 3:
				if (--p4->team < 1) {
					p4->team = 4;
				}
			}
			drawTeamSelectScreen(currentOption);
		}
		if (_keytest(RR_RIGHT)) { // toggle team to the right
			waitForKeyReleased();
			switch (currentOption) {
				case 0:
				if (++p1->team > 4) {
					p1->team = 1;
				}
				break;
				case 1:
				if (++p2->team > 4) {
					p2->team = 1;
				}
				break;
				case 2:
				if (++p3->team > 4) {
					p3->team = 1;
				}
				break;
				case 3:
				if (++p4->team > 4) {
					p4->team = 1;
				}
				break;
			}
			drawTeamSelectScreen(currentOption);
		}
		if (_keytest(RR_ESC)) {
			if (linked) {
				teamsHostPress = EXIT;
				return;
			}
			mainMenu();
		}
		if (_keytest(RR_2ND)) { // confirm team stuff - if all have the same, don't exit the loop
			TEAM t = p1->team;			
			if (numPlayers == 3) {
				if (t != p2->team || t != p3->team) {
					break;
				}
			} else if (numPlayers > 3) {
				if (t != p2->team || t != p3->team || t != p4->team) {
					break;
				}
			}
		}
	} while (TRUE);
	
	if (linked) {
		teamsHostPress = TEAMS_COMPLETE;
	}
}

static void drawTeamSelectScreen(int co) {
	memset(v2,0xFF,LCD_SIZE);
	memset(v3,0xFF,LCD_SIZE);		
	char charName[16], teamNumber[2];
	
	GrayDrawStrExt2B(HCENTER("Team Select",8),6,"Team Select",A_XOR,F_8x10,v2,v3);
	FontSetSys(F_4x6);
	
	sprintf(teamNumber,"%d",p1->team);
	sprintf(charName,"%s",characterNames[p1->characterIndex]);
	GrayDrawStr2B(36,30,teamNumber,A_XOR,v2,v3);
	GrayDrawStr2B(48,30,charName,A_XOR,v2,v3);
	
	sprintf(teamNumber,"%d",p2->team);
	sprintf(charName,"%s",characterNames[p2->characterIndex]);
	GrayDrawStr2B(36,40,teamNumber,A_XOR,v2,v3);
	GrayDrawStr2B(48,40,charName,A_XOR,v2,v3);
	
	sprintf(teamNumber,"%d",p3->team);
	sprintf(charName,"%s",characterNames[p3->characterIndex]);
	GrayDrawStr2B(36,50,teamNumber,A_XOR,v2,v3);
	GrayDrawStr2B(48,50,charName,A_XOR,v2,v3);
	
	if (numPlayers > 3) {
		sprintf(teamNumber,"%d",p4->team);
		sprintf(charName,"%s",characterNames[p4->characterIndex]);
		GrayDrawStr2B(36,60,teamNumber,A_XOR,v2,v3);
		GrayDrawStr2B(48,60,charName,A_XOR,v2,v3);
	}
	
	GraySprite8_XOR_R(26,co*10+30,6,arrow,arrow+6,v2,v3); // draw cursor
	copyScreens();
}


// Linkup Select Menu

static void howManyUsersMenu(void) {
	waitForKeyReleased();
	unsigned int choice = 1;
	mc = &(MOVING_CURSOR){16,1};
	
	do {
		drawAskUsersMenu();
		drawOptionsCursor(choice-1);
		if (_keytest(RR_DOWN)) {
			waitForKeyReleased();
			if (++choice > 2) {
				choice = 1;
			}
			continue;
		}
		if (_keytest(RR_UP)) {
			waitForKeyReleased();
			if (--choice < 1) {
				choice = 2;
			}
			continue;
		}
		if (_keytest(RR_2ND)) {		
			if (choice == 2) {
				switch (establishConnection()) { // in this method, on escape, return to this method
					case EXIT: case NO_SIGNAL:
					OSLinkClose();
					linked = FALSE;
					break;
					
					case SUCCESSFUL: // if reaches here, then should all be good (failure = comes back to howManyUsersMenu)
					doGame();					
					OSLinkClose();
					linked = FALSE;
					break;
				}
			} else {
				if (characterSelectMenu()) {
					if (currentProfile.matchType && numPlayers > 2) { // only for stock matches
						teamSelectScreen();
					}
					if (stageSelectMenu()) {
						doGame();
					}
				}
			}
		}		
		if (_keytest(RR_ESC)) {
			return;
		}
		
		pokeIO(0x600005,0x17);
	} while (TRUE);	
}

static void drawAskUsersMenu(void) {
	drawScreen(skylight);
	drawCustomFontString(40,6,(unsigned char*)"ARENA MODE"); // title

	drawCustomFontString(36,30,(unsigned char*)"SINGLE");
	drawCustomFontString(36,40,(unsigned char*)"LINK PLAY");
}


// Story Mode execution methods
static void storyMode(void) {
	BOOL hasContinued = FALSE;
	MATCHTYPE mTemp = currentProfile.matchType;
	unsigned int randFightCounter = 0;
	
	savePlayers = numPlayers;
	numPlayers = 2; // just makes the character select menu look a little bit better
	
	p1ClassicLives = currentProfile.matchLives;	
	p1->dead = FALSE;
	
	generateStoryModeList(); // create the list of random fights		
	
	do {
		memset(points,0,8*sizeof(unsigned long));
		cont = FALSE;
		
		if (p1->dead) {
			if (!hasContinued && askToContinue()) { // will now only let a player continue once
				p1->currentItem = NULL;
				totalPoints >>= 1; // divide points in half - penalty for losing
				cont = TRUE;
				if (--battleCounter > 999) {
					battleCounter = 0, p2->team = BLACK_TEAM;
				}
				if (--randFightCounter > 999) {
					randFightCounter = 0;
				}
				hasContinued = TRUE;							
			} else {
				totalPoints = 0;
				break;
			}
		}
		
		switch (battleCounter) {
			case 3:
			p2->team = LIGHTGRAY_TEAM;
			
			case 0: case 1: case 4: case 5: case 7: // regular fights (6)
			currentProfile.matchType = STOCK;
			stageIndex = DEREF_SMALL(storyModeStages,randFightCounter);
			numPlayers = DEREF_SMALL(storyStageNumPlayers,randFightCounter);
			doGame();
			randFightCounter++;
			break;
			
			case RACE1:
			currentProfile.matchType = TIMED;
			raceToTheFinish(0);
			complete = FALSE;
			break;
			
			case RACE2:
			currentProfile.matchType = TIMED;
			raceToTheFinish(1);
			complete = FALSE;
			break;
			
			case METAL_CHAR_FIGHT:
			currentProfile.matchType = STOCK;
			stageIndex = BATTLEFIELD, numPlayers = 2, fightMetal = TRUE;
			doGame();
			fightMetal = FALSE;
			break;
			
			case BOSS_FIGHT:
			currentProfile.matchType = STOCK;
			numPlayers = 1, numHands = (currentProfile.difficulty<ADMIRAL)?2:1, stageIndex = FINAL_DESTINATION;
			doGame();
			crazyHand->dead = FALSE, masterHand->dead = FALSE;
			if (!p1->dead) {
				points[CLASSIC_COMPLETE] = 50000;
			}
			break;
		}

		if (p1->percent < 10) {
			points[INVULNERABLE] = 4000;
		}
		
		if (_keytest(RR_ESC)) {
			totalPoints = 0;
			break;
		}		
		if (!cont) {// points screen will still be shown after the player dies to let them know of their total score
			doPointsStuff();
		}
		
		p2->characterIndex = random(24), p3->characterIndex = random(24), p4->characterIndex = random(24);
		
		p1ClassicLives = p1->numLives; // reset number of lives after each round that player survives
		battleCounter++;
		
	} while (battleCounter < 10);
	
	
	if (totalPoints > currentProfile.classicCharacterHighScores[p1->characterIndex]) {// high score!
		currentProfile.classicCharacterHighScores[p1->characterIndex] = totalPoints;
	}
	if (totalPoints > 0 && currentProfile.difficulty < currentProfile.classicCharacterDifficultiesDone[p1->characterIndex]) { // completed a new classic difficulty
		currentProfile.classicCharacterDifficultiesDone[p1->characterIndex] = currentProfile.difficulty;
	}
						
	numHands = 0;
	currentProfile.matchType = mTemp; // restore original settings
	battleCounter = 0;
	
	p1->team = WHITE_TEAM;
	p2->team = LIGHTGRAY_TEAM;
	p3->team = DARKGRAY_TEAM;
	p4->team = BLACK_TEAM;
	
	totalPoints = 0;
	hasContinued = FALSE;
	numPlayers = savePlayers;	
}

// Makes the list of classic stages so there are no duplicates
static void generateStoryModeList(void) {
	int c = 0;	
	do {
		storyModeStages[c] = random(NUM_STAGES);
		if (c > 0) {
			int c2 = 0;
			while (c2 < c) {
				if (storyModeStages[c2] == storyModeStages[c]) { // if duplicate found, start the loop over and do another random number
					storyModeStages[c] = random(NUM_STAGES), c2 = -1;
				}
				c2++;
			}
		}
		c++;
	} while (c < NUM_STORY_STAGES);
}

// record the proper points for each classic/story mode stage
static void doPointsStuff(void) {
	memset(v2,0x00,LCD_SIZE);
	memset(v3,0x00,LCD_SIZE);
	
	drawCustomFontString(36,2,(unsigned char*)"STAGE CLEAR");
	FontSetSys(F_4x6);
	
	static const char* const pointStrings[8] = {
		"Complete", // race to the finish
		"Story Done", // +50000
		"Hand Killer", // +20000 per hand
		"Slayer", // numKills*2000
		"Slayed", // numTimesKilled*800
		"Invincible", // percent < 10 at end of level? +5000
		"Iron Grip", // grab = +1000, not = -500
		"Bonus", // 20000-mycounter
	};
	
	unsigned char str[25]; // could change this later
	unsigned int y = 16, i = 0;
	unsigned long subtotal = 0;
	do {
		if (points[i] > 0) {
			if (i == SLAYED) {
				sprintf(str,"-%lu  %s",DEREF_SMALL(points,SLAYED),DEREF_SMALL(pointStrings,SLAYED));
			} else {
				sprintf(str,"+%lu  %s",DEREF_SMALL(points,i),DEREF_SMALL(pointStrings,i));
			}
			GrayDrawStr2B(20,y,str,A_NORMAL,v2,v3);
			
			if (i == SLAYED) {
				subtotal-=points[i];
			} else {
				subtotal+=points[i];
			}
			y += 7;
		}
		i++;
	} while (i < 8);
	
	totalPoints+=subtotal;

	sprintf(str,"SCORE: %lu,%lu",subtotal,totalPoints);
	GrayDrawStr2B(20,80,str,A_NORMAL,v2,v3);
	
	copyScreens();
	
	waitForKeyReleased();
	while (!_rowread(0));
}

// if player has failed during the story/classic mode, ask if they want to try again
static BOOL askToContinue(void) {
	waitForKeyReleased();
	memset(v2,0x00,LCD_SIZE);
	memset(v3,0x00,LCD_SIZE);
	
	drawScreen(skylight);
	drawCustomFontString(48,10,(unsigned char*)"CONTINUE");
	drawCustomFontString(36,20,(unsigned char*)"2ND FOR YES");
	drawCustomFontString(40,30,(unsigned char*)"ESC FOR NO");
	
	copyScreens();

	while (TRUE) {
		if (_keytest(RR_2ND)) {
			return TRUE;
		}
		if (_keytest(RR_ESC)) {
			return FALSE;
		}
	}
}


// Episode mode custom menu for selecting which episode to start
static void episodeMenu(void) {
	waitForKeyReleased();
	FontSetSys(F_4x6);
	drawEpisodeMenu();
	savePlayers = numPlayers;	
		
	do {
		if (_keytest(RR_DOWN)) {
			if (++option > 31) {
				option = 31;
			}
			if (++cursor > 5) {
				cursor = 5;
			}
			drawEpisodeMenu();
		}
		if (_keytest(RR_UP)) {
			if (--option < 1) {
				option = 1;
			}
			if (--cursor < 1) {
				cursor = 1;
			}
			drawEpisodeMenu();
		}
		if (_keytest(RR_2ND)) { // episode selected, so start it up
			unsigned int e = option-1;
			
			if (extraptr->episodes[e].p1Index > 23) { // user can select their character
				if (!characterSelectMenu()) {
					drawEpisodeMenu();
					continue;
				}
			} else {
				p1->characterIndex = extraptr->episodes[e].p1Index;
			}
			
			doEpisode(e);			
			
			evaluationScreen();
			drawEpisodeMenu(); // need this so the thing will draw again so user doesnt have to hit up/down first
		}
		if (_keytest(RR_ESC)) {
			numPlayers = savePlayers;
			return;
		}
		
		pokeIO(0x600005,0x17);
	} while (TRUE);
}

static void drawEpisodeMenu(void) {
	memset(v2,0xFF,LCD_SIZE);
	memset(v3,0xFF,2400);
	memset(v3+2400,0x00,600);
	
	unsigned int h = 0;
	do {
		GrayFastDrawHLine2B_R(v2,v3,0,159,(h*9)+18,COLOR_LIGHTGRAY);
		h++;
	} while (h < 6);

	GrayFastDrawLine2B_R(v2,v3,0,18,0,63,COLOR_LIGHTGRAY);
	GrayFastDrawLine2B_R(v2,v3,159,18,159,63,COLOR_LIGHTGRAY);
	
	GrayDrawStrExt2B(48,5,"Episodes",A_XOR,F_8x10,v2,v3);
	
	if (option < 31) {
		GraySprite8_XOR_R(4,65,8,menuarrows[0],menuarrows[0]+8,v2,v3);
	}
	if (option > 1) {
		GraySprite8_XOR_R(4,9,8,menuarrows[1],menuarrows[1]+8,v2,v3);
	}
	
	unsigned i = 0; // this will eventually use the profile data to tell which to display
	do {
		GraySprite8_TRANB_R(4,(i*9)+21,4,episodeMarker[random(2)],episodeMarker[random(2)]+4,v2,v3);
		i++;
	} while (i < 5);
	
	drawStrings();
	copyScreens();
}

static inline void drawStrings(void) {
	unsigned int s = option-cursor, y = 20;
	
	do {
		GrayDrawStr2B(12,y,extraptr->episodeTitlesText[s++],A_XOR,v2,v3);
		y += 9;
	} while (s < option-cursor+5);
	
	GrayInvertRect2B(0,(cursor*9)+9,159,(cursor*9)+18,v2,v3);
	GrayDrawStr2B(12,84,extraptr->episodeDescriptions[(option<<1)-2],A_NORMAL,v2,v3); // draws the event descriptions
	GrayDrawStr2B(12,92,extraptr->episodeDescriptions[(option<<1)-1],A_NORMAL,v2,v3);
	GrayDrawStr2B(110,87,characterNames[extraptr->episodes[option-1].p1Index],A_NORMAL,v2,v3);
}

// draw simple screen that shows whether or not an episode was considered completed successfully
static void evaluationScreen(void) {	
	memset(v2,0x00,LCD_SIZE);
	memset(v3,0x00,LCD_SIZE);
	
	drawCustomFontString(52,46,(episodeSuccess?(unsigned char*)"SUCCESS":(unsigned char*)"FAILURE"));
	
	copyScreens();
	
	waitForKeyReleased();
	while (!_rowread(0));
}


// Tournament Mode functions
static void tournamentMenu(void) {
	waitForKeyReleased();
	savePlayers = numPlayers;
	fighter1 = 0, fighter2 = 1; // 0 is the only human player in a tournament
	playerLost = FALSE, playerWon = FALSE;

	if (!currentProfile.iHaveTournament) {
		mc = &(MOVING_CURSOR){16,1};
		int choice = 0;
		
		do {
			drawTournamentMenu();
			drawOptionsCursor(choice);
			
			if (_keytest(RR_DOWN)) { // move the cursor down
				waitForKeyReleased();
				if (++choice > 1) {
					choice = 0;
				}
			}
			if (_keytest(RR_UP)) { // move the cursor up
				waitForKeyReleased();
				if (--choice < 0) {
					choice = 1;
				}
			}
			if (_keytest(RR_2ND)) { // select the current option
				currentProfile.roundNum = 0;
				memmove(&currentProfile.tourBracketLayout,&extraptr->bracketLayout,455); // sets this to a blank bracket
				
				switch (choice) {
					case 0: // new tournament - initialize all fields			
					
					if (!settingsMenu()) {
						break;
					}
					
					numPlayers = 1; // only so that drawing the character select menu will include one character
					if (!characterSelectMenu()) {
						break;
					}
					
					currentProfile.tourBracketLayout[0][2] = currentProfile.characterIndexes[0] = p1->characterIndex;
					unsigned int i = 1, y = 2;
					do {
						currentProfile.tourBracketLayout[y][2] = currentProfile.characterIndexes[i] = (char)random(24);
						y+=2, i++;
					} while (i < 16);
					
					numPlayers = 2;
					bracketPlane.matrix = currentProfile.tourBracketLayout;  
					bracketPlane.width = 13;
					bracketPlane.sprites = extraptr->tournamentTiles;
					bracketPlane.big_vscreen = tournamentBlock;
					bracketPlane.force_update = 1;
					
					renderBracket(); // Main Tournament Loop
					return;
					
					case 1: // invitational menu
					if (!invitationalMenu()) {
						break;
					}
					
					numPlayers = 1;
					if (!characterSelectMenu()) {
						break;
					}
					
					currentProfile.tourBracketLayout[0][2] = currentProfile.characterIndexes[0] = p1->characterIndex;
					
					i = 1, y = 2; // this is only for creating a tournament - not for invitationals
					do {
						currentProfile.tourBracketLayout[y][2] = currentProfile.characterIndexes[i] = (extraptr->invitationalList[currentProfile.invitationalID]).opponents[i - 1];
						y+=2, i++;
					} while (i < 16); // make sure this doesn't go out of bounds
					
					numPlayers = 2;
					bracketPlane.matrix = currentProfile.tourBracketLayout;  
					bracketPlane.width = 13;
					bracketPlane.sprites = extraptr->tournamentTiles;
					bracketPlane.big_vscreen = tournamentBlock;
					bracketPlane.force_update = 1;
					
					renderBracket(); // Main Tournament Loop
					return;
				}
				waitForKeyReleased();
			}
			if (_keytest(RR_ESC)) { // exit back to the main menu
				return;
			}
			
			pokeIO(0x600005,0x17);			
		} while (TRUE);	
	}
	
	numPlayers = 2;
	bracketPlane.matrix = currentProfile.tourBracketLayout;  
	bracketPlane.width = 13;
	bracketPlane.sprites = extraptr->tournamentTiles;
	bracketPlane.big_vscreen = tournamentBlock;
	bracketPlane.force_update = 1;
	
	renderBracket();
}

static void drawTournamentMenu(void) {
	drawScreen(skylight);
	drawCustomFontString(40,6,(unsigned char*)"TOURNAMENT");

	drawCustomFontString(36,30,(unsigned char*)"CREATE");
	drawCustomFontString(36,40,(unsigned char*)"INVITATIONALS");
}

// each round of the tournament
static void drawRoundScreen(void) {
	char str[8];
  	sprintf(str,"ROUND %d",currentProfile.roundNum+1);
	drawScreen(powbufferlight);
  	GrayDrawStrExt2B(52,45,str,A_NORMAL,F_8x10,v2,v3);
  	copyScreens();
  	WaitForMillis(3000);
}

// animations for the bracket for all tournament matches - the main loop
static void renderBracket(void)	{
	unsigned int x_fg = 0, y_fg = 0;
	BOOL scrolling = FALSE;
	BOOL updating = TRUE;
	
  	do {
    	if (!numBattles && updating) {
			drawRoundScreen();
			updating = FALSE;
    	} else {
	    	DrawGrayPlane(x_fg,y_fg,&bracketPlane,v2,v3,TM_GRPLC89,TM_G16B);
  			copyScreens();
					
			if ((scrolling = (y_fg < (currentProfile.roundNum == 0 ? (numBattles << 6) : currentProfile.roundNum == 1 ? (numBattles << 7) + 16 : currentProfile.roundNum == 2 ? (numBattles * 256 + 48) : 112)))) {
				y_fg+=2;
			} else {
				WaitForMillis(2000);
				FadeOutToBlack_LR_R(GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE),100,30,20);
	    		if (!prepScreen()) {
	    			return;
				}
			}
		}
	    
		if (playerWon) {
	    	currentProfile.iHaveTournament = FALSE, numPlayers = savePlayers, currentProfile.invitationalID = NONE;
			
	    	memset(GrayGetPlane(LIGHT_PLANE),0x00,LCD_SIZE);
	    	memset(GrayGetPlane(DARK_PLANE),0x00,LCD_SIZE);
	    	GrayDrawStrExt2B(48,5,"You win!",A_XOR,F_8x10,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));

			waitForKeyReleased();
	    	while (!_rowread(0)); // wait for any keypress
	    	break;
	    }	    
	    if (numBattles == maxRoundBattles[currentProfile.roundNum]) { // reached max num battles - move on to next round
	    	currentProfile.roundNum++, updating = TRUE, numBattles = 0, y_fg = 0;
	    	if (currentProfile.roundNum == 2) {
	    		x_fg = 32;
			} else if (currentProfile.roundNum == 3) {
	    		x_fg = 48, y_fg = 112;
			}
	    }	    
	    if (playerLost) { // tell the player that he or she has lost the tournament
	    	currentProfile.iHaveTournament = FALSE, numPlayers = savePlayers, currentProfile.invitationalID = NONE;
	    	
	    	memset(GrayGetPlane(LIGHT_PLANE),0x00,LCD_SIZE);
	    	memset(GrayGetPlane(DARK_PLANE),0x00,LCD_SIZE);
	    	GrayDrawStrExt2B(48,5,"You lose!",A_XOR,F_8x10,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
	    	
	    	waitForKeyReleased();
	    	while (!_rowread(0));
	    	break;
	    }  
  	} while (TRUE);
}

// preparation screen showing all characters before any battle
static BOOL prepScreen(void) {
	waitForKeyReleased();
	if (currentProfile.roundNum > 0) {
		BOOL p1Found = FALSE;
		unsigned int i = (currentProfile.roundNum==1?(numBattles<<2):(currentProfile.roundNum==2?(numBattles<<3):0));
		do {			
			if (p1Found && currentProfile.characterIndexes[i] > -1) {
				fighter2 = i;
				break;
			}
			if (!p1Found && currentProfile.characterIndexes[i] > -1) {
				fighter1 = i;
				p1Found = TRUE;
			}			
			i++;
		} while (i < 16);		
	}
	
	p1->type = CPU;
	if (!fighter1) {
		p1->type = HUMAN;
	}
	p2->type = CPU; // p2 is always the computer player, no matter the fight
	
	p1->characterIndex = currentProfile.characterIndexes[fighter1];
	p2->characterIndex = currentProfile.characterIndexes[fighter2];
	
	do {
		drawScreen(skylight);
		
		drawCustomFontString(44,6,(unsigned char*)"GET READY");		
		drawCustomFontString(36,24,(unsigned char*)"P1");
		drawCustomFontString(116,24,(unsigned char*)"P2");
		drawCustomFontString(8,90,(unsigned char*)"PRESS 2ND TO START");
		
		GraySprite32_SMASK_R(24,34,characters[p1->characterIndex].h,characters[p1->characterIndex].frames[TAUNT1].data,characters[p1->characterIndex].frames[TAUNT1].data+characters[p1->characterIndex].h,characters[p1->characterIndex].frames[TAUNT1].data+2*characters[p1->characterIndex].h,v2,v3);
		GraySprite32_SMASK_R(104,34,characters[p2->characterIndex].h,characters[p2->characterIndex].frames[TAUNT1].data,characters[p2->characterIndex].frames[TAUNT1].data+characters[p2->characterIndex].h,characters[p2->characterIndex].frames[TAUNT1].data+2*characters[p2->characterIndex].h,v2,v3);
		copyScreens();
		
		if (_keytest(RR_2ND)) // endless here - soon, need to add to it
			{
			// start the game - either watch, or play if one of the indexes (not characterIndex) of tour is 0				
			
			if (currentProfile.tStageSelect == ALL_SELECT) {
				stageSelectMenu(); // don't worry about stageSelect return value here - user cannot quit tournament mode
			} else if (currentProfile.tStageSelect == ONE_SELECT) {
				if (!currentProfile.roundNum && !numBattles && currentProfile.invitationalID < 0) {
					stageSelectMenu();
					currentProfile.savedStage = stageIndex;
				} else {
					stageIndex = currentProfile.savedStage;				
				}
			} else if (currentProfile.tStageSelect == ALL_RANDOM || (currentProfile.tStageSelect == ONE_RANDOM && !currentProfile.roundNum && !numBattles)) {
				stageIndex = (char)random(26);
			}
			
			doGame();
			
			// tests for victory and eliminates the loser player
			if (!fighter1) {
				if (p1->team == winningTeam) {
					if (currentProfile.roundNum > 2) {
						currentProfile.numTournamentWins++, playerWon = TRUE;
						break;
					}
					currentProfile.numTournamentWins++;
					currentProfile.tourBracketLayout[roundReturnMethods[currentProfile.roundNum]()][currentProfile.roundNum*3+5] = (char)currentProfile.characterIndexes[fighter1];
					currentProfile.characterIndexes[fighter2] = -1; // eliminate player from tourny (p2)
				} else {// p1 lost the game (as a human), so exit the tournament
					currentProfile.numTournamentLosses++, playerLost = TRUE;
				}
    			break;
			} else {
				if (p1->team == winningTeam) {
					currentProfile.characterIndexes[fighter2] = -1, currentProfile.tourBracketLayout[roundReturnMethods[currentProfile.roundNum]()][currentProfile.roundNum*3+5] = (char)currentProfile.characterIndexes[fighter1];
				} else {// p2 won
					currentProfile.characterIndexes[fighter1] = -1, currentProfile.tourBracketLayout[roundReturnMethods[currentProfile.roundNum]()][currentProfile.roundNum*3+5] = (char)currentProfile.characterIndexes[fighter2];
				}
			}			
			numBattles++;
			
			if (!currentProfile.roundNum) {
				fighter1+=2;
				fighter2+=2; // increment to next battle
			}					
			break;
		}
		if (_keytest(RR_ESC)) // exit the tournament/skip a battle
			{
			// if not player1 (human) in this fight, ignore
			if (!fighter1) {
				currentProfile.iHaveTournament = TRUE;
				return FALSE;
			} else {
				// simulates the game between two cpus
				winningTeam = random(2);
				if (winningTeam) {
					currentProfile.characterIndexes[fighter2] = -1, currentProfile.tourBracketLayout[roundReturnMethods[currentProfile.roundNum]()][currentProfile.roundNum*3+5] = (char)currentProfile.characterIndexes[fighter1];
				} else {
					currentProfile.characterIndexes[fighter1] = -1, currentProfile.tourBracketLayout[roundReturnMethods[currentProfile.roundNum]()][currentProfile.roundNum*3+5] = (char)currentProfile.characterIndexes[fighter2];
				}
				numBattles++;
				
				if (!currentProfile.roundNum) {
					fighter1+=2, fighter2+=2; // increment to next battle
				}
			}
			break;
		}
	} while (TRUE);
	
	return TRUE;
}

// Bracket drawing methods
static unsigned int round0LayoutReturn(void) {
	return (numBattles<<2)+1;
}

static unsigned int round1LayoutReturn(void) {
	return (numBattles<<3)+3;
}

static unsigned int round2LayoutReturn(void) {
	return (numBattles<<4)+7;
}

// Invitational mode menu - predefined tournaments
static BOOL invitationalMenu(void) {
	waitForKeyReleased();
	static unsigned int choice = 0;
	FontSetSys(F_4x6);
	
	do {
		drawInvitationalMenu(choice);
		if (_keytest(RR_DOWN) && choice < 9) { // move the cursor down (doesn't wrap here)
			waitForKeyReleased();
			choice++;
			continue;
		}
		if (_keytest(RR_UP) && choice > 0) { // move the cursor up
			waitForKeyReleased();
			choice--;
			continue;
		}
		if (_keytest(RR_2ND)) {
			currentProfile.invitationalID = choice; // user selected that invitational, so let's play that one
			return TRUE;			
		}
		if (_keytest(RR_ESC)) {
			currentProfile.invitationalID = NONE;
			return FALSE;
		}
		pokeIO(0x600005,0x17);
	
	} while (TRUE);
}

// only let user in this menu go up and down, not wrap around like other menus
static void drawInvitationalMenu(unsigned int choice) {
	drawScreen(skylight);
	drawCustomFontString(28,6,(unsigned char*)"INVITATIONALS");
	
	unsigned int i = 0, y = 18;
	do {
		GrayDrawStr2B(20,y,invitationalsText[i++],A_NORMAL,v2,v3);
		y+=8;
	} while (i < 10); // need to test for unlocking here
	
	GraySprite8_TRANW_R(10,(choice<<3)+18,6,arrow,arrow+6,v2,v3);
	copyScreens();
}

// Options Menu - settings, profile summary, credits, etc.
static void optionsMenu(void) {
	waitForKeyReleased();
	mc = &(MOVING_CURSOR){16,1};
	WaitForMillis(500);
	int choice = 0;
	
	do {
		drawOptionsMenu();
		drawOptionsCursor(choice);
		if (_keytest(RR_DOWN)) {
			waitForKeyReleased();
			if (++choice > 2) {
				choice = 0;
			}
		}
		if (_keytest(RR_UP)) {
			waitForKeyReleased();
			if (--choice < 0) {
				choice = 2;
			}
		}
		if (_keytest(RR_2ND)) { // select the current option
			switch (choice) {
				case 0: // settings menu
				mode = NONE_SELECTED;
				settingsMenu();
				break;
				case 1: // profile printout				
				profileScreen();
				break;
				case 2: // credits screen
				creditsScroller();
				break;
			}
			waitForKeyReleased();
		}
		if (_keytest(RR_ESC)) { // exit and break out of this loop!
			break;
		}
		
		pokeIO(0x600005,0x17);
	} while (TRUE);
}

static void drawOptionsMenu(void) {
	drawScreen(skylight);
	drawCustomFontString(52,6,(unsigned char*)"OPTIONS");
	
	drawCustomFontString(40,30,(unsigned char*)"SETTINGS");
	drawCustomFontString(40,40,(unsigned char*)"PROFILES");
	drawCustomFontString(40,50,(unsigned char*)"CREDITS");
}

static void drawOptionsCursor(int yc) {
	if (mc->x < 10 || mc->x > 24) {
		mc->direction = -mc->direction;
	}
	if (mc->direction > 0) {
		mc->x+=2;
	}
	if (mc->direction < 0) {
		mc->x-=2;
	}
	GraySprite16_SMASK_R(mc->x,yc*10+30,8,fist,fist+8,fist+16,v2,v3);
	copyScreens();
}

// Settings Menu
static BOOL settingsMenu(void) { // now called by settings mode and tournament menu when creating a tournament
	waitForKeyReleased();
	int choice = 0;
	char receiveValue = 0;

	FontSetSys(F_4x6);
	
	do {		
		drawSettingsMenu(choice);
		if (_keytest(RR_DOWN)) {
			waitForKeyReleased();
			if (++choice > ((mode==TOURNAMENT_MODE || linked)?6:5)) {
				choice = 0;
			}
			continue;
		}
		if (_keytest(RR_UP)) {
			waitForKeyReleased();
			if (--choice < 0) {
				choice = ((mode==TOURNAMENT_MODE || linked)?6:5);
			}
			continue;
		}
		if (_keytest(RR_LEFT)) {
			waitForKeyReleased();
			switch (choice) {
				case CROWD_PRESSURE_SETT: // crowd pressure
				if (mode==TOURNAMENT_MODE) {
					currentProfile.tCrowdPressure = !currentProfile.tCrowdPressure;
				} else {
					currentProfile.crowdPressure = !currentProfile.crowdPressure;
				}
				break;
				
				case MATCH_TYPE_SETT: // match type
				if (mode==TOURNAMENT_MODE) {
					currentProfile.tType = !currentProfile.tType;
				} else {
					currentProfile.matchType = !currentProfile.matchType;
				}
				break;
				
				case MATCH_MINUTES_SETT: // match length
				if (mode==TOURNAMENT_MODE) {
					if (--currentProfile.tMatchMinutes < 1) {
						currentProfile.tMatchMinutes = 90;
					}
				} else {
					if (--currentProfile.matchMinutes < 1) {
						currentProfile.matchMinutes = 90;
					}
				}
				break;
				
				case MATCH_LIVES_SETT: // life count
				if (mode==TOURNAMENT_MODE) {
					if (--currentProfile.tMatchLives < 1) {
						currentProfile.tMatchLives = 5;
					}
				} else {
					if (--currentProfile.matchLives < 1) {
						currentProfile.matchLives = 5;
					}
				}
				break;
				
				case AI_DIFFICULTY_SETT: // difficulty for AI				
				if (mode==TOURNAMENT_MODE) {
					currentProfile.tDifficulty += AI_DIFFERENCE;
					if (currentProfile.tDifficulty > CLASSIC) {
						currentProfile.tDifficulty = ELITE;
					}
				} else {
					currentProfile.difficulty += AI_DIFFERENCE;
					if (currentProfile.difficulty > CLASSIC) {
						currentProfile.difficulty = ELITE;
					}
				}				
				break;
				
				case ITEM_PROBABILITY_SETT: // item probability
				if (mode==TOURNAMENT_MODE) {
					if (--currentProfile.tItemProb < 1) {
						currentProfile.tItemProb = 5;
					}
				} else {
					if (--currentProfile.itemProb < 1) {
						currentProfile.itemProb = 5;	
					}			
				}
				break;
				
				case STAGE_SELECT_SETT: // stage select - used mainly for Tournament mode
				if (mode == TOURNAMENT_MODE) {
					if (--currentProfile.tStageSelect < 0) {
						currentProfile.tStageSelect = 3;
					}
				} else {
					if (linked) {
						if (--numPlayers < 2) {
							numPlayers = 4;
						}
					}
				}
			}
			continue;
		}
		if (_keytest(RR_RIGHT)) {
			waitForKeyReleased();
			switch (choice) {
				case CROWD_PRESSURE_SETT: // crowd pressure
				if (mode == TOURNAMENT_MODE) {
					currentProfile.tCrowdPressure = !currentProfile.tCrowdPressure;
				} else {
					currentProfile.crowdPressure = !currentProfile.crowdPressure;
				}
				break;
				
				case MATCH_TYPE_SETT: // match type
				if (mode == TOURNAMENT_MODE) {
					currentProfile.tType = !currentProfile.tType;
				} else {
					currentProfile.matchType = !currentProfile.matchType;
				}
				break;
				
				case MATCH_MINUTES_SETT: // match length
				if (mode == TOURNAMENT_MODE) {
					if (++currentProfile.tMatchMinutes > 90) {
						currentProfile.tMatchMinutes = 1;
					}
				} else {
					if (++currentProfile.matchMinutes > 90) {
						currentProfile.matchMinutes = 1;
					}
				}				
				break;
				
				case MATCH_LIVES_SETT: // life count
				if (mode == TOURNAMENT_MODE) {
					if (++currentProfile.tMatchLives > 5) {
						currentProfile.tMatchLives = 1;
					}
				} else {
					if (++currentProfile.matchLives > 5) {
						currentProfile.matchLives = 1;
					}
				}
				break;
				
				case AI_DIFFICULTY_SETT: // difficulty
				if (mode == TOURNAMENT_MODE) {
					currentProfile.tDifficulty -= AI_DIFFERENCE;
					if (currentProfile.tDifficulty < ELITE) {
						currentProfile.tDifficulty = CLASSIC;
					}
				} else {
					currentProfile.difficulty -= AI_DIFFERENCE;
					if (currentProfile.difficulty < ELITE) {
						currentProfile.difficulty = CLASSIC;
					}
				}
				break;				
				
				case ITEM_PROBABILITY_SETT: // item probability
				if (mode == TOURNAMENT_MODE) {
					if (++currentProfile.tItemProb > 5) {
						currentProfile.tItemProb = 1;
					}
				} else {
					if (++currentProfile.itemProb > 5) {
						currentProfile.itemProb = 1;
					}
				}
				break;				
				
				case STAGE_SELECT_SETT: // stage select - tournament mode only				
				if (mode == TOURNAMENT_MODE) {
					if (++currentProfile.tStageSelect > 3) {
						currentProfile.tStageSelect = 0;
					}
				} else {
					if (linked) {
						if (++numPlayers > 4) {
							numPlayers = 2;
						}
					}
				}
			}
			continue;
		}
		if (_keytest(RR_2ND)) {
			if (linked) {
				settingsHostPress = SETTINGS_COMPLETE; // this informs that the host calc has pressed 2nd
			}	
			return TRUE;
		}		
		if (_keytest(RR_ESC)) {
			if (mode == TOURNAMENT_MODE) {
				return FALSE;
			} else if (linked) {
				settingsHostPress = EXIT;
				return FALSE;
			} else {
				return FALSE;
			}
		}
		
		 // test to see if the joining calc has pressed EXIT
		if (linked && calc == HOST_CALC && OSReadLinkBlock(&receiveValue,1) == 1 && receiveValue == EXIT) {
			settingsHostPress = EXIT;
			return FALSE;
		}
		
	} while (TRUE);

}

static void drawSettingsMenu(int choice) {
	char settings[3];
	static const unsigned int X_TEXT = 30, X_VALUE = 100, X_CURSOR = 20;
	drawScreen(skylight);
	
	drawCustomFontString((mode==TOURNAMENT_MODE)?20:48,6,((mode==TOURNAMENT_MODE)?(unsigned char*)"TOURNY SETTINGS":(unsigned char*)"SETTINGS"));
		
	unsigned int i = 0, y = 30; // this optimization saves 82 bytes!
	do {
		GrayDrawStr2B(X_TEXT,y,settingsText[i++],A_NORMAL,v2,v3);
		y+=10;
	} while (i < ((mode==TOURNAMENT_MODE)?7:6));
	
	GrayDrawStr2B(X_VALUE,30,onOrOffText[(mode==TOURNAMENT_MODE)?currentProfile.tCrowdPressure:currentProfile.crowdPressure],A_NORMAL,v2,v3);
	GrayDrawStr2B(X_VALUE,40,matchTypeText[(mode==TOURNAMENT_MODE)?currentProfile.tType:currentProfile.matchType],A_NORMAL,v2,v3);
	sprintf(settings,"%d",(mode==TOURNAMENT_MODE)?currentProfile.tMatchMinutes:currentProfile.matchMinutes);
	GrayDrawStr2B(X_VALUE,50,settings,A_NORMAL,v2,v3);
	sprintf(settings,"%d",(mode==TOURNAMENT_MODE)?currentProfile.tMatchLives:currentProfile.matchLives);
	GrayDrawStr2B(X_VALUE,60,settings,A_NORMAL,v2,v3);	
	if (mode==TOURNAMENT_MODE) {
		GrayDrawStr2B(X_VALUE,70,difficultyText[(currentProfile.tDifficulty==CLASSIC)?0:(currentProfile.tDifficulty==ADMIRAL?1:(currentProfile.tDifficulty==PREMIERE?2:3))],A_NORMAL,v2,v3);	
	} else {
		GrayDrawStr2B(X_VALUE,70,difficultyText[(currentProfile.difficulty==CLASSIC)?0:(currentProfile.difficulty==ADMIRAL?1:(currentProfile.difficulty==PREMIERE?2:3))],A_NORMAL,v2,v3);
	}
	sprintf(settings,"%d",(mode==TOURNAMENT_MODE)?currentProfile.tItemProb:currentProfile.itemProb);
	GrayDrawStr2B(X_VALUE,80,settings,A_NORMAL,v2,v3);
	
	if (mode==TOURNAMENT_MODE) {
		GrayDrawStr2B(X_VALUE,90,stageSelectText[currentProfile.tStageSelect],A_NORMAL,v2,v3);
	}	
	if (linked) {
		GrayDrawStr2B(X_TEXT,90,"Number of Players",A_NORMAL,v2,v3); //"Number of Players"
		sprintf(settings,"%d",numPlayers);
		GrayDrawStr2B(X_VALUE,90,settings,A_NORMAL,v2,v3);
	}
	
	GraySprite8_TRANW_R(X_CURSOR,choice*10+30,6,arrow,arrow+6,v2,v3); // draw cursor
	copyScreens();	
}

// Credits Screen
// Credit to Fisch2 for originally developing this routine
static void creditsScroller(void) {
	unsigned int counter = 0, null = 0, y = 8;
	int delay = 50;	
	
	memset(v2,0xFF,LCD_SIZE);
	memset(v3,0xFF,LCD_SIZE);
	FontSetSys(F_6x8);
	
	do {
		if (++y > 8) {
			y = 0;
			memset(v2+3000,0xFF,270);  // fill bottom 9 rows to cover the unveiling of the text
			memset(v3+3000,0xFF,270);
			if (!null) {
				if (strcmp(extraptr->credits[counter],"exit") == 0) {
					null = 1;
				} else {
					GrayDrawStr2B(HCENTER(extraptr->credits[counter],6),100,(char*)extraptr->credits[counter],A_XOR,v2,v3);
					counter++;
				} 
			} else {
				if (null++ > 10) { // reached the final text, so return
					break;
				}
			}
		}
		if (_keytest(RR_PLUS) && delay > 0) { // speed up or slow down text
			delay -= 50; // these interval increments work pretty well, 50 does better than 100
		}
		if (_keytest(RR_MINUS) && delay < 1500) {
			delay += 50;
		}
		if (delay < 0) {
			delay = 0;
		}
		
		ScrollUp160_R((short*)v2,109);
		ScrollUp160_R((short*)v3,109);
		
		WaitForMillis(delay); // slows down the scrolling here - will have to adjust the numbers for the real TI-89/Titanium
		copyScreens();
	} while (!_keytest(RR_ESC));
}

// Profile Loading Menu (load or create new)
void doProfileLoadingOrCreating(void) {	
	FolderOp(SYMSTR("profiles"),FOP_LOCK);
	unsigned int counter = 0;
	SYM_ENTRY* symptr = SymFindFirst(SYMSTR("profiles"),FO_SINGLE_FOLDER);
	while (symptr) {
		strcpy(profileNames[counter++],symptr->name);
		numProfiles++;
		symptr = SymFindNext();
	}
	FolderOp(SYMSTR("profiles"),FOP_UNLOCK);
	
	if (numProfiles == 0) {
		createProfileScreen();
	} else {
		loadProfileScreen();
	}
}

 // borrowed from Hockey 68k by Fisch2, slightly modified by me
static void createProfileScreen(void) {	
	short key, pos = 0, oldPos, width;
	
	char buffer[MAX_LEN] = "name";
	char buffer2[MAX_LEN2] = "p1";
	
	void* kbq = kbd_queue();
	
	while (buffer[pos]) {
		if (pos++ > MAX_LEN) {
			pos = 0, buffer[0] = 0;
		}
	}
	
	unsigned char temp[MAX_LEN+1], a = 0;
	while (a < MAX_LEN) {
		temp[a++] = 'G', temp[a] = 0;
	}
	
	width = DrawStrWidth(temp,F_6x8)+6;
	
	while (_rowread(0));
	GraySetInt1Handler(save_int_1); // temporarily reinstall the original auto_int_1
	drawCreateProfileScreen(buffer,width,NAME);
	
	// can't use this for compatibility with hardware 1 and such (min AMS = 2.00)
	do {
		oldPos = pos;		
		while (OSdequeue(&key,kbq)) {
			continue; // Get a keypress
		}
		key &= 0xF7FF;
		
		if (pos < MAX_LEN-1 && isprint(key)) {
			buffer[pos++] = key;
		}
		
		if (key == KEY_BACKSPACE && pos) {
			pos--;
		}
		if (key == KEY_CLEAR) {
			pos = 0;
		}
		if (key == KEY_ESC) {
			ER_success();
			exit(0);
		}		
		if (pos != oldPos) {
			buffer[pos] = 0;
			drawCreateProfileScreen(buffer,width,NAME);
		}
	} while (key != KEY_ENTER);
	
	GraySetInt1Handler(DUMMY_HANDLER);
	while (_rowread(0));
	
	if (!strncmp(buffer,"",8) || !isalpha(buffer[0])) { // does not allow any empty strings or crap strings
		ER_success();
		exit(0);
	}
	
	int i = 0; // check if that profile name already exists
	do {
		if (strncmp(buffer,profileNames[i],8) == 0) {
			ER_success();
			exit(0);
		}
		i++;
	} while (i < numProfiles);
	
	// Enter Initials now
	pos = 0;
	while (buffer2[pos]) {
		if (pos++ > MAX_LEN2) {
			pos = 0, buffer2[0] = 0;
		}
	}
	
	unsigned char temp2[MAX_LEN2+1], b = 0;
	while (b < MAX_LEN2) {
		temp2[b++] = 'G', temp2[b] = 0;
	}
	
	width = DrawStrWidth(temp2,F_6x8)+6;
	
	while (_rowread(0));
	GraySetInt1Handler(save_int_1);
	drawCreateProfileScreen(buffer2,width,INITIALS);
	
	do {
		oldPos = pos;
		
		while (OSdequeue(&key,kbq)) {
			continue; // Get a keypress
		}
		key &= 0xF7FF; // Mask out the repeat flag
		
		if (pos < MAX_LEN2 && isprint(key)) {
			buffer2[pos++] = key;
		}
		
		if (key == KEY_BACKSPACE && pos) {
			pos--;
		}
		if (key == KEY_CLEAR) {
			pos = 0;
		}
		if (key == KEY_ESC) {
			ER_success();
			exit(0);
		}
		
		if (pos != oldPos) {
			buffer2[pos] = 0;
			drawCreateProfileScreen(buffer2,width,INITIALS);
		}
	} while (key != KEY_ENTER);
	
	GraySetInt1Handler(DUMMY_HANDLER);
	while (_rowread(0));
	
	if (strncmp(buffer2,"",3) == 0) {
		ER_success();
		exit(0);
	}

	memcpy(&currentProfile.name,buffer,8);
	memcpy(&currentProfile.initials,buffer2,3);
	
	sprintf(fileName,"profiles\\%s",currentProfile.name);
	
	currentProfile.numWins = 0, currentProfile.numLosses = 0;
	currentProfile.numTournamentWins = 0, currentProfile.numTournamentLosses = 0;
	currentProfile.numSuddenDeathWins = 0, currentProfile.numSuddenDeathLosses = 0;
	
	currentProfile.invitationalID = -1;
	
	currentProfile.crowdPressure = currentProfile.tCrowdPressure = OFF;
	currentProfile.matchType = currentProfile.tType = TIMED;
	currentProfile.matchMinutes = currentProfile.tMatchMinutes = 2;
	currentProfile.matchLives = currentProfile.tMatchLives = 5;
	currentProfile.difficulty = currentProfile.tDifficulty = ADMIRAL;
	currentProfile.itemProb = currentProfile.tItemProb = 3;
	currentProfile.tStageSelect = ONE_RANDOM;
	currentProfile.savedStage = ANTAIRA_VILLAGE; // default: 0 = ANTAIRA VILLAGE
	currentProfile.roundNum = 0;
	
	currentProfile.iHaveTournament = FALSE;
	currentProfile.iHaveBattle = FALSE;
	
	memcpy(&currentProfile.tourBracketLayout,&extraptr->bracketLayout,455);
	
	memset(&currentProfile.classicCharacterDifficultiesDone,11,24*sizeof(unsigned int));
	memset(&currentProfile.classicCharacterHighScores,0,24*sizeof(unsigned long));
	
	goAheadAndSave = TRUE;
}

static void drawCreateProfileScreen(char* buffer, short width, PROFILE_ENTRY whatToEnter) {
	drawScreen(skylight);
	DrawGrayRect2B((160-width)/2,41,(160-width)/2+width,55,COLOR_WHITE,RECT_FILLED,v2,v3);
	DrawGrayRect2B((160-width)/2,41,(160-width)/2+width,55,COLOR_BLACK,RECT_EMPTY,v2,v3);
	DrawGrayRect2B((160-width)/2+1,42,(160-width)/2+width-1,54,COLOR_DARKGRAY,RECT_EMPTY,v2,v3);
	DrawGrayRect2B((160-width)/2+2,43,(160-width)/2+width-2,53,COLOR_LIGHTGRAY,RECT_EMPTY,v2,v3);
	
	drawCustomFontString((whatToEnter==NAME)?24:4,30,(whatToEnter==NAME)?(unsigned char*)"ENTER USERNAME":(unsigned char*)"ENTER YOUR INITIALS");
	
	GrayDrawStrExt2B((160-DrawStrWidth(buffer,F_6x8))>>1,45,(const char*)buffer,A_NORMAL,F_6x8,v2,v3);	
	copyScreens();
}

// for when 1+ existing profiles exist
static void loadProfileScreen(void) {
	waitForKeyReleased();
	mc = &(MOVING_CURSOR){16,1};
	int choice = 0;
	do {
		drawLoadProfileScreen();
		drawOptionsCursor(choice);
		
		if (_keytest(RR_DOWN)) {
			waitForKeyReleased();
			if (++choice > numProfiles) {
				choice = 0;
			}
			continue;
		}
		if (_keytest(RR_UP)) {
			waitForKeyReleased();
			if (--choice < 0) {
				choice = numProfiles;
			}
			continue;
		}			
		if (_keytest(RR_2ND)) {
			if (choice == numProfiles) { // no profiles exist, so only "Create New" is displayed
				if (numProfiles < 5) {
					createProfileScreen();
					return;
				}
				else {
					ER_success();
					exit(0);
				}
			}
			
			// load the current profile after unarchiving it			
			sprintf(fileName,"profiles\\%s",profileNames[choice]);
			unarchiveFile((const char*)fileName);
			SYM_ENTRY* mySym = DerefSym(SymFind(SYMSTR((const char*)fileName)));
			if (mySym) {
				PROFILE* load = (PROFILE*)((char*)HeapDeref(mySym->handle)+2);
				currentProfile = *load;
				SymDel(SYMSTR((const char*)fileName)); // overwrite the current file if it exists
				goAheadAndSave = TRUE;
				return;
			}			
		}
		if (_keytest(RR_ESC)) {
			goAheadAndSave = FALSE;
			ER_success();
			exit(0); 
		}
	} while (TRUE);
}

static void drawLoadProfileScreen(void) {
	drawScreen(skylight);
	drawCustomFontString(28,6,(unsigned char*)"LOAD PROFILE"); // main menu title
	int i = 0;
	unsigned int y = 30;
	while (i < numProfiles) {
		drawCustomFontString(36,y,(unsigned char*)profileNames[i]);
		i++;
		y+=10;
	}
	drawCustomFontString(36,y,(unsigned char*)"CREATE NEW");
}

static void profileScreen(void) {
	char input[26];
	static const unsigned int PROFILE_TEXT_X = 20;
	
	WaitForMillis(500);
	drawScreen(skylight);
	drawCustomFontString(40,6,(unsigned char*)"MY PROFILE");
	drawCustomFontString(20,20,(unsigned char*)currentProfile.name);
	
	FontSetSys(F_4x6);
	
	// Wins and Losses in various situations
	sprintf(input,"Record: %u-%u",currentProfile.numWins,currentProfile.numLosses);
	GrayDrawStr2B(PROFILE_TEXT_X,36,input,A_NORMAL,v2,v3);
	sprintf(input,"Tournaments: %u-%u",currentProfile.numTournamentWins,currentProfile.numTournamentLosses);
	GrayDrawStr2B(PROFILE_TEXT_X,44,input,A_NORMAL,v2,v3);	
	sprintf(input,"Sudden Death: %u-%u",currentProfile.numSuddenDeathWins,currentProfile.numSuddenDeathLosses);
	GrayDrawStr2B(PROFILE_TEXT_X,52,input,A_NORMAL,v2,v3);
	
	copyScreens();
	waitForKeyReleased();
	while (!_rowread(0)); // wait for a key to be pressed - once hit, return to the options menu
}


////////////////////////////////////////////////////////   Non-static methods   /////////////////////////////////////////////////////////////////////////

// Introductory screen for each battle of any type
void VSScreen(void) {
	memset(v2,0xFF,LCD_SIZE);
	memset(v3,0xFF,LCD_SIZE);
	memset(v2+480,0x00,1920);
	
	FontSetSys(F_4x6);
	
	unsigned int i = 1, x = 0;
	do {
		GraySprite16_AND_R(x,0,16,extraptr->vsscreendata[i],extraptr->vsscreendata[i]+16,v2,v3);
		x += 16,i++;
	} while (i < 11);
	
	GrayClipISprite16_TRAND_R(72,16,16,extraptr->vsscreendata[0],v2,v3);
	GrayClipISprite16_TRAND_R(72,32,16,extraptr->vsscreendata[0],v2,v3);
	GrayClipISprite16_TRAND_R(72,48,16,extraptr->vsscreendata[0],v2,v3);
	GrayClipISprite16_TRAND_R(72,64,16,extraptr->vsscreendata[0],v2,v3);
	
	GrayDrawStr2B(10,84,characterNames[p1->characterIndex],A_REVERSE,v2,v3);
	
	if (mode == STORY_MODE && fightMetal) { // cool addition: "metal falco" for example so user is aware of resemblance to SSBM
		char str[19];
		sprintf(str,"Metal %s",characterNames[p2->characterIndex]);
		GrayDrawStr2B(10,92,str,A_REVERSE,v2,v3);
	} else {
		GrayDrawStr2B(10,92,characterNames[p2->characterIndex],A_REVERSE,v2,v3);
	}
	
	char teamNum[2];
	sprintf(teamNum,"%u",p1->team);
	GrayDrawStr2B(8,23,teamNum,A_XOR,v2,v3);
	
	sprintf(teamNum,"%u",p2->team);
	GrayDrawStr2B(8,57,teamNum,A_XOR,v2,v3);
	
	// draw player sprites - play with drawing coordinates
	if (mode == ARENA_MODE) {
		GraySprite32_SMASK_R(20,16,charHeights[0],charsSelected[0],charsSelected[0]+charHeights[0],charsSelected[0]+(charHeights[0]<<1),v2,v3);
	} else {
		GraySprite32_SMASK_R(20,16,characters[p1->characterIndex].h,characters[p1->characterIndex].frames[TAUNT1].data,characters[p1->characterIndex].frames[TAUNT1].data+characters[p1->characterIndex].h,characters[p1->characterIndex].frames[TAUNT1].data+2*characters[p1->characterIndex].h,v2,v3);
	}
	
	if (mode == ARENA_MODE) {
		GraySprite32_SMASK_R(20,50,charHeights[1],charsSelected[1],charsSelected[1]+charHeights[1],charsSelected[1]+(charHeights[1]<<1),v2,v3);
	} else {
		GraySprite32_SMASK_R(20,50,characters[p2->characterIndex].h,characters[p2->characterIndex].frames[TAUNT1].data,characters[p2->characterIndex].frames[TAUNT1].data+characters[p2->characterIndex].h,characters[p2->characterIndex].frames[TAUNT1].data+2*characters[p2->characterIndex].h,v2,v3);
	}

	if (numPlayers > 2) {
		GrayDrawStr2B(90,84,characterNames[p3->characterIndex],A_REVERSE,v2,v3);
		if (mode == ARENA_MODE) {
			GraySprite32_SMASK_R(108,16,charHeights[2],charsSelected[2],charsSelected[2]+charHeights[2],charsSelected[2]+2*charHeights[2],v2,v3);
		} else {
			GraySprite32_SMASK_R(108,16,characters[p3->characterIndex].h,characters[p3->characterIndex].frames[TAUNT1].data,characters[p3->characterIndex].frames[TAUNT1].data+characters[p3->characterIndex].h,characters[p3->characterIndex].frames[TAUNT1].data+2*characters[p3->characterIndex].h,v2,v3);
		}

		sprintf(teamNum,"%u",p3->team);
		GrayDrawStr2B(146,23,teamNum,A_XOR,v2,v3);
	}
	if (numPlayers > 3) {
		GrayDrawStr2B(90,92,characterNames[p4->characterIndex],A_REVERSE,v2,v3);
		if (mode == ARENA_MODE) {
			GraySprite32_SMASK_R(108,50,charHeights[3],charsSelected[3],charsSelected[3]+charHeights[3],charsSelected[3]+2*charHeights[3],v2,v3);
		} else {
			GraySprite32_SMASK_R(108,50,characters[p4->characterIndex].h,characters[p4->characterIndex].frames[TAUNT1].data,characters[p4->characterIndex].frames[TAUNT1].data+characters[p4->characterIndex].h,characters[p4->characterIndex].frames[TAUNT1].data+2*characters[p4->characterIndex].h,v2,v3);
		}

		sprintf(teamNum,"%u",p4->team);
		GrayDrawStr2B(146,57,teamNum,A_XOR,v2,v3);
	}
	
	pokeIO(0x600005,0x17);
	copyScreens();
	WaitForMillis(4000);
	FadeOutToBlack_CWS1_R(GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE),100,30,20);
}

// Decompresses the Run-length encryption Vertically Compressed Titlescreen
void RLE_Decompress(unsigned char *src, unsigned char *dest, short size) {
	int i = 0, j;
	int pos = 0;
	int dcmpNum = 0;
	
	while (dcmpNum < size) {
		// decompress a run
		if (src[i] == SENTINEL_VALUE) {
			i++;
			for (j = 0; j < src[i+1]; j++) {
				dest[pos] = src[i];
				pos += 20;
				if (pos > 2000) {
					pos -= 2000;
					pos++;
				}
			}
			i++;
			dcmpNum += src[i];
		} else {
			dest[pos] = src[i];
			pos += 20; // next line
			if (pos > 2000) { // 100 lines, 20 bytes per line
				pos -= 2000;
				pos++;
			}
			dcmpNum++;
		}
		i++;
	}
}


// End of Source File