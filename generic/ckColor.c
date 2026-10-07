/*
 * ckColor.c --
 *
 *	Colour handling: parsing and naming colour specifications (the
 *	classic curses colour names, palette indices and #rrggbb), working
 *	out how many colours the terminal can show, mapping colours onto
 *	what it can show, and allocating colour pairs.
 *
 *	A colour value is an int:
 *	    -1				unset (eg. a text tag without -foreground)
 *	    0 - 255			a palette index (0-7 are the curses
 *					COLOR_* colours, 8-15 their bright
 *					variants, 16-255 the xterm 256 colour
 *					cube and grey ramp)
 *	    CK_COLOR_RGB_FLAG | rgb	a 24 bit colour
 *
 *	Values are stored as given, and only translated to something the
 *	terminal can display when a colour pair is allocated, so that
 *	[cget] returns what was configured whatever the terminal.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "ckPort.h"
#include "ck.h"
#ifdef USE_NCURSES
#   include <term.h>
#endif

static const struct {
    const char *name;
    int value;
} colorNames[] = {
    /*
     * Names are matched by unique-or-first prefix (so "b" is black), in
     * this order, for compatibility with older Ck versions.
     */
    { "black", COLOR_BLACK },
    { "blue", COLOR_BLUE },
    { "cyan", COLOR_CYAN },
    { "green", COLOR_GREEN },
    { "magenta", COLOR_MAGENTA },
    { "red", COLOR_RED },
    { "white", COLOR_WHITE },
    { "yellow", COLOR_YELLOW },
    { "brightblack", 8 + COLOR_BLACK },
    { "brightblue", 8 + COLOR_BLUE },
    { "brightcyan", 8 + COLOR_CYAN },
    { "brightgreen", 8 + COLOR_GREEN },
    { "brightmagenta", 8 + COLOR_MAGENTA },
    { "brightred", 8 + COLOR_RED },
    { "brightwhite", 8 + COLOR_WHITE },
    { "brightyellow", 8 + COLOR_YELLOW }
};

static const struct {
    const char *name;
    int mode;
} colorModes[] = {
    { "none", CK_COLORS_NONE },
    { "8", CK_COLORS_8 },
    { "16", CK_COLORS_16 },
    { "256", CK_COLORS_256 },
    { "direct", CK_COLORS_DIRECT }
};

/*
 * The usual (xterm) RGB values of the 16 ANSI colours, used to map
 * between palette indices and RGB.  The colours a terminal actually shows
 * for these depend on its theme.
 */

static const int ansiRgb[16] = {
    0x000000, 0xcd0000, 0x00cd00, 0xcdcd00,
    0x0000ee, 0xcd00cd, 0x00cdcd, 0xe5e5e5,
    0x7f7f7f, 0xff0000, 0x00ff00, 0xffff00,
    0x5c5cff, 0xff00ff, 0x00ffff, 0xffffff
};

/* Channel values of the 6x6x6 colour cube, palette entries 16 - 231 */
static const int cubeLevels[6] = { 0x00, 0x5f, 0x87, 0xaf, 0xd7, 0xff };

#ifndef CK_EXT_COLORS
typedef struct {
    int fg, bg;
} CPair;

static CPair *cPairs = NULL;
static int numPairs, newPair;
#endif

/*
 *------------------------------------------------------------------------
 *
 * Ck_GetColor --
 *
 *	Given a color specification, return curses color value.  The
 *	specification can be one of the color names, colorN for a palette
 *	index N in 0-255, or #rgb / #rrggbb.
 *
 * Results:
 *	TCL_OK if color found, color value in *colorPtr.
 *	TCL_ERROR if color not found; interp's result contains an
 *	error message.
 *
 * Side effects:
 *	None.
 *
 *------------------------------------------------------------------------
 */

