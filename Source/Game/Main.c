// Twilight Legion for TI-89(Titanium), TI-92(+), Voyage 200
// C Source File - Main.c
// Michael Hergenrader
// Compiled with TIGCC 0.96 Beta 8
// Please see README for license/disclaimer information. In short, please feel free to use code you see here,
// and any credit to me would be greatly appreciated! :-)
// Copyright 2005-2010 Michael Hergenrader

#define COMMENT_STRING         "Twilight Legion by Michael Hergenrader"
#define COMMENT_PROGRAM_NAME   "<<< Twilight Legion >>>" // shown after exiting game
#define COMMENT_VERSION_STRING "Version 1.9"
#define COMMENT_VERSION_NUMBER 1, 9, 1, 0 // major, minor, revision, subrevision
#define COMMENT_AUTHORS        "Michael Hergenrader"

#include <tigcclib.h>
#include "headers.h"

char* mainBlock = NULL;
char* tournamentBlock = NULL;
unsigned char* profileNameBlock = NULL; // enough data to save custom player profiles (simulates memory cards)
unsigned char* Home = NULL; // main LCD buffer for the Home screen from which program is launched

unsigned char initialContrast;

SYM_ENTRY *stagegfx = NULL;
SYM_ENTRY *chargfx1 = NULL;
SYM_ENTRY *chargfx2 = NULL;
SYM_ENTRY *chargfx3 = NULL;
SYM_ENTRY *extragfx = NULL;

HSym stagefile = HS_NULL;
HSym charfile1 = HS_NULL;
HSym charfile2 = HS_NULL;
HSym charfile3 = HS_NULL;
HSym extrafile = HS_NULL;

static void initializeCharacters(void);
static void initializeStages(void);

static unsigned char* getDataPtr(const char* file, short offset);
static void archiveFile(const char* file);

static void saveCurrentProfile(void);

DEFINE_INT_HANDLER(on_handle_interrupt) { // need to reserve a5 register to enable grayscale; save its old state
	register void* olda5 asm("%a4");
	asm volatile("move.l %%a5,%0" : "=a"(olda5));
	asm volatile("lea __ld_entry_point_plus_0x8000(%pc),%a5"); // this should prevent any more [ON] errors from address errors
	
	ExecuteHandler(save_int_6); // execute original handler for interrupt #6
	pokeIO(0x600003,0xFF);
	if (!peekIO_bit(0x60001A,1) && !linked) { // make sure the connection isn't broken with ON pause
		off(); // mimic pressing ON button
	}
	
	asm("move.l %0,%%a5" : : "a"(olda5)); // restore a5 register
}

// Main Function - Total malloc'ed memory (not including items) = ~54188 bytes
void _main(void)
{
	atexit((atexit_t)exitGame); // set onExit interrupt handler (like callbacks in JavaScript)
	
	// ensure that level sprite data file installed
	if ((stagefile = SymFind(SYMSTR(STAGE_FILENAME))).folder == H_NULL) {
		ST_helpMsg(STAGE_NOT_INSTALLED);
		ER_throw(ER_PRGM_NOT_FOUND);
	}
	
	// ensure that all character sprite data files are installed
	if ((charfile1 = SymFind(SYMSTR(CHAR1_FILENAME))).folder == H_NULL || (charfile2 = SymFind(SYMSTR(CHAR2_FILENAME))).folder == H_NULL || (charfile3 = SymFind(SYMSTR(CHAR3_FILENAME))).folder == H_NULL) {
		ST_helpMsg(CHARS_NOT_INSTALLED);
		ER_throw(ER_PRGM_NOT_FOUND);
	}
	
	// ensure extra external data file is installed on calculator
	if ((extrafile = SymFind(SYMSTR(EXTRA_FILENAME))).folder == H_NULL) {
		ST_helpMsg(EXTRA_NOT_INSTALLED);
		ER_throw(ER_PRGM_NOT_FOUND);
	}
	
	// attempt to allocate sufficient memory to start up program: main memory items + external data sprites
	// provides good speedup to have this allocated in memory directly on startup (which is itself pretty darn fast)
	if ((mainBlock = malloc(MCARD)) == NULL || (block = malloc(TCARD)) == NULL || (characters = malloc((24 * sizeof(CHARACTER)))) == NULL || (Home = malloc(LCD_SIZE)) == NULL || (profileNameBlock = malloc(45)) == NULL || (tournamentBlock = malloc(GRAY_BIG_VSCREEN_SIZE)) == NULL) {
		ST_helpMsg(NOT_ENOUGH_MEM);
		ER_throw(ER_MEMORY);
	}

	TRY
		initialize();
	ONERR
		register void* olda5 asm("%a4"); // restore custom registers and interrupts
		asm volatile("move.l %%a5,%0" : "=a"(olda5));
		asm volatile("lea __ld_entry_point_plus_0x8000(%pc),%a5");
		exit(0);
		asm("move.l %0,%%a5" : : "a"(olda5));
	ENDTRY
}

