/* --------------------------------------------------------------------------
 * The foreign function interface (ffi.c), as the static analysis sees it.
 *
 * ffi.c both checks foreign import and export declarations and writes the
 * C stubs for them; the checking moved here from static.c.
 * ------------------------------------------------------------------------*/
#ifndef __FFI_H__
#define __FFI_H__

extern Void checkForeignImport(Name); /* check one foreign import          */
extern Void checkForeignExport(Name); /* check one foreign export          */
extern Void linkForeign(Name);        /* make the import callable          */

#endif /* __FFI_H__ */