int
Ck_GetColor(
    Tcl_Interp *interp,
    char *name,
    int *colorPtr)
{
    size_t i, len;
    int value = -1;

    len = strlen(name);
    if (name[0] == '#') {
	unsigned long rgb = 0;

	for (i = 1; i < len && isxdigit((unsigned char) name[i]); i++) {
	    rgb = (rgb << 4) | (isdigit((unsigned char) name[i]) ? name[i] - '0' :
				tolower((unsigned char) name[i]) - 'a' + 10);
	}
	if (i == len && len == 7) {
	    value = CK_COLOR_RGB_FLAG | (int) rgb;
	} else if (i == len && len == 4) {
	    /* #rgb is short for #rrggbb, as in CSS */
	    value = CK_COLOR_RGB_FLAG |
		    (((rgb >> 8) & 0xf) * 0x110000) |
		    (((rgb >> 4) & 0xf) * 0x1100) |
		    ((rgb & 0xf) * 0x11);
	}
    } else if (len > 5 && len <= 8 && strncmp(name, "color", 5) == 0) {
	int n = 0;

	for (i = 5; i < len && isdigit((unsigned char) name[i]); i++) {
	    n = n * 10 + name[i] - '0';
	}
	if (i == len && n <= 255) {
	    value = n;
	}
    } else if (len > 0) {
	for (i = 0; i < sizeof (colorNames) / sizeof (colorNames[0]); i++) {
	    if (strncmp(name, colorNames[i].name, len) == 0) {
		value = colorNames[i].value;
		break;
	    }
	}
    }
    if (value == -1) {
	Tcl_AppendResult(interp, "bad color \"", name, "\"", (char *) NULL);
	return TCL_ERROR;
    }
    if (colorPtr != NULL) {
	*colorPtr = value;
    }
    return TCL_OK;
}

/*
 *------------------------------------------------------------------------
 *
 * Ck_NameOfColor --
 *
 *	Given a color value, return its name.
 *
 * Results:
 *	String: name of color, or NULL if no valid color.  The string
 *	may be in a static buffer that is overwritten by the next call.
 *
 * Side effects:
 *	None.
 *
 *------------------------------------------------------------------------
 */

char *
Ck_NameOfColor(int color)	/* Color value to get name for */
{
    static char buf[16];
    size_t i;

    if (color < 0) {
	return NULL;
    }
    if (CK_COLOR_IS_RGB(color)) {
	snprintf(buf, sizeof (buf), "#%06x", CK_COLOR_RGB(color));
	return buf;
    }
    for (i = 0; i < sizeof (colorNames) / sizeof (colorNames[0]); i++) {
	if (colorNames[i].value == color) {
	    return (char *) colorNames[i].name;
	}
    }
    if (color <= 255) {
	snprintf(buf, sizeof (buf), "color%d", color);
	return buf;
    }
    return NULL;
}

/*
 *------------------------------------------------------------------------
 *
 * CkGetColorMode, CkNameOfColorMode --
 *
 *	Convert between a color mode name (none, 8, 16, 256, direct) and
 *	its CK_COLORS_* value.
 *
 *------------------------------------------------------------------------
 */

int
CkGetColorMode(
    Tcl_Interp *interp,		/* For error messages, may be NULL. */
    const char *name,
    int *modePtr)
{
    size_t i;

    for (i = 0; i < sizeof (colorModes) / sizeof (colorModes[0]); i++) {
	if (strcmp(name, colorModes[i].name) == 0) {
	    *modePtr = colorModes[i].mode;
	    return TCL_OK;
	}
    }
    if (strcmp(name, "auto") == 0) {
	*modePtr = CK_COLORS_AUTO;
	return TCL_OK;
    }
    if (interp != NULL) {
	Tcl_AppendResult(interp, "bad color mode \"", name,
		"\": must be auto, direct, 256, 16, 8 or none", (char *) NULL);
    }
    return TCL_ERROR;
}

const char *
CkNameOfColorMode(int mode)
{
    size_t i;

    for (i = 0; i < sizeof (colorModes) / sizeof (colorModes[0]); i++) {
	if (colorModes[i].mode == mode) {
	    return colorModes[i].name;
	}
    }
    return "auto";
}

