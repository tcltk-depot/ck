#!/bin/bash -norc
dnl	This file is an input file used by the GNU "autoconf" program to
dnl	generate the file "configure", which is run during Tcl installation
dnl	to configure the system for the local environment.

#-----------------------------------------------------------------------
# Set your package name and version numbers here.
#
# This initializes the environment with PACKAGE_NAME and PACKAGE_VERSION
# set as provided.  These will also be added as -D defs in your Makefile
# so you can encode the package version directly into the source files.
# This will also define a special symbol for Windows (BUILD_<PACKAGE_NAME>
# so that we create the export library with the dll.
#-----------------------------------------------------------------------

AC_INIT([ck],[1.0])

#--------------------------------------------------------------------
# Call TEA_INIT as the first TEA_ macro to set up initial vars.
# This will define a ${TEA_PLATFORM} variable == "unix" or "windows"
# as well as PKG_LIB_FILE and PKG_STUB_LIB_FILE.
#--------------------------------------------------------------------

TEA_INIT()

AC_CONFIG_AUX_DIR(tclconfig)
AC_CONFIG_HEADERS([generic/config.h])

#--------------------------------------------------------------------
# Load the tclConfig.sh file
#--------------------------------------------------------------------

TEA_PATH_TCLCONFIG
TEA_LOAD_TCLCONFIG

#--------------------------------------------------------------------
# Load the tkConfig.sh file if necessary (Tk extension)
#--------------------------------------------------------------------

#TEA_PATH_TKCONFIG
#TEA_LOAD_TKCONFIG

#-----------------------------------------------------------------------
# Handle the --prefix=... option by defaulting to what Tcl gave.
# Must be called after TEA_LOAD_TCLCONFIG and before TEA_SETUP_COMPILER.
#-----------------------------------------------------------------------

TEA_PREFIX

#-----------------------------------------------------------------------
# Standard compiler checks.
# This sets up CC by using the CC env var, or looks for gcc otherwise.
# This also calls AC_PROG_CC and a few others to create the basic setup
# necessary to compile executables.
#-----------------------------------------------------------------------

TEA_SETUP_COMPILER

#-----------------------------------------------------------------------
# Specify the C source files to compile in TEA_ADD_SOURCES,
# public headers that need to be installed in TEA_ADD_HEADERS,
# stub library C source files to compile in TEA_ADD_STUB_SOURCES,
# and runtime Tcl library files in TEA_ADD_TCL_SOURCES.
# This defines PKG(_STUB)_SOURCES, PKG(_STUB)_OBJECTS, PKG_HEADERS
# and PKG_TCL_SOURCES.
#-----------------------------------------------------------------------

TEA_ADD_SOURCES([
    generic/ckBuildInfo.c
    generic/ckBind.c
    generic/ckBorder.c
    generic/ckButton.c
    generic/ckCmds.c
    generic/ckConfig.c
    generic/ckEntry.c
    generic/ckEvent.c
    generic/ckFocus.c
    generic/ckFrame.c
    generic/ckGeometry.c
    generic/ckGet.c
    generic/ckGrid.c
    generic/ckListbox.c
    generic/ckMenubutton.c
    generic/ckMenu.c
    generic/ckMessage.c
    generic/ckOption.c
    generic/ckPack.c
    generic/ckPlace.c
    generic/ckPreserve.c
    generic/ckRecorder.c
    generic/ckScrollbar.c
    generic/ckTextBTree.c
    generic/ckText.c
    generic/ckTextDisp.c
    generic/ckTextIndex.c
    generic/ckTextMark.c
    generic/ckTextTag.c
    generic/ckTree.c
    generic/ckUtil.c
    generic/ckWindow.c
])
TEA_ADD_HEADERS([generic/ck.h])
TEA_ADD_INCLUDES([])
TEA_ADD_LIBS([])
TEA_ADD_CFLAGS([])
TEA_ADD_STUB_SOURCES([])
TEA_ADD_TCL_SOURCES([
    library/bgerror.tcl
    library/button.tcl
    library/ckfbox.tcl
    library/ck.tcl
    library/clrpick.tcl
    library/comdlg.tcl
    library/command.tcl
    library/dialog.tcl
    library/entry.tcl
    library/entryx.tcl
    library/focus.tcl
    library/keylpr.tcl
    library/listbox.tcl
    library/menu.tcl
    library/msgbox.tcl
    library/optMenu.tcl
    library/parray.tcl
    library/scrollbar.tcl
    library/showglob.tcl
    library/showproc.tcl
    library/text.tcl
    library/tclIndex
])

