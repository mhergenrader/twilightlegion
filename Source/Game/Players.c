// Twilight Legion for TI-89(Titanium), TI-92(+), Voyage 200
// C Source File - Players.c
// Michael Hergenrader
// Compiled with TIGCC 0.96 Beta 8
// Please see README for license/disclaimer information. In short, please feel free to use code you see here,
// and any credit to me would be greatly appreciated! :-)
// Copyright 2005-2010 Michael Hergenrader

// This file contains all routines for handling human control of characters and AI control as well.
// In addition, this file contains all logic for a player's interaction with the different types of terrain.

#include <tigcclib.h>
#include "headers.h"

static BOOL holding = FALSE;
static BOOL specialHolding = FALSE;
static BOOL invHolding = FALSE;
static BOOL grabHolding = FALSE;

static void smashAttack(PLAYER* player);
static void standStillSpecial(PLAYER* p);
static void fireProjectile(PLAYER* p);
static void missileSpecial(PLAYER* p);
static void grabPlayer(PLAYER* p);
static void dodge(PLAYER* p, DIRECTION d);

static void movePlayerLeft(PLAYER* player);
static void movePlayerRight(PLAYER* player);
static BOOL canMovePlayer(PLAYER* player, DIRECTION dirx, VERT_DIRECTION diry);
static BOOL findTerrain(PLAYER* player, VERT_DIRECTION diry, int tileX, int tileY);
static inline BOOL playerFall(PLAYER* player);
static inline BOOL playerClimb(PLAYER* player);

static inline BOOL ladder(int px, int py); // auxiliary terrain finding methods (helper methods)
static inline BOOL hill(int px, int py, unsigned long slope);
static inline BOOL hotTile(int px, int py);
static inline BOOL waterTile(int px, int py);

static void checkForFallingPlayer(PLAYER* me);
static void checkForMovingLevelFallingPlayer(PLAYER* me);
static void updateFrames(PLAYER* p);

// AI Version 2.2 functions
static void scanForEnemy(PLAYER* cpu); // if I don't already have an enemy, need to find one (decides how here)
static inline BOOL solidTileBeneath(int x, int y);

// Enemy scanning method(s)
static PLAYER* closestEnemyScan(PLAYER* cpu); // obviously scan for the closest player (the one I have now)
static PLAYER* fatiguedEnemyScan(PLAYER* cpu); // AI will look for an easy kill

// Strategy decisions
static void moveToEnemy(PLAYER* cpu); // if ready to, maneuver through level to get player
static void attackEnemy(PLAYER* cpu); // decide best idea to hurt enemy

static int horizontalDistanceBetween(PLAYER* a, PLAYER* b);
static int xHorizontalDistanceBetween(int x1, int x2);

static inline BOOL projLook(int tileX, int tileY);

static void (*specialAttacks[3])(PLAYER* p) =  {
	standStillSpecial,
	fireProjectile,
	missileSpecial
};

static void (*fallingPlayerChecks[2])(PLAYER* me) = {
	checkForFallingPlayer,
	checkForMovingLevelFallingPlayer
};