/*
 *------------------------------------------------------------------------
 *
 * CkDirectColorTerm --
 *
 *	Decide whether to use a direct color (24 bit) variant of the
 *	terminal description named by $TERM.  terminfo can only describe
 *	24 bit color support in a separate entry (eg. xterm-direct), so
 *	terminals that support it usually still run with TERM set to a 256
 *	color entry, and advertise 24 bit color with COLORTERM=truecolor.
 *
 *	Called before newterm(), and only when the caller didn't name a
 *	terminal type.  A <base>-direct entry is used in place of
 *	<base>, <base>-256color, etc. when mode is CK_COLORS_DIRECT, or
 *	when mode is CK_COLORS_AUTO and COLORTERM is truecolor or 24bit.
 *
 * Results:
 *	The name of the terminfo entry to use (in a static buffer), or
 *	NULL to use $TERM.
 *
 * Side effects:
 *	None.
 *
 *------------------------------------------------------------------------
 */

const char *
CkDirectColorTerm(
    int mode,			/* Requested CK_COLORS_* mode. */
    int fd)			/* Terminal output fd. */
{
#ifdef USE_NCURSES
    static char buf[128];
    static const char *suffixes[] = {
	"-truecolor", "-256color", "-88color", "-16color", "-color", NULL
    };
    const char *term = getenv("TERM");
    const char *colorterm = getenv("COLORTERM");
    size_t len, slen;
    int i, err, colors;

    if (mode == CK_COLORS_AUTO) {
	if (colorterm == NULL || (strcmp(colorterm, "truecolor") != 0 &&
		strcmp(colorterm, "24bit") != 0)) {
	    return NULL;
	}
    } else if (mode != CK_COLORS_DIRECT) {
	return NULL;
    }
    if (term == NULL || *term == '\0' || strlen(term) + 8 > sizeof (buf)) {
	return NULL;
    }

    /*
     * Nothing to do if $TERM already describes a direct color terminal.
     */

    if (setupterm((char *) term, fd, &err) != OK) {
	return NULL;
    }
    colors = tigetnum("colors");
    del_curterm(cur_term);
    if (colors >= CK_COLORS_DIRECT) {
	return NULL;
    }

    strcpy(buf, term);
    len = strlen(buf);
    for (i = 0; suffixes[i] != NULL; i++) {
	slen = strlen(suffixes[i]);
	if (len > slen && strcmp(buf + len - slen, suffixes[i]) == 0) {
	    buf[len - slen] = '\0';
	    break;
	}
    }
    strcat(buf, "-direct");
    if (setupterm(buf, fd, &err) != OK) {
	return NULL;
    }
    colors = tigetnum("colors");
    del_curterm(cur_term);
    return (colors >= CK_COLORS_DIRECT) ? buf : NULL;
#else
    return NULL;
#endif
}

/*
 *------------------------------------------------------------------------
 *
 * CkInitColors --
 *
 *	Called after start_color() to record how many colors the terminal
 *	supports, and the color mode to use: the lesser of that and the
 *	requested mode.
 *
 * Results:
 *	None.
 *
 * Side effects:
 *	Sets mainPtr->termColors, mainPtr->colorMode and CK_HAS_COLOR.
 *
 *------------------------------------------------------------------------
 */

void
CkInitColors(
    CkMainInfo *mainPtr,
    int mode)			/* Requested CK_COLORS_* mode. */
{
    int avail;

    mainPtr->termColors = has_colors() ? COLORS : 0;
    if (mainPtr->termColors >= CK_COLORS_DIRECT) {
	avail = CK_COLORS_DIRECT;
    } else if (mainPtr->termColors >= 256) {
	avail = CK_COLORS_256;
    } else if (mainPtr->termColors >= 16) {
	avail = CK_COLORS_16;
    } else if (mainPtr->termColors >= 8) {
	avail = CK_COLORS_8;
    } else {
	avail = CK_COLORS_NONE;
    }
    if (mode == CK_COLORS_AUTO || mode > avail) {
	mode = avail;
    }
    mainPtr->colorMode = mode;
    if (mode != CK_COLORS_NONE) {
	mainPtr->flags |= CK_HAS_COLOR;
    } else {
	mainPtr->flags &= ~CK_HAS_COLOR;
    }
}

/*
 *------------------------------------------------------------------------
 *
 * Color mapping helpers --
 *
 *	Conversions between palette indices and RGB, and nearest palette
 *	entry searches.
 *
 *------------------------------------------------------------------------
 */

static int
IndexToRgb(int i)
{
    if (i < 16) {
	return ansiRgb[i];
    }
    if (i < 232) {
	i -= 16;
	return (cubeLevels[i / 36] << 16) | (cubeLevels[(i / 6) % 6] << 8) |
		cubeLevels[i % 6];
    }
    i = 8 + (i - 232) * 10;
    return (i << 16) | (i << 8) | i;
}

/*
 * Approximate perceptual distance between two RGB colors ("redmean").
 */

static long
ColorDistance(int a, int b)
{
    long r1 = (a >> 16) & 0xff, g1 = (a >> 8) & 0xff, b1 = a & 0xff;
    long r2 = (b >> 16) & 0xff, g2 = (b >> 8) & 0xff, b2 = b & 0xff;
    long rmean = (r1 + r2) / 2;
    long dr = r1 - r2, dg = g1 - g2, db = b1 - b2;

    return (((512 + rmean) * dr * dr) >> 8) + 4 * dg * dg +
	    (((767 - rmean) * db * db) >> 8);
}

static int
CubeLevel(int v)
{
    return (v < 48) ? 0 : (v < 115) ? 1 : (v - 35) / 40;
}

/*
 * Nearest entry in the 256 color palette.  Entries 0-15 are skipped:
 * their actual colors depend on the terminal's theme.
 */

static int
RgbTo256(int rgb)
{
    int r = (rgb >> 16) & 0xff, g = (rgb >> 8) & 0xff, b = rgb & 0xff;
    int cube, grey, avg;

    cube = 16 + 36 * CubeLevel(r) + 6 * CubeLevel(g) + CubeLevel(b);
    avg = (r + g + b) / 3;
    grey = (avg < 8) ? 0 : (avg > 238) ? 23 : (avg - 3) / 10;
    if (grey > 23) {
	grey = 23;
    }
    grey += 232;
    return (ColorDistance(rgb, IndexToRgb(grey)) <
	    ColorDistance(rgb, IndexToRgb(cube))) ? grey : cube;
}

/*
 * Nearest of the first n (8 or 16) ANSI colors.
 */

static int
RgbToAnsi(int rgb, int n)
{
    int i, best = 0;
    long d, bestDist = -1;

    for (i = 0; i < n; i++) {
	d = ColorDistance(rgb, ansiRgb[i]);
	if (bestDist < 0 || d < bestDist) {
	    best = i;
	    bestDist = d;
	}
    }
    return best;
}

/*
 *------------------------------------------------------------------------
 *
 * CkCursesColor --
 *
 *	Translate a Ck color value to the curses color number to use for
 *	it on this terminal: first reduce it to the palette of the color
 *	mode, then encode it the way the terminal description expects.
 *
 * Results:
 *	A curses color number, -1 for an unset color.
 *
 * Side effects:
 *	None.
 *
 *------------------------------------------------------------------------
 */

int
CkCursesColor(
    CkMainInfo *mainPtr,
    int color)
{
    if (color < 0) {
	return -1;
    }

    switch (mainPtr->colorMode) {
    case CK_COLORS_DIRECT:
	break;
    case CK_COLORS_256:
	if (CK_COLOR_IS_RGB(color)) {
	    color = RgbTo256(CK_COLOR_RGB(color));
	}
	break;
    case CK_COLORS_16:
	if (CK_COLOR_IS_RGB(color)) {
	    color = RgbToAnsi(CK_COLOR_RGB(color), 16);
	} else if (color >= 16) {
	    color = RgbToAnsi(IndexToRgb(color), 16);
	}
	break;
    default:
	if (CK_COLOR_IS_RGB(color)) {
	    color = RgbToAnsi(CK_COLOR_RGB(color), 8);
	} else if (color >= 16) {
	    color = RgbToAnsi(IndexToRgb(color), 8);
	} else if (color >= 8) {
	    color -= 8;
	}
	break;
    }

    if (mainPtr->termColors >= CK_COLORS_DIRECT) {
	/*
	 * Direct color terminal descriptions treat color numbers below 8
	 * as the ANSI colors, and anything else as a packed RGB value.
	 */

	int rgb;

	if (!CK_COLOR_IS_RGB(color) && color < 8) {
	    return color;
	}
	rgb = CK_COLOR_IS_RGB(color) ? CK_COLOR_RGB(color) : IndexToRgb(color);
	return (rgb < 8) ? 8 : rgb;
    }
    return color;
}

/*
 *------------------------------------------------------------------------
 *
 * CkAllocPair --
 *
 *	Given foreground/background Ck colors, a color pair is allocated
 *	(or an existing one for the same colors found) and returned.
 *
 *	With ncurses 6.1+ alloc_pair() manages the pairs, and when they
 *	run out recycles the least recently allocated one.  Terminals with
 *	256 or direct colors typically have 65536 pairs.
 *
 * Results:
 *	The color pair number, 0 if colors aren't in use.
 *
 * Side effects:
 *	May (re)define a color pair.
 *
 *------------------------------------------------------------------------
 */

int
CkAllocPair(
    CkMainInfo *mainPtr,
    int fg, int bg)
{
#ifdef CK_EXT_COLORS
    int pair;

    if (!(mainPtr->flags & CK_HAS_COLOR)) {
	return 0;
    }
    pair = alloc_pair(CkCursesColor(mainPtr, fg), CkCursesColor(mainPtr, bg));
    return (pair < 0) ? 0 : pair;
#else
    int i;

    if (!(mainPtr->flags & CK_HAS_COLOR)) {
	return 0;
    }
    fg = CkCursesColor(mainPtr, fg);
    bg = CkCursesColor(mainPtr, bg);
    if (cPairs == NULL) {
	cPairs = (CPair *) ckalloc(sizeof (CPair) * (COLOR_PAIRS + 2));
	numPairs = 0;
	newPair = 1;
    }
    for (i = 1; i < numPairs; i++) {
	if (cPairs[i].fg == fg && cPairs[i].bg == bg) {
	    return i;
	}
    }
    i = newPair;
    cPairs[i].fg = fg;
    cPairs[i].bg = bg;
    init_pair((short) i, (short) fg, (short) bg);
    if (++newPair >= COLOR_PAIRS) {
	newPair = 1;
    } else {
	numPairs = newPair;
    }
    return i;
#endif
}

/*
 *------------------------------------------------------------------------
 *
 * Ck_GetPair --
 *
 *	Given background/foreground colors, a color pair is allocated
 *	and returned as a curses attribute.
 *
 *	Only pairs 0 - 255 can be expressed as an attribute
 *	(COLOR_PAIR()), so this is limited to those.  Ck_SetWindowAttr
 *	doesn't have that limitation.
 *
 * Results:
 *	COLOR_PAIR() of the allocated pair, or of pair 0 if the pair
 *	number is too large.
 *
 * Side effects:
 *	May (re)define a color pair.
 *
 *------------------------------------------------------------------------
 */

int
Ck_GetPair(CkWindow *winPtr, int fg, int bg)
{
    int pair = CkAllocPair(winPtr->mainPtr, fg, bg);

    return (pair < 256) ? COLOR_PAIR(pair) : COLOR_PAIR(0);
}

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * tab-width: 8
 * End:
 */
