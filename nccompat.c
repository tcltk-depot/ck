/*
 * Equalizer for ncurses 5.x and 6.x symbols.
 *
 * Only required functions for ck8.x are provided.
 */

#include <dlfcn.h>
#include "tcl.h"

struct window;
typedef struct window WINDOW;
struct mevent;
typedef struct mevent MEVENT;
typedef int NCURSES_PAIRS_T;
typedef int NCURSES_COLOR_T;
typedef unsigned long mmask_t;
typedef unsigned long chtype;

WINDOW *stdscr = NULL;
WINDOW *curscr = NULL;
int ESCDELAY = 0;
int LINES = 0;
int COLS = 0;
int COLOR_PAIRS = 0;

#define GENSTUB(type, name, args, ret, callargs)		\
    typedef type ( _fn ##name ) args;				\
    static _fn ##name * _fp ##name = NULL;			\
    type name args {						\
	if (_fp ##name == NULL) return ret;			\
	return _fp ##name callargs;				\
    }

#define P0 ()
#define P1 (p1)
#define P2 (p1, p2)
#define P3 (p1, p2, p3)
#define P4 (p1, p2, p3, p4)

GENSTUB(int, baudrate, (void), -1, P0)
GENSTUB(int, beep, (void), -1, P0)
GENSTUB(int, curs_set, (int p1), -1, P1)
GENSTUB(int, delwin, (WINDOW *p1), -1, P1)
GENSTUB(int, doupdate, (void), -1 , P0)
GENSTUB(int, endwin, (void), -1, P0)
GENSTUB(int, getcurx, (WINDOW *p1), -1, P1)
GENSTUB(int, getcury, (WINDOW *p1), -1, P1)
GENSTUB(int, getmaxx, (WINDOW *p1), -1, P1)
GENSTUB(int, getmaxy, (WINDOW *p1), -1, P1)
GENSTUB(int, getmouse, (MEVENT *p1), -1, P1)
GENSTUB(int, has_colors, (void), 0, P0)
GENSTUB(int, idlok, (WINDOW *p1, int p2), -1, P2)
GENSTUB(int, init_pair, (NCURSES_PAIRS_T p1, NCURSES_COLOR_T p2, NCURSES_COLOR_T p3), -1, P3)
GENSTUB(WINDOW *, initscr, (void), NULL, P0)
GENSTUB(int, keypad, (WINDOW *p1, int p2), -1, P2)
GENSTUB(int, meta, (WINDOW *p1, int p2), -1, P2)
GENSTUB(int, mouseinterval, (int p1), -1, P1)
GENSTUB(mmask_t, mousemask, (mmask_t p1, mmask_t *p2), 0, P2)
GENSTUB(int, mvwaddch, (int p1, int p2, const chtype p3), -1, P3)
GENSTUB(int, mvwaddstr, (int p1, int p2, const char *p3), -1, P3)
GENSTUB(int, mvwin, (WINDOW *p1, int p2, int p3), -1, P3)
GENSTUB(WINDOW *, newwin, (int p1, int p2, int p3, int p4), NULL, P4)
GENSTUB(int, nodelay, (WINDOW *p1, int p2), -1, P2)
GENSTUB(int, noecho, (void), -1, P0)
GENSTUB(int, nonl, (void), -1, P0)
GENSTUB(int, raw, (void), -1, P0)
GENSTUB(int, resizeterm, (int p1, int p2), -1, P2)
GENSTUB(int, scr_dump, (const char *p1), -1, P1)
GENSTUB(int, scrollok, (WINDOW *p1, int p2), -1, P2)
GENSTUB(int, start_color, (void), -1, P0)
GENSTUB(char *, tigetstr, (const char *p1), NULL, P1)
GENSTUB(int, waddch, (WINDOW *p1, const chtype p2), -1, P2)
GENSTUB(int, waddnstr, (WINDOW *p1, const chtype p2, int p3), -1, P3)
GENSTUB(int, waddnwstr, (WINDOW *p1, const int *p2, int p3), -1, P3)
GENSTUB(int, wattrset, (WINDOW *p1, int p2), -1, P2)
GENSTUB(int, wclear, (WINDOW *p1), -1, P1)
GENSTUB(int, wgetch, (WINDOW *p1), -1, P1)
GENSTUB(int, wmove, (WINDOW *p1, int p2, int p3), -1, P3)
GENSTUB(int, wnoutrefresh, (WINDOW *p1), -1, P1)
GENSTUB(int, wrefresh, (WINDOW *p1), -1, P1)
GENSTUB(int, wtouchln, (WINDOW *p1, int p2, int p3, int p4), -1, P4)

struct sym {
    const char *fn;
    const void **fp;
};

#define SYMTAB						\
    static const struct sym syms []

#define SYM(name)					\
    { "_" #name, (const void **) & _fp ##name }

#define SYMEND						\
    { NULL, NULL }

#define SYMDONE(symp)		(symp->fn == NULL)
#define SYMNAME(symp)		symp->fn
#define SYMSET(symp, val)	*(symp->fp) = val

SYMTAB = {
    SYM(baudrate),
    SYM(beep),
    SYM(curs_set),
    SYM(delwin),
    SYM(doupdate),
    SYM(endwin),
    SYM(getcurx),
    SYM(getcury),
    SYM(getmaxx),
    SYM(getmaxy),
    SYM(getmouse),
    SYM(has_colors),
    SYM(idlok),
    SYM(init_pair),
    SYM(initscr),
    SYM(keypad),
    SYM(meta),
    SYM(mouseinterval),
    SYM(mousemask),
    SYM(mvwaddch),
    SYM(mvwaddstr),
    SYM(mvwin),
    SYM(newwin),
    SYM(nodelay),
    SYM(noecho),
    SYM(nonl),
    SYM(raw),
    SYM(resizeterm),
    SYM(scr_dump),
    SYM(scrollok),
    SYM(start_color),
    SYM(tigetstr),
    SYM(waddch),
    SYM(waddnstr),
    SYM(waddnwstr),
    SYM(wattrset),
    SYM(wclear),
    SYM(wgetch),
    SYM(wmove),
    SYM(wnoutrefresh),
    SYM(wrefresh),
    SYM(wtouchln),
    SYMEND
};

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
    void *v;
    const struct sym *s;

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
	Tcl_MutexLock(&mutex);
	if (++initialized != 1) {
	    goto done;
	}
        i = 0;
	while (names[i] != NULL) {
	    lib = dlopen(names[i], RTLD_LOCAL | RTLD_NOW);
	    if (lib != NULL) {
		s = syms;
		while (!SYMDONE(s)) {
		    v = dlsym(lib, SYMNAME(s) + 1);
		    if (v == NULL) {
			v = dlsym(lib, SYMNAME(s));
		    }
		    if (v != NULL) {
			SYMSET(s, v);
		    }
		    s++;
		}
		break;
	    }
	    i++;
	}
done:
	Tcl_MutexUnlock(&mutex);

    }
    return Tcl_PkgProvide(interp, "nccompat", PACKAGE_VERSION);
}

#ifdef SUPPORT_UNLOAD
int
Nccompat_Unload(Tcl_Interp *interp)
{
    const struct sym *s;

    Tcl_MutexLock(&mutex);
    if (--initialized <= 0) {
        if (lib != NULL) {
            s = syms;
            while (!SYMDONE(s)){
                SYMSET(s, NULL);
                s++;
            }
            dlclose(lib);
	    lib = NULL;
	}
	initialized = 0;
    }
    Tcl_MutexUnlock(&mutex);
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