#--------------------------------------------------------------------
# __CHANGE__
#
# You can add more files to clean if your extension creates any extra
# files by extending CLEANFILES.
# Add pkgIndex.tcl if it is generated in the Makefile instead of ./configure
# and change Makefile.in to move it from CONFIG_CLEAN_FILES to BINARIES var.
#
# A few miscellaneous platform-specific items:
# TEA_ADD_* any platform specific compiler/build info here.
#--------------------------------------------------------------------

#CLEANFILES="$CLEANFILES pkgIndex.tcl"
if test "${TEA_PLATFORM}" = "windows" ; then
    # Ensure no empty if clauses
    :
    #TEA_ADD_SOURCES([win/winFile.c])
    #TEA_ADD_INCLUDES([-I\"$(${CYGPATH} ${srcdir}/win)\"])
else
    # Ensure no empty else clauses
    :
    #TEA_ADD_SOURCES([unix/unixFile.c])
    #TEA_ADD_LIBS([-lsuperfly])
fi

#--------------------------------------------------------------------
# Choose which headers you need.  Extension authors should try very
# hard to only rely on the Tcl public header files.  Internal headers
# contain private data structures and are subject to change without
# notice.
# This MUST be called after TEA_LOAD_TCLCONFIG / TEA_LOAD_TKCONFIG
#--------------------------------------------------------------------

TEA_PUBLIC_TCL_HEADERS
#TEA_PRIVATE_TCL_HEADERS

#TEA_PUBLIC_TK_HEADERS
#TEA_PRIVATE_TK_HEADERS
#TEA_PATH_X

#--------------------------------------------------------------------
# Check whether --enable-threads or --disable-threads was given.
# This auto-enables if Tcl was compiled threaded.
#--------------------------------------------------------------------

TEA_ENABLE_THREADS

#--------------------------------------------------------------------
# The statement below defines a collection of symbols related to
# building as a shared library instead of a static library.
#--------------------------------------------------------------------

TEA_ENABLE_SHARED

#--------------------------------------------------------------------
# This macro figures out what flags to use with the compiler/linker
# when building shared/static debug/optimized objects.  This information
# can be taken from the tclConfig.sh file, but this figures it all out.
#--------------------------------------------------------------------

TEA_CONFIG_CFLAGS

#--------------------------------------------------------------------
# Set the default compiler switches based on the --enable-symbols option.
#--------------------------------------------------------------------

TEA_ENABLE_SYMBOLS

#--------------------------------------------------------------------
# Check for required headers and functions specific to this extension
#--------------------------------------------------------------------

AC_CHECK_HEADERS([unistd.h stdint.h limits.h])

#--------------------------------------------------------------------
# Supply a substitute for stdlib.h if it doesn't define strtol,
# strtoul, or strtod (which it doesn't in some versions of SunOS).
#--------------------------------------------------------------------