void initialize(void) {
	initialContrast = *ActiveContrastAddr(); // returns contrast value from the method	
	setContrast(TI89_CLASSIC ? CON_CLASSIC : CON_TITANIUM); // set to specific contrast for best visuals
	
	stagegfx = DerefSym(stagefile); // load stage/level file from symbol table address
	if (stagegfx) {
		if (!HeapGetLock(stagegfx->handle)) { // obtain the file handle and lock for no other modification
			HeapLock(stagegfx->handle);
		}
		dataptr = (EXTERNAL*)((char*)HeapDeref(stagegfx->handle) + 2); // finally, index into the file (its stored struct) with the char* data pointer
	}
	
	chargfx1 = DerefSym(charfile1);
	if (chargfx1) {
		if (!HeapGetLock(chargfx1->handle)) {
			HeapLock(chargfx1->handle);
		}
		c1 = (unsigned long*)((char*)HeapDeref(chargfx1->handle) + 2);	
	}
	
	chargfx2 = DerefSym(charfile2);
	if (chargfx2) {
		if (!HeapGetLock(chargfx2->handle)) {
			HeapLock(chargfx2->handle);
		}
		c2 = (unsigned long*)((char*)HeapDeref(chargfx2->handle) + 2);	
	}
	
	chargfx3 = DerefSym(charfile3);
	if (chargfx3) {
		if (!HeapGetLock(chargfx3->handle)) {
			HeapLock(chargfx3->handle);
		}
		c3 = (unsigned long*)((char*)HeapDeref(chargfx3->handle) + 2);
	}
	
	extragfx = DerefSym(extrafile);
	if (extragfx) {
		if (!HeapGetLock(extragfx->handle)) {
			HeapLock(extragfx->handle);
		}
		extraptr = (EXTRA_EXTERNAL*)((char*)HeapDeref(extragfx->handle) + 2);	
	}
	
	// main pointer initializations for loaded structures and allocated memory (with appropriate offsets into the heap)
	v2 = (char*)mainBlock;
	v3 = ((char*)v2) + LCD_SIZE;	
	p1 = (PLAYER*)(((char*)v3) + LCD_SIZE);
	p2 = (PLAYER*)(((char*)p1) + sizeof(PLAYER));
	p3 = (PLAYER*)(((char*)p2) + sizeof(PLAYER));
	p4 = (PLAYER*)(((char*)p3) + sizeof(PLAYER));
	timer = (TIMER*)(((char*)p4) + sizeof(PLAYER));
	p1->myProjectile = (PROJECTILE*)(((char*)timer) + sizeof(TIMER));
	p2->myProjectile = (PROJECTILE*)(((char*)p1->myProjectile) + sizeof(PROJECTILE));
	p3->myProjectile = (PROJECTILE*)(((char*)p2->myProjectile) + sizeof(PROJECTILE));
	p4->myProjectile = (PROJECTILE*)(((char*)p3->myProjectile) + sizeof(PROJECTILE));
	
	masterHand = (HAND*)((char*)p4->myProjectile + sizeof(PROJECTILE));
	crazyHand = (HAND*)((char*)masterHand + sizeof(HAND));
		
	skylight = ((char*)crazyHand + sizeof(HAND));
	skydark = ((char*)skylight + 2001); // 2001 = size of sky buffer (screen)
	
	powbufferlight = ((char*)skydark + 2001);
	powbufferdark = ((char*)powbufferlight + 2001);
		
	RLE_Decompress(extraptr->coatlightsplash, skylight, 1999); // decompress the splash screens
	RLE_Decompress(extraptr->coatdarksplash, skydark, 1999);
	
	RLE_Decompress(extraptr->powlightsplash, powbufferdark, 1999);
	RLE_Decompress(extraptr->powdarksplash, powbufferlight, 1999);
	
	*((char*)skylight + 1) = 0xE7;
	*((char*)skydark + 1) = 0x18;
	
	*((char*)powbufferlight + 1) = 0xE7;
	*((char*)powbufferdark + 1) = 0x18;
		
	virtual = block; // set up the grayscale buffers to the virtual pointer
	
	profileNames[0] = ((unsigned char*)profileNameBlock); // up to four total player profiles (8 characters + null character)
	profileNames[1] = ((unsigned char*)profileNames[0] + 9);
	profileNames[2] = ((unsigned char*)profileNames[1] + 9);
	profileNames[3] = ((unsigned char*)profileNames[2] + 9);
	profileNames[4] = ((unsigned char*)profileNames[3] + 9);
	
	initializeCharacters();
	initializeStages();
	
	if (FolderFind(SYMSTR(PROFILE_FOLDERNAME)) == NOT_FOUND) {
		FolderAdd(SYMSTR(PROFILE_FOLDERNAME)); // add/create profiles folder if currently doesn't exist
	}

	save_int_1 = GetIntVec(AUTO_INT_1);
	save_int_5 = GetIntVec(AUTO_INT_5);
	save_int_6 = GetIntVec(AUTO_INT_6); // save old interrupt vectors
	
	SetIntVec(AUTO_INT_1, DUMMY_HANDLER); // redirect Interrupt handlers for grayscale and ON button to pause event
  	SetIntVec(AUTO_INT_6, on_handle_interrupt);

	LCD_save(Home); // save buffer of Home screen before launching program
	randomize();
	if (!GrayOn()) {
		return;
	}

	doProfileLoadingOrCreating();
		
	if (currentProfile.iHaveBattle) {
		setupLoadedGame(); // if player quit in the middle of a battle, load it up automatically
	}
	
	numPlayers = 2;
	p1->team = WHITE_TEAM;
	p2->team = LIGHTGRAY_TEAM;
	p3->team = DARKGRAY_TEAM;
	p4->team = BLACK_TEAM; // start all on different teams
	mainMenu();
}

