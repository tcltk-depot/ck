#ifndef CKINT_H
#define CKINT_H

#include <tcl.h>
#if HAVE_CONFIG_H
#    include "config.h"
#endif

#include "ck.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Internal procedures.
 */

int	CkAllKeyNames(Tcl_Interp *interp);
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
