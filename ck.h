/*
 * ck.h --
 *
 *	Declaration of all curses wish related things.
 *
 * Copyright (c) 1989-1994 The Regents of the University of California.
 * Copyright (c) 1994-1995 Sun Microsystems, Inc.
 * Copyright (c) 1995-2001 Christian Werner
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#ifndef _CK_H
#define _CK_H

#ifndef _TCL
#include <tcl.h>
#endif

#if (TCL_MAJOR_VERSION < 8)
#error Tcl major version must be 8 or greater
#endif

#define CK_MAJOR_VERSION 8
#if (TCL_MINOR_VERSION == 7)
#define CK_VERSION "8.7"
#define CK_MINOR_VERSION 7
#elif (TCL_MINOR_VERSION == 6)
#define CK_VERSION "8.6"
#define CK_MINOR_VERSION 6
#elif (TCL_MINOR_VERSION == 5)
#define CK_VERSION "8.5"
#define CK_MINOR_VERSION 5
#elif (TCL_MINOR_VERSION == 4)
#define CK_VERSION "8.4"
#define CK_MINOR_VERSION 4
#elif (TCL_MINOR_VERSION == 3)
#define CK_VERSION "8.3"
#define CK_MINOR_VERSION 3
#elif (TCL_MINOR_VERSION == 2)
#define CK_VERSION "8.2"
#define CK_MINOR_VERSION 2
#elif (TCL_MINOR_VERSION == 1)
#define CK_VERSION "8.1"
#define CK_MINOR_VERSION 1
#else
#error unsupported Tcl minor version
#endif

#ifndef RESOURCE_INCLUDED

#ifdef __STDC__
#include <stddef.h>
#endif

#ifdef USE_NCURSES
#include <ncurses.h>
#else
#include <curses.h>
#endif

/*
 * Keyboard symbols (KeySym)
 */

typedef long KeySym;
#define NoSymbol (-2)

/*
 * Event structures.
 */

typedef struct {
    long type;
    struct CkWindow *winPtr;
} CkAnyEvent;

typedef struct {
    long type;
    struct CkWindow *winPtr;
    int keycode;
    int is_uch;
    int uch;
} CkKeyEvent;

typedef struct {
    long type;
    struct CkWindow *winPtr;
    int button, x, y, rootx, rooty;
} CkMouseEvent;

typedef struct {
    long type;
    struct CkWindow *winPtr;
} CkWindowEvent;

typedef union {
    long type;
    CkAnyEvent any;
    CkKeyEvent key;
    CkMouseEvent mouse;
    CkWindowEvent win;
} CkEvent;

/*
 * Event types/masks
 */

#define CK_EV_KEYPRESS   0x00000001
#define CK_EV_MOUSE_DOWN 0x00000002
#define CK_EV_MOUSE_UP   0x00000004
#define CK_EV_UNMAP      0x00000010
#define CK_EV_MAP        0x00000020
#define CK_EV_EXPOSE     0x00000040
#define CK_EV_DESTROY    0x00000080
#define CK_EV_FOCUSIN    0x00000100
#define CK_EV_FOCUSOUT   0x00000200
#define CK_EV_BARCODE    0x10000000
#define CK_EV_ALL        0xffffffff

/*
 * Additional types exported to clients.
 */

typedef char *Ck_Uid;
typedef char *Ck_BindingTable;

/*
 *--------------------------------------------------------------
 *
 * Additional procedure types defined by curses wish.
 *
 *--------------------------------------------------------------
 */

typedef void (Ck_EventProc)(ClientData clientData, CkEvent *eventPtr);
typedef int  (Ck_GenericProc)(ClientData clientData, CkEvent *eventPtr);
#define Ck_FreeProc Tcl_FreeProc

/*
 * Each geometry manager (the packer, the placer, etc.) is represented
 * by a structure of the following form, which indicates procedures
 * to invoke in the geometry manager to carry out certain functions.
 */

typedef void (Ck_GeomRequestProc)(ClientData clientData,
		struct CkWindow *winPtr);