static void initializeCharacters(void) { // set up the animation progressions of frames for each character
	unsigned int k, p;
	
	for (k = 0; k < 24; k++) { // all characters
		characters[k].w = characterWidths[k];
		characters[k].h = characterHeights[k];
		characters[k].specialType = charSpecialAttacks[k];
		
		for (p = 0; p < 23; p++) { // all animation frames for each character
			unsigned short soffset = (characterHeights[k]*3*p+offset[k]);
			characters[k].frames[p].data = (k<8)?c1+soffset:((k>=16)?c3+soffset:c2+soffset);
		}
		
		// facing right frames - set up the linked list of animation frames
		characters[k].frames[STAND_RIGHT].next = &characters[k].frames[RUN1_RIGHT];
		characters[k].frames[BREATHE_RIGHT].next = &characters[k].frames[RUN1_RIGHT];
		characters[k].frames[RUN1_RIGHT].next = &characters[k].frames[RUN2_RIGHT];
		characters[k].frames[RUN2_RIGHT].next = &characters[k].frames[STAND_RIGHT];
		
		characters[k].frames[SMASH_RIGHT].next = &characters[k].frames[STAND_RIGHT];
		characters[k].frames[SKY_RIGHT].next = &characters[k].frames[STAND_RIGHT];
		characters[k].frames[SPECIAL_RIGHT].next = &characters[k].frames[STAND_RIGHT];		
		characters[k].frames[JUMPUP_RIGHT].next = &characters[k].frames[STAND_RIGHT];
		characters[k].frames[HURT_RIGHT].next = &characters[k].frames[STAND_LEFT];
		
		characters[k].frames[CLIMB_RIGHT].next = &characters[k].frames[STAND_RIGHT];
		
		// facing left
		characters[k].frames[STAND_LEFT].next = &characters[k].frames[RUN1_LEFT];
		characters[k].frames[BREATHE_LEFT].next = &characters[k].frames[RUN1_LEFT];
		characters[k].frames[RUN1_LEFT].next = &characters[k].frames[RUN2_LEFT];
		characters[k].frames[RUN2_LEFT].next = &characters[k].frames[STAND_LEFT];
		
		characters[k].frames[SMASH_LEFT].next = &characters[k].frames[STAND_LEFT];
		characters[k].frames[SKY_LEFT].next = &characters[k].frames[STAND_LEFT];
		characters[k].frames[SPECIAL_LEFT].next = &characters[k].frames[STAND_LEFT];		
		characters[k].frames[JUMPUP_LEFT].next = &characters[k].frames[STAND_LEFT];
		characters[k].frames[HURT_LEFT].next = &characters[k].frames[STAND_RIGHT];
		
		characters[k].frames[CLIMB_LEFT].next = &characters[k].frames[STAND_LEFT];
		
		// crouching and taunting (not facing any specific direction)
		characters[k].frames[CROUCH].next = &characters[k].frames[STAND_RIGHT];
		characters[k].frames[TAUNT1].next = &characters[k].frames[TAUNT2];
		characters[k].frames[TAUNT2].next = &characters[k].frames[STAND_RIGHT];
	}
}

