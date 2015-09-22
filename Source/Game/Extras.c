// Twilight Legion for TI-89(Titanium), TI-92(+), Voyage 200
// C Source File - Extras.c
// Michael Hergenrader
// Compiled with TIGCC 0.96 Beta 8
// Please see README for license/disclaimer information. In short, please feel free to use code you see here,
// and any credit to me would be greatly appreciated! :-)
// Copyright 2005-2010 Michael Hergenrader

// Small utility file - was originally much larger, but has been reduced. May decide to get rid
// of this file in the future to consolidate.

#include <tigcclib.h>
#include "headers.h"

// Centers the text of a string with certain font width - saves 6 bytes by declaring pure
inline unsigned int __attribute__ ((pure)) HCENTER(const char* const str, int width) {
	return (160 - (strlen(str) * width)) >> 1;
}

// End of Source File