// main method for handling a character during a battle
void handlePlayer(PLAYER* player) {
	player->invincible = FALSE;
	if (player->enemy == NULL && !numHands) {
		player->enemy = closestEnemyScan(player);
	}

	if (player->paralyzed) {
		player->onHillL = FALSE, player->onHillR = FALSE;
		player->grabbing = FALSE;
		
		if (!numHands) {
			player->enemy->beingHeld = FALSE;
		}
		
		player->smashAttacking = FALSE;
		player->specialAttacking = FALSE;
		player->skyAttacking = FALSE; // so that not continuously under same attack
		
		// -8 gives the arc a parabolic shape instead of a straight drop
		if (player->yspeed <= -8 || !canMovePlayer(player,0,UP) || (player->yspeed < 0 && !canMovePlayer(player,0,DOWN))) {
			player->paralyzed = FALSE;
			player->yspeed = 0;
			player->xspeed = 0;
		} else {
			player->y -= player->yspeed, player->yspeed -= 2;
			
			if (player->xspeed < 0) { // paralyzed to the left
				if (canMovePlayer(player,LEFT,0)) {
					player->x += player->xspeed;
					player->rightCurrent = &characters[player->characterIndex].frames[HURT_LEFT];
					player->leftCurrent = &characters[player->characterIndex].frames[HURT_LEFT];
				} else {
					player->xspeed = -player->xspeed;
				}
			} else if (player->xspeed > 0) { // paralyzed to the right
				if (canMovePlayer(player,RIGHT,0)) {
					player->x += player->xspeed;
					player->rightCurrent = &characters[player->characterIndex].frames[HURT_RIGHT];
					player->leftCurrent = &characters[player->characterIndex].frames[HURT_RIGHT];
				} else {
					player->xspeed = -player->xspeed;
				}
			}
			return; // if player is paralyzed, don't take in any keypress input/further actions
		}
	}
	
	if (player->jumpValue > 0) { // will perform an automatic jump when placed here (recommended for this action game)
 		player->y -= JUMPSPEED, player->jumpValue -= JUMPSPEED;
	}
	if (player->jumpValue < 0 || !canMovePlayer(player,0,UP)) { // don't allow jumping if at highest point or a solid tile above
  		player->jumpValue = 0;
	}
	
	fallingPlayerChecks[stageTemp->movingLevel](player); // check for how to adjust the player when he/she is falling
	if (!playerFall(player)) {
		player->numJumps = 0;
		player->skyAttacking = FALSE;
	}
	
	if (player->falling) {
		if (!canMovePlayer(player,RIGHT,0) || !canMovePlayer(player,LEFT,0)) {
  			player->hanging = TRUE;
		}
  	}
	
  	// Makes player jump
  	if (_keytest(RR_SHIFT) && player->numJumps == 0 && !player->beingHeld) {
		player->jumpValue = JUMPVALUE;
		player->numJumps = 1;
		player->onHillL = FALSE;
		player->onHillR = FALSE;
		player->climbing = FALSE;
		player->hanging = FALSE;
		player->leftCurrent = &characters[player->characterIndex].frames[JUMPUP_LEFT]; // jumping frames
		player->rightCurrent = &characters[player->characterIndex].frames[JUMPUP_RIGHT];
	}

	if (!player->running || disabled) {
		player->moveSpeed = WALKSPEED;
	}
	
	player->running = FALSE;
	
	if (!disabled) {
		if (_keytest(RR_LEFT)) {
			if (player->grabbing && !numHands) {
				player->enemy->xspeed = -4; // if player is moving, then adjust his enemy position
				player->enemy->yspeed = (((player->enemy->percent/18)*2)+2);
				player->enemy->paralyzed = TRUE;
				player->enemy->beingHeld = FALSE;
				player->grabbing = FALSE;
				points[STRONG_GRIP] = 1000; // special points category in classic mode
			}
			if (_keytest(RR_ALPHA)) { // try to dodge
				if (!invHolding && !player->beingHeld) {
					dodge(player,LEFT);
					invHolding = TRUE;
					player->direction = -player->direction; // automatically change direction to attack quickly after dodge!
				}
			} else{
				invHolding = FALSE;
				if (!player->beingHeld) { // general moving to the left
					movePlayerLeft(player);
				}
			}			
		}		
		if (_keytest(RR_RIGHT)) {
			if (player->grabbing && !numHands) {
				player->enemy->xspeed = 4;
				player->enemy->yspeed = (((player->enemy->percent/18)*2)+2);
				player->enemy->paralyzed = TRUE;
				player->enemy->beingHeld = FALSE;
				player->grabbing = FALSE;
				points[STRONG_GRIP] = 1000;
			}			
			if (_keytest(RR_ALPHA)) {
				if (!invHolding && !player->beingHeld) { // try to dodge
					dodge(player,RIGHT);
					invHolding = TRUE, player->direction = -player->direction; // automatically change direction to attack quickly!
				}
			} else {
				invHolding = FALSE;
				if (!player->beingHeld) {
					movePlayerRight(player);
				}
			}
		}
	}
	
  	if (_keytest(RR_2ND)) { // attack/pick up items
		if (player->beingHeld) {
   			if (numHands) {
   				masterHand->attackMarker-=2;
				crazyHand->attackMarker-=2;
			} else {
   				if (player->enemy) {
   					player->enemy->attackMarker-=2;
				}
			}			
   		}
	}
   	if (player->currentItem == NULL) { // if overlapping an item, pick it up
		if (head != NULL && existsItemInRegion(player)) {				
			ITEM* it = myItem(player);
 			if (it != NULL && !it->beenUsed) {
	 			if (it->replenish > 0) { // food item, so don't keep it
	 				player->percent -= it->replenish;
	 				if (player->percent < 1 || player->percent > 63000) {
	 					player->percent = MIN_HP;
					}
	 				it->beenUsed = TRUE; // instantly get rid of all the food items
	 			} else {
	 				player->currentItem = it;
				}
	 				
	 			if (it->index > 13 && it->index < 16) { // EXPLOSIVE ITEMS! (use it immediately)
	 				player->myProjectile->x = player->x+8;
					player->myProjectile->y = player->y+4;
					player->myProjectile->e = 0;
					player->myProjectile->exploding = TRUE;
	 				player->canFire = TRUE;
	 				player->xspeed = 4-(random(2)*8);
					player->yspeed = ((player->percent/18)*2)+2;
					player->percent += 50+random(10);
					player->paralyzed = TRUE;
	  				player->currentItem->beenUsed = TRUE;
					player->currentItem->beingHeld = FALSE;
					player->currentItem = NULL;
	 			}
	 		}
 		}
 	} else { // otherwise, if already holding an item, use it (call the function that the item has that the player holds)
  		int i = methodVals[player->currentItem->index];
  		if (i > -1) {
  			player = itemFuncs[i](player);
		}
	 	player->currentItem->beenUsed = TRUE;
		player->currentItem->beingHeld = FALSE;
		player->currentItem = NULL; // allow player to use more items once it has been used
  	}
  	
  	if (!player->smashAttacking && !player->specialAttacking && !player->beingHeld && !holding) {
 		smashAttack(player); // will still just attack anyway while picking up the item
 		holding = TRUE; // causes that player must release the key before they try to attack again
	} else {
		holding = FALSE;
	}

	if (_keytest(RR_DIAMOND) && !player->beingHeld) { // for some special attacks (like missile)
		if (!player->smashAttacking && !player->specialAttacking && !specialHolding) {
			specialAttacks[characters[player->characterIndex].specialType](player);
		}
		specialHolding = TRUE;		
	} else {
		specialHolding = FALSE;
	}

	if (_keytest(RR_UP) && !player->beingHeld) { // now includes center of gravity; can go up a ladder or jump
		player->crouching = FALSE;
		
		// if a player is currently overlapping a ladder tile
		if (ladder(player->x+x_fg+((characters[player->characterIndex].w)/2)-8+(((characters[player->characterIndex].w)/2)&1),player->y+y_fg+characters[player->characterIndex].h-16) || ladder(player->x+x_fg+((characters[player->characterIndex].w)/2)+7,player->y+y_fg+characters[player->characterIndex].h-16)) {
			if (((player->x+x_fg+((characters[player->characterIndex].w)/2)-8+(((characters[player->characterIndex].w)/2)&1))&15) != 0) {
				if (ladder(player->x+x_fg+((characters[player->characterIndex].w)/2)+7,player->y+y_fg+characters[player->characterIndex].h-16)) {
					if (x_fg > (stageTemp->sw*16)-160) {
						player->x+=2;
					} else {
						 scrollR = TRUE; // fixed ladder bug in this while loop
					}
				} else {
					if (x_fg <= 0) {
						player->x-=2;
					} else {
						scrollL = TRUE;
					}
				}
			}
			
			if (playerClimb(player)) { // if the player can climb (may be on a ladder tile, but the player's state doesn't necessarily allow him to climb)
				player->climbing = TRUE;
				if (y_fg <= 0 || player->y != 42) {
					player->y-=2;
				} else {
					scrollU = TRUE;
				}
			} else {
				player->climbing = FALSE;
			}
			
			if (player->climbing) { // now if player is actually climbing
				if (!(player->playerCounter & 7)) {
					player->direction = -player->direction; // simulate climbing procedure of L-R-L-R
				}
				if (player->direction < 0) {
					player->leftCurrent = &characters[player->characterIndex].frames[CLIMB_LEFT];
				}
				else {
					player->rightCurrent = &characters[player->characterIndex].frames[CLIMB_RIGHT];
				}
			}
		}
		
		if (player->hanging) { // hanging on a ledge: up = jump up from the ledge
			player->jumpValue = JUMPVALUE;
			player->hanging = FALSE;
			player->leftCurrent = &characters[player->characterIndex].frames[JUMPUP_LEFT];
			player->rightCurrent = &characters[player->characterIndex].frames[JUMPUP_RIGHT];
		}		
		if (player->jumpValue > 0 && player->jumpValue < 16 && player->numJumps == 1) { // double jump (if already in air from shift)
			player->jumpValue = JUMPVALUE;
			player->numJumps = 2; // for double jumps, it is always possible if holding up and shift at the same time
		}
		
		if (!player->falling && !player->climbing && !player->hanging && !player->jumpValue) { // make the player taunt his opponents!
			player->direction = RIGHT;
			player->taunting = TRUE;
			player->attackMarker = player->playerCounter;
			player->rightCurrent = &characters[player->characterIndex].frames[TAUNT1];
		}
	}

	if (_keytest(RR_DOWN) && !player->beingHeld) { // if player wants to climb back down the ladder, else crouch
		player->hanging = FALSE; // if on the edge, then drop down from it (if holding on)
		if (player->climbing) {
			if (canMovePlayer(player,0,DOWN)) { // offset at bottom is added in here
				if (y_fg > (stageTemp->sh*16)-100) {
					player->y+=2;
				} else {
					if (!stageTemp->movingLevel) {
						scrollD = TRUE;
					}
				}
			} else {
				player->climbing = FALSE;
			}
		}
		
		if (player->climbing) { // check if player still climbing or whether reached solid ground/air
			if (!(player->playerCounter&7)) {
				player->direction = -player->direction;
			}
			if (player->direction < 0) {
				player->leftCurrent = &characters[player->characterIndex].frames[CLIMB_LEFT];
			} else {
				player->rightCurrent = &characters[player->characterIndex].frames[CLIMB_RIGHT];
			}
		}
		
		if (player->falling) { // sky attack! (similar to down-A attacks in SSBM)
			player->skyAttacking = TRUE;
			player->leftCurrent = &characters[player->characterIndex].frames[SKY_LEFT];
			player->rightCurrent = &characters[player->characterIndex].frames[SKY_RIGHT];
		}
		
		// check the two tiles beneath the character
		unsigned long sd = stageTemp->tileInfo[getTile(player->x+x_fg+(characters[player->characterIndex].w/2)-8+((characters[player->characterIndex].w/2)&1),player->y+y_fg+characters[player->characterIndex].h+((characters[player->characterIndex].h)&1))];
		unsigned long sd2 = stageTemp->tileInfo[getTile(player->x+x_fg+(characters[player->characterIndex].w/2)+7+((characters[player->characterIndex].w/2)&1),player->y+y_fg+characters[player->characterIndex].h+((characters[player->characterIndex].h)&1))];
		if ((sd == (SOLID | CLOUD) && sd2 != SOLID) || (sd2 == (SOLID | CLOUD) && sd != SOLID)) {
			player->y += 4; // fall through cloud block
		}
		
		if (!playerFall(player) && !player->climbing) { // crouch is default action after checking for falling/climbing
			player->crouching = TRUE;
		}
	} else {
		player->crouching = FALSE; // revert to non-crouching state once DOWN is released
	}
	
	if (_keytest(RR_F1) && !grabHolding && !player->beingHeld) { // F1 is now the grab button
		if (!player->grabbing) {
			grabPlayer(player);
		}
		grabHolding = TRUE;
	} else {
		grabHolding = FALSE;
	}
	
	if (player->grabbing && player->playerCounter - player->attackMarker > 50) { // can only grab for a limited time, then release
		player->grabbing = FALSE;
		if (!numHands) {
			player->enemy->beingHeld = FALSE;
		}
	}
	
	updateFrames(player);
	player->playerCounter++;
}

// Make the passed player do a smash attack
static void smashAttack(PLAYER* player) {
	player->attackMarker = player->playerCounter;
	player->smashAttacking = TRUE;
}

// Special attacks
static void standStillSpecial(PLAYER* p) {
	p->attackMarker = p->playerCounter;
	p->specialAttacking = TRUE;
}

static void fireProjectile(PLAYER* p) {
	p->attackMarker = p->playerCounter;
	p->specialAttacking = TRUE;
	if (p->canFire) {
		p->myProjectile = setupProj(p);
	}
}

static void missileSpecial(PLAYER* p) {
	p->specialAttacking = TRUE;
	p->climbing = FALSE;
}

static void grabPlayer(PLAYER* p) {
	p->attackMarker = p->playerCounter;
	PLAYER* temp = pHead;
	while (temp != NULL) {
		if (p != temp && playersCollided(p,temp) && ((p->x < temp->x && p->direction > 0) || (p->x > temp->x && p->direction < 0)) && !temp->onStage && !temp->invincible && !temp->beingHeld) {
			temp->beingHeld = TRUE;
			p->grabbing = TRUE;
			p->enemy = temp;
			temp->percent += 11; // just a dummy "magic" value for now - can add constant/decision processes for this later
			if ((temp->x > p->x && temp->direction > 0) || (temp->x < p->x && temp->direction < 0)) {
				temp->direction = -temp->direction;
			}
			break;
		}
		temp = temp->next;
	}
}

