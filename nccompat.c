/*
 * Equalizer for ncurses 5.x and 6.x.
 */

#include <dlfcn.h>
#include "tcl.h"

TCL_DECLARE_MUTEX(mutex);
static int initialized = 0;
static void *lib = NULL;

int
Nccompat_Init(Tcl_Interp *interp)
{
    static const char *names[] = {
	"libncursesw.so.6", "libncursesw.so.5", "libncursesw.so", NULL
    };
    int i;

#ifdef USE_TCL_STUBS
    if (Tcl_InitStubs(interp, "8.5", 0) == NULL) {
	return TCL_ERROR;
    }
#else
    if (Tcl_PkgRequire(interp, "Tcl", "8.5", 0) != TCL_OK) {
	return TCL_ERROR;
    }
#endif

    if (!initialized) {
#ifndef SUPPORT_UNLOAD
	int fin = 0;
#endif

	Tcl_MutexLock(&mutex);
	if (++initialized != 1) {
	    goto done;
	}
	i = 0;
	while (names[i] != NULL) {
	    lib = dlopen(names[i], RTLD_GLOBAL | RTLD_NOW);
	    if (lib != NULL) {
		break;
	    }
	    i++;
	}
#ifndef SUPPORT_UNLOAD
	if (lib == NULL) {
	    fin++;
	}
#endif
done:
	Tcl_MutexUnlock(&mutex);
	if (lib == NULL) {
#ifndef SUPPORT_UNLOAD
	    if (fin) {
		Tcl_MutexFinalize(&mutex);
	    }
#endif
	    Tcl_SetResult(interp, "libncursesw is missing", TCL_STATIC);
	    return TCL_ERROR;
	}
    }
    return Tcl_PkgProvide(interp, "nccompat", PACKAGE_VERSION);
}

#ifdef SUPPORT_UNLOAD
int
Nccompat_Unload(Tcl_Interp *interp)
{
    Tcl_MutexLock(&mutex);
    if (--initialized <= 0) {
	if (lib != NULL) {
	    dlclose(lib);
	    lib = NULL;
	}
	initialized = 0;
    }
    Tcl_MutexUnlock(&mutex);
    Tcl_MutexFinalize(&mutex);
    return TCL_OK;
}
#endif

/*
 * Local Variables:
 * mode: c
 * c-basic-offset: 4
 * fill-column: 78
 * tab-width: 8
 * End:
 */