static void initializeStages(void) {
	unsigned int i = 0;
	do {
		switch (i) { // set the proper tileInfo and sprite pointers based on the loaded heap pointers
			case 0 ... 5:
			DEREF_SMALL(stages, i).tileInfo = dataptr->tileinfo1; // DEREF_SMALL is an optimized pointer dereferencing routine (for size over speed)
			DEREF_SMALL(stages, i).fgPlane.sprites = (short*)dataptr->tiles1;
			break;
			case 6 ... 10:
			DEREF_SMALL(stages, i).tileInfo = dataptr->tileinfo2;
			DEREF_SMALL(stages, i).fgPlane.sprites = (short*)dataptr->tiles2;
			break;
			case 11 ... 15:
			DEREF_SMALL(stages, i).tileInfo = dataptr->tileinfo3;
			DEREF_SMALL(stages, i).fgPlane.sprites = (short*)dataptr->tiles3;
			break;
			case 16 ... 19:
			DEREF_SMALL(stages, i).tileInfo = dataptr->tileinfo4;
			DEREF_SMALL(stages, i).fgPlane.sprites = (short*)dataptr->tiles4;
			break;
			case 20 ... 25:
			DEREF_SMALL(stages, i).tileInfo = dataptr->tileinfo5;
			DEREF_SMALL(stages, i).fgPlane.sprites = (short*)dataptr->tiles5;
			break;
		}		
		i++;
	} while (i < NUM_STAGES);
	
	stages[0].fgPlane.matrix = (char*)dataptr->antairavillage; // the level foreground data
	stages[1].fgPlane.matrix = (char*)dataptr->battlefield;
	stages[2].fgPlane.matrix = (char*)dataptr->bigblue;
	stages[3].fgPlane.matrix = (char*)dataptr->brinstar;
	stages[4].fgPlane.matrix = (char*)dataptr->caidruscathedraloftime;
	stages[5].fgPlane.matrix = (char*)dataptr->coresanctuarymonument;
	stages[6].fgPlane.matrix = (char*)dataptr->corneria;
	stages[7].fgPlane.matrix = (char*)dataptr->crasphonecity;
	stages[8].fgPlane.matrix = (char*)dataptr->dreamland;
	stages[9].fgPlane.matrix = (char*)dataptr->eratower;
	stages[10].fgPlane.matrix = (char*)dataptr->finaldestination;
	stages[11].fgPlane.matrix = (char*)dataptr->fourside;
	stages[12].fgPlane.matrix = (char*)dataptr->flatzone;
	stages[13].fgPlane.matrix = (char*)dataptr->glasssubmarine;
	stages[14].fgPlane.matrix = (char*)dataptr->greatbay;
	stages[15].fgPlane.matrix = (char*)dataptr->hethorforest;
	stages[16].fgPlane.matrix = (char*)dataptr->hyrule;
	stages[17].fgPlane.matrix = (char*)dataptr->icemountain;
	stages[18].fgPlane.matrix = (char*)dataptr->junglejapes;
	stages[19].fgPlane.matrix = (char*)dataptr->lunorstronghold;
	stages[20].fgPlane.matrix = (char*)dataptr->markedmosque;
	stages[21].fgPlane.matrix = (char*)dataptr->reatenbase;
	stages[22].fgPlane.matrix = (char*)dataptr->redskybay;
	stages[23].fgPlane.matrix = (char*)dataptr->soultower;
	stages[24].fgPlane.matrix = (char*)dataptr->terrarock;
	stages[25].fgPlane.matrix = (char*)dataptr->yoshisisland;
	
	// set up race to the finish stages
	rtfstages[0].tileInfo = extraptr->raceTileInfo;
	rtfstages[1].tileInfo = extraptr->raceTileInfo;	
	rtfstages[0].fgPlane.matrix = (char*)DEREF_SMALL(extraptr->raceLayouts, 0);
	rtfstages[1].fgPlane.matrix = (char*)DEREF_SMALL(extraptr->raceLayouts, 1);	
	rtfstages[0].fgPlane.sprites = (short*)extraptr->raceToTheFinishSprites;
	rtfstages[1].fgPlane.sprites = (short*)extraptr->raceToTheFinishSprites;
}