static void dodge(PLAYER* p, DIRECTION d) {
	if (!p->falling && !p->jumpValue) {
		p->invincible = TRUE;
	}
	
	if (canMovePlayer(p,d,0)) {
		p->x+=(d*32);
		p->direction = d;
	} else {
		p->invincible = FALSE;
	}
}

// Player moving functions - check interaction with environment

// Moves the player left
static void movePlayerLeft(PLAYER* player) {
	if (player->direction > 0 && !player->falling) {
		player->moveSpeed = RUNSPEED;
	}
	player->running = TRUE;
	player->direction = LEFT;
	player->climbing = FALSE; // stop special movements when moving laterally
	player->crouching = FALSE;
	player->hanging = FALSE;
	
	// Going up a hill to the left
	if (hill(player->x+x_fg+((characters[player->characterIndex].w)/2)-7-player->moveSpeed-((characters[player->characterIndex].w/2)&1),player->y+y_fg+characters[player->characterIndex].h-16+((characters[player->characterIndex].h)&1),SLOPELEFT) || player->onHillL) {
		if (((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 8 || ((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 9) {
			player->onHillL = TRUE;
		} else {
			if (!player->onHillL) {
				player->x-=2;
			}
		}
		if ((((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 8 || ((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 9) && !hill(player->x+x_fg+((characters[player->characterIndex].w)/2)-6-player->moveSpeed,player->y+y_fg+characters[player->characterIndex].h-16+((characters[player->characterIndex].h)&1),SLOPELEFT)) {
			player->onHillL = FALSE;
		}
		if (player->onHillL) { // not entering hill, so move up the 45 degree angle block
			player->y-=2;
			player->x-=2;
		}
	} else if (hill(player->x+x_fg+((characters[player->characterIndex].w)/2)-7-player->moveSpeed,player->y+y_fg+characters[player->characterIndex].h+((characters[player->characterIndex].h)&1),SLOPERIGHT) || player->onHillR) {
		// Going down a hill to the left	
		if ((((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 8 || ((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 9)) {
			player->onHillR = TRUE;
		}
		if ((((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 8 || ((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 9) && !hill(player->x+x_fg+((characters[player->characterIndex].w)/2)-6-player->moveSpeed,player->y+y_fg+characters[player->characterIndex].h+((characters[player->characterIndex].h)&1),SLOPERIGHT)) {
			player->onHillR = FALSE;
		}
		
		if (player->onHillR) {
			player->x-=2;
			player->y+=2;
		}
	}	
	if (!player->onHillL && !player->onHillR && canMovePlayer(player,LEFT,0)) { // no hills, so lateral movement
		if (!player->type && player->x < 160-32 && x_fg > 0 && !stageTemp->movingLevel) {
			scrollL = TRUE; // can still scroll (not near edge), so move the level as opposed to the player
		}
		else {
			player->x-=player->moveSpeed; // at edge = no scrolling left, so just move the player
		}
	}
	if (!player->jumpValue && !player->falling && !player->climbing && !player->hanging) {
		player->leftCurrent = player->leftCurrent->next; // update to next animation frame in running
	}
}

static void movePlayerRight(PLAYER* player) {
	if (player->direction < 0 && !player->falling) {
		player->moveSpeed = RUNSPEED;
	}
	player->running = TRUE;
	player->direction = RIGHT;
	player->climbing = FALSE;
	player->crouching = FALSE;
	player->hanging = FALSE; // disable special actions since back to running now
	
	// going up a hill to the right
	if (hill(player->x+x_fg+((characters[player->characterIndex].w)/2)+6+player->moveSpeed+2*((characters[player->characterIndex].w/2)&1),player->y+y_fg+(characters[player->characterIndex].h)-16+((characters[player->characterIndex].h)&1),SLOPERIGHT) || player->onHillR) {
		if ((((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 8 || ((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 9)) {
			player->onHillR = TRUE;
		} else {
			if (!player->onHillR) {
				player->x+=2;
			}
		}
		
		if ((((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 8 || ((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 9) && !hill(player->x+x_fg+((characters[player->characterIndex].w)/2)+5+player->moveSpeed,player->y+y_fg+(characters[player->characterIndex].h)-16+((characters[player->characterIndex].h)&1),SLOPERIGHT)) {
			player->onHillR = FALSE; // no more hill terrain
		}
		
		if (player->onHillR) {
			player->y-=2;
			player->x+=2;
		}
	} else if (hill(player->x+x_fg+((characters[player->characterIndex].w)/2)+6+player->moveSpeed,player->y+y_fg+characters[player->characterIndex].h+((characters[player->characterIndex].h)&1),SLOPELEFT) || player->onHillL) {
		// going down a hill to the right
		if ((((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 8 || ((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 9)) {
			player->onHillL = TRUE;
		} else {
			if (!player->onHillL) {
				player->x+=2;
			}
		}
		if ((((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 8 || ((player->x+x_fg+((characters[player->characterIndex].w)/2)-8)&15) == 9) && !hill(player->x+x_fg+((characters[player->characterIndex].w)/2)+5+player->moveSpeed,player->y+y_fg+characters[player->characterIndex].h+((characters[player->characterIndex].h)&1),SLOPELEFT)) {
			player->onHillL = FALSE; // no more hill terrain
		}		
		if (player->onHillL) {
			player->x+=2;
			player->y+=2;
		}
	} else if (!player->onHillL && !player->onHillR && canMovePlayer(player,1,0)) { // no hill - just lateral movement						
		if (!player->type && player->x > 32 && x_fg < (stageTemp->sw*16)-160 && !stageTemp->movingLevel) {
			scrollR = TRUE;
		} else {
			player->x+=player->moveSpeed;
		}
	}
		
	if (!player->jumpValue && !player->falling && !player->climbing && !player->hanging) {
		player->rightCurrent = player->rightCurrent->next; // update to next animation frame
	}
}

// check whether the player can move in a direction (x, y) with their speed and what adjustments may need
// to be made from the environment
static BOOL canMovePlayer(PLAYER* player, DIRECTION dirx, VERT_DIRECTION diry) {
	if (player->x+x_fg+(characters[player->characterIndex].w/2)+7 < 0 || player->x+x_fg+characters[player->characterIndex].w/2-8+((characters[player->characterIndex].w/2)&1) > (stageTemp->sw<<4) || player->y+y_fg <= 0 || player->y+y_fg > (stageTemp->sh*16)) {
		return TRUE; 
	}
	
	// location/sprite centered on a tile
	if (((player->x+x_fg+(characters[player->characterIndex].w)/2-8+((characters[player->characterIndex].w/2)&1))&15) == 0 && (((player->y+y_fg+characters[player->characterIndex].h)-16)&15) == 0) {		
		if ((dirx < 0 && !findTerrain(player,diry,(player->x+x_fg+(characters[player->characterIndex].w)/2-7-player->moveSpeed-(player->invincible*32)-((characters[player->characterIndex].w/2)&1)),player->y+y_fg+((characters[player->characterIndex].h)-16))) ||
		   (dirx > 0 && !findTerrain(player,diry,(player->x+x_fg+(characters[player->characterIndex].w)/2+6+player->moveSpeed+(player->invincible*32)+2*((characters[player->characterIndex].w/2)&1)),player->y+y_fg+((characters[player->characterIndex].h)-16))) ||
		   (diry < 0 && !findTerrain(player,diry,(player->x+x_fg+(characters[player->characterIndex].w)/2),player->y+y_fg+((characters[player->characterIndex].h)-16/*-JUMPSPEED*/))) ||
		   (diry > 0 && !findTerrain(player,diry,(player->x+x_fg+(characters[player->characterIndex].w)/2),player->y+y_fg+characters[player->characterIndex].h+((characters[player->characterIndex].h)&1)+((stageTemp->movingLevel&&scrollU)*2)))) {
			   return FALSE;
		}
	} else { // not centered on a tile
		if ((dirx < 0 && (!findTerrain(player,diry,(player->x+x_fg+(characters[player->characterIndex].w)/2-7-player->moveSpeed-(player->invincible*32)-((characters[player->characterIndex].w/2)&1)),player->y+y_fg+((characters[player->characterIndex].h)-1)) || !findTerrain(player,diry,(player->x+x_fg+(characters[player->characterIndex].w)/2-7-player->moveSpeed-(player->invincible*32)-((characters[player->characterIndex].w/2)&1)),player->y+y_fg+((characters[player->characterIndex].h)-16)))) ||
		   (dirx > 0 && (!findTerrain(player,diry,(player->x+x_fg+(characters[player->characterIndex].w)/2+6+player->moveSpeed+(player->invincible*32)+2*((characters[player->characterIndex].w/2)&1)),player->y+y_fg+((characters[player->characterIndex].h)-1)) || !findTerrain(player,diry,(player->x+x_fg+(characters[player->characterIndex].w)/2+6+player->moveSpeed+(player->invincible*32)+2*((characters[player->characterIndex].w/2)&1)),player->y+y_fg+((characters[player->characterIndex].h)-16)))) ||
		   (diry < 0 && !(findTerrain(player,diry,(player->x+x_fg+(characters[player->characterIndex].w)/2-8),player->y+y_fg+((characters[player->characterIndex].h)-16/*-JUMPSPEED*/)) && findTerrain(player,diry,(player->x+x_fg+(characters[player->characterIndex].w)/2+7),player->y+y_fg+((characters[player->characterIndex].h)-16/*-JUMPSPEED*/)))) ||
		   (diry > 0 && (!findTerrain(player,diry,(player->x+x_fg+(characters[player->characterIndex].w)/2-8/*-player->moveSpeed//((characters[player->characterIndex].w/2)&1)*/),player->y+y_fg+characters[player->characterIndex].h+((characters[player->characterIndex].h)&1)+((stageTemp->movingLevel&&scrollU)*2)) || !findTerrain(player,diry,(player->x+x_fg+(characters[player->characterIndex].w)/2+7),player->y+y_fg+characters[player->characterIndex].h+((characters[player->characterIndex].h)&1)+((stageTemp->movingLevel&&scrollU)*2))))) {
			   return FALSE;
		}
	}
	return TRUE;
}

// Environment functions

// returns the index of the tile at a particular location (txx, tyy) on the tilemap (all tiles are 16x16)
inline unsigned int __attribute__ ((pure)) getTile(int txx, int tyy) {
	return (unsigned int)(*((char*)(stageTemp->fgPlane.matrix + (tyy / 16) * stageTemp->fgPlane.width + (txx / 16))));
}

// findTerrain() function - used for determining whether a player can move for special terrain items
static BOOL findTerrain(PLAYER* player, VERT_DIRECTION diry, int tileX, int tileY) {
	unsigned long tempTileInfo = stageTemp->tileInfo[getTile(tileX,tileY)];
	
	if (tempTileInfo & DOOR) {
		if (((player->y+y_fg+characters[player->characterIndex].h+((characters[player->characterIndex].h)&1))&15) == 0) {
			complete = TRUE;
		}
		return TRUE;
	}
	
	if (tempTileInfo & COLLAPSING) {// if there is a collapsing tile there, increment, then make player fall
		if (++collapse < 250) { // block disappear counter
			return FALSE;
		}
	} else {
		collapse = 0; // seal up the collapsing tiles again to reuse it
	}

	if (tempTileInfo & SOLID) { // adds in partial tiles - trick tiles not used anymore
		if (((tempTileInfo & CLOUD) && !(tempTileInfo & SLOPERIGHT) && !(tempTileInfo & SLOPELEFT) && (diry <= 0 || ((diry > 0) && ((tileY&15) != 0)))) || ((tempTileInfo & PARTIAL) && (diry > 0) && ((player->y+y_fg+characters[player->characterIndex].h+((characters[player->characterIndex].h)&1))&15) < 8)) {
			return TRUE;
		}
		if ((tempTileInfo & PARTIAL) && diry == 0) {
			return TRUE;
		}
		return FALSE;
	}
	if ((tempTileInfo & SLOPELEFT) || (tempTileInfo & SLOPERIGHT)) { // can't come up from under hill blocks - they are solid
		return FALSE;
	}
	/* Tiles that will return true for "walkable" (what returns true)
	 * ladders
	 * empty
	 * trick tiles
	 * cloud tiles on specific cases (every direction except down)
	 * if collapse is past its limit
	 */		
	return TRUE;
}

// Handy terrain shortcut utility functions
// Player falling function
static inline BOOL playerFall(PLAYER* player) {
	return (canMovePlayer(player,0,DOWN));
}

// Player climbing function
static inline BOOL playerClimb(PLAYER* player) {
	return (canMovePlayer(player,0,UP));
}

// Returns if there is a ladder at the desired coordinates
static inline BOOL ladder(int px, int py) {
	return (stageTemp->tileInfo[getTile(px,py)]&LADDER);
}

static inline BOOL hill(int px, int py, unsigned long slope) {
	return ((stageTemp->tileInfo[getTile(px,py)]&SOLID) && (stageTemp->tileInfo[getTile(px,py)]&slope));
}

// Tests if there is a hot tile to make player automatically jump (and deal damage to him/her)
static inline BOOL hotTile(int px, int py) {
	return (stageTemp->tileInfo[getTile(px,py)]&HOTTILE);
}

// Tests if there is a water tile at the desired coordinates
static inline BOOL waterTile(int px, int py) {
	return (stageTemp->tileInfo[getTile(px,py)]&WATERTILE);
}

// determine if two characters collided (match their sprite dimensions via the player w,h fields)
inline BOOL playersCollided(PLAYER* playerA, PLAYER* playerB) {
	if (playerA->x+characters[playerA->characterIndex].w-1 < playerB->x || playerA->x > playerB->x+characters[playerB->characterIndex].w-1 || playerA->y > playerB->y+characters[playerB->characterIndex].h-1 || playerA->y+characters[playerA->characterIndex].h-1 < playerB->y) {
		return FALSE;
	}
	return TRUE;
}

static void checkForFallingPlayer(PLAYER* me) { // if player is not on solid ground, check how fast he/she should fall and update
	int waterGrav = 0; // increased drop to add for water (not really gravity, but you get the idea)
	
	if (!me->onStage && me->jumpValue == 0 && playerFall(me) && !me->climbing && !me->hanging) {
		me->falling = TRUE;
		if (hotTile(me->x+x_fg,me->y+y_fg+characters[me->characterIndex].h)) {
   			me->percent+=20+random(3), me->jumpValue = JUMPVALUE; // now does an auto jump from burn
		}
   		if (waterTile(me->x+x_fg,me->y+y_fg+characters[me->characterIndex].h)) {
   			waterGrav = 2; // increase the amount the player will drop/"sink" by if in water
		}
	} else if (!playerFall(me) && me->falling && hill(me->x+x_fg+(characters[me->characterIndex].w/2)+((characters[me->characterIndex].w/2)&1),me->y+y_fg+characters[me->characterIndex].h+((characters[me->characterIndex].h)&1),SLOPERIGHT)) {
		// hill with right slope below
		disabled = TRUE;
		me->y+= (((me->x+x_fg+(characters[me->characterIndex].w/2))&15)==0)?16:(16-((me->x+x_fg+(characters[me->characterIndex].w/2))&15))-((characters[me->characterIndex].w/2)&1);
		me->falling = FALSE;
		me->onHillR = TRUE;
	} else if (!playerFall(me) && me->falling && hill(me->x+x_fg+(characters[me->characterIndex].w/2)-1-((characters[me->characterIndex].w/2)&1),me->y+y_fg+characters[me->characterIndex].h+((characters[me->characterIndex].h)&1),SLOPELEFT)) {
		// falling onto left sloped hill
		disabled = TRUE;
		me->y+= (((me->x+x_fg+(characters[me->characterIndex].w/2)-1)&15)==15)?16:((((me->x+x_fg+(characters[me->characterIndex].w/2)-2+((characters[me->characterIndex].w/2)&1))&15)));
		me->falling = FALSE;
		me->onHillL = TRUE;
	} else { // solid ground
		me->falling = FALSE, disabled = FALSE;
	}
	
	if (me->falling) { // update player/tilemap scrolling locations as player drops
		if (!me->type && me->y > 32 && y_fg <= (stageTemp->sh*16)-100 && !stageTemp->movingLevel) {
			scrollD = TRUE;
		} else {
			me->y+=2+waterGrav;
			if (waterGrav > 0) {
				me->falling = FALSE;
			}
		}
	}
}

// special function to check for how players fall on auto-moving levels; depends on whether level is actively moving and in which direction
static void checkForMovingLevelFallingPlayer(PLAYER* me) {
	switch (scrollType) {
 		case SCROLL_DOWN:
		me->falling = (playerFall(me) || _keytest(RR_DOWN));
		if (!me->falling) {
			me->y-=2;
		}
 		break;
 		
		case REST_AT_BOTTOM:
		checkForFallingPlayer(me);		
 		break;
 		
		case SCROLL_UP:
 		me->falling = (me->jumpValue == 0 && playerFall(me));
		if (me->falling) {
  			me->y+=2;
		}
 		break;
		
 		case REST_AT_TOP:
 		checkForFallingPlayer(me);
		break;
	}
}

// central function for updating all animation frames
static void updateFrames(PLAYER* p) {
	if (p->leftCurrent == &characters[p->characterIndex].frames[STAND_RIGHT]) { // for switching directions
		p->leftCurrent = &characters[p->characterIndex].frames[STAND_LEFT];
	}
	if (p->rightCurrent == &characters[p->characterIndex].frames[STAND_LEFT]) {
		p->rightCurrent = &characters[p->characterIndex].frames[STAND_RIGHT];
	}
	
	if ((p->playerCounter&31) == 0) { // could add a fatigue factor - breathe faster if the p has larger HP (between 32 and 64)
		p->breathing = !p->breathing;
	}
	
	// no special action under way, so just breathing or standing
	if (!p->running && !p->jumpValue && !p->taunting && !p->climbing && !p->falling) {
		if (p->breathing) {
			p->leftCurrent = &characters[p->characterIndex].frames[BREATHE_LEFT];
			p->rightCurrent = &characters[p->characterIndex].frames[BREATHE_RIGHT];
		} else {
			p->leftCurrent = &characters[p->characterIndex].frames[STAND_LEFT];
			p->rightCurrent = &characters[p->characterIndex].frames[STAND_RIGHT];
		}
	}	
	if (p->smashAttacking) {
		if (p->playerCounter-p->attackMarker > ATTACK_ANIM_DELAY) {
			p->leftCurrent = &characters[p->characterIndex].frames[STAND_LEFT];
			p->rightCurrent = &characters[p->characterIndex].frames[STAND_RIGHT];
			p->smashAttacking = FALSE;
		} else {
			p->leftCurrent = &characters[p->characterIndex].frames[SMASH_LEFT];
			p->rightCurrent = &characters[p->characterIndex].frames[SMASH_RIGHT];
		}
	}
	if (p->specialAttacking) {
		if (p->playerCounter-p->attackMarker > ATTACK_ANIM_DELAY) {
			p->leftCurrent = &characters[p->characterIndex].frames[STAND_LEFT];
			p->rightCurrent = &characters[p->characterIndex].frames[STAND_RIGHT];
			p->specialAttacking = FALSE;
		} else {
			p->leftCurrent = &characters[p->characterIndex].frames[SPECIAL_LEFT];
			p->rightCurrent = &characters[p->characterIndex].frames[SPECIAL_RIGHT];
		}
		if (characters[p->characterIndex].specialType == 2) { // missile-type special attack for this player
			static unsigned int missile = 0;
			if (++missile < 15 && canMovePlayer(p,p->direction,0)) {
				p->specialAttacking = TRUE;
				p->leftCurrent = &characters[p->characterIndex].frames[SPECIAL_LEFT];
				p->rightCurrent = &characters[p->characterIndex].frames[SPECIAL_RIGHT];
				p->x+=(p->direction*4); // recoil from the shot!
			}
			else {
				missile = 0;
				p->specialAttacking = FALSE;		
			}
		}
	}
	
	// miscellaneous moves
	if (p->invincible || p->crouching) {
		p->leftCurrent = &characters[p->characterIndex].frames[CROUCH];
		p->rightCurrent = &characters[p->characterIndex].frames[CROUCH];
	}
	if (p->beingHeld) {
		p->leftCurrent = &characters[p->characterIndex].frames[HURT_LEFT];
		p->rightCurrent = &characters[p->characterIndex].frames[HURT_RIGHT];
	}
	if (p->taunting) { // loop through the player taunting frames based on counter
		if (p->playerCounter - p->attackMarker > 8) {
			p->rightCurrent = &characters[p->characterIndex].frames[TAUNT2];
		}
		if (p->playerCounter - p->attackMarker > 16) {
			p->rightCurrent = &characters[p->characterIndex].frames[STAND_RIGHT];
			p->taunting = FALSE;
		}
	}
}

// current rule-based AI for computer players
void executeNewAI(PLAYER* cpu) {
	/* Order: (similar to player except for auto-detecting enemies)
	 * scanning for enemy
	 * updating usual things like paralysis, jumping, falling
	 * "keytest" - moving* and attacking enemies, etc.
	 * update animation frames
	 */
	
	if (numPlayers > 2 && cpu->enemy != NULL && (cpu->enemy->onStage || cpu->enemy->dead || cpu->enemy->paralyzed)) {
		cpu->enemy = NULL; // reset the enemy that I'm targeting
	}
	if (cpu->enemy == NULL) {
		scanForEnemy(cpu); // find new enemy if I don't have one (what drives the AI players)
	}
	
	// updating physical attributes in level	
	cpu->invincible = FALSE;
	cpu->onStage = FALSE; // ensure that these flags adhere to their counters to not be permanently turned on
	BOOL recovering = FALSE;
	
	int x2 = cpu->xspeed;	
	if (cpu->paralyzed) {
		cpu->onHillL = FALSE;
		cpu->onHillR = FALSE;
		cpu->grabbing = FALSE;
		cpu->enemy->beingHeld = FALSE;
		cpu->smashAttacking = FALSE;
		cpu->specialAttacking = FALSE;
		cpu->skyAttacking = FALSE;
		
		if (cpu->yspeed <= 0 || !canMovePlayer(cpu,0,UP) || (cpu->yspeed < 0 && !canMovePlayer(cpu,0,DOWN))) {
			cpu->paralyzed = FALSE, cpu->xspeed = 0, cpu->yspeed = 0;
		} else {// yspeed > -8, so cpu is still flying up
			cpu->y -= cpu->yspeed;
			cpu->yspeed -= 2; // could even be 4
			
			if (cpu->xspeed < 0) { // paralyzed to the left
				if (canMovePlayer(cpu,LEFT,0)) {
					cpu->x += cpu->xspeed;
					cpu->rightCurrent = &characters[cpu->characterIndex].frames[HURT_LEFT];
					cpu->leftCurrent = &characters[cpu->characterIndex].frames[HURT_LEFT];
				} else {
					cpu->xspeed = -cpu->xspeed;
				}
			} else if (cpu->xspeed > 0) { // paralyzed to the right
				if (canMovePlayer(cpu,RIGHT,0)) {
					cpu->x += cpu->xspeed;
					cpu->rightCurrent = &characters[cpu->characterIndex].frames[HURT_RIGHT];
					cpu->leftCurrent = &characters[cpu->characterIndex].frames[HURT_RIGHT];
				} else {
					cpu->xspeed = -cpu->xspeed;
				}
			}
			return;
		}
	}
	
	if (cpu->jumpValue > 0) { // will perform an automatic jump when placed here (recommended for this action game)
		cpu->y -= JUMPSPEED;
		cpu->jumpValue -= JUMPSPEED;
	} else if (cpu->jumpValue < 0 || !canMovePlayer(cpu,0,UP) || cpu->beingHeld || cpu->y+y_fg < 0) { // prevents player from stuck condition
		cpu->jumpValue = 0;
	
		fallingPlayerChecks[stageTemp->movingLevel](cpu);
		if (!playerFall(cpu)) {
			cpu->numJumps = 0;
			cpu->skyAttacking = FALSE;
		}
	
		if (cpu->currentItem == NULL) { // picking up items
			if (head != NULL && existsItemInRegion(cpu)) {
				ITEM* it = myItem(cpu);
				if (it != NULL && !it->beenUsed) {
					if (it->replenish > 0) {
 						cpu->percent -= it->replenish;
						if (cpu->percent < 1 || cpu->percent > 63000) {
 							cpu->percent = MIN_HP;
						}
 						it->beenUsed = TRUE; // instantly get rid of all the food items
 					} else {
	 					cpu->currentItem = it;
					}
	 			
					if (it->index > 13 && it->index < 16) { // EXPLOSIVE ITEMS!
						cpu->myProjectile->x = cpu->x+8;
						cpu->myProjectile->y = cpu->y+4;
						cpu->myProjectile->e = 0;
						cpu->myProjectile->exploding = TRUE;
	 					cpu->canFire = TRUE;
	 					cpu->xspeed = 4-(random(2)*8);
						cpu->yspeed = ((cpu->percent/18)*2)+2;
						cpu->percent += 50+random(10);
						cpu->paralyzed = TRUE;
	  					cpu->currentItem->beenUsed = TRUE;
						cpu->currentItem->beingHeld = FALSE;
						cpu->currentItem = NULL;
					}
				}
			}
		} else { // use their item and return
			int i = methodVals[cpu->currentItem->index];
			if (i > -1) {
	 			cpu = itemFuncs[i](cpu);
			}		
			cpu->currentItem->beenUsed = TRUE;
			cpu->currentItem->beingHeld = FALSE;
			cpu->currentItem = NULL;
		}
	}
	
	if (cpu->climbing) {
		if (cpu->y > cpu->enemy->y) {
			cpu->y-=2;
		} else if (cpu->y < cpu->enemy->y) {
			cpu->y+=2;
		}
		if (!(cpu->playerCounter&7)) {
			cpu->direction = -cpu->direction;
		}
		if (cpu->direction < 0) {
			cpu->leftCurrent = &characters[cpu->characterIndex].frames[CLIMB_LEFT];
		}
		else {
			cpu->rightCurrent = &characters[cpu->characterIndex].frames[CLIMB_RIGHT];
		}
	}
	
	// this double jumping is specifically for enemy tracking, not normally yet
	if (((cpu->y > cpu->enemy->y && cpu->enemy->y > 0) || recovering) && cpu->numJumps == 1 && cpu->jumpValue > 0 && cpu->jumpValue < 4) {
		cpu->jumpValue = JUMPVALUE;
		cpu->numJumps = 2;
	}
	
	if (cpu->falling) {
		if (!solidTileBeneath(cpu->x+x_fg+(characters[cpu->characterIndex].w/2)-((characters[cpu->characterIndex].w/2)&1),(cpu->y+y_fg+characters[cpu->characterIndex].h-16+((characters[cpu->characterIndex].h)&1)))) {
			// start "panicking"
			recovering = TRUE;
			if (cpu->yspeed < 0) { // horizontal tests
				if (x2 > 0) {
					movePlayerLeft(cpu);
				} else if (x2 < 0) {
					movePlayerRight(cpu);
				}
			}
			
			// vertical tests			
			if (!canMovePlayer(cpu,RIGHT,0) || !canMovePlayer(cpu,LEFT,0)) { // check if can hang - if so, do it and get back to jumping
				cpu->hanging = TRUE;
				cpu->leftCurrent = &characters[cpu->characterIndex].frames[JUMPUP_LEFT];
				cpu->rightCurrent = &characters[cpu->characterIndex].frames[JUMPUP_RIGHT];
			}
						
			if (cpu->hanging) { // jump up from hanging on the ledge to get back into the action
				cpu->jumpValue = JUMPVALUE;
				cpu->hanging = FALSE;
			}
			if (cpu->jumpValue > 0 && cpu->jumpValue < 8) {
				recovering = FALSE;
			}
			
			if (cpu->numJumps < 2 && (waterTile(cpu->x+x_fg+(characters[cpu->characterIndex].w/2)+((characters[cpu->characterIndex].w/2)&1),cpu->y+y_fg+characters[cpu->characterIndex].h+((characters[cpu->characterIndex].h)&1)) || (cpu->y+y_fg > (stageTemp->sh*16)-32))) {
				cpu->jumpValue = JUMPVALUE;
				cpu->numJumps++;
			}
		}
	}
	
	int a0 = cpu->enemy->myProjectile->x; // dodge enemy bullets!
	int a1 = cpu->enemy->myProjectile->y;
	if (!cpu->enemy->canFire && a1 > cpu->y && a1 < cpu->y+characters[cpu->characterIndex].h-1) {
		if (a0 < cpu->x && cpu->enemy->myProjectile->dir > 0) {
			if (xHorizontalDistanceBetween(a0,cpu->x) < 10 && !random(gameDifficulty)) {
				dodge(cpu,LEFT); // dodge to the left
			}
		} else if (a0 > cpu->x && cpu->enemy->myProjectile->dir < 0) {
			if (xHorizontalDistanceBetween(a0+8,cpu->x+characters[cpu->characterIndex].w-1) < 10 && !random(gameDifficulty)) {
				dodge(cpu,RIGHT); // dodge to the right
			}
		}
	}		
	
	// "KEYTEST" STUFF - moving, attacking actions much like a human player would	
	cpu->running = FALSE;
	
	// if enemy y (cog included) is < 0, don't move there - they are probably flying up, and I don't want to go with them
	if (!playersCollided(cpu,cpu->enemy) && !cpu->enemy->paralyzed && !cpu->beingHeld && cpu->enemy->y + y_fg + characters[cpu->enemy->characterIndex].h - 16 + ((characters[cpu->enemy->characterIndex].h) & 1) >= 4) {  //don't go for enemy or food if I am trying to *survive*
		moveToEnemy(cpu);
	}
	
	if (!cpu->invincible && !cpu->smashAttacking && !cpu->specialAttacking && !cpu->skyAttacking && !cpu->grabbing && !cpu->beingHeld) {
		attackEnemy(cpu);
	}
	
	if (cpu->grabbing) {
		if (!random(4)) {
			cpu->enemy->xspeed = 4-(random(2)*8);
			cpu->enemy->yspeed = (((cpu->enemy->percent/18)*2)+2);
			cpu->enemy->paralyzed = TRUE; // if the current CPU is grabbing, then affect its enemy
			cpu->enemy->beingHeld = FALSE;
			cpu->grabbing = FALSE;
		}
		if (cpu->grabbing && cpu->playerCounter - cpu->attackMarker > 50) {
			cpu->grabbing = FALSE; // release other player after certain amount of time
			cpu->enemy->beingHeld = FALSE;
		}
	}	
	
	if (cpu->beingHeld && cpu->enemy->playerCounter-cpu->enemy->attackMarker > 32) {
		cpu->enemy->grabbing = FALSE;
		cpu->beingHeld = FALSE;
	}
	
	updateFrames(cpu);
	cpu->playerCounter++;
}

// find which player is best suited to be the player this AI player targets
static void scanForEnemy(PLAYER* cpu) {
	if (numPlayers == 2) {
		PLAYER* temp = pHead; // if no enemy for two players, just scan for the one that is not like me (unequal teams)
		while (temp != NULL) {
			if (temp->team != cpu->team) {
				cpu->enemy = temp;
				break;
			} // the break speeds it up so don't have to keep checking - just grab the first one
			temp = temp->next;
		}
	} else { // more than 2 players = a little more complex - need to decide how to find an enemy		
		memset(cpu->pointsHolder,0,3*sizeof(unsigned int)); // each other player not on my team is a potential enemy; points are assigned to determine best
		
		PLAYER* temp1 = closestEnemyScan(cpu);
		
		// first criterion: how far away a potential adversary is (in discrete quantities for simplicity)
		unsigned int hyp = ((temp1->x+x_fg-(cpu->x+x_fg))*(temp1->x+x_fg-(cpu->x+x_fg)))+((temp1->y+y_fg-(cpu->y+y_fg))*(temp1->y+y_fg-(cpu->y+y_fg)));
		if (hyp < 2048) {
			cpu->pointsHolder[0] += 20;
		} else if (hyp < 4608) {
			cpu->pointsHolder[0] += 8;
		} else {
			cpu->pointsHolder[0] += 4;
		}
		
		// second criterion: find out who has most fatigue: this helps find easy kills/poaching for players that are about to be smashed off anyway
		PLAYER* temp2 = fatiguedEnemyScan(cpu);
		if (temp2->percent > 200) {
			cpu->pointsHolder[1]+= 6 + random(5); // creates more random behavior in the intermediates
		} else if (temp2->percent > 100) {
			cpu->pointsHolder[1] += 3 + random(3);
		}
		
		// third criterion: found a slacker/"pro" in a stock match! attack lazy players so that they can't just hold onto their lives while watching the rest fight
		if (gameMatchType && myPlayer->numLives == currentProfile.matchLives && myPlayer->percent < 30 && !myPlayer->running && !myPlayer->jumpValue && !myPlayer->falling && !myPlayer->smashAttacking && !myPlayer->specialAttacking) { 
			cpu->pointsHolder[2] += 19 + random(5);
		}
		
		// Data is now gathered about other level entities, so now choose enemy as one with highest score
		unsigned int counter = 0, highest = 0, marker = 0;
		do {
			if (cpu->pointsHolder[counter] > highest) {
				highest = cpu->pointsHolder[counter];
				marker = counter;
			}
			counter++;
		} while (counter < 3);
		
		switch (marker) {
			case PROXIMITY:
			cpu->enemy = temp1;
			break;
			
			case FATIGUE:
			cpu->enemy = temp2;
			break;
			
			case PLAYERONE_LAZINESS:
			cpu->enemy = myPlayer;
			break;
		}
	}
	if (cpu->enemy == NULL) {
		cpu->enemy = myPlayer; // by default, if an enemy has not been assigned/tie, then just assign the human player as simple heuristic
	}
}

// determine if there is a solid tile on which a player can land in order to prevent falls (either full solid or cloud)
// the AI will scan all the way down the column to see if there is one on which to land; if none, then should look for one elsewhere and move laterally
static inline BOOL solidTileBeneath(int x, int y) {
	int myX = x;
	int myY = (y/16)*16;
	int dropY = myY;
		
	do {
		if (((stageTemp->tileInfo[getTile(myX,dropY)])&SOLID) || ((stageTemp->tileInfo[getTile(myX,dropY)])&(SOLID|CLOUD))) {
			return TRUE;
		}
		dropY+=16;
	} while (dropY <= stageTemp->sh);
	return FALSE;
}

// returns the other player on a different team that is closest to the AI
static PLAYER* closestEnemyScan(PLAYER* cpu) {
	PLAYER *temp = pHead, *closest = NULL;
	unsigned int dist = 65000, d;
	while (temp != NULL) {
		if (!temp->dead && !temp->onStage && cpu->team != temp->team && (d = (cpu->x-temp->x)*(cpu->x-temp->x)+(cpu->y-temp->y)*(cpu->y-temp->y)) < dist) {
			closest = temp;
			dist = d;
		}
		temp = temp->next;
	}
	if (closest) {
		return closest;
	}
	return NULL;
}

// returns most fatigued enemy
static PLAYER* fatiguedEnemyScan(PLAYER* cpu) {
	PLAYER *temp = pHead;
	PLAYER *most = NULL;
	unsigned int fatigue = 0;
	
	while (temp != NULL) {
		if (!temp->dead && !temp->onStage && temp->team != cpu->team && temp->percent > fatigue) {
			most = temp, fatigue = temp->percent;
		}
		temp = temp->next;
	}
	if (most) {
		return most;
	}
	return NULL;
}

// Strategy Stuff - go to either enemy if I feel good, or go to food to reload and kill again!
// here, will move toward enemy, if close enough, don't need to really move, just attackEnemy(cpu)!
static void moveToEnemy(PLAYER* cpu) {
	if (stageTemp->movingLevel && !scrollType) {
		return;
	}
	
	if (cpu->x > cpu->enemy->x) { // move to the left
		if (canMovePlayer(cpu,LEFT,0)) { // or if a hill, can move
			// player is below the enemy
			if ((cpu->y+characters[cpu->characterIndex].h-16+((characters[cpu->characterIndex].h)&1)) > (cpu->enemy->y+characters[cpu->enemy->characterIndex].h-16+((characters[cpu->enemy->characterIndex].h)&1))) {
				// ladder testing here
				if (cpu->enemy->climbing) {
					if (ladder(cpu->x+x_fg+((characters[cpu->characterIndex].w)/2)-8+(((characters[cpu->characterIndex].w)/2)&1),cpu->y+y_fg+characters[cpu->characterIndex].h-16)) {
						if (((cpu->x+x_fg+((characters[cpu->characterIndex].w)/2)-8+(((characters[cpu->characterIndex].w)/2)&1))&15) != 0) {
							cpu->x-=2;
						}
						cpu->climbing = TRUE;
					} else {
						if (cpu->numJumps == 0) {
							cpu->direction = LEFT;
							cpu->jumpValue = JUMPVALUE;
							cpu->numJumps = 1;
							cpu->onHillL = FALSE;
							cpu->onHillR = FALSE;
							cpu->climbing = FALSE;
							cpu->hanging = FALSE;
							cpu->leftCurrent = &characters[cpu->characterIndex].frames[JUMPUP_LEFT];
							cpu->rightCurrent = &characters[cpu->characterIndex].frames[JUMPUP_RIGHT];
						}
						movePlayerLeft(cpu);
					}
				} else {
					// get the tile info below the current CPU player
					unsigned long sa = stageTemp->tileInfo[getTile(cpu->x+x_fg+(characters[cpu->characterIndex].w/2)-9-((characters[cpu->characterIndex].w/2)&1),cpu->y+y_fg+characters[cpu->characterIndex].h+((characters[cpu->characterIndex].h)&1))];
					if (!cpu->enemy->falling || sa == EMPTY || sa == WATERTILE || sa == HOTTILE) {
						if (cpu->numJumps == 0) {
							cpu->direction = LEFT;
							cpu->jumpValue = JUMPVALUE;
							cpu->numJumps = 1;
							cpu->onHillL = FALSE;
							cpu->onHillR = FALSE;
							cpu->climbing = FALSE;
							cpu->hanging = FALSE;
							cpu->leftCurrent = &characters[cpu->characterIndex].frames[JUMPUP_LEFT];
							cpu->rightCurrent = &characters[cpu->characterIndex].frames[JUMPUP_RIGHT];
						}
						movePlayerLeft(cpu);
					}
				}
			} else if ((cpu->y+characters[cpu->characterIndex].h-16+((characters[cpu->characterIndex].h)&1)) < (cpu->enemy->y+characters[cpu->enemy->characterIndex].h-16+((characters[cpu->enemy->characterIndex].h)&1))) {
				// current CPU player is above the enemy
				if (!playerFall(cpu)) { // player cannot move down, so need to either move manually or find alternate route
					unsigned long sd = stageTemp->tileInfo[getTile(cpu->x+x_fg+(characters[cpu->characterIndex].w/2)-8+((characters[cpu->characterIndex].w/2)&1),cpu->y+y_fg+characters[cpu->characterIndex].h+((characters[cpu->characterIndex].h)&1))];
					unsigned long sd2 = stageTemp->tileInfo[getTile(cpu->x+x_fg+(characters[cpu->characterIndex].w/2)+7+((characters[cpu->characterIndex].w/2)&1),cpu->y+y_fg+characters[cpu->characterIndex].h+((characters[cpu->characterIndex].h)&1))];
					if (sd == (SOLID | CLOUD) || sd2 == (SOLID | CLOUD)) {
						cpu->y+=4;
						cpu->onHillL = FALSE;
						cpu->onHillR = FALSE;
					} else {
						movePlayerLeft(cpu);
					}
				} else {
					movePlayerLeft(cpu);
				}
			} else {
				movePlayerLeft(cpu);
			}
		} else { // block in the way, so need to jump over it
			if (cpu->numJumps == 0) {
				cpu->direction = LEFT;
				cpu->jumpValue = JUMPVALUE;
				cpu->numJumps = 1;
				cpu->onHillL = FALSE;
				cpu->onHillR = FALSE;
				cpu->climbing = FALSE;
				cpu->hanging = FALSE;
				cpu->leftCurrent = &characters[cpu->characterIndex].frames[JUMPUP_LEFT];
				cpu->rightCurrent = &characters[cpu->characterIndex].frames[JUMPUP_RIGHT];
			}
		}
	} else if (cpu->x == cpu->enemy->x) {
		// current CPU player is below the enemy
		if ((cpu->y+characters[cpu->characterIndex].h-16+((characters[cpu->characterIndex].h)&1)) > (cpu->enemy->y+characters[cpu->enemy->characterIndex].h-16+((characters[cpu->enemy->characterIndex].h)&1))) {
			if (cpu->numJumps == 0) {
				cpu->jumpValue = JUMPVALUE;
				cpu->numJumps = 1;
				cpu->onHillL = FALSE;
				cpu->onHillR = FALSE;
				cpu->climbing = FALSE;
				cpu->hanging = FALSE;
				cpu->leftCurrent = &characters[cpu->characterIndex].frames[JUMPUP_LEFT];
				cpu->rightCurrent = &characters[cpu->characterIndex].frames[JUMPUP_RIGHT];
			}
		} else if ((cpu->y+characters[cpu->characterIndex].h-16+((characters[cpu->characterIndex].h)&1)) < (cpu->enemy->y+characters[cpu->enemy->characterIndex].h-16+((characters[cpu->enemy->characterIndex].h)&1))) {
			if (!playerFall(cpu)) {
				// scan the two tiles beneath the player
				unsigned long sd = stageTemp->tileInfo[getTile(cpu->x+x_fg+(characters[cpu->characterIndex].w/2)-8+((characters[cpu->characterIndex].w/2)&1),cpu->y+y_fg+characters[cpu->characterIndex].h+((characters[cpu->characterIndex].h)&1))];
				unsigned long sd2 = stageTemp->tileInfo[getTile(cpu->x+x_fg+(characters[cpu->characterIndex].w/2)+7+((characters[cpu->characterIndex].w/2)&1),cpu->y+y_fg+characters[cpu->characterIndex].h+((characters[cpu->characterIndex].h)&1))];
				if (sd == (SOLID | CLOUD) || sd2 == (SOLID | CLOUD)) {
					cpu->y+=4, cpu->onHillL = FALSE, cpu->onHillR = FALSE;
				} else {
					movePlayerRight(cpu);
				}
			}
		}
	} else { // move to the right
		if (canMovePlayer(cpu,RIGHT,0)) { // or if a hill, can move
			if ((cpu->y+characters[cpu->characterIndex].h-16+((characters[cpu->characterIndex].h)&1)) > (cpu->enemy->y+characters[cpu->enemy->characterIndex].h-16+((characters[cpu->enemy->characterIndex].h)&1))) {
				// ladder testing here
				if (cpu->enemy->climbing) {
					if (ladder(cpu->x+x_fg+((characters[cpu->characterIndex].w)/2)+7,cpu->y+y_fg+characters[cpu->characterIndex].h-16)) {
						if (((cpu->x+x_fg+((characters[cpu->characterIndex].w)/2)-8+(((characters[cpu->characterIndex].w)/2)&1))&15) != 0) {
							cpu->x+=2;
						}
						cpu->climbing = TRUE;
					} else {
						if (cpu->numJumps == 0) {
							cpu->direction = RIGHT;
							cpu->jumpValue = JUMPVALUE;
							cpu->numJumps = 1;
							cpu->onHillL = FALSE;
							cpu->onHillR = FALSE;
							cpu->climbing = FALSE;
							cpu->hanging = FALSE;
							cpu->leftCurrent = &characters[cpu->characterIndex].frames[JUMPUP_LEFT];
							cpu->rightCurrent = &characters[cpu->characterIndex].frames[JUMPUP_RIGHT];
						}
						movePlayerRight(cpu);
					}
				} else {
					unsigned long sa = stageTemp->tileInfo[getTile(cpu->x+x_fg+(characters[cpu->characterIndex].w/2)+8+((characters[cpu->characterIndex].w/2)&1),cpu->y+y_fg+characters[cpu->characterIndex].h+((characters[cpu->characterIndex].h)&1))];
					if (!cpu->enemy->falling || sa == EMPTY || sa == WATERTILE || sa == HOTTILE) {
						if (cpu->numJumps == 0) {
							cpu->direction = RIGHT;
							cpu->jumpValue = JUMPVALUE;
							cpu->numJumps = 1;
							cpu->onHillL = FALSE;
							cpu->onHillR = FALSE;
							cpu->climbing = FALSE;
							cpu->hanging = FALSE;
							cpu->leftCurrent = &characters[cpu->characterIndex].frames[JUMPUP_LEFT];
							cpu->rightCurrent = &characters[cpu->characterIndex].frames[JUMPUP_RIGHT];
						}
						movePlayerRight(cpu);
					}
				}
			} else if ((cpu->y+characters[cpu->characterIndex].h-16+((characters[cpu->characterIndex].h)&1)) < (cpu->enemy->y+characters[cpu->enemy->characterIndex].h-16+((characters[cpu->enemy->characterIndex].h)&1))) {
				// player is above the enemy
				if (!playerFall(cpu)) { // player cannot move down, so need to either move manually or find alternate route
					unsigned long sd = stageTemp->tileInfo[getTile(cpu->x+x_fg+(characters[cpu->characterIndex].w/2)-8+((characters[cpu->characterIndex].w/2)&1),cpu->y+y_fg+characters[cpu->characterIndex].h+((characters[cpu->characterIndex].h)&1))];
					unsigned long sd2 = stageTemp->tileInfo[getTile(cpu->x+x_fg+(characters[cpu->characterIndex].w/2)+7+((characters[cpu->characterIndex].w/2)&1),cpu->y+y_fg+characters[cpu->characterIndex].h+((characters[cpu->characterIndex].h)&1))];
					if (sd == (SOLID | CLOUD) || sd2 == (SOLID | CLOUD)) {
						cpu->y+=4;
					} else {
						movePlayerRight(cpu);
					}
				} else {
					movePlayerRight(cpu);
				}
			} else {
				movePlayerRight(cpu);
			}
		} else { // jump over solid block in the way
			if (cpu->numJumps == 0) {
				cpu->direction = RIGHT;
				cpu->jumpValue = JUMPVALUE;
				cpu->numJumps = 1;
				cpu->onHillL = FALSE;
				cpu->onHillR = FALSE;
				cpu->climbing = FALSE;
				cpu->hanging = FALSE;
				cpu->leftCurrent = &characters[cpu->characterIndex].frames[JUMPUP_LEFT];
				cpu->rightCurrent = &characters[cpu->characterIndex].frames[JUMPUP_RIGHT];
			}
		}
	}
}

static void attackEnemy(PLAYER* cpu) {
	if (cpu->falling) { // if falling, do a sky attack
		if (cpu->enemy->y > cpu->y && horizontalDistanceBetween(cpu,cpu->enemy) < 16 && !random(gameDifficulty)) {
			cpu->skyAttacking = TRUE;
			cpu->leftCurrent = &characters[cpu->characterIndex].frames[SKY_LEFT];
			cpu->rightCurrent = &characters[cpu->characterIndex].frames[SKY_RIGHT];
			
			if (cpu->x+(characters[cpu->characterIndex].w/2)-8 < cpu->enemy->x+(characters[cpu->enemy->characterIndex].w/2)-8) {
				cpu->direction = RIGHT;
			} else {
				cpu->direction = LEFT;
			}
		}
	}
	
	// AI sees the player is about to hit them (collisions tested after me so it gives a fair playing field), so try to dodge
	if (playersCollided(cpu,cpu->enemy) && !cpu->onStage && (cpu->enemy->smashAttacking || cpu->enemy->specialAttacking || cpu->enemy->skyAttacking) && !random(gameDifficulty)) { 
		if (!random(2)) {
			if (cpu->x > cpu->enemy->x && cpu->enemy->direction > 0) {
				dodge(cpu,LEFT);
				cpu->direction = -cpu->direction;
			} else if (cpu->x < cpu->enemy->x && cpu->enemy->direction < 0) {
				dodge(cpu,RIGHT);
				cpu->direction = -cpu->direction;
			}
		} else {
			grabPlayer(cpu);
		}
	}
	// very important to make smashing only occur once for p1 and then turn it off instantly so doesn't last through frame, because cpu can keep changing
	// directions and dodging
	
	if (playersCollided(cpu,cpu->enemy) && !random(gameDifficulty)) {
		if (cpu->x+(characters[cpu->characterIndex].w/2)-8 < cpu->enemy->x+(characters[cpu->enemy->characterIndex].w/2)-8 && cpu->direction < 0) {
			cpu->direction = RIGHT;
		}
		if (cpu->x+(characters[cpu->characterIndex].w/2)+7 >= cpu->enemy->x+(characters[cpu->enemy->characterIndex].w/2)+7 && cpu->direction > 0) {
			cpu->direction = LEFT;
		}
		
		unsigned int a = random(24); // random number to help determine which type of attack in general to perform (relatively even, except for special)
		if (a < 9) {
			grabPlayer(cpu);
		} else if (a < 18) {
			smashAttack(cpu); // this is hardcoded for elite difficulty, it is tough to even hit them
		} else {
			specialAttacks[characters[cpu->characterIndex].specialType](cpu);
		}
	}
}

// this one takes into account the "center of gravity" of each character (the center of the sprite)
static int horizontalDistanceBetween(PLAYER* a, PLAYER* b) {
	return abs(((a->x+(characters[a->characterIndex].w/2)-8+(((characters[a->characterIndex].w)/2)&1))-(b->x+(characters[b->characterIndex].w/2)-8+(((characters[b->characterIndex].w)/2)&1))));
}

static int xHorizontalDistanceBetween(int x1, int x2) {
	return abs(x1-x2);
}

// Initializes a player's projectile
PROJECTILE* setupProj(PLAYER* player) { 
	player->myProjectile->x = (player->direction>0)?player->x+characters[player->characterIndex].w:player->x-8;
	player->myProjectile->y = player->y+4;
	player->myProjectile->dir = player->direction;
	player->myProjectile->e = 0;
	player->myProjectile->exploding = FALSE;
	player->myProjectile->data = bullet;
	player->canFire = FALSE;
	return player->myProjectile;
}

// Projectile functions
BOOL moveProjectile(PROJECTILE* projectile, DIRECTION direction) {	
	if ((direction < 0 && !projLook(projectile->x+x_fg-1,projectile->y+y_fg)) || (direction > 0 && !projLook(projectile->x+x_fg+8,projectile->y+y_fg))) {
		return FALSE;
	}
	return TRUE;
}

void explode(PROJECTILE* projectile) {
	if (12-projectile->e > 8) {
		GrayClipSprite16_TRANW_R(projectile->x,projectile->y,16,explosion1,explosion1+16,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
	} else if (12-projectile->e > 4) {
		GrayClipSprite32_TRANW_R(projectile->x-8,projectile->y-8,32,explosion2,explosion2+32,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
	} else if (12-projectile->e > 0) {
		GrayClipSprite32_TRANW_R(projectile->x-8,projectile->y-8,32,explosion3,explosion3+32,GrayGetPlane(LIGHT_PLANE),GrayGetPlane(DARK_PLANE));
	} else {
		projectile->exploding = FALSE;
		projectile->e = 0;
	}
	projectile->e++; // projectile explosion counter
}

// Find terrain method for projectiles - determine whether can keep proceeding or whether hit a block
static inline BOOL projLook(int tileX, int tileY) {
	return (!((stageTemp->tileInfo[getTile(tileX,tileY)])&SOLID));
}

// End of Source File