typedef void (Ck_GeomLostSlaveProc)(ClientData clientData,
		struct CkWindow *winPtr);

typedef struct Ck_GeomMgr {
    char *name;			/* Name of the geometry manager (command
				 * used to invoke it, or name of widget
				 * class that allows embedded widgets). */
    Ck_GeomRequestProc *requestProc;
				/* Procedure to invoke when a slave's
				 * requested geometry changes. */
    Ck_GeomLostSlaveProc *lostSlaveProc;
				/* Procedure to invoke when a slave is
				 * taken away from one geometry manager
				 * by another.  NULL means geometry manager
				 * doesn't care when slaves are lost. */
} Ck_GeomMgr;

/*
 * One of the following structures exists for each event handler
 * created by calling Ck_CreateEventHandler.  This information
 * is used by ckEvent.c only.
 */

typedef struct CkEventHandler {
    long mask;				/* Events for which to invoke proc. */
    Ck_EventProc *proc;			/* Procedure to invoke when an event
    					 * in mask occurs. */
    ClientData clientData;		/* Argument to pass to proc. */
    struct CkEventHandler *nextPtr;	/* Next in list of handlers
					 * associated with window (NULL means
					 * end of list). */
} CkEventHandler;

/*
 * Ck keeps the following data structure for the main
 * window (created by a call to Ck_CreateMainWindow). It stores
 * information that is shared by all of the windows associated
 * with the application.
 */

typedef struct CkMainInfo {
    struct CkWindow *winPtr;	/* Pointer to main window. */
    Tcl_Interp *interp;		/* Interpreter associated with application. */
    Tcl_HashTable nameTable;	/* Hash table mapping path names to CkWindow
				 * structs for all windows related to this
				 * main window.  Managed by ckWindow.c. */
    Tcl_HashTable winTable;	/* Ditto, for event handling. */
    struct CkWindow *topLevPtr; /* Anchor for toplevel window list. */
    struct CkWindow *focusPtr;	/* Identifies window that currently has the
				 * focus. NULL means nobody has the focus.
				 * Managed by ckFocus.c. */
    Ck_BindingTable bindingTable;
				/* Used in conjunction with "bind" command
				 * to bind events to Tcl commands. */
    struct ElArray *optionRootPtr;
                                /* Top level of option hierarchy for this
                                 * main window.  NULL means uninitialized.
                                 * Managed by ckOption.c. */
    int maxWidth, maxHeight;    /* Max dimensions of curses screen. */
    int refreshCount;		/* Counts number of calls
    				 * to Ck_EventuallyRefresh. */
    int refreshDelay;		/* Delay in milliseconds between updates;
				 * see comment in ckWindow.c. */
    double lastRefresh;		/* Delay computation for updates. */
    Tcl_TimerToken refreshTimer;/* Timer for delayed updates. */
    ClientData mouseData;       /* Value used by mouse handling code. */
    ClientData barcodeData;	/* Value used by bar code handling code. */
    int flags;			/* See definitions below. */
#ifdef USE_NCURSES
    int winchFd[2];		/* Pipe for SIGWINCH handling. */
#endif
    Tcl_Encoding isoEncoding;
} CkMainInfo;

#define CK_HAS_COLOR        1
#define CK_REVERSE_KLUDGE   2
#define CK_HAS_MOUSE        4
#define CK_MOUSE_XTERM      8
#define CK_REFRESH_TIMER   16
#define CK_HAS_BARCODE     32
#define CK_NOCLR_ON_EXIT   64

/*
 * Ck keeps one of the following structures for each window.
 * This information is (mostly) managed by ckWindow.c.
 */

