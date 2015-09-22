// Twilight Legion for TI-89(Titanium), TI-92(+), Voyage 200
// Header File - playerdata.h
// Michael Hergenrader
// Compiled with TIGCC 0.96 Beta 8
// Please see README for license/disclaimer information. In short, please feel free to use code you see here,
// and any credit to me would be greatly appreciated! :-)
// Copyright 2005-2010 Michael Hergenrader

// Some miscellaneous character-level details. These details allow different character sizes
// despite using the same dimensions for character sprites (32x32), allowing interaction with
// the levels to a finer detail.

#ifndef PLAYERS_H
#define PLAYERS_H

const int characterWidths[24] = {
	26, 24, 32, 24, 16, 22, 20, 20, 32, 32, 22, 22, 16, 16, 26, 22, 18, 22, 26, 20, 22, 24, 20, 20
};

const int characterHeights[24] = {
	26, 26, 32, 28, 28, 31, 25, 25, 32, 32, 22, 21, 24, 20, 26, 22, 18, 22, 26, 23, 22, 25, 22, 26
};

// What type of special attack each player possesses
const unsigned int charSpecialAttacks[24] = {
	0,  1,  0,  0,  2,  0,  2,  0,  0,  0,  2,  1,  2,  2,  0,  0,  1,  2,  2,  1,  0,  2,  0,  2
};

// The sprite pointer offsets in the external data file (one array used for all three data files),
// hence the reset to zero twice in this array.
const int offset[24] = {
	0, 1794, 3588, 5796, 7728, 9660, 11799, 13524,   0, 2208, 4416, 5934, 7383, 9039, 10419, 12213,   0, 1242, 2760, 4554, 6141, 7659, 9384, 10902
};

// The franchise/"team" backgrounds that belong to each charcter (see huddata.h)
const unsigned int hudIndexes[24] = {
	10, 10, 5, 1, 5, 10, 2, 2, 7, 9, 8, 7, 5, 5, 6, 8, 3, 5, 6, 4, 11, 5, 0, 7
};

// Now new characters and ranked in order alphabetically
const char* characterNames[25] = {
	"A. Azzurro", // Admiral Azzurro, a custom character of mine
	"Axion", // Axion, a custom character of mine, much like the Halo armor
	"Bowser",
	"Capt. Falcon",
	"Dr. Mario",
	"Don Dorado", // Don Dorado, a custom character of mine
	"Falco",
	"Fox",
	"Ganondorf",
	"King Boo",
	"Kirby",
	"Link",
	"Luigi",
	"Mario",
	"Marth",
	"Metaknight",
	"Game n Watch", // In this order because "Mr. Game n' Watch" is the official title, but full/official name too long
	"Peach",
	"Roy",
	"Samus",
	"Sonic",
	"Wario",
	"Yoshi",
	"Zelda",
	"Select",
};

// End of Header File

#endif