unsigned char* ActiveContrastAddr(void); // find current contrast value
asm("ActiveContrastAddr:\n"
    "    movem.l  %d0-%d1,-(%a7)\n"
    "    move.w   #4,%d0\n"
    "    trap     #9\n"
    "    movem.l  (%a7)+,%d0-%d1\n"
    "    rts");

void WaitForMillis(unsigned short); // custom 68k ASM routine to wait for a certain amount of milliseconds
asm("xdef WaitForMillis\n"
"WaitForMillis:  movem.l %d0-%d3,-(%sp)\n"
"           move.w (20,%sp),%d2\n"
"           move.l #31,%d1\n"
"           move.l #31,%d3\n"
"_wl2_:     move.w #120,%d0    /* modify this value for exact timing !!! */\n"
"_wl1_:     rol.l  %d3,%d1\n"
"           dbra   %d0,_wl1_\n"
"           dbra   %d2,_wl2_\n"
"           movem.l (%sp)+,%d0-%d3\n"
"           rts");

void setContrast(unsigned int con) {
 	while ((unsigned int)initialContrast > con) {
		OSContrastDn();
		initialContrast--;
	}
 	while ((unsigned int)initialContrast < con) {
		OSContrastUp();
		initialContrast++;
	}
}

// returns a character pointer to the data stored in a particular archived data file
static unsigned char* getDataPtr(const char* file, short offset) {
	unsigned char* loc;
	SYM_ENTRY* sym;
	HANDLE h;
	
	if ((sym = SymFindPtr(SYMSTR(file), 0)) == NULL) {
		return NULL;
	}
	
	h = sym->handle;
	if ((loc = (unsigned char*)HeapDeref(h)) == (void*)0xFFFFFFFF) {
		return NULL;
	}
	
	return (loc + 2 + offset);
}