AC_MSG_CHECKING(stdlib.h)
AC_EGREP_HEADER([strtol],[stdlib.h],[tk_ok=yes],[tk_ok=no])
AC_EGREP_HEADER([strtoul],[stdlib.h],[],[tk_ok=no])
AC_EGREP_HEADER([strtod],[stdlib.h],[],[tk_ok=no])
if test $tk_ok = no; then
    AC_DEFINE(NO_STDLIB_H, 1, [Define if stdlib.h is missing or doesn't define strtol/strtoul/strtod])
fi
AC_MSG_RESULT($tk_ok)

#--------------------------------------------------------------------
# Check for various typedefs and provide substitutes if
# they don't exist.
#--------------------------------------------------------------------

AC_TYPE_MODE_T
AC_TYPE_PID_T
AC_TYPE_SIZE_T
AC_TYPE_UID_T

#--------------------------------------------------------------------
# Do we have sigaction?
#--------------------------------------------------------------------

AC_CHECK_FUNCS([sigaction])

#--------------------------------------------------------------------
# Include sys/select.h if it exists and if it supplies things
# that appear to be useful and aren't already in sys/types.h.
# This appears to be true only on the RS/6000 under AIX.  Some
# systems like OSF/1 have a sys/select.h that's of no use, and
# other systems like SCO UNIX have a sys/select.h that's
# pernicious.  If "fd_set" isn't defined anywhere then set a
# special flag.
#--------------------------------------------------------------------

AC_MSG_CHECKING([fd_set and sys/select])
AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>]], [[fd_set readMask, writeMask;]])],[tk_ok=yes],[tk_ok=no])
if test $tk_ok = no; then
    AC_EGREP_HEADER([fd_mask],[sys/select.h],[tk_ok=yes])
    if test $tk_ok = yes; then
	AC_DEFINE(HAVE_SYS_SELECT_H, 1, [Define to 1 if you have sys/select.h])
    fi
fi
AC_MSG_RESULT($tk_ok)
if test $tk_ok = no; then
    AC_DEFINE(NO_FD_SET, 1, [Define if fd_set is not defined])
fi

#--------------------------------------------------------------------
# Check for the existence of various libraries.  The order here
# is important, so that then end up in the right order in the
# command line generated by make.  The -lsocket and -lnsl libraries
# require a couple of special tricks:
# 1. Use "connect" and "accept" to check for -lsocket, and
#    "gethostbyname" to check for -lnsl.
# 2. Use each function name only once:  can't redo a check because
#    autoconf caches the results of the last check and won't redo it.
# 3. Use -lnsl and -lsocket only if they supply procedures that
#    aren't already present in the normal libraries.  This is because
#    IRIX 5.2 has libraries, but they aren't needed and they're
#    bogus:  they goof up name resolution if used.
# 4. On some SVR4 systems, can't use -lsocket without -lnsl too.
#    To get around this problem, check for both libraries together
#    if -lsocket doesn't work by itself.
#--------------------------------------------------------------------

AC_CHECK_LIB(Xbsd, main, [LIBS="$LIBS -lXbsd"])

tk_checkBoth=0
AC_CHECK_FUNC(connect, tk_checkSocket=0, tk_checkSocket=1)
if test "$tk_checkSocket" = 1; then
    AC_CHECK_LIB(socket, main, LIBS="$LIBS -lsocket", tk_checkBoth=1)
fi
if test "$tk_checkBoth" = 1; then
    tk_oldLibs=$LIBS
    LIBS="$LIBS -lsocket -lnsl"
    AC_CHECK_FUNC(accept, tk_checkNsl=0, [LIBS=$tk_oldLibs])
fi
AC_CHECK_FUNC(gethostbyname, , AC_CHECK_LIB(nsl, main, [LIBS="$LIBS -lnsl"]))

#--------------------------------------------------------------------
# On Interactive the str(n)casecmp is burried in libinet.a
#--------------------------------------------------------------------

AC_CHECK_FUNC(strncasecmp, , AC_CHECK_LIB(inet, main, [LIBS="$LIBS -linet"]))

#--------------------------------------------------------------------
# On a few very rare systems, all of the libm.a stuff is
# already in libc.a.  Set compiler flags accordingly.
# Also, Linux requires the "ieee" library for math to
# work right (and it must appear before "-lm").
#--------------------------------------------------------------------

MATH_LIBS=""
AC_CHECK_FUNC(sin, , MATH_LIBS="-lm")
AC_CHECK_LIB(ieee, main, [MATH_LIBS="-lieee $MATH_LIBS"])