typedef struct CkWindow {

    /*
     * Structural information:
     */

    WINDOW *window;		/* Curses window. NULL means window
				 * hasn't actually been created yet, or it's
				 * been deleted. */
    struct CkWindow *childList;	/* First in list of child windows,
				 * or NULL if no children. */
    struct CkWindow *lastChildPtr;
				/* Last in list of child windows, or NULL
				 * if no children. */
    struct CkWindow *parentPtr;	/* Pointer to parent window. */
    struct CkWindow *nextPtr;	/* Next in list of children with
				 * same parent (NULL if end of list). */
    struct CkWindow *topLevPtr; /* Next toplevel if this is toplevel. */
    CkMainInfo *mainPtr;	/* Information shared by all windows
				 * associated with the main window. */

    /*
     * Name and type information for the window:
     */

    char *pathName;		/* Path name of window (concatenation
				 * of all names between this window and
				 * its top-level ancestor).  This is a
				 * pointer into an entry in mainPtr->nameTable.
				 */
    Ck_Uid nameUid;		/* Name of the window within its parent
				 * (unique within the parent). */
    Ck_Uid classUid;		/* Class of the window.  NULL means window
				 * hasn't been given a class yet. */

    /*
     * Information kept by the event manager (ckEvent.c):
     */

    CkEventHandler *handlerList;/* First in list of event handlers
				 * declared for this window, or
				 * NULL if none. */

    /*
     * Information kept by the bind/bindtags mechanism (ckCmds.c):
     */

    ClientData *tagPtr;		/* Points to array of tags used for bindings
				 * on this window.  Each tag is a Ck_Uid.
				 * Malloc'ed.  NULL means no tags. */
    int numTags;		/* Number of tags at *tagPtr. */

    /*
     * Information used by ckFocus.c for toplevel windows.
     */

    struct CkWindow *focusPtr;  /* If toplevel, this was the last child
				 * which had the focus. */

    /*
     * Information used by ckGeometry.c for geometry managers.
     */

    Ck_GeomMgr *geomMgrPtr;	/* Procedure to manage geometry, NULL
    				 * means unmanaged. */
    ClientData geomData;	/* Argument for geomProc. */
    int reqWidth, reqHeight;	/* Requested width/height of window. */

    /*
     * Information used by ckOption.c to manage options for the
     * window.
     */

    int optionLevel;            /* -1 means no option information is
                                 * currently cached for this window.
                                 * Otherwise this gives the level in
                                 * the option stack at which info is
                                 * cached. */

    /*
     * Geometry and other attributes of window.
     */

    int x, y;			/* Top-left corner with respect to
    				 * parent window. */
    int width, height;		/* Width and height of window. */
    int fg, bg;			/* Foreground/background colors. */
    int attr;			/* Video attributes. */
    int flags;			/* Various flag values, see below. */


} CkWindow;

/*
 * Flag values for CkWindow structures are:
 *
 * CK_MAPPED:			1 means window is currently mapped,
 *				0 means unmapped.
 * CK_BORDER:			1 means the window has a one character
 *				cell wide border around it.
 *				0 means no border.
 * CK_TOPLEVEL:			1 means this is a toplevel window.
 * CK_SHOW_CURSOR:              1 means show the terminal's cursor if
 *                              this window has the focus.
 * CK_RECURSIVE_DESTROY:	1 means a recursive destroy is in
 *				progress, so some cleanup operations
 *				can be omitted.
 * CK_ALREADY_DEAD:		1 means the window is in the process of
 *				being destroyed already.
 */

#define CK_MAPPED		1
#define	CK_BORDER		2
#define CK_TOPLEVEL		4
#define CK_SHOW_CURSOR          8
#define CK_RECURSIVE_DESTROY	16
#define CK_ALREADY_DEAD		32
#define CK_DONTRESTRICTSIZE     64

/*
 * Window stacking literals
 */

#define CK_ABOVE	0
#define	CK_BELOW	1

/*
 * Window border structure.
 */

typedef struct {
    char *name;			/* Name of border, malloc'ed. */
    int gchar[9];		/* ACS chars making up border. */
} CkBorder;

/*
 * Enumerated type for describing a point by which to anchor something:
 */

typedef enum {
    CK_ANCHOR_N, CK_ANCHOR_NE, CK_ANCHOR_E, CK_ANCHOR_SE,
    CK_ANCHOR_S, CK_ANCHOR_SW, CK_ANCHOR_W, CK_ANCHOR_NW,
    CK_ANCHOR_CENTER
} Ck_Anchor;

