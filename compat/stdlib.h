/*
 * stdlib.h --
 *
 *	Declares facilities exported by the "stdlib" portion of
 *	the C library.  This file isn't complete in the ANSI-C
 *	sense;  it only declares things that are needed by Tcl.
 *	This file is needed even on many systems with their own
 *	stdlib.h (e.g. SunOS) because not all stdlib.h files
 *	declare all the procedures needed here (such as strtod).
 *
 * Copyright (c) 1991 The Regents of the University of California.
 * Copyright (c) 1994 Sun Microsystems, Inc.
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 *
 * @(#) stdlib.h 1.9 94/12/17 16:26:20
 */

#ifndef _STDLIB
#define _STDLIB

#include <tcl.h>

extern void		abort(void);
extern double		atof(CONST char *string);
extern int		atoi(CONST char *string);
extern long		atol(CONST char *string);
extern char *		calloc(unsigned int numElements,
			    unsigned int size);
extern void		exit(int status);
extern int		free(char *blockPtr);
extern char *		getenv(CONST char *name);
extern char *		malloc(unsigned int numBytes);
extern void		qsort(VOID *base, int n, int size,
			    int (*compar)(CONST VOID *element1, CONST VOID
			    *element2));
extern char *		realloc(char *ptr, unsigned int numBytes);
extern double		strtod(CONST char *string, char **endPtr);
extern long		strtol(CONST char *string, char **endPtr,
			    int base);
extern unsigned long	strtoul(CONST char *string,
			    char **endPtr, int base);

#endif /* _STDLIB */
