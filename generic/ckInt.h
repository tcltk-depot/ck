#ifndef CKINT_H
#define CKINT_H

#include <tcl.h>
#if HAVE_CONFIG_H
#    include "config.h"
#endif

#include "ck.h"

/*
 * Hack to force ncurses to free all static resources (leak debugging).
 * Only safe when ncurses itself was built with --disable-leaks; gated
 * on CK_NC_LEAK_DEBUG so a system ncurses build doesn't crash on exit.
 */
#if CK_NC_LEAK_DEBUG
extern void _nc_freeall(void);
#endif

static inline void replace_tclobj(Tcl_Obj** target, Tcl_Obj* replacement)
{
    Tcl_Obj*	old = *target;

#if DEBUG
    if (*target && (*target)->refCount <= 0) Tcl_Panic("replace_tclobj target exists but has refcount <= 0: %d", (*target)->refCount);
#endif
    *target = replacement;
    if (*target) Tcl_IncrRefCount(*target);
    if (old) {
	Tcl_DecrRefCount(old);
	old = NULL;
    }
}

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Internal procedures.
 */

int	CkAllKeyNames(Tcl_Interp *interp);

/*
 * Options for opening a Ck main window.  Default-initialised values
 * (CkOpenOptions opts = {0}; opts.pty_fd = -1) reproduce the legacy
 * "take over the controlling tty + capture stdout/stderr" behaviour.
 *
 * pty_fd >= 0 means "don't touch fds 0/1/2 — instead drive ncurses
 * directly off this fd (typically the slave end of a pty pair the
 * caller created via ck::pty open)".  rows/cols, when both > 0, force
 * a TIOCSWINSZ on the pty before newterm() picks up the size.  term
 * overrides the TERM environment variable for newterm().
 */
typedef struct CkOpenOptions {
    int		pty_fd;
    int		rows;
    int		cols;
    const char *term;
} CkOpenOptions;

#define CK_OPEN_OPTIONS_INIT_DEFAULTS(opts) do { \
    (opts).pty_fd = -1; \
    (opts).rows   = 0;  \
    (opts).cols   = 0;  \
    (opts).term   = NULL; \
} while (0)

/*
 * Internal entry point used by ck::open: allows the caller to specify
 * an external pty fd and skip the fd-swap.  Ck_CreateMainWindow remains
 * the public, default-options wrapper.
 */
CkWindow *CkCreateMainWindowEx(Tcl_Interp *interp, char *className,
			       const CkOpenOptions *opts);

/* ckStdioSwap.c */
int	CkStdioSwap_Init(CkMainInfo *mainPtr, const CkOpenOptions *opts);
void	CkStdioSwap_Restore(CkMainInfo *mainPtr);

/* ckOpen.c */
Tcl_ObjCmdProc CkOpenObjCmd;
Tcl_ObjCmdProc CkPtyObjCmd;

int	CkBarcodeCmd(ClientData clientData,
		    Tcl_Interp *interp, int argc, const char **argv);
void	CkBindEventProc(CkWindow *winPtr,
		    CkEvent *eventPtr);
int	CkCopyAndGlobalEval(Tcl_Interp *interp, char *string);
void	CkDisplayChars(CkMainInfo *mainPtr,
		    WINDOW *window, const char *string,
		    int numChars, int x, int y, int tabOrigin, int flags);
void	CkEventDeadWindow(CkWindow *winPtr);
#if defined(USE_NCURSES) || defined(_WIN32)
void	CkFocusRestore(ClientData clientData);
#endif
void	CkFreeBindingTags(CkWindow *winPtr);
char *	CkGetBarcodeData(CkMainInfo *mainPtr);
void	CkHandleInput(ClientData clientData, int mask);
int	CkInitFrame(Tcl_Interp *interp, CkWindow *winPtr,
		    int argc, const char **argv);
char *	CkKeysymToString(KeySym keySym, int printControl);
int	CkMeasureChars(CkMainInfo *mainPtr,
		    char *source, int maxChars,
		    int startX, int maxX, int tabOrigin, int flags,
		    int *nextPtr, int *nextCPtr);
void	CkOptionClassChanged(CkWindow *winPtr);
void	CkOptionDeadWindow(CkWindow *winPtr);
KeySym	CkStringToKeysym(const char *name);
int	CkTermHasKey(Tcl_Interp *interp, const char *name);
void	CkUnderlineChars(CkMainInfo *mainPtr,
		    WINDOW *window, char *string,
		    int numChars, int x, int y, int tabOrigin, int flags,
		    int first, int last);

#ifdef __cplusplus
}
#endif

#endif
