/*
 * ckOpen.c --
 *
 *	Tcl commands for explicit Ck-session lifecycle management.
 *
 *	    ck::open ?-pty fd? ?-rows N? ?-cols M? ?-class CLASS? ?path?
 *	        Create the main window.  Without -pty, takes over the
 *	        controlling tty and turns on stdout/stderr capture.  With
 *	        -pty, drives ncurses off the supplied fd (typically the
 *	        slave end of a pty pair) and leaves the host's fds 0/1/2
 *	        alone — the path tests use to inspect ck output.
 *
 *	    ck::pty open ?-rows N? ?-cols M?
 *	        Allocate a pty pair via openpty(3) and return a 2-list
 *	        {masterChan slaveChan} of Tcl channels wrapping the two
 *	        ends.  Hand slaveChan to ck::open -pty (we extract the
 *	        underlying fd via Tcl_GetChannelHandle); read/write
 *	        masterChan to drive and inspect the session.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "ckPort.h"
#include "ck.h"
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

extern CkMainInfo *ckMainInfo;

/*
 * openpty / forkpty live in different headers across systems.  Linux glibc
 * exposes them in <pty.h>; the BSDs use <util.h> or <libutil.h>.  We only
 * need openpty() here.
 */
#if defined(__linux__)
#  include <pty.h>
#elif defined(__APPLE__) || defined(__FreeBSD__)
#  include <util.h>
#else
#  include <libutil.h>
#endif

/*
 *----------------------------------------------------------------------
 *
 * DeriveClassName --
 *
 *	Default the option-database class name from argv0 (matching the
 *	old Ck_Init behaviour) when the caller didn't pass -class.  The
 *	returned string is malloc'd via ckalloc — caller frees.
 *
 *----------------------------------------------------------------------
 */
static char *
DeriveClassName(Tcl_Interp *interp, const char *override)
{
    const char *p, *name;
    char *className;

    if (override != NULL && override[0] != '\0') {
	p = override;
    } else {
	p = Tcl_GetVar(interp, "argv0", TCL_GLOBAL_ONLY);
	if (p == NULL || *p == '\0') {
	    p = PACKAGE_NAME;
	}
    }
    name = strrchr(p, '/');
    if (name != NULL) {
	name++;
    } else {
	name = p;
    }
    className = (char *) ckalloc((unsigned) (strlen(name) + 1));
    strcpy(className, name);
    className[0] = (char) toupper((unsigned char) className[0]);
    return className;
}

/*
 *----------------------------------------------------------------------
 *
 * GetFdFromObj --
 *
 *	Resolve a Tcl value to a numeric file descriptor.  Accepts either
 *	a small non-negative integer (a raw fd) or the name of a Tcl
 *	channel — for the latter we pull the underlying fd out via
 *	Tcl_GetChannelHandle so test scripts can pass the slave channel
 *	returned by ck::pty open without having to dig out the fd
 *	themselves.
 *
 *----------------------------------------------------------------------
 */
static int
GetFdFromObj(Tcl_Interp *interp, Tcl_Obj *obj, int *fdPtr)
{
    int fd;

    if (Tcl_GetIntFromObj(NULL, obj, &fd) == TCL_OK && fd >= 0) {
	*fdPtr = fd;
	return TCL_OK;
    }
    {
	const char *name = Tcl_GetString(obj);
	int mode;
	Tcl_Channel chan = Tcl_GetChannel(interp, name, &mode);
	if (chan != NULL) {
	    ClientData handle;
	    if (Tcl_GetChannelHandle(chan,
		    (mode & TCL_WRITABLE) ? TCL_WRITABLE : TCL_READABLE,
		    &handle) == TCL_OK) {
		*fdPtr = (int)(intptr_t) handle;
		return TCL_OK;
	    }
	}
    }
    Tcl_SetObjResult(interp, Tcl_ObjPrintf(
	"expected file descriptor or channel but got \"%s\"",
	Tcl_GetString(obj)));
    return TCL_ERROR;
}

/*
 *----------------------------------------------------------------------
 *
 * CkOpenObjCmd --
 *
 *	ck::open ?-pty fd? ?-rows N? ?-cols M? ?-class CLASS? ?-term TERM? ?path?
 *
 *	Returns the path of the created main window (currently always ".",
 *	since the underlying CkMainInfo is still single-rooted) on success.
 *
 *----------------------------------------------------------------------
 */
