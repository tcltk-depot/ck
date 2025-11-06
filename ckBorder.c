/*
 * ckBorder.c --
 *
 *	Manage borders by using alternate character set.
 *
 * Copyright (c) 1995 Christian Werner
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "ckPort.h"
#include "ck.h"

/*
 * Variables used in this module.
 */

static Tcl_HashTable gCharTable;	/* Maps gChar names to values. */
static int initialized = 0;		/* gCharTable initialized. */


/*
 *------------------------------------------------------------------------
 *
 * Ck_GetGChar --
 *
 *	Return curses ACS character given string.
 *
 *------------------------------------------------------------------------
 */

int
Ck_GetGChar(
    Tcl_Interp *interp,
    const char *name,
    long *gchar)
{
    Tcl_HashEntry *hPtr;

    if (!initialized) {
	int new;

#ifdef USE_NCURSES
	/*
	 * Problem: varying ABIs between ncurses versions on LP64.
	 * Sometimes acs_map is an unsigned long array, sometimes
	 * an unsigned int. Instead of using the ACS_* macros,
	 * we build the map from the tigetstr("acsc") information.
	 */
	uintptr_t local_map[128];
	char *acsc = tigetstr("acsc");
	int i;

	for (i = 0; i < 128; i++)
	    local_map[i] = 0;

	local_map[(int)'l'] = '+';  /* upper left corner */
	local_map[(int)'m'] = '+';  /* lower left corner */
	local_map[(int)'k'] = '+';  /* upper right corner */
	local_map[(int)'j'] = '+';  /* lower right corner */
	local_map[(int)'u'] = '+';  /* tee pointing left */
	local_map[(int)'t'] = '+';  /* tee pointing right */
	local_map[(int)'v'] = '+';  /* tee pointing up */
	local_map[(int)'w'] = '+';  /* tee pointing down */
	local_map[(int)'q'] = '-';  /* horizontal line */
	local_map[(int)'x'] = '|';  /* vertical line */
	local_map[(int)'n'] = '+';  /* large plus or crossover */
	local_map[(int)'o'] = '~';  /* scan line 1 */
	local_map[(int)'s'] = '_';  /* scan line 9 */
	local_map[(int)'`'] = '+';  /* diamond */
	local_map[(int)'a'] = ':';  /* checker board (stipple) */
	local_map[(int)'f'] = '\''; /* degree symbol */
	local_map[(int)'g'] = '#';  /* plus/minus */
	local_map[(int)'~'] = 'o';  /* bullet */
	local_map[(int)','] = '<';  /* arrow pointing left */
	local_map[(int)'+'] = '>';  /* arrow pointing right */
	local_map[(int)'.'] = 'v';  /* arrow pointing down */
	local_map[(int)'-'] = '^';  /* arrow pointing up */
	local_map[(int)'h'] = '#';  /* board of squares */
	local_map[(int)'i'] = '#';  /* lantern symbol */
	local_map[(int)'0'] = '#';  /* solid square block */
	local_map[(int)'p'] = '-';  /* scan line 3 */
	local_map[(int)'r'] = '-';  /* scan line 7 */
	local_map[(int)'y'] = '<';  /* less-than-or-equal-to */
	local_map[(int)'z'] = '>';  /* greater-than-or-equal-to */
	local_map[(int)'{'] = '*';  /* greek pi */
	local_map[(int)'|'] = '!';  /* not-equal */
	local_map[(int)'}'] = 'f';  /* pound-sterling symbol */
	local_map[(int)'L'] = '+';  /* upper left corner */
	local_map[(int)'M'] = '+';  /* lower left corner */
	local_map[(int)'K'] = '+';  /* upper right corner */
	local_map[(int)'J'] = '+';  /* lower right corner */
	local_map[(int)'T'] = '+';  /* tee pointing left */
	local_map[(int)'U'] = '+';  /* tee pointing right */
	local_map[(int)'V'] = '+';  /* tee pointing up */
	local_map[(int)'W'] = '+';  /* tee pointing down */
	local_map[(int)'Q'] = '-';  /* horizontal line */
	local_map[(int)'X'] = '|';  /* vertical line */
	local_map[(int)'N'] = '+';  /* large plus or crossover */
	local_map[(int)'C'] = '+';  /* upper left corner */
	local_map[(int)'D'] = '+';  /* lower left corner */
	local_map[(int)'B'] = '+';  /* upper right corner */
	local_map[(int)'A'] = '+';  /* lower right corner */
	local_map[(int)'G'] = '+';  /* tee pointing left */
	local_map[(int)'F'] = '+';  /* tee pointing right */
	local_map[(int)'H'] = '+';  /* tee pointing up */
	local_map[(int)'I'] = '+';  /* tee pointing down */
	local_map[(int)'R'] = '-';  /* horizontal line */
	local_map[(int)'Y'] = '|';  /* vertical line */
	local_map[(int)'E'] = '+';  /* large plus or crossover */

	if (acsc != NULL && acsc != (char *) -1) {
	    for (i = 0; acsc[i] != 0; i += 2) {
		if (acsc[i] < 0 || acsc[i] >= 128)
		    continue;
		local_map[(int)acsc[i]] = (acsc[i+1] & 0xFF) | A_ALTCHARSET;
	    }
	}

	Tcl_InitHashTable(&gCharTable, TCL_STRING_KEYS);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "ulcorner", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'l']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "urcorner", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'k']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "llcorner", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'m']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "lrcorner", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'j']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "rtee", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'u']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "ltee", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'t']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "btee", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'v']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "ttee", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'w']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "hline", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'q']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "vline", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'x']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "plus", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'n']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "s1", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'o']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "s9", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'s']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "diamond", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'`']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "ckboard", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'a']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "degree", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'f']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "plminus", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'g']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "bullet", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'~']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "larrow", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)',']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "rarrow", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'+']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "darrow", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'.']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "uarrow", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'-']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "board", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'h']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "lantern", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'i']);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "block", &new);
	Tcl_SetHashValue(hPtr, (ClientData) local_map[(int)'0']);
