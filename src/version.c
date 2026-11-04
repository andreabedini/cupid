/* --------------------------------------------------------------------------
 * Version string
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/

#include "prelude.h"

/* CUPID_VERSION comes from config.h, where the build system sets it */
#ifndef CUPID_VERSION
# define CUPID_VERSION "<snapshot>"
#endif
#define VERSION_STRING CUPID_VERSION

char *versionString = VERSION_STRING;