int
CkOpenObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const objv[])
{
    static const char *const optStrings[] = {
	"-pty", "-rows", "-cols", "-class", "-term", NULL
    };
    enum { OPT_PTY, OPT_ROWS, OPT_COLS, OPT_CLASS, OPT_TERM };
    CkOpenOptions opts;
    const char *classOverride = NULL;
    const char *path = ".";
    char *className = NULL;
    CkWindow *winPtr;
    int i;

    (void) clientData;
    CK_OPEN_OPTIONS_INIT_DEFAULTS(opts);

    for (i = 1; i < objc; i++) {
	const char *arg = Tcl_GetString(objv[i]);
	int idx;

	if (arg[0] != '-') {
	    /* Positional path argument */
	    if (i != objc - 1) {
		Tcl_SetObjResult(interp, Tcl_NewStringObj(
		    "wrong # args: path must be the last argument", -1));
		return TCL_ERROR;
	    }
	    path = arg;
	    break;
	}
	if (Tcl_GetIndexFromObj(interp, objv[i], optStrings, "option",
		0, &idx) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (i + 1 >= objc) {
	    Tcl_SetObjResult(interp, Tcl_ObjPrintf(
		"option \"%s\" requires a value", arg));
	    return TCL_ERROR;
	}
	i++;
	switch (idx) {
	case OPT_PTY:
	    if (GetFdFromObj(interp, objv[i], &opts.pty_fd) != TCL_OK) {
		return TCL_ERROR;
	    }
	    break;
	case OPT_ROWS:
	    if (Tcl_GetIntFromObj(interp, objv[i], &opts.rows) != TCL_OK
		    || opts.rows <= 0) {
		Tcl_SetObjResult(interp, Tcl_NewStringObj(
		    "-rows must be a positive integer", -1));
		return TCL_ERROR;
	    }
	    break;
	case OPT_COLS:
	    if (Tcl_GetIntFromObj(interp, objv[i], &opts.cols) != TCL_OK
		    || opts.cols <= 0) {
		Tcl_SetObjResult(interp, Tcl_NewStringObj(
		    "-cols must be a positive integer", -1));
		return TCL_ERROR;
	    }
	    break;
	case OPT_CLASS:
	    classOverride = Tcl_GetString(objv[i]);
	    break;
	case OPT_TERM:
	    opts.term = Tcl_GetString(objv[i]);
	    break;
	}
    }

    if (strcmp(path, ".") != 0) {
	Tcl_SetObjResult(interp, Tcl_NewStringObj(
	    "ck::open currently only supports path \".\"", -1));
	return TCL_ERROR;
    }
    if (ckMainInfo != NULL) {
	Tcl_SetObjResult(interp, Tcl_NewStringObj(
	    "main window already created", -1));
	return TCL_ERROR;
    }

    className = DeriveClassName(interp, classOverride);
    winPtr = CkCreateMainWindowEx(interp, className, &opts);
    ckfree(className);

    if (winPtr == NULL) {
	Tcl_SetObjResult(interp, Tcl_NewStringObj(
	    "couldn't initialise curses", -1));
	return TCL_ERROR;
    }

    /*
     * Pull in the runtime Tcl-side bindings (class bindings, ckCommand,
     * etc.) which depend on the curses commands ck::open just registered.
     * The lookup honours $::ck_library, set by pkgIndex.tcl.
     */
    if (Tcl_VarEval(interp,
	    "uplevel #0 [list source [file join $::ck_library ck.tcl]]",
	    (char *) NULL) != TCL_OK) {
	return TCL_ERROR;
    }

    Tcl_SetObjResult(interp, Tcl_NewStringObj(winPtr->pathName, -1));
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * CkPtyOpen --
 *
 *	Implementation of `ck::pty open ?-rows N? ?-cols M?`.  Allocates a
 *	pty pair, optionally sets its window size, wraps both fds as Tcl
 *	channels (registered with the interp), and returns them as a
 *	2-element list {master slave}.
 *
 *----------------------------------------------------------------------
 */
static int
CkPtyOpen(Tcl_Interp *interp, int objc, Tcl_Obj *const objv[])
{
    static const char *const optStrings[] = { "-rows", "-cols", NULL };
    enum { OPT_ROWS, OPT_COLS };
    int rows = 0, cols = 0;
    int master = -1, slave = -1;
    Tcl_Channel masterChan = NULL, slaveChan = NULL;
    Tcl_Obj *result;
    int i;

    for (i = 0; i < objc; i++) {
	int idx;
	if (Tcl_GetIndexFromObj(interp, objv[i], optStrings, "option",
		0, &idx) != TCL_OK) {
	    return TCL_ERROR;
	}
	if (i + 1 >= objc) {
	    Tcl_SetObjResult(interp, Tcl_ObjPrintf(
		"option \"%s\" requires a value", Tcl_GetString(objv[i])));
	    return TCL_ERROR;
	}
	i++;
	switch (idx) {
	case OPT_ROWS:
	    if (Tcl_GetIntFromObj(interp, objv[i], &rows) != TCL_OK
		    || rows <= 0) {
		Tcl_SetObjResult(interp, Tcl_NewStringObj(
		    "-rows must be a positive integer", -1));
		return TCL_ERROR;
	    }
	    break;
	case OPT_COLS:
	    if (Tcl_GetIntFromObj(interp, objv[i], &cols) != TCL_OK
		    || cols <= 0) {
		Tcl_SetObjResult(interp, Tcl_NewStringObj(
		    "-cols must be a positive integer", -1));
		return TCL_ERROR;
	    }
	    break;
	}
    }

    if (openpty(&master, &slave, NULL, NULL, NULL) != 0) {
	Tcl_SetObjResult(interp, Tcl_ObjPrintf(
	    "openpty failed: %s", Tcl_PosixError(interp)));
	return TCL_ERROR;
    }
    (void) fcntl(master, F_SETFD, FD_CLOEXEC);
    (void) fcntl(slave,  F_SETFD, FD_CLOEXEC);

    if (rows > 0 && cols > 0) {
	struct winsize ws;
	memset(&ws, 0, sizeof(ws));
	ws.ws_row = (unsigned short) rows;
	ws.ws_col = (unsigned short) cols;
	if (ioctl(master, TIOCSWINSZ, &ws) != 0) {
	    Tcl_SetObjResult(interp, Tcl_ObjPrintf(
		"TIOCSWINSZ failed: %s", Tcl_PosixError(interp)));
	    close(master);
	    close(slave);
	    return TCL_ERROR;
	}
    }

    masterChan = Tcl_MakeFileChannel((ClientData)(intptr_t) master,
	    TCL_READABLE | TCL_WRITABLE);
    slaveChan  = Tcl_MakeFileChannel((ClientData)(intptr_t) slave,
	    TCL_READABLE | TCL_WRITABLE);
    if (masterChan == NULL || slaveChan == NULL) {
	if (masterChan != NULL) Tcl_UnregisterChannel(interp, masterChan);
	if (slaveChan  != NULL) Tcl_UnregisterChannel(interp, slaveChan);
	close(master);
	close(slave);
	Tcl_SetObjResult(interp, Tcl_NewStringObj(
	    "Tcl_MakeFileChannel failed", -1));
	return TCL_ERROR;
    }
    /* Default to binary, unbuffered — what tests almost always want. */
    (void) Tcl_SetChannelOption(NULL, masterChan, "-translation", "binary");
    (void) Tcl_SetChannelOption(NULL, masterChan, "-buffering",   "none");
    (void) Tcl_SetChannelOption(NULL, masterChan, "-blocking",    "0");
    (void) Tcl_SetChannelOption(NULL, slaveChan,  "-translation", "binary");
    (void) Tcl_SetChannelOption(NULL, slaveChan,  "-buffering",   "none");

    Tcl_RegisterChannel(interp, masterChan);
    Tcl_RegisterChannel(interp, slaveChan);

    result = Tcl_NewListObj(0, NULL);
    Tcl_ListObjAppendElement(NULL, result,
	    Tcl_NewStringObj(Tcl_GetChannelName(masterChan), -1));
    Tcl_ListObjAppendElement(NULL, result,
	    Tcl_NewStringObj(Tcl_GetChannelName(slaveChan),  -1));
    Tcl_SetObjResult(interp, result);
    return TCL_OK;
}

/*
 *----------------------------------------------------------------------
 *
 * CkPtyObjCmd --
 *
 *	`ck::pty subcmd ?args ...?` ensemble dispatcher.
 *
 *----------------------------------------------------------------------
 */
int
CkPtyObjCmd(
    ClientData clientData,
    Tcl_Interp *interp,
    int objc,
    Tcl_Obj *const objv[])
{
    static const char *const subStrings[] = { "open", NULL };
    enum { SUB_OPEN };
    int idx;

    (void) clientData;
    if (objc < 2) {
	Tcl_WrongNumArgs(interp, 1, objv, "subcommand ?arg ...?");
	return TCL_ERROR;
    }
    if (Tcl_GetIndexFromObj(interp, objv[1], subStrings, "subcommand",
	    0, &idx) != TCL_OK) {
	return TCL_ERROR;
    }
    switch (idx) {
    case SUB_OPEN:
	return CkPtyOpen(interp, objc - 2, objv + 2);
    }
    /* Unreachable. */
    return TCL_ERROR;
}