#else
	Tcl_InitHashTable(&gCharTable, TCL_STRING_KEYS);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "ulcorner", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_ULCORNER);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "urcorner", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_URCORNER);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "llcorner", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_LLCORNER);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "lrcorner", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_LRCORNER);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "rtee", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_RTEE);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "ltee", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_LTEE);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "btee", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_BTEE);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "ttee", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_TTEE);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "hline", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_HLINE);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "vline", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_VLINE);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "plus", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_PLUS);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "s1", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_S1);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "s9", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_S9);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "diamond", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_DIAMOND);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "ckboard", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_CKBOARD);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "degree", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_DEGREE);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "plminus", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_PLMINUS);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "bullet", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_BULLET);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "larrow", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_LARROW);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "rarrow", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_RARROW);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "darrow", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_DARROW);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "uarrow", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_UARROW);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "board", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_BOARD);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "lantern", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_LANTERN);
	hPtr = Tcl_CreateHashEntry(&gCharTable, "block", &new);
	Tcl_SetHashValue(hPtr, (ClientData) ACS_BLOCK);
#endif
	initialized = 1;
    }

    hPtr = Tcl_FindHashEntry(&gCharTable, name);
    if (hPtr == NULL) {
	if (interp != NULL)
	    Tcl_AppendResult(interp,
		"bad gchar \"", name, "\"", (char *) NULL);
	return TCL_ERROR;
    }
    if (gchar != NULL)
	*gchar = (long) Tcl_GetHashValue(hPtr);
    return TCL_OK;
}

/*
 *------------------------------------------------------------------------
 *
 * Ck_SetGChar --
 *
 *	Modify ACS mapping.
 *
 *------------------------------------------------------------------------
 */

int
Ck_SetGChar(
    Tcl_Interp *interp,
    const char *name,
    long gchar)
{
    Tcl_HashEntry *hPtr;

    if (!initialized)
	Ck_GetGChar(interp, "ulcorner", NULL);
    hPtr = Tcl_FindHashEntry(&gCharTable, name);
    if (hPtr == NULL) {
	Tcl_AppendResult(interp, "bad gchar \"", name, "\"", (char *) NULL);
	return TCL_ERROR;
    }
    Tcl_SetHashValue(hPtr, (ClientData) gchar);
    return TCL_OK;
}

/*
 *------------------------------------------------------------------------
 *
 * Ck_GetBorder --
 *
 *	Create border from string.
 *
 *------------------------------------------------------------------------
 */

