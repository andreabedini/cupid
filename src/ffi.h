/* --------------------------------------------------------------------------
 * The foreign function interface (ffi.c), as the static analysis sees it.
 *
 * ffi.c both checks foreign import and export declarations and writes the
 * C stubs for them; the checking moved here from static.c.
 * ------------------------------------------------------------------------*/
#ifndef __FFI_H__
#define __FFI_H__

#include "prelude.h"
#include "storage.h"

extern void checkForeignImport(Name); /* check one foreign import          */
extern void checkForeignExport(Name); /* check one foreign export          */
extern void linkForeign(Name);        /* make the import callable          */
extern void installFfi(void); /* register roots; INSTALL only              */
extern void resetFfi(void);   /* close any stub file left open, clear includes*/

#endif /* __FFI_H__ */