#--------------------------------------------------------------------
# If this system doesn't have a memmove procedure, use memcpy
# instead.
#--------------------------------------------------------------------

AC_CHECK_FUNC(memmove, , [AC_DEFINE(memmove, memcpy, [Define to memcpy if memmove is not available])])

#--------------------------------------------------------------------
# SGI systems don't use the BSD form of the gettimeofday function,
# but they have a BSDgettimeofday function that can be used instead.
#
# Also, check for the existence of a gettimeofday declaration,
# to tkPort.h can declare it if it isn't already declared.
#--------------------------------------------------------------------

AC_CHECK_FUNC(BSDgettimeofday, [AC_DEFINE(HAVE_BSDGETTIMEOFDAY, 1, [Define to 1 if you have the BSDgettimeofday function])])
AC_MSG_CHECKING([for gettimeofday declaration])
AC_EGREP_HEADER(gettimeofday, sys/time.h, AC_MSG_RESULT(present), [
    AC_MSG_RESULT(missing)
    AC_DEFINE(GETTOD_NOT_DECLARED, 1, [Define if gettimeofday is not declared])
])

#--------------------------------------------------------------------
# Under Solaris 2.4, strtod returns the wrong value for the
# terminating character under some conditions.  Check for this
# and if the problem exists use a substitute procedure
# "fixstrtod" (provided by Tcl) that corrects the error.
#--------------------------------------------------------------------

AC_CHECK_FUNC(strtod, tk_strtod=1, tk_strtod=0)
if test "$tk_strtod" = 1; then
    AC_MSG_CHECKING([for Solaris 2.4 strtod bug])
    AC_RUN_IFELSE([AC_LANG_SOURCE([[
        extern double strtod();
        int main()
        {
            char *string = "NaN";
            char *term;
            strtod(string, &term);
            if ((term != string) && (term[-1] == 0)) {
                exit(1);
            }
            exit(0);
        }]])],[tk_ok=1],[tk_ok=0],[tk_ok=0])
    if test "$tk_ok" = 1; then
        AC_MSG_RESULT(ok)
    else
        AC_MSG_RESULT(buggy)
        AC_DEFINE(strtod, fixstrtod, [Define to fixstrtod if strtod is buggy])
    fi
fi

#--------------------------------------------------------------------
# See if there was a command-line option for building cwsh;
# if not, default to no.
#--------------------------------------------------------------------

AC_ARG_ENABLE(cwsh, [  --enable-cwsh           build and install cwsh binary],
        [BUILD_CWSH=$enableval], [BUILD_CWSH=no])
if test "$BUILD_CWSH" = "yes" ; then
    CWSH_TARGET=cwsh
    CWSH_INSTALL=install-cwsh
    AC_MSG_RESULT([building cwsh])
else
    CWSH_TARGET=
    CWSH_INSTALL=
    AC_MSG_RESULT([not building cwsh])
fi

#---------------------------------------------------------------------
# Locate the curses header files and the curses library archive.
# The order is:
#     ../ncurses
#     /usr/include and /usr/lib
#     /opt/ncurses
#     /usr/local
#     /usr/local/ncurses
#---------------------------------------------------------------------

CURSESINCLUDES=nope
CURSESLIBSW=nope
USE_NCURSES=0
USE_NCURSESW=0
NOGPM=0

AC_ARG_WITH(cursesinc, [  --with-cursesinc=DIR    use curses includes from DIR],
        CURSESINCLUDES="-I$withval")
AC_ARG_WITH(curseslib, [  --with-curseslib=VAL    use curses library VAL],
        CURSESLIBSW=$withval)

