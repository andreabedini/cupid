/* Type sizes and alignments for the supported targets, for the C bits of the base
 * library.  Every
 * supported target is LP64 except Windows, which is LLP64.
 */
#ifndef __GHCCONFIG_H__
#define __GHCCONFIG_H__

#include "platform.h"

#define SIZEOF_CHAR			1
#define SIZEOF_SHORT			2
#define SIZEOF_INT			4
#if defined(_WIN32)
# define SIZEOF_LONG			4
#else
# define SIZEOF_LONG			8
#endif
#define SIZEOF_LONG_LONG		8
#define SIZEOF_UNSIGNED_CHAR		SIZEOF_CHAR
#define SIZEOF_UNSIGNED_SHORT		SIZEOF_SHORT
#define SIZEOF_UNSIGNED_INT		SIZEOF_INT
#define SIZEOF_UNSIGNED_LONG		SIZEOF_LONG
#define SIZEOF_UNSIGNED_LONG_LONG	SIZEOF_LONG_LONG
#define SIZEOF_FLOAT			4
#define SIZEOF_DOUBLE			8
#define SIZEOF_VOID_P			8

/* x86-64 and ARM64 align every scalar to its own size */
#define ALIGNMENT_CHAR			SIZEOF_CHAR
#define ALIGNMENT_SHORT			SIZEOF_SHORT
#define ALIGNMENT_INT			SIZEOF_INT
#define ALIGNMENT_LONG			SIZEOF_LONG
#define ALIGNMENT_LONG_LONG		SIZEOF_LONG_LONG
#define ALIGNMENT_UNSIGNED_CHAR		SIZEOF_CHAR
#define ALIGNMENT_UNSIGNED_SHORT	SIZEOF_SHORT
#define ALIGNMENT_UNSIGNED_INT		SIZEOF_INT
#define ALIGNMENT_UNSIGNED_LONG		SIZEOF_LONG
#define ALIGNMENT_UNSIGNED_LONG_LONG	SIZEOF_LONG_LONG
#define ALIGNMENT_FLOAT			SIZEOF_FLOAT
#define ALIGNMENT_DOUBLE		SIZEOF_DOUBLE
#define ALIGNMENT_VOID_P		SIZEOF_VOID_P

#define HAVE_LONG_LONG			1
#define HAVE_INTTYPES_H			1
#define HAVE_STDINT_H			1
#define HAVE_STDLIB_H			1
#define HAVE_STRING_H			1
#define HAVE_SYS_STAT_H			1
#define HAVE_SYS_TYPES_H		1
#define STDC_HEADERS			1
#if !defined(_WIN32) || defined(__MINGW32__)
# define HAVE_STRINGS_H			1
# define HAVE_UNISTD_H			1
#endif

#endif /* __GHCCONFIG_H__ */