/*
 * Enumerated type for describing a style of justification:
 */

typedef enum {
    CK_JUSTIFY_LEFT, CK_JUSTIFY_RIGHT,
    CK_JUSTIFY_CENTER, CK_JUSTIFY_FILL
} Ck_Justify;

/*
 * Result values returned by Ck_GetScrollInfo:
 */

#define CK_SCROLL_MOVETO        1
#define CK_SCROLL_PAGES         2
#define CK_SCROLL_UNITS         3
#define CK_SCROLL_ERROR         4

/*
 * Flags passed to CkMeasureChars/CkDisplayChars:
 */

#define CK_WHOLE_WORDS           1
#define CK_AT_LEAST_ONE          2
#define CK_PARTIAL_OK            4
#define CK_NEWLINES_NOT_SPECIAL  8
#define CK_IGNORE_TABS          16
#define CK_FILL_UNTIL_EOL	32

/*
 * Priority levels to pass to Ck_AddOption:
 */

#define CK_WIDGET_DEFAULT_PRIO  20
#define CK_STARTUP_FILE_PRIO    40
#define CK_USER_DEFAULT_PRIO    60
#define CK_INTERACTIVE_PRIO     80
#define CK_MAX_PRIO             100

/*
 * Structure used to describe application-specific configuration
 * options:  indicates procedures to call to parse an option and
 * to return a text string describing an option.
 */

typedef int (Ck_OptionParseProc)(ClientData clientData,
	Tcl_Interp *interp, CkWindow *winPtr, char *value, char *widgRec,
	int offset);
typedef char *(Ck_OptionPrintProc)(ClientData clientData,
	CkWindow *winPtr, char *widgRec, int offset,
	Tcl_FreeProc **freeProcPtr);

typedef struct Ck_CustomOption {
    Ck_OptionParseProc *parseProc;	/* Procedure to call to parse an
					 * option and store it in converted
					 * form. */
    Ck_OptionPrintProc *printProc;	/* Procedure to return a printable
					 * string describing an existing
					 * option. */
    ClientData clientData;		/* Arbitrary one-word value used by
					 * option parser:  passed to
					 * parseProc and printProc. */
} Ck_CustomOption;

/*
 * Structure used to specify information for Ck_ConfigureWidget.  Each
 * structure gives complete information for one option, including
 * how the option is specified on the command line, where it appears
 * in the option database, etc.
 */

typedef struct Ck_ConfigSpec {
    int type;			/* Type of option, such as CK_CONFIG_COLOR;
				 * see definitions below.  Last option in
				 * table must have type CK_CONFIG_END. */
    char *argvName;		/* Switch used to specify option in argv.
				 * NULL means this spec is part of a group. */
    char *dbName;		/* Name for option in option database. */
    char *dbClass;		/* Class for option in database. */
    char *defValue;		/* Default value for option if not
				 * specified in command line or database. */
    int offset;			/* Where in widget record to store value;
				 * use Ck_Offset macro to generate values
				 * for this. */
    int specFlags;		/* Any combination of the values defined
				 * below;  other bits are used internally
				 * by ckConfig.c. */
    Ck_CustomOption *customPtr;	/* If type is CK_CONFIG_CUSTOM then this is
				 * a pointer to info about how to parse and
				 * print the option.  Otherwise it is
				 * irrelevant. */
} Ck_ConfigSpec;

/*
 * Type values for Ck_ConfigSpec structures.  See the user
 * documentation for details.
 */

#define CK_CONFIG_BOOLEAN	1
#define CK_CONFIG_INT		2
#define CK_CONFIG_DOUBLE	3
#define CK_CONFIG_STRING	4
#define CK_CONFIG_UID		5
#define CK_CONFIG_COLOR		6
#define CK_CONFIG_BORDER	7
#define CK_CONFIG_JUSTIFY	8
#define CK_CONFIG_ANCHOR	9
#define CK_CONFIG_SYNONYM	10
#define CK_CONFIG_WINDOW	11
#define CK_CONFIG_COORD         12
#define CK_CONFIG_ATTR		13
#define CK_CONFIG_CUSTOM	14
#define CK_CONFIG_END		15

