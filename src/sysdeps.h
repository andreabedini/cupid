/* --------------------------------------------------------------------------
 * System facts for the supported targets: x86-64 and ARM64 machines
 * running a current Linux, macOS, Windows or BSD.
 *
 * This replaces the feature probing that configure used to do.  Every
 * value here is either constant across the supported targets or follows
 * directly from the operating system, which the compiler tells us.  The
 * operating system and architecture names that the Haskell libraries test
 * for are in platform.h, which the build system generates.
 *
 * Preprocessor directives only: prelude.h checks the size constants
 * against the compiler.
 * ------------------------------------------------------------------------*/
#ifndef __SYSDEPS_H__
#define __SYSDEPS_H__

#if !defined(_WIN32) && !defined(__APPLE__) && !defined(__linux__) &&          \
    !defined(__FreeBSD__) && !defined(__DragonFly__) &&                        \
    !defined(__NetBSD__) && !defined(__OpenBSD__)
# error "Cupid supports Linux, macOS, Windows and the BSDs only"
#endif
#if !defined(__x86_64__) && !defined(_M_X64) && !defined(__aarch64__) &&       \
    !defined(_M_ARM64)
# error "Cupid supports x86-64 and ARM64 only"
#endif

/* --------------------------------------------------------------------------
 * Compiler and machine characteristics.  All supported targets have a
 * 32-bit int, 64-bit pointers, IEEE float and double, a downward-growing
 * C stack, void(int) signal handlers and jmp_bufs that can live in arrays.
 * ------------------------------------------------------------------------*/

#define SIZEOF_INT 4
#define SIZEOF_FLOAT 4
#define SIZEOF_DOUBLE 8
#define SIZEOF_INTP 8

#define STACK_DIRECTION -1

#if defined(__GNUC__) || defined(__clang__)
# define HAVE_LABELS_AS_VALUES 1 /* computed goto for bytecode dispatch */
#endif

/* --------------------------------------------------------------------------
 * Headers and functions present on every supported target.
 * ------------------------------------------------------------------------*/
#define HAVE_ISATTY 1

/* --------------------------------------------------------------------------
 * Windows versus POSIX.
 * ------------------------------------------------------------------------*/

#if defined(_WIN32)

# define HAVE_WINDOWS_H 1
# define HAVE_IO_H 1
# define HAVE_DIRECT_H 1
# define HAVE_GETMODULEFILENAME 1
# define HAVE__POPEN 1
# define HAVE__PCLOSE 1
# define HAVE__STRICMP 1
# define HAVE__FULLPATH 1
# define HAVE__ALLOCA 1
# define HAVE_SYS_TIMEB_H 1
# define HAVE_FTIME 1
# define HAVE_DECL__TIMEZONE 1
# define HAVE_TZNAME 1
# define DOS_FILENAMES 1 /* backslash paths, ';'-separated PATHSEP, ".dll" */
# if defined(__MINGW32__)
#  define HAVE_UNISTD_H 1
#  define HAVE_DIRENT_H 1
#  define HAVE_SYS_TIME_H 1
#  define HAVE_SYS_PARAM_H 1
#  define HAVE_GETTIMEOFDAY 1
#  define HAVE_STRCASECMP 1
# endif

#else /* POSIX: Linux, macOS, the BSDs */

# define HAVE_UNISTD_H 1
# define HAVE_DIRENT_H 1
# define HAVE_DLFCN_H 1
# define HAVE_TERMIOS_H 1
# define HAVE_ALLOCA_H 1
# define HAVE_SYS_TIME_H 1
# define HAVE_SYS_TIMES_H 1
# define HAVE_SYS_RESOURCE_H 1
# define HAVE_SYS_PARAM_H 1
# define HAVE_SYS_IOCTL_H 1
# define HAVE_SYS_WAIT_H 1
# define HAVE_POPEN 1
# define HAVE_PCLOSE 1
# define HAVE_STRCASECMP 1
# define HAVE_REALPATH 1
# define HAVE_GETRUSAGE 1
# define HAVE_GETTIMEOFDAY 1
# define HAVE_SELECT 1
# define HAVE_TIMES 1
# define HAVE_STRUCT_TM_TM_ZONE 1 /* tm_zone and tm_gmtoff */
# define HAVE_BIN_SH 1
# define DOS_FILENAMES 0 /* slash paths, ':'-separated PATHSEP, ".so" */

#endif

#endif /* __SYSDEPS_H__ */
