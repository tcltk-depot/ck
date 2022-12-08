/*
 * Equalizer for ncurses 5.x and 6.x symbols.
 *
 * Only required functions are provided as weak symbols.
 */

#include <dlfcn.h>
#include "tcl.h"

static void __baudrate(void) {}
static int __beep(void) { return -1; }
static int __curs_set(void) { return -1; }
static int __delwin(void) { return -1; }
static int __doupdate(void) { return -1; }
static int __endwin(void) { return -1; }
static int __getmouse(void) { return -1; }
static int __has_colors(void) { return 0; }
static int __idlok(void) { return -1; }
static int __init_pair(void) { return -1; }
static void *__initscr(void) { return 0; }
static int __keypad(void) { return -1; }
static int __meta(void) { return -1; }
static int __mouseinterval(void) { return -1; }
static int __mousemask(void) { return 0; }
static int __mvwaddch(void) { return -1; }
static int __mvwaddstr(void) { return -1; }
static int __mvwin(void) { return -1; }
static void *__newwin(void) { return 0; }
static int __nodelay(void) { return -1; }
static int __noecho(void) { return -1; }
static int __nonl(void) { return -1; }
static int __raw(void) { return -1; }
static int __resizeterm(void) { return -1; }
static int __scr_dump(void) { return -1; }
static int __scrollok(void) { return -1; }
static int __start_color(void) { return -1; }
static void *__tigetstr(void) { return 0; }
static int __waddch(void) { return -1; }
static int __waddnstr(void) { return -1; }
static int __waddnwstr(void) { return -1; }
static int __wattrset(void) { return -1; }
static int __wclear(void) { return -1; }
static int __wgetch(void) { return -1; }
static int __wmove(void) { return -1; }
static int __wnoutrefresh(void) { return -1; }
static int __wrefresh(void) { return -1; }
static int __wtouchln(void) { return -1; }

void baudrate(void) __attribute__((weak, alias("__baudrate")));
int beep(void) __attribute__((weak, alias("__beep")));
int curs_set(void) __attribute__((weak, alias("__curs_set")));
int delwin(void) __attribute__((weak, alias("__delwin")));
int doupdate(void) __attribute__((weak, alias("__doupdate")));
int endwin(void) __attribute__((weak, alias("__endwin")));
int getmouse(void) __attribute__((weak, alias("__getmouse")));
int has_colors(void) __attribute__((weak, alias("__has_colors")));
int idlok(void) __attribute__((weak, alias("__idlok")));
int init_pair(void) __attribute__((weak, alias("__init_pair")));
void *initscr(void) __attribute__((weak, alias("__initscr")));
int keypad(void) __attribute__((weak, alias("__keypad")));
int meta(void) __attribute__((weak, alias("__meta")));
int mouseinterval(void) __attribute__((weak, alias("__mouseinterval")));
int mousemask(void) __attribute__((weak, alias("__mousemask")));
int mvwadch(void) __attribute__((weak, alias("__mvwaddch")));
int mvwaddstr(void) __attribute__((weak, alias("__mvwaddstr")));
int mvwin(void) __attribute__((weak, alias("__mvwin")));
void *newwin(void) __attribute__((weak, alias("__newwin")));
int nodelay(void) __attribute__((weak, alias("__nodelay")));
int noecho(void) __attribute__((weak, alias("__noecho")));
int nonl(void) __attribute__((weak, alias("__nonl")));
int raw(void) __attribute__((weak, alias("__raw")));
int resizeterm(void) __attribute__((weak, alias("__resizeterm")));
int scr_dump(void) __attribute__((weak, alias("__scr_dump")));
int scrollok(void) __attribute__((weak, alias("__scrollok")));
int start_color(void) __attribute__((weak, alias("__start_color")));
void *tigetstr(void) __attribute__((weak, alias("__tigetstr")));
int waddch(void) __attribute__((weak, alias("__waddch")));
int waddnstr(void) __attribute__((weak, alias("__waddnstr")));
int waddnwstr(void) __attribute__((weak, alias("__waddnwstr")));
int wattrset(void) __attribute__((weak, alias("__wattrset")));
int wclear(void) __attribute__((weak, alias("__wclear")));
int wgetch(void) __attribute__((weak, alias("__wgetch")));
int wmove(void) __attribute__((weak, alias("__wmove")));
int wnoutrefresh(void) __attribute__((weak, alias("__wnoutrefresh")));
int wrefresh(void) __attribute__((weak, alias("__wrefresh")));
int wtouchln(void) __attribute__((weak, alias("__wtouchln")));

int Nccompat_Init(Tcl_Interp *interp)
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

    i = 0;
    while (names[i] != NULL) {
	if (dlopen(names[i], RTLD_GLOBAL | RTLD_NOW) != NULL) {
	    break;
	}
	i++;
    }
    return Tcl_PkgProvide(interp, "nccompat", PACKAGE_VERSION);
}