// Thanks and credit to Fisch2 for these two helper routines
static void archiveFile(const char* file) { // all files after read/writes should be archived for protection in case of system crashes/RAM clears
	SYM_ENTRY* sym = NULL;
	HSym hsym = SymFind(SYMSTR(file));
	
	sym = DerefSym(hsym);
	
	if (EM_findEmptySlot(*getDataPtr(file, -2)) == NULL) {
		return;
	}
	if (!sym->flags.bits.archived) {
		EM_moveSymToExtMem(SYMSTR(file), HS_NULL);
	}
}

void unarchiveFile(const char* file) { // all files, in order to be loaded into memory and read/modified, must be unarchived
	SYM_ENTRY* sym = DerefSym(SymFind(SYMSTR(file)));
	
	if (sym->flags.bits.archived) {
		EM_moveSymFromExtMem(SYMSTR(file),HS_NULL);
	}
}

void saveCurrentProfile(void) {
	static const unsigned char extension[7] = { 0, 'u', 's', 'e', 'r', 0, OTH_TAG }; // file custom extension
	unsigned short size = sizeof(PROFILE) + 9; // add space for 8 character name + null terminator (8 characters allowed for TI files)
	char* base;
	HANDLE h;

	if ((h = HeapAlloc(size)) == H_NULL) { // could not allocate space on the heap to write out this profile
		return;
	}
	
	// note: HeapDeref(H_NULL) == 0xFFFFFFFF
	if ((base = HeapDeref(DerefSym(SymAdd(SYMSTR((const char*)fileName)))->handle = h)) == (void*)0xFFFFFFFF) {
		HeapFree(h);
		return;
	} // could not execute write to file, due to null heap reference
	
	// write custom profile data to profile VAT file
	*(short*)base = size - 2;
	base += 2;
	memcpy(base, &currentProfile, sizeof(PROFILE));
	base += sizeof(PROFILE);
	memcpy(base, extension, sizeof(extension));
	archiveFile((const char*)fileName); // archive to avoid deletion during crashes and RAM clears
}

// interrupt handler called when player wants to exit the game
atexit_t exitGame(void) {
	register void* olda5 asm("%a4"); // restore a5 value from a4 register (direct register access)
	asm volatile("move.l %%a5,%0" : "=a"(olda5)); 
	asm volatile("lea __ld_entry_point_plus_0x8000(%pc),%a5");

	GrayOff();
	
	if (goAheadAndSave) {// must check this to make sure not writing anything on exiting
		saveCurrentProfile();
	}
	
	OSLinkClose(); // in case of link play, close the link
		
	if (block) {
		free(block);
	}
	if (mainBlock) {
		free(mainBlock);
	}
	if (characters) {
		free(characters);
	}
	if (profileNameBlock) {
		free(profileNameBlock);
	}
	if (tournamentBlock) {
		free(tournamentBlock);
	}
	
	if (HeapGetLock(stagegfx->handle)) {
		HeapUnlock(stagegfx->handle); // free the lock currently on the file handles (return it to the system)
	}
	if (HeapGetLock(chargfx1->handle)) {
		HeapUnlock(chargfx1->handle);
	}
	if (HeapGetLock(chargfx2->handle)) {
		HeapUnlock(chargfx2->handle);
	}
	if (HeapGetLock(chargfx3->handle)) {
		HeapUnlock(chargfx3->handle);
	}
	if (HeapGetLock(extragfx->handle)) {
		HeapUnlock(extragfx->handle);
	}
		
	SetIntVec(AUTO_INT_1, save_int_1); // restore interrupts
	SetIntVec(AUTO_INT_5, save_int_5);
	SetIntVec(AUTO_INT_6, save_int_6);
	setContrast(TI89_CLASSIC ? CON_CLASSIC : CON_TITANIUM); // restores the contrast to the user's original setting
	
	if (Home) { // restore the Home screen buffer where this game was launched from
		GKeyFlush();
		LCD_restore(Home);
		ST_helpMsg(COMMENT_PROGRAM_NAME);
		free(Home);
	}

	asm("move.l %0,%%a5" : : "a"(olda5));
	return 0;
}

// End of Source File