/*
 * Macro to use to fill in "offset" fields of Ck_ConfigInfos.
 * Computes number of bytes from beginning of structure to a
 * given field.
 */

#ifdef offsetof
#define Ck_Offset(type, field) ((int) offsetof(type, field))
#else
#define Ck_Offset(type, field) ((int) ((char *) &((type *) 0)->field))
#endif

/*
 * Possible values for flags argument to Ck_ConfigureWidget:
 */

#define CK_CONFIG_ARGV_ONLY	1

/*
 * Possible flag values for Ck_ConfigInfo structures.  Any bits at
 * or above CK_CONFIG_USER_BIT may be used by clients for selecting
 * certain entries.  Before changing any values here, coordinate with
 * tkConfig.c (internal-use-only flags are defined there).
 */

#define CK_CONFIG_COLOR_ONLY		1
#define CK_CONFIG_MONO_ONLY		2
#define CK_CONFIG_NULL_OK		4
#define CK_CONFIG_DONT_SET_DEFAULT	8
#define CK_CONFIG_OPTION_SPECIFIED	0x10
#define CK_CONFIG_USER_BIT		0x100

extern Ck_Uid ckNormalUid;
extern Ck_Uid ckActiveUid;
extern Ck_Uid ckDisabledUid;

/*
 * Internal procedures.
 */

#if defined(_WIN32) || defined(WIN32)
#   ifdef BUILD_ck
#      undef EXTERN
#      define EXTERN __declspec(dllexport)
#   endif
#endif


EXTERN int	CkAllKeyNames(Tcl_Interp *interp);
EXTERN int	CkBarcodeCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN void	CkBindEventProc(CkWindow *winPtr,
		    CkEvent *eventPtr);
EXTERN int	CkCopyAndGlobalEval(Tcl_Interp *interp, char *string);
EXTERN void	CkDisplayChars(CkMainInfo *mainPtr,
		    WINDOW *window, char *string,
		    int numChars, int x, int y, int tabOrigin, int flags);
EXTERN void	CkEventDeadWindow(CkWindow *winPtr);
#if defined(USE_NCURSES) || defined(_WIN32)
EXTERN void	CkFocusRestore(ClientData clientData);
#endif
EXTERN void	CkFreeBindingTags(CkWindow *winPtr);
EXTERN char *	CkGetBarcodeData(CkMainInfo *mainPtr);
EXTERN void	CkHandleInput(ClientData clientData, int mask);
EXTERN int	CkInitFrame(Tcl_Interp *interp, CkWindow *winPtr,
		    int argc, char **argv);
EXTERN char *	CkKeysymToString(KeySym keySym, int printControl);
EXTERN int	CkMeasureChars(CkMainInfo *mainPtr,
		    char *source, int maxChars,
		    int startX, int maxX, int tabOrigin, int flags,
		    int *nextPtr, int *nextCPtr);
EXTERN void     CkOptionClassChanged(CkWindow *winPtr);
EXTERN void     CkOptionDeadWindow(CkWindow *winPtr);
EXTERN KeySym	CkStringToKeysym(char *name);
EXTERN int	CkTermHasKey(Tcl_Interp *interp, char *name);
EXTERN void	CkUnderlineChars(CkMainInfo *mainPtr,
		    WINDOW *window, char *string,
		    int numChars, int x, int y, int tabOrigin, int flags,
		    int first, int last);

/*
 * Exported procedures.
 */

EXTERN void     Ck_AddOption(CkWindow *winPtr, char *name,
		    char *value, int priority);
EXTERN void	Ck_BindEvent(Ck_BindingTable bindingTable,
		    CkEvent *eventPtr, CkWindow *winPtr, int numObjects,
		    ClientData *objectPtr);
