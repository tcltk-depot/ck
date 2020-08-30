/* 
 * ckPreserve.c --
 *
 *	This file contains a collection of procedures that are used
 *	to make sure that widget records and other data structures
 *	aren't reallocated when there are nested procedures that
 *	depend on their existence.
 *
 * Copyright (c) 1990-1994 The Regents of the University of California.
 * Copyright (c) 1994-1995 Sun Microsystems, Inc.
 * Copyright (c) 1995 Christian Werner
 *
 * See the file "license.terms" for information on usage and redistribution
 * of this file, and for a DISCLAIMER OF ALL WARRANTIES.
 */

#include "ckPort.h"
#include "ck.h"