if test "$CURSESINCLUDES" = "nope" -a "$CURSESLIBSW" = "nope"; then
    echo checking for curses/ncurses header files

    VAL=`pkg-config --cflags ncursesw 2>/dev/null`
    if test -n "$VAL" ; then
        USE_NCURSESW=1
        USE_NCURSES=1
        CURSESINCLUDES=$VAL
        CURSESLIBSW=`pkg-config --libs ncursesw`
        NOGPM=1
    else
        VAL=`pkg-config --cflags ncurses 2>/dev/null`
        if test -n "$VAL" ; then
            USE_NCURSES=1
            CURSESINCLUDES=$VAL
            CURSESLIBSW=`pkg-config --libs ncurses`
        fi
    fi

    if test $USE_NCURSES -eq 0 ; then
        dirs="../ncurses/include /usr/include/ncursesw /usr/include /usr/include/ncurses /opt/ncurses/include /usr/local/include /usr/local/include/ncurses"
        for i in $dirs ; do
            if test -r $i/ncurses.h; then
                AC_MSG_CHECKING([ncurses headers])
                tk_oldCFlags=$CFLAGS
                CFLAGS="$CFLAGS -I$i"
                AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <ncurses.h>]], [[int c; initscr(); c = ACS_ULCORNER; curs_set(1);]])],[AC_MSG_RESULT(yes)
                    CURSESINCLUDES="-I$i"],[AC_MSG_RESULT(no)])
                CFLAGS=$tk_oldCFlags
                if test "$CURSESINCLUDES" != "nope"; then
                    USE_NCURSES=1
                    break
                fi
            fi
            if test -r $i/curses.h; then
                AC_MSG_CHECKING([curses headers])
                tk_oldCFlags=$CFLAGS
                CFLAGS="$CFLAGS -I$i"
                AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <curses.h>]], [[int c; initscr(); c = ACS_ULCORNER; curs_set(1);]])],[AC_MSG_RESULT(yes)
                    CURSESINCLUDES="-I$i"],[AC_MSG_RESULT(no)])
                CFLAGS=$tk_oldCFlags
                if test "$CURSESINCLUDES" != "nope"; then
                    break
                fi
            fi
        done
    fi
fi

if test "$CURSESINCLUDES" = "nope"; then
    echo "Warning:  couldn't find any curses header file."
    CURSESINCLUDES="# no header file found"
else
    if test $USE_NCURSES = 1 ; then
        echo "using ncurses.h from $CURSESINCLUDES"
    else
        echo "using curses.h from $CURSESINCLUDES"
    fi
fi

if test "$CURSESLIBSW" = "nope" ; then
    echo checking for curses/ncurses library files
    dirs="../ncurses/lib /usr/lib /usr/lib64 /usr/lib/ncurses /usr/lib64/ncurses /opt/ncurses/lib /usr/local/lib /usr/local/ncurses/lib"
    for i in $dirs ; do
        if test $USE_NCURSES = 0 ; then
            if test -r $i/libcurses.a || test -r $i/libcurses${SHLIB_SUFFIX} ; then
	        AC_MSG_CHECKING([curses library])
	        tk_oldCFlags=$CFLAGS
	        CFLAGS="$CFLAGS $CURSESINCLUDES"
	        if test "$i" = "/usr/lib" ; then
	            LIBSW="-lcurses -ltermcap"
	        else
	            LIBSW="-L$i -lcurses -ltermcap"
	        fi
	        tk_oldLibs=$LIBS
	        LIBS="$LIBSW"
	        AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <curses.h>]], [[int c; initscr(); c = ACS_ULCORNER; curs_set(1);]])],[AC_MSG_RESULT(yes)
	            CURSESLIBSW="$LIBSW"],[AC_MSG_RESULT(no)])
	        CFLAGS=$tk_oldCFlags
	        LIBS=$tk_oldLibs
	        if test "$CURSESLIBSW" != "nope"; then
	            break
	        fi
            fi
        else
            if test -r $i/libncurses.a || test -r $i/libncurses${SHLIB_SUFFIX} ; then
	        AC_MSG_CHECKING([ncurses library])
	        tk_oldCFlags=$CFLAGS
	        CFLAGS="$CFLAGS $CURSESINCLUDES -D_XOPEN_SOURCE_EXTENDED"
	        if test "$i" = "/usr/lib" ; then
	            LIBSW="-lncursesw"
	        else
	            LIBSW="-L$i -lncursesw"
	        fi
	        tk_oldLibs=$LIBS
	        LIBS="$LIBSW"
	        AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <ncurses.h>]], [[int c; initscr(); c = ACS_ULCORNER; addwstr("");]])],[AC_MSG_RESULT(yes)
	            USE_NCURSESW=1
	            CURSESINCLUDES="$CURSESINCLUDES -D_XOPEN_SOURCE_EXTENDED"
	            CURSESLIBSW="$LIBSW"],[AC_MSG_RESULT(no)])
	        CFLAGS=$tk_oldCFlags
	        LIBS=$tk_oldLibs
	        if test "$CURSESLIBSW" != "nope"; then
	            break
	        fi
	        tk_oldCFlags=$CFLAGS
	        CFLAGS="$CFLAGS $CURSESINCLUDES"
	        if test "$i" = "/usr/lib" ; then
	            LIBSW="-lncurses"
	        else
	            LIBSW="-L$i -lncurses"
	        fi
	        tk_oldLibs=$LIBS
	        LIBS="$LIBSW"
	        AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <ncurses.h>]], [[int c; initscr(); c = ACS_ULCORNER; curs_set(1);]])],[AC_MSG_RESULT(yes)
	            CURSESLIBSW="$LIBSW"],[AC_MSG_RESULT(no)])
	        CFLAGS=$tk_oldCFlags
	        LIBS=$tk_oldLibs
	        if test "$CURSESLIBSW" != "nope"; then
	            break
	        fi
            fi
        fi
    done
fi

if test "$CURSESLIBSW" = "nope" ; then
    echo "Warning:  couldn't find the curses library archive.  Using -lcurses."
    CURSESLIBSW="-lcurses -ltermcap"
else
    echo "using curses library: $CURSESLIBSW"
    AC_MSG_CHECKING([curses scr_dump function])
    tk_oldCFlags=$CFLAGS
    CFLAGS="$CFLAGS $CURSESINCLUDES"
    tk_oldLibs=$LIBS
    LIBS="$CURSESLIBSW"
    if test $USE_NCURSES = 1 ; then
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <ncurses.h>]], [[int c; initscr(); scr_dump("xx");]])],[AC_MSG_RESULT(yes)
           AC_DEFINE(HAVE_SCR_DUMP, 1, [Define to 1 if you have the scr_dump function])],[AC_MSG_RESULT(no)])
    else
	AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <curses.h>]], [[int c; initscr(); scr_dump("xx");]])],[AC_MSG_RESULT(yes)
           AC_DEFINE(HAVE_SCR_DUMP, 1, [Define to 1 if you have the scr_dump function])],[AC_MSG_RESULT(no)])
    fi
    CFLAGS=$tk_oldCFlags
    LIBS=$tk_oldLibs
fi

if test $USE_NCURSES = 1 ; then
    AC_DEFINE(USE_NCURSES, 1, [Define to use ncurses])
fi
if test $USE_NCURSESW = 1 ; then
    AC_DEFINE(USE_NCURSESW, 1, [Define to use ncurses wide character support])
fi

#---------------------------------------------------------------------
# Check for GPM (General Purpose Mouse)
#---------------------------------------------------------------------

if test $NOGPM -eq 0 ; then
    AC_MSG_CHECKING([GPM library])
    tk_oldLibs=$LIBS
    LIBS="-lgpm"
    AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <gpm.h>]], [[Gpm_Connect conn; Gpm_Open(&conn, 0);]])],[AC_MSG_RESULT(yes)
        AC_DEFINE(HAVE_GPM, 1, [Define to 1 if you have the GPM library])
        CURSESLIBSW="$CURSESLIBSW -lgpm"],[AC_MSG_RESULT(no)])
    LIBS=$tk_oldLibs
fi

#--------------------------------------------------------------------
# Setup Ck version information (independent from Tcl version)
#--------------------------------------------------------------------