EXTERN void     Ck_ClearToBot(CkWindow *winPtr, int x, int y);
EXTERN void	Ck_ClearToEol(CkWindow *winPtr, int x, int y);
EXTERN int      Ck_ConfigureInfo(Tcl_Interp *interp,
		    CkWindow *winPtr, Ck_ConfigSpec *specs, char *widgRec,
		    char *argvName, int flags);
EXTERN int      Ck_ConfigureValue(Tcl_Interp *interp,
		    CkWindow *winPtr, Ck_ConfigSpec *specs, char *widgRec,
		    char *argvName, int flags);
EXTERN int      Ck_ConfigureWidget(Tcl_Interp *interp,
		    CkWindow *winPtr, Ck_ConfigSpec *specs,
		    int argc, char **argv, char *widgRec, int flags);
EXTERN int	Ck_CreateBinding(Tcl_Interp *interp,
		    Ck_BindingTable bindingTable, ClientData object,
		    char *eventString, char *command, int append);
EXTERN Ck_BindingTable Ck_CreateBindingTable(Tcl_Interp *interp);
EXTERN void	Ck_CreateEventHandler(CkWindow *winPtr, long mask,
		    Ck_EventProc *proc, ClientData clientData);
EXTERN void	Ck_CreateGenericHandler(Ck_GenericProc *proc,
		    ClientData clientData);
EXTERN CkWindow *Ck_CreateMainWindow(Tcl_Interp *interp, char *className);
EXTERN CkWindow	*Ck_CreateWindow(Tcl_Interp *interp,
		    CkWindow *parentPtr, char *name, int toplevel);
EXTERN CkWindow	*Ck_CreateWindowFromPath(Tcl_Interp *interp,
		    CkWindow *anywin, char *pathName, int toplevel);
EXTERN void	Ck_DeleteAllBindings(Ck_BindingTable bindingTable,
		    ClientData object);
EXTERN int	Ck_DeleteBinding(Tcl_Interp *interp,
		    Ck_BindingTable bindingTable, ClientData object,
		    char *eventString);
EXTERN void	Ck_DeleteBindingTable(Ck_BindingTable bindingTable);
EXTERN void	Ck_DeleteEventHandler(CkWindow *winPtr, long mask,
		    Ck_EventProc *proc, ClientData clientData);
EXTERN void	Ck_DeleteGenericHandler(Ck_GenericProc *proc,
		    ClientData clientData);
EXTERN void	Ck_DestroyWindow(CkWindow *winPtr);
EXTERN void	Ck_DrawBorder(CkWindow *winPtr,
		    CkBorder *borderPtr, int x, int y, int width, int height);
EXTERN void	Ck_EventuallyRefresh(CkWindow *winPtr);
EXTERN void	Ck_FreeBorder(CkBorder *borderPtr);
EXTERN void     Ck_FreeOptions(Ck_ConfigSpec *specs,
		    char *widgrec, int needFlags);
EXTERN void	Ck_GeometryRequest(CkWindow *winPtr,
		    int reqWidth, int reqHeight);
EXTERN void	Ck_GetAllBindings(Tcl_Interp *interp,
		    Ck_BindingTable bindingTable, ClientData object);
EXTERN int	Ck_GetAnchor(Tcl_Interp *interp, char *string,
		    Ck_Anchor *anchorPtr);
EXTERN int	Ck_GetAttr(Tcl_Interp *interp, char *name, int *attrPtr);
EXTERN char *	Ck_GetBinding(Tcl_Interp *inter,
		    Ck_BindingTable bindingTable, ClientData object,
		    char *eventString);
EXTERN CkBorder *Ck_GetBorder(Tcl_Interp *interp, char *string);
EXTERN int	Ck_GetColor(Tcl_Interp *interp, char *name, int *colorPtr);
EXTERN int	Ck_GetCoord(Tcl_Interp *interp, CkWindow *winPtr,
		    char *string, int *intPtr);
EXTERN int	Ck_GetEncoding(Tcl_Interp *interp);
EXTERN int	Ck_GetGChar(Tcl_Interp *interp, char *name, long *gchar);
EXTERN int	Ck_GetJustify(Tcl_Interp *interp, char *string,
		    Ck_Justify *justifyPtr);
