// Twilight Legion for TI-89(Titanium), TI-92(+), Voyage 200
// C Source File - Items.c
// Michael Hergenrader
// Compiled with TIGCC 0.96 Beta 8
// Please see README for license/disclaimer information. In short, please feel free to use code you see here,
// and any credit to me would be greatly appreciated! :-)
// Copyright 2005-2010 Michael Hergenrader

#include <tigcclib.h>
#include "headers.h" // All items are perfect and bug free

// Useful macro to determine when two rectangles of size (k, l) and (m, n) are overlapping
#define OBJECTS_COLLIDE_2HW(x0, y0, x1, y1, w0, w1, h0, h1) (((abs((x1) - (x0))) <= (min((w0), (w1)))) && ((abs((y1) - (y0))) <= (min((h0), (h1)))))

// Replenish values for healing food items (8 possible items; subtract the HP down to the difference or zero, whichever is higher)
unsigned int rVals[8] = {
	100, 50, 48, 36, 24, 12, 10, 10
};

static inline BOOL moveItem(ITEM* item);

// Item methods

// Adds an item to the front of the linked list - created dynamically each time
inline void addItem(unsigned int index) {
	ITEM* newItem = malloc(sizeof(ITEM));
	
	// Choose a random position within the screen to drop an item from the sky
	// Items under index 5 are larger items
	newItem->x = (index < 5) ? random((stageTemp->sw << 4) - 16) : random((stageTemp->sw << 4) - 8);
	newItem->y = 0;
	newItem->h = (index < 5) ? 16 : 8; // specify the dimensions of the item - big or small
	
	newItem->beingHeld = FALSE;
	newItem->beenUsed = FALSE;
	
	newItem->replenish = (index < 18) ? 0 : rVals[index-18]; // if a healing item, assign its HP replenish value
	newItem->index = index;
	
	if (index < 5) {
		newItem->data = dataptr->bigitems[index]; // attach the proper sprites for rendering
	} else {
		newItem->data = dataptr->smallitems[index-ITEM_OFFSET];	
	}	
	newItem->next = NULL;
	
	if (head != NULL) {
		newItem->next = head;
	}
	head = newItem;
}

// Draws all items in the level each iteration
void drawAllItems(void *dest) {
	ITEM* temp = head;
	while (temp != NULL) {
		if (!temp->beenUsed && !temp->beingHeld) {
			if (scrollR)	{
				temp->x -= myPlayer->moveSpeed;
			}
			if (scrollL) {
				temp->x += myPlayer->moveSpeed;
			}
			if (scrollD) {
				temp->y -= 2;
			}
			if (scrollU) {
				temp->y += 2; // shift the drawing position of the items as scrolling occurs
			}
			if (moveItem(temp)) { // if item is falling from the sky, keep dropping it
				temp->y += 2;
			}
			
			if (temp->h > 8) {
				GrayClipISprite16_TRANW_R(temp->x, temp->y, temp->h, temp->data, dest, dest + LCD_SIZE); // draw the item based on its size
			} else {
				GrayClipISprite8_TRANW_R(temp->x, temp->y, temp->h, temp->data, dest, dest + LCD_SIZE);
			}
		}
		temp = temp->next;
	}
}

// Returns if an item can be moved or whether it has hit a ground tile
inline BOOL moveItem(ITEM* item) {
	return (!((stageTemp->tileInfo[getTile(item->x + x_fg, item->y + y_fg + item->h)]) & SOLID));
}

// Destroys and frees the item linked list in a current level
void freeItemList(ITEM** head) {
	ITEM *temp = *head, *next;
	while (temp != NULL) {
		next = temp->next;
		free(temp);
		temp = next;
	}
	*head = NULL; // assign the front of the list to the empty list
}

//==========================================================================================================================================

// Returns the item to pick up that is in the region (and hasn't already been used)
ITEM* myItem(PLAYER* p) {
	ITEM* temp = head;
	while (temp != NULL) {
		if (!temp->beenUsed && OBJECTS_COLLIDE_2HW(temp->x + x_fg, temp->y + y_fg, p->x + x_fg + ((characters[p->characterIndex].w) / 2) - 8, p->y + y_fg + characters[p->characterIndex].h - 16 + ((characters[p->characterIndex].h) & 1), temp->h, 16, temp->h, 16)) {
			return temp;
		}
		temp = temp->next;
	}
	return NULL;
}

// Checks for if an item is in the reach of a player
BOOL existsItemInRegion(PLAYER* p) {
	ITEM* temp = head;
	while (temp != NULL) {
		if (OBJECTS_COLLIDE_2HW(temp->x + x_fg, temp->y + y_fg, p->x + x_fg + ((characters[p->characterIndex].w) / 2) - 8, p->y + y_fg + characters[p->characterIndex].h - 16 + ((characters[p->characterIndex].h) & 1), temp->h, 16, temp->h, 16)) {
			return TRUE;
		}
		temp = temp->next;
	}
	return FALSE;
}

// Item Helper Functions:

// Player has a free projectile to do damage with
PLAYER* throw(PLAYER* p) {
	p->myProjectile = setupProj(p);
	return p;
}

// Gives player more power for smash attacks (currently throughout entire level; no decrease as of yet for temporary boosts)
PLAYER* smashBoost(PLAYER* p) {
	p->power += 5;
	return p;
}

// Temporarily makes the player metal to increase their defensive stat (size)
PLAYER* metalInit(PLAYER* p) {
	p->size += 5;
	p->metal = TRUE;
	return p;
}

// Player temporarily turns invisible (small text indicator still remains on top of the player)
PLAYER* cloaking(PLAYER* p) {
	p->cloaked = TRUE;
	return p;
}

// Player temporarily turns invincible
PLAYER* makeInvincible(PLAYER* p) {
	p->invincible = TRUE;
	return p;
}

// End of Source File