AC_MSG_CHECKING([Ck version])
VERSION=$PACKAGE_VERSION
CK_VERSION=$VERSION
CK_MAJOR_VERSION=`echo $VERSION | sed 's/^\([[0-9]]*\)\.\([[0-9]]*\).*/\1/'`
CK_MINOR_VERSION=`echo $VERSION | sed 's/^\([[0-9]]*\)\.\([[0-9]]*\).*/\2/'`
AC_MSG_RESULT(${CK_VERSION} (major: ${CK_MAJOR_VERSION}, minor: ${CK_MINOR_VERSION}))

#--------------------------------------------------------------------
# Setup library specifications for ckConfig.sh
#--------------------------------------------------------------------

if test "${TCL_LIB_VERSIONS_OK}" = "ok"; then
    CK_BUILD_LIB_SPEC="-L`pwd` -lck${VERSION}"
    CK_LIB_SPEC="-L${exec_prefix}/lib -lck${VERSION}"
else
    CK_BUILD_LIB_SPEC="-L`pwd` -lck`echo ${VERSION} | tr -d .`"
    CK_LIB_SPEC="-L${exec_prefix}/lib -lck`echo ${VERSION} | tr -d .`"
fi

if test "$TCL_SUPPORTS_STUBS" = 1; then
    CWSH_LIB_SPEC="$TCL_BUILD_STUB_LIB_SPEC $TCL_BUILD_LIB_SPEC"
else
    CWSH_LIB_SPEC="$TCL_BUILD_LIB_SPEC"
fi

#--------------------------------------------------------------------
# Setup nccompat shared library name
#--------------------------------------------------------------------

if test "${SHARED_BUILD}" = "1"; then
    NCCOMPAT_LIB_FILE="nccompat${SHLIB_SUFFIX}"
else
    NCCOMPAT_LIB_FILE=""
fi

#--------------------------------------------------------------------
# Substitute configuration variables
#--------------------------------------------------------------------

AC_SUBST(MATH_LIBS)
AC_SUBST(CK_VERSION)
AC_SUBST(CK_MAJOR_VERSION)
AC_SUBST(CK_MINOR_VERSION)
AC_SUBST(CK_BUILD_LIB_SPEC)
AC_SUBST(CK_LIB_SPEC)
AC_SUBST(CURSESINCLUDES)
AC_SUBST(CURSESLIBSW)
AC_SUBST(CWSH_LIB_SPEC)
AC_SUBST(CWSH_TARGET)
AC_SUBST(CWSH_INSTALL)
AC_SUBST(NCCOMPAT_LIB_FILE)

#--------------------------------------------------------------------
# This macro generates a line to use when building a library.  It
# depends on values set by the TEA_ENABLE_SHARED, TEA_ENABLE_SYMBOLS,
# and TEA_LOAD_TCLCONFIG macros above.
#--------------------------------------------------------------------

TEA_MAKE_LIB

#--------------------------------------------------------------------
# Determine the name of the tclsh and/or wish executables in the
# Tcl and Tk build directories or the location they were installed
# into. These paths are used to support running test cases only,
# the Makefile should not be making use of these paths to generate
# a pkgIndex.tcl file or anything else at extension build time.
#--------------------------------------------------------------------

TEA_PROG_TCLSH
#TEA_PROG_WISH

#--------------------------------------------------------------------
# Setup a *Config.sh.in configuration file.
#--------------------------------------------------------------------

#TEA_EXPORT_CONFIG([sample])
#AC_SUBST(SAMPLE_VAR)

#--------------------------------------------------------------------
# Specify files to substitute AC variables in. You may alternatively
# have a special pkgIndex.tcl.in or other files which require
# substituting the AC variables in. Include these here.
#--------------------------------------------------------------------

AC_CONFIG_FILES([Makefile ckConfig.sh pkgIndex.tcl])

#--------------------------------------------------------------------
# Finally, substitute all of the various values into the files
# specified with AC_CONFIG_FILES.
#--------------------------------------------------------------------

AC_OUTPUT