EXTERN Ck_Uid   Ck_GetOption(CkWindow *winPtr, char *name, char *class);
EXTERN int	Ck_GetPair(CkWindow *winPtr, int fg, int bg);
EXTERN void	Ck_GetRootGeometry(CkWindow *winPtr, int *xPtr,
		    int *yPtr, int *widthPtr, int *heightPtr);
EXTERN int      Ck_GetScrollInfo(Tcl_Interp *interp,
		    int argc, char **argv, double *dblPtr, int *intPtr);
EXTERN Ck_Uid	Ck_GetUid(char *string);
EXTERN CkWindow *Ck_GetWindowXY(CkMainInfo *mainPtr, int *xPtr,
		    int *yPtr, int mode);
EXTERN void	Ck_HandleEvent(CkMainInfo *mainPtr, CkEvent *eventPtr);
EXTERN int	Ck_Init(Tcl_Interp *interp);
EXTERN void	Ck_Main(int argc, char **argv,
		    int (*appInitProc)(Tcl_Interp *), Tcl_Interp *interp);
EXTERN void	Ck_MainLoop(void);
EXTERN CkWindow	*Ck_MainWindow(Tcl_Interp *interp);
EXTERN void	Ck_MaintainGeometry(CkWindow *slave, CkWindow *master,
		    int x, int y, int width, int height);
EXTERN void	Ck_MakeWindowExist(CkWindow *winPtr);
EXTERN void	Ck_ManageGeometry(CkWindow *winPtr,
			    Ck_GeomMgr *mgrPtr, ClientData clientData);
EXTERN void	Ck_MapWindow(CkWindow *winPtr);
EXTERN void	Ck_MoveWindow(CkWindow *winPtr, int x, int y);
EXTERN char *	Ck_NameOfAnchor(Ck_Anchor anchor);
EXTERN char *	Ck_NameOfAttr(int attr);
EXTERN char *	Ck_NameOfBorder(CkBorder *borderPtr);
EXTERN char *	Ck_NameOfColor(int color);
EXTERN char *	Ck_NameOfJustify(Ck_Justify justify);
EXTERN CkWindow *Ck_NameToWindow(Tcl_Interp *interp,
		    char *pathName, CkWindow *winPtr);
EXTERN void	Ck_ResizeWindow(CkWindow *winPtr, int width, int height);
EXTERN int	Ck_RestackWindow(CkWindow *winPtr, int aboveBelow,
		    CkWindow *otherPtr);
EXTERN void	Ck_SetClass(CkWindow *winPtr, char *className);
EXTERN int	Ck_SetEncoding(Tcl_Interp *interp, char *name);
EXTERN void	Ck_SetFocus(CkWindow *winPtr);
EXTERN int	Ck_SetGChar(Tcl_Interp *interp, char *name, long gchar);
EXTERN void	Ck_SetHWCursor(CkWindow *winPtr, int newState);
EXTERN void	Ck_SetInternalBorder(CkWindow *winPtr, int onoff);
EXTERN void	Ck_SetWindowAttr(CkWindow *winPtr, int fg, int bg, int attr);
EXTERN void	Ck_UnmaintainGeometry(CkWindow *slave, CkWindow *master);
EXTERN void	Ck_UnmapWindow(CkWindow *winPtr);

/*
 * Command procedures.
 */

EXTERN int	Ck_BellCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_BindCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_BindtagsCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_CursesCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_DestroyCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_ExitCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_FocusCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_GridCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_LowerCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_OptionCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_PackCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_PlaceCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_RaiseCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_RecorderCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_TkwaitCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_UpdateCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_WinfoCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);

/*
 * Widget creation procedures.
 */

EXTERN int	Ck_ButtonCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_EntryCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_FrameCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_ListboxCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_MenuCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_MenubuttonCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_MessageCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_ScrollbarCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_TextCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);
EXTERN int	Ck_TreeCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, char **argv);

#endif  /* RESOURCE_INCLUDED */
#endif  /* _CK_H */