CkBorder *
Ck_GetBorder(
    Tcl_Interp *interp,
    const char *string)
{
    int i, largc;
    long bchar[8];
    const char **largv;
    CkBorder *borderPtr;

    if (Tcl_SplitList(interp, string, &largc, &largv) != TCL_OK)
	return NULL;
    if (largc != 1 && largc != 3 && largc != 6 && largc != 8) {
	ckfree((char *) largv);
	Tcl_AppendResult(interp, "illegal number of box characters",
	    (char *) NULL);
	return NULL;
    }
    for (i = 0; i < sizeof (bchar) / sizeof (bchar[0]); i++)
	bchar[i] = ' ';
    for (i = 0; i < largc; i++) {
	if (strlen(largv[i]) == 1)
	    bchar[i] = (unsigned char) largv[i][0];
	else if (Ck_GetGChar(interp, largv[i], &bchar[i]) != TCL_OK) {
	    ckfree((char *) largv);
	    return NULL;
	}
    }
    if (largc == 1) {
    	for (i = 1; i < sizeof (bchar) / sizeof (bchar[0]); i++)
	    bchar[i] = bchar[0];
    } else if (largc == 3) {
	bchar[3] = bchar[7] = bchar[2];
	bchar[2] = bchar[4] = bchar[6] = bchar[0];
	bchar[5] = bchar[1];
    } else if (largc == 6) {
	bchar[6] = bchar[5];
	bchar[5] = bchar[1];
	bchar[7] = bchar[3];
    }
    ckfree((char *) largv);
    borderPtr = (CkBorder *) ckalloc(sizeof (CkBorder));
    memset(borderPtr, 0, sizeof (CkBorder));
    for (i = 0; i < 8; i++)
	borderPtr->gchar[i] = bchar[i];
    borderPtr->name = ckalloc(strlen(string) + 1);
    strcpy(borderPtr->name, string);
    return borderPtr;
}

/*
 *------------------------------------------------------------------------
 *
 * Ck_FreeBorder --
 *
 *	Release memory related to border.
 *
 *------------------------------------------------------------------------
 */

void
Ck_FreeBorder(CkBorder *borderPtr)
{
    ckfree(borderPtr->name);
    ckfree((char *) borderPtr);
}

/*
 *------------------------------------------------------------------------
 *
 * Ck_NameOfBorder --
 *
 *	Create border from string.
 *
 *------------------------------------------------------------------------
 */

char *
Ck_NameOfBorder(CkBorder *borderPtr)
{
    return borderPtr->name;
}

/*
 *------------------------------------------------------------------------
 *
 * Ck_DrawBorder --
 *
 *	Given window, border and bounding box, draw border.
 *
 *------------------------------------------------------------------------
 */

void
Ck_DrawBorder(
    CkWindow *winPtr,
    CkBorder *borderPtr,
    int x, int y, int width, int height)
{
    int i;
    long *gchar;
    WINDOW *w;

    if (winPtr->window == NULL)
	return;
    w = winPtr->window;
    gchar = borderPtr->gchar;
    if (width < 1 || height < 1)
	return;
    if (width == 1) {
	for (i = y; i < height + y; i++)
	    mvwaddch(w, i, x, gchar[3]);
	return;
    }
    if (height == 1) {
	for (i = x; i < width + x; i++)
	    mvwaddch(w, y, i, gchar[1]);
	return;
    }
    if (width == 2) {
	mvwaddch(w, y, x, gchar[0]);
	mvwaddch(w, y, x + 1, gchar[2]);
	for (i = y + 1; i < height - 1 + y; i++)
	    mvwaddch(w, i, x, gchar[7]);
	for (i = y + 1; i < height - 1 + y; i++)
	    mvwaddch(w, i, x + 1, gchar[3]);
	mvwaddch(w, height - 1 + y, x, gchar[6]);
	mvwaddch(w, height - 1 + y, x + 1, gchar[4]);
	return;
    }
    if (height == 2) {
	mvwaddch(w, y, x, gchar[0]);
	mvwaddch(w, y + 1, x, gchar[6]);
	for (i = x + 1; i < width - 1 + x; i++)
	    mvwaddch(w, y, i, gchar[1]);
	for (i = x + 1; i < width - 1 + x; i++)
	    mvwaddch(w, y + 1, i, gchar[5]);
	mvwaddch(w, y, width - 1 + x, gchar[2]);
	mvwaddch(w, y + 1, width - 1 + x, gchar[4]);
	return;
    }
    mvwaddch(w, y, x, gchar[0]);
    for (i = x + 1; i < width - 1 + x; i++)
	mvwaddch(w, y, i, gchar[1]);
    mvwaddch(w, y, width - 1 + x, gchar[2]);
    for (i = y + 1; i < height - 1 + y; i++)
	mvwaddch(w, i, width - 1 + x, gchar[3]);
    mvwaddch(w, height - 1 + y, width - 1 + x, gchar[4]);
    for (i = x + 1; i < width - 1 + x; i++)
	mvwaddch(w, height - 1 + y, i, gchar[5]);
    mvwaddch(w, height - 1 + y, x, gchar[6]);
    for (i = y + 1; i < height - 1 + y; i++)
	mvwaddch(w, i, x, gchar[7]);
}
