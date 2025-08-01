/*
 * ckPort.h --
 *
 *	This file is included by all of the curses wish C files.
 *	It contains information that may be configuration-dependent,
 *	such as #includes for system include files and a few other things.
 *
 * Copyright (c) 1991-1993 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 * Copyright (c) 1995 Christian Werner
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 */

#ifndef _CKPORT
#define _CKPORT

#if defined(_WIN32) || defined(WIN32)
#   include <windows.h>
#else
#   ifndef _XOPEN_SOURCE
#	define _XOPEN_SOURCE
#   endif
#   ifdef __APPLE__
#	define _XOPEN_SOURCE_EXTENDED
#   endif
#endif

#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#if defined(_WIN32) || defined(WIN32)
#   include <limits.h>
#else
#   ifdef HAVE_LIMITS_H
#      include <limits.h>
#   else
#      include "compat/limits.h"
#   endif
#endif
#include <math.h>
#if !defined(_WIN32) && !defined(WIN32)
#   include <pwd.h>
#endif
#ifdef NO_STDLIB_H
#   include "compat/stdlib.h"
#else
#   include <stdlib.h>
#endif
#include <string.h>
#include <sys/types.h>
#if !defined(_WIN32) && !defined(WIN32)
#   include <sys/file.h>
#endif
#ifdef HAVE_SYS_SELECT_H
#   include <sys/select.h>
#endif
#include <sys/stat.h>
#if !defined(_WIN32) && !defined(WIN32)
#   include <sys/time.h>
#endif
#ifndef _TCL
#   include <tcl.h>
#endif
#if !defined(_WIN32) && !defined(WIN32)
#   ifdef HAVE_UNISTD_H
#      include <unistd.h>
#   else
#      include "compat/unistd.h"
#   endif
#endif

#if (TCL_MAJOR_VERSION < 8)
#error Tcl major version must be 8 or greater
#endif

/*
 * Not all systems declare the errno variable in errno.h. so this
 * file does it explicitly.
 */

#if !defined(_WIN32) && !defined(WIN32)
extern int errno;
#endif

/*
 * Return type for signal(), this taken from TclX.
 */

#ifndef RETSIGTYPE
#   define RETSIGTYPE void
#endif

typedef RETSIGTYPE (*Ck_SignalProc)(int);


#endif /* _CKPORT */
