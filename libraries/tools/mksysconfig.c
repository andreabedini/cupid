/* --------------------------------------------------------------------------
 * Generate the platform-dependent parts of the base library.
 *
 * Compiled with the same C compiler as Cuddles and run at build time.  Two
 * outputs, selected by the first argument:
 *
 *   --header    HsBaseConfig.h, included by HsBase.h and the C bits
 *   --haskell   Cuddles/SysConfig.hs, imported by the library modules that
 *               need to know how C types map to Haskell types, what the
 *               errno and signal constants are, and the host OS and
 *               architecture names
 *
 * This is the one question that genuinely differs between the supported
 * targets, answered by asking the compiler and the system headers.
 * Everything else is fixed per operating system.
 *
 * Generated once by a scratch script; edit this file directly.
 * ------------------------------------------------------------------------*/

/* Ask for the POSIX and BSD interfaces even when compiled as strict C */
#if !defined(_WIN32)
# define _DEFAULT_SOURCE 1
# define _DARWIN_C_SOURCE 1
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <wchar.h>
#include <sys/types.h>
#include <sys/stat.h>
#if !defined(_WIN32)
# include <unistd.h>
# include <termios.h>
# include <sys/resource.h>
#endif

/* -- tables ---------------------------------------------------------------- */

struct constant { const char *name; long long value; int defined; };
struct htype { const char *macro; const char *hs; };

/* Classify an arithmetic type the way autoconf's FPTOOLS_CHECK_HTYPE did:
 * integral types become IntN or WordN by signedness and width, floating
 * types become Float, Double or LDouble by size. */
#define CLASSIFY(T, OUT)						\
    do {								\
	if (((T)((int)((T)1.4))) == ((T)1.4))				\
	    sprintf(OUT, "%s%d", ((T)(-1) < (T)0) ? "Int" : "Word",	\
		    (int)(sizeof(T) * 8));				\
	else								\
	    strcpy(OUT, sizeof(T) >  sizeof(double) ? "LDouble" :	\
			sizeof(T) == sizeof(double) ? "Double" : "Float"); \
    } while (0)

static struct htype htypes[64];
static int nhtypes;
static char hsbuf[64][16];

#define HTYPE(T, NAME)							\
    do {								\
	CLASSIFY(T, hsbuf[nhtypes]);					\
	htypes[nhtypes].macro = NAME;					\
	htypes[nhtypes].hs = hsbuf[nhtypes];				\
	nhtypes++;							\
    } while (0)

static void collect_htypes(void)
{
    HTYPE(char, "CHAR");
    HTYPE(signed char, "SIGNED_CHAR");
    HTYPE(unsigned char, "UNSIGNED_CHAR");
    HTYPE(short, "SHORT");
    HTYPE(unsigned short, "UNSIGNED_SHORT");
    HTYPE(int, "INT");
    HTYPE(unsigned int, "UNSIGNED_INT");
    HTYPE(long, "LONG");
    HTYPE(unsigned long, "UNSIGNED_LONG");
    HTYPE(long long, "LONG_LONG");
    HTYPE(unsigned long long, "UNSIGNED_LONG_LONG");
    HTYPE(float, "FLOAT");
    HTYPE(double, "DOUBLE");
    HTYPE(ptrdiff_t, "PTRDIFF_T");
    HTYPE(size_t, "SIZE_T");
    HTYPE(wchar_t, "WCHAR_T");
    HTYPE(sig_atomic_t, "SIG_ATOMIC_T");
    HTYPE(clock_t, "CLOCK_T");
    HTYPE(time_t, "TIME_T");
    HTYPE(dev_t, "DEV_T");
    HTYPE(ino_t, "INO_T");
    HTYPE(mode_t, "MODE_T");
    HTYPE(off_t, "OFF_T");
    HTYPE(pid_t, "PID_T");
#if !defined(_WIN32)
    HTYPE(gid_t, "GID_T");
#endif
#if !defined(_WIN32)
    HTYPE(uid_t, "UID_T");
#endif
#if !defined(_WIN32)
    HTYPE(cc_t, "CC_T");
#endif
#if !defined(_WIN32)
    HTYPE(speed_t, "SPEED_T");
#endif
#if !defined(_WIN32)
    HTYPE(tcflag_t, "TCFLAG_T");
#endif
#if !defined(_WIN32)
    HTYPE(nlink_t, "NLINK_T");
#endif
#if !defined(_WIN32)
    HTYPE(ssize_t, "SSIZE_T");
#endif
#if !defined(_WIN32)
    HTYPE(rlim_t, "RLIM_T");
#endif
    HTYPE(wint_t, "WINT_T");
    HTYPE(intptr_t, "INTPTR_T");
    HTYPE(uintptr_t, "UINTPTR_T");
    HTYPE(intmax_t, "INTMAX_T");
    HTYPE(uintmax_t, "UINTMAX_T");
}

static struct constant errnos[] = {
#ifdef E2BIG
    { "E2BIG", (long long)E2BIG, 1 },
#else
    { "E2BIG", -1, 0 },
#endif
#ifdef EACCES
    { "EACCES", (long long)EACCES, 1 },
#else
    { "EACCES", -1, 0 },
#endif
#ifdef EADDRINUSE
    { "EADDRINUSE", (long long)EADDRINUSE, 1 },
#else
    { "EADDRINUSE", -1, 0 },
#endif
#ifdef EADDRNOTAVAIL
    { "EADDRNOTAVAIL", (long long)EADDRNOTAVAIL, 1 },
#else
    { "EADDRNOTAVAIL", -1, 0 },
#endif
#ifdef EADV
    { "EADV", (long long)EADV, 1 },
#else
    { "EADV", -1, 0 },
#endif
#ifdef EAFNOSUPPORT
    { "EAFNOSUPPORT", (long long)EAFNOSUPPORT, 1 },
#else
    { "EAFNOSUPPORT", -1, 0 },
#endif
#ifdef EAGAIN
    { "EAGAIN", (long long)EAGAIN, 1 },
#else
    { "EAGAIN", -1, 0 },
#endif
#ifdef EALREADY
    { "EALREADY", (long long)EALREADY, 1 },
#else
    { "EALREADY", -1, 0 },
#endif
#ifdef EBADF
    { "EBADF", (long long)EBADF, 1 },
#else
    { "EBADF", -1, 0 },
#endif
#ifdef EBADMSG
    { "EBADMSG", (long long)EBADMSG, 1 },
#else
    { "EBADMSG", -1, 0 },
#endif
#ifdef EBADRPC
    { "EBADRPC", (long long)EBADRPC, 1 },
#else
    { "EBADRPC", -1, 0 },
#endif
#ifdef EBUSY
    { "EBUSY", (long long)EBUSY, 1 },
#else
    { "EBUSY", -1, 0 },
#endif
#ifdef ECHILD
    { "ECHILD", (long long)ECHILD, 1 },
#else
    { "ECHILD", -1, 0 },
#endif
#ifdef ECOMM
    { "ECOMM", (long long)ECOMM, 1 },
#else
    { "ECOMM", -1, 0 },
#endif
#ifdef ECONNABORTED
    { "ECONNABORTED", (long long)ECONNABORTED, 1 },
#else
    { "ECONNABORTED", -1, 0 },
#endif
#ifdef ECONNREFUSED
    { "ECONNREFUSED", (long long)ECONNREFUSED, 1 },
#else
    { "ECONNREFUSED", -1, 0 },
#endif
#ifdef ECONNRESET
    { "ECONNRESET", (long long)ECONNRESET, 1 },
#else
    { "ECONNRESET", -1, 0 },
#endif
#ifdef EDEADLK
    { "EDEADLK", (long long)EDEADLK, 1 },
#else
    { "EDEADLK", -1, 0 },
#endif
#ifdef EDESTADDRREQ
    { "EDESTADDRREQ", (long long)EDESTADDRREQ, 1 },
#else
    { "EDESTADDRREQ", -1, 0 },
#endif
#ifdef EDIRTY
    { "EDIRTY", (long long)EDIRTY, 1 },
#else
    { "EDIRTY", -1, 0 },
#endif
#ifdef EDOM
    { "EDOM", (long long)EDOM, 1 },
#else
    { "EDOM", -1, 0 },
#endif
#ifdef EDQUOT
    { "EDQUOT", (long long)EDQUOT, 1 },
#else
    { "EDQUOT", -1, 0 },
#endif
#ifdef EEXIST
    { "EEXIST", (long long)EEXIST, 1 },
#else
    { "EEXIST", -1, 0 },
#endif
#ifdef EFAULT
    { "EFAULT", (long long)EFAULT, 1 },
#else
    { "EFAULT", -1, 0 },
#endif
#ifdef EFBIG
    { "EFBIG", (long long)EFBIG, 1 },
#else
    { "EFBIG", -1, 0 },
#endif
#ifdef EFTYPE
    { "EFTYPE", (long long)EFTYPE, 1 },
#else
    { "EFTYPE", -1, 0 },
#endif
#ifdef EHOSTDOWN
    { "EHOSTDOWN", (long long)EHOSTDOWN, 1 },
#else
    { "EHOSTDOWN", -1, 0 },
#endif
#ifdef EHOSTUNREACH
    { "EHOSTUNREACH", (long long)EHOSTUNREACH, 1 },
#else
    { "EHOSTUNREACH", -1, 0 },
#endif
#ifdef EIDRM
    { "EIDRM", (long long)EIDRM, 1 },
#else
    { "EIDRM", -1, 0 },
#endif
#ifdef EILSEQ
    { "EILSEQ", (long long)EILSEQ, 1 },
#else
    { "EILSEQ", -1, 0 },
#endif
#ifdef EINPROGRESS
    { "EINPROGRESS", (long long)EINPROGRESS, 1 },
#else
    { "EINPROGRESS", -1, 0 },
#endif
#ifdef EINTR
    { "EINTR", (long long)EINTR, 1 },
#else
    { "EINTR", -1, 0 },
#endif
#ifdef EINVAL
    { "EINVAL", (long long)EINVAL, 1 },
#else
    { "EINVAL", -1, 0 },
#endif
#ifdef EIO
    { "EIO", (long long)EIO, 1 },
#else
    { "EIO", -1, 0 },
#endif
#ifdef EISCONN
    { "EISCONN", (long long)EISCONN, 1 },
#else
    { "EISCONN", -1, 0 },
#endif
#ifdef EISDIR
    { "EISDIR", (long long)EISDIR, 1 },
#else
    { "EISDIR", -1, 0 },
#endif
#ifdef ELOOP
    { "ELOOP", (long long)ELOOP, 1 },
#else
    { "ELOOP", -1, 0 },
#endif
#ifdef EMFILE
    { "EMFILE", (long long)EMFILE, 1 },
#else
    { "EMFILE", -1, 0 },
#endif
#ifdef EMLINK
    { "EMLINK", (long long)EMLINK, 1 },
#else
    { "EMLINK", -1, 0 },
#endif
#ifdef EMSGSIZE
    { "EMSGSIZE", (long long)EMSGSIZE, 1 },
#else
    { "EMSGSIZE", -1, 0 },
#endif
#ifdef EMULTIHOP
    { "EMULTIHOP", (long long)EMULTIHOP, 1 },
#else
    { "EMULTIHOP", -1, 0 },
#endif
#ifdef ENAMETOOLONG
    { "ENAMETOOLONG", (long long)ENAMETOOLONG, 1 },
#else
    { "ENAMETOOLONG", -1, 0 },
#endif
#ifdef ENETDOWN
    { "ENETDOWN", (long long)ENETDOWN, 1 },
#else
    { "ENETDOWN", -1, 0 },
#endif
#ifdef ENETRESET
    { "ENETRESET", (long long)ENETRESET, 1 },
#else
    { "ENETRESET", -1, 0 },
#endif
#ifdef ENETUNREACH
    { "ENETUNREACH", (long long)ENETUNREACH, 1 },
#else
    { "ENETUNREACH", -1, 0 },
#endif
#ifdef ENFILE
    { "ENFILE", (long long)ENFILE, 1 },
#else
    { "ENFILE", -1, 0 },
#endif
#ifdef ENOBUFS
    { "ENOBUFS", (long long)ENOBUFS, 1 },
#else
    { "ENOBUFS", -1, 0 },
#endif
#ifdef ENODATA
    { "ENODATA", (long long)ENODATA, 1 },
#else
    { "ENODATA", -1, 0 },
#endif
#ifdef ENODEV
    { "ENODEV", (long long)ENODEV, 1 },
#else
    { "ENODEV", -1, 0 },
#endif
#ifdef ENOENT
    { "ENOENT", (long long)ENOENT, 1 },
#else
    { "ENOENT", -1, 0 },
#endif
#ifdef ENOEXEC
    { "ENOEXEC", (long long)ENOEXEC, 1 },
#else
    { "ENOEXEC", -1, 0 },
#endif
#ifdef ENOLCK
    { "ENOLCK", (long long)ENOLCK, 1 },
#else
    { "ENOLCK", -1, 0 },
#endif
#ifdef ENOLINK
    { "ENOLINK", (long long)ENOLINK, 1 },
#else
    { "ENOLINK", -1, 0 },
#endif
#ifdef ENOMEM
    { "ENOMEM", (long long)ENOMEM, 1 },
#else
    { "ENOMEM", -1, 0 },
#endif
#ifdef ENOMSG
    { "ENOMSG", (long long)ENOMSG, 1 },
#else
    { "ENOMSG", -1, 0 },
#endif
#ifdef ENONET
    { "ENONET", (long long)ENONET, 1 },
#else
    { "ENONET", -1, 0 },
#endif
#ifdef ENOPROTOOPT
    { "ENOPROTOOPT", (long long)ENOPROTOOPT, 1 },
#else
    { "ENOPROTOOPT", -1, 0 },
#endif
#ifdef ENOSPC
    { "ENOSPC", (long long)ENOSPC, 1 },
#else
    { "ENOSPC", -1, 0 },
#endif
#ifdef ENOSR
    { "ENOSR", (long long)ENOSR, 1 },
#else
    { "ENOSR", -1, 0 },
#endif
#ifdef ENOSTR
    { "ENOSTR", (long long)ENOSTR, 1 },
#else
    { "ENOSTR", -1, 0 },
#endif
#ifdef ENOSYS
    { "ENOSYS", (long long)ENOSYS, 1 },
#else
    { "ENOSYS", -1, 0 },
#endif
#ifdef ENOTBLK
    { "ENOTBLK", (long long)ENOTBLK, 1 },
#else
    { "ENOTBLK", -1, 0 },
#endif
#ifdef ENOTCONN
    { "ENOTCONN", (long long)ENOTCONN, 1 },
#else
    { "ENOTCONN", -1, 0 },
#endif
#ifdef ENOTDIR
    { "ENOTDIR", (long long)ENOTDIR, 1 },
#else
    { "ENOTDIR", -1, 0 },
#endif
#ifdef ENOTEMPTY
    { "ENOTEMPTY", (long long)ENOTEMPTY, 1 },
#else
    { "ENOTEMPTY", -1, 0 },
#endif
#ifdef ENOTSOCK
    { "ENOTSOCK", (long long)ENOTSOCK, 1 },
#else
    { "ENOTSOCK", -1, 0 },
#endif
#ifdef ENOTTY
    { "ENOTTY", (long long)ENOTTY, 1 },
#else
    { "ENOTTY", -1, 0 },
#endif
#ifdef ENXIO
    { "ENXIO", (long long)ENXIO, 1 },
#else
    { "ENXIO", -1, 0 },
#endif
#ifdef EOPNOTSUPP
    { "EOPNOTSUPP", (long long)EOPNOTSUPP, 1 },
#else
    { "EOPNOTSUPP", -1, 0 },
#endif
#ifdef EPERM
    { "EPERM", (long long)EPERM, 1 },
#else
    { "EPERM", -1, 0 },
#endif
#ifdef EPFNOSUPPORT
    { "EPFNOSUPPORT", (long long)EPFNOSUPPORT, 1 },
#else
    { "EPFNOSUPPORT", -1, 0 },
#endif
#ifdef EPIPE
    { "EPIPE", (long long)EPIPE, 1 },
#else
    { "EPIPE", -1, 0 },
#endif
#ifdef EPROCLIM
    { "EPROCLIM", (long long)EPROCLIM, 1 },
#else
    { "EPROCLIM", -1, 0 },
#endif
#ifdef EPROCUNAVAIL
    { "EPROCUNAVAIL", (long long)EPROCUNAVAIL, 1 },
#else
    { "EPROCUNAVAIL", -1, 0 },
#endif
#ifdef EPROGMISMATCH
    { "EPROGMISMATCH", (long long)EPROGMISMATCH, 1 },
#else
    { "EPROGMISMATCH", -1, 0 },
#endif
#ifdef EPROGUNAVAIL
    { "EPROGUNAVAIL", (long long)EPROGUNAVAIL, 1 },
#else
    { "EPROGUNAVAIL", -1, 0 },
#endif
#ifdef EPROTO
    { "EPROTO", (long long)EPROTO, 1 },
#else
    { "EPROTO", -1, 0 },
#endif
#ifdef EPROTONOSUPPORT
    { "EPROTONOSUPPORT", (long long)EPROTONOSUPPORT, 1 },
#else
    { "EPROTONOSUPPORT", -1, 0 },
#endif
#ifdef EPROTOTYPE
    { "EPROTOTYPE", (long long)EPROTOTYPE, 1 },
#else
    { "EPROTOTYPE", -1, 0 },
#endif
#ifdef ERANGE
    { "ERANGE", (long long)ERANGE, 1 },
#else
    { "ERANGE", -1, 0 },
#endif
#ifdef EREMCHG
    { "EREMCHG", (long long)EREMCHG, 1 },
#else
    { "EREMCHG", -1, 0 },
#endif
#ifdef EREMOTE
    { "EREMOTE", (long long)EREMOTE, 1 },
#else
    { "EREMOTE", -1, 0 },
#endif
#ifdef EROFS
    { "EROFS", (long long)EROFS, 1 },
#else
    { "EROFS", -1, 0 },
#endif
#ifdef ERPCMISMATCH
    { "ERPCMISMATCH", (long long)ERPCMISMATCH, 1 },
#else
    { "ERPCMISMATCH", -1, 0 },
#endif
#ifdef ERREMOTE
    { "ERREMOTE", (long long)ERREMOTE, 1 },
#else
    { "ERREMOTE", -1, 0 },
#endif
#ifdef ESHUTDOWN
    { "ESHUTDOWN", (long long)ESHUTDOWN, 1 },
#else
    { "ESHUTDOWN", -1, 0 },
#endif
#ifdef ESOCKTNOSUPPORT
    { "ESOCKTNOSUPPORT", (long long)ESOCKTNOSUPPORT, 1 },
#else
    { "ESOCKTNOSUPPORT", -1, 0 },
#endif
#ifdef ESPIPE
    { "ESPIPE", (long long)ESPIPE, 1 },
#else
    { "ESPIPE", -1, 0 },
#endif
#ifdef ESRCH
    { "ESRCH", (long long)ESRCH, 1 },
#else
    { "ESRCH", -1, 0 },
#endif
#ifdef ESRMNT
    { "ESRMNT", (long long)ESRMNT, 1 },
#else
    { "ESRMNT", -1, 0 },
#endif
#ifdef ESTALE
    { "ESTALE", (long long)ESTALE, 1 },
#else
    { "ESTALE", -1, 0 },
#endif
#ifdef ETIME
    { "ETIME", (long long)ETIME, 1 },
#else
    { "ETIME", -1, 0 },
#endif
#ifdef ETIMEDOUT
    { "ETIMEDOUT", (long long)ETIMEDOUT, 1 },
#else
    { "ETIMEDOUT", -1, 0 },
#endif
#ifdef ETOOMANYREFS
    { "ETOOMANYREFS", (long long)ETOOMANYREFS, 1 },
#else
    { "ETOOMANYREFS", -1, 0 },
#endif
#ifdef ETXTBSY
    { "ETXTBSY", (long long)ETXTBSY, 1 },
#else
    { "ETXTBSY", -1, 0 },
#endif
#ifdef EUSERS
    { "EUSERS", (long long)EUSERS, 1 },
#else
    { "EUSERS", -1, 0 },
#endif
#ifdef EWOULDBLOCK
    { "EWOULDBLOCK", (long long)EWOULDBLOCK, 1 },
#else
    { "EWOULDBLOCK", -1, 0 },
#endif
#ifdef EXDEV
    { "EXDEV", (long long)EXDEV, 1 },
#else
    { "EXDEV", -1, 0 },
#endif
#ifdef ENOCIGAR
    { "ENOCIGAR", (long long)ENOCIGAR, 1 },
#else
    { "ENOCIGAR", -1, 0 },
#endif
};

static struct constant signals[] = {
#ifdef SIGABRT
    { "SIGABRT", (long long)SIGABRT, 1 },
#else
    { "SIGABRT", -1, 0 },
#endif
#ifdef SIGALRM
    { "SIGALRM", (long long)SIGALRM, 1 },
#else
    { "SIGALRM", -1, 0 },
#endif
#ifdef SIGBUS
    { "SIGBUS", (long long)SIGBUS, 1 },
#else
    { "SIGBUS", -1, 0 },
#endif
#ifdef SIGCHLD
    { "SIGCHLD", (long long)SIGCHLD, 1 },
#else
    { "SIGCHLD", -1, 0 },
#endif
#ifdef SIGCONT
    { "SIGCONT", (long long)SIGCONT, 1 },
#else
    { "SIGCONT", -1, 0 },
#endif
#ifdef SIGFPE
    { "SIGFPE", (long long)SIGFPE, 1 },
#else
    { "SIGFPE", -1, 0 },
#endif
#ifdef SIGHUP
    { "SIGHUP", (long long)SIGHUP, 1 },
#else
    { "SIGHUP", -1, 0 },
#endif
#ifdef SIGILL
    { "SIGILL", (long long)SIGILL, 1 },
#else
    { "SIGILL", -1, 0 },
#endif
#ifdef SIGINT
    { "SIGINT", (long long)SIGINT, 1 },
#else
    { "SIGINT", -1, 0 },
#endif
#ifdef SIGKILL
    { "SIGKILL", (long long)SIGKILL, 1 },
#else
    { "SIGKILL", -1, 0 },
#endif
#ifdef SIGPIPE
    { "SIGPIPE", (long long)SIGPIPE, 1 },
#else
    { "SIGPIPE", -1, 0 },
#endif
#ifdef SIGQUIT
    { "SIGQUIT", (long long)SIGQUIT, 1 },
#else
    { "SIGQUIT", -1, 0 },
#endif
#ifdef SIGSEGV
    { "SIGSEGV", (long long)SIGSEGV, 1 },
#else
    { "SIGSEGV", -1, 0 },
#endif
#ifdef SIGSTOP
    { "SIGSTOP", (long long)SIGSTOP, 1 },
#else
    { "SIGSTOP", -1, 0 },
#endif
#ifdef SIGTERM
    { "SIGTERM", (long long)SIGTERM, 1 },
#else
    { "SIGTERM", -1, 0 },
#endif
#ifdef SIGTSTP
    { "SIGTSTP", (long long)SIGTSTP, 1 },
#else
    { "SIGTSTP", -1, 0 },
#endif
#ifdef SIGTTIN
    { "SIGTTIN", (long long)SIGTTIN, 1 },
#else
    { "SIGTTIN", -1, 0 },
#endif
#ifdef SIGTTOU
    { "SIGTTOU", (long long)SIGTTOU, 1 },
#else
    { "SIGTTOU", -1, 0 },
#endif
#ifdef SIGUSR1
    { "SIGUSR1", (long long)SIGUSR1, 1 },
#else
    { "SIGUSR1", -1, 0 },
#endif
#ifdef SIGUSR2
    { "SIGUSR2", (long long)SIGUSR2, 1 },
#else
    { "SIGUSR2", -1, 0 },
#endif
#ifdef SIGPOLL
    { "SIGPOLL", (long long)SIGPOLL, 1 },
#else
    { "SIGPOLL", -1, 0 },
#endif
#ifdef SIGPROF
    { "SIGPROF", (long long)SIGPROF, 1 },
#else
    { "SIGPROF", -1, 0 },
#endif
#ifdef SIGSYS
    { "SIGSYS", (long long)SIGSYS, 1 },
#else
    { "SIGSYS", -1, 0 },
#endif
#ifdef SIGTRAP
    { "SIGTRAP", (long long)SIGTRAP, 1 },
#else
    { "SIGTRAP", -1, 0 },
#endif
#ifdef SIGURG
    { "SIGURG", (long long)SIGURG, 1 },
#else
    { "SIGURG", -1, 0 },
#endif
#ifdef SIGVTALRM
    { "SIGVTALRM", (long long)SIGVTALRM, 1 },
#else
    { "SIGVTALRM", -1, 0 },
#endif
#ifdef SIGXCPU
    { "SIGXCPU", (long long)SIGXCPU, 1 },
#else
    { "SIGXCPU", -1, 0 },
#endif
#ifdef SIGXFSZ
    { "SIGXFSZ", (long long)SIGXFSZ, 1 },
#else
    { "SIGXFSZ", -1, 0 },
#endif
#ifdef SIG_BLOCK
    { "SIG_BLOCK", (long long)SIG_BLOCK, 1 },
#else
    { "SIG_BLOCK", -1, 0 },
#endif
#ifdef SIG_SETMASK
    { "SIG_SETMASK", (long long)SIG_SETMASK, 1 },
#else
    { "SIG_SETMASK", -1, 0 },
#endif
#ifdef SIG_UNBLOCK
    { "SIG_UNBLOCK", (long long)SIG_UNBLOCK, 1 },
#else
    { "SIG_UNBLOCK", -1, 0 },
#endif
#ifdef SIG_DFL
    { "SIG_DFL", (long long)(intptr_t)SIG_DFL, 1 },
#else
    { "SIG_DFL", -1, 0 },
#endif
#ifdef SIG_IGN
    { "SIG_IGN", (long long)(intptr_t)SIG_IGN, 1 },
#else
    { "SIG_IGN", -1, 0 },
#endif
#ifdef SIG_ERR
    { "SIG_ERR", (long long)(intptr_t)SIG_ERR, 1 },
#else
    { "SIG_ERR", -1, 0 },
#endif
};

static struct constant misc[] = {
#ifdef O_BINARY
    { "O_BINARY", (long long)O_BINARY, 1 },
#else
    { "O_BINARY", 0, 1 },
#endif
#if !defined(_WIN32)
    { "RUSAGE_SELF", (long long)RUSAGE_SELF, 1 },
#else
    { "RUSAGE_SELF", -1, 0 },
#endif
#if !defined(_WIN32)
    { "_SC_CLK_TCK", (long long)_SC_CLK_TCK, 1 },
#else
    { "_SC_CLK_TCK", -1, 0 },
#endif
};

#define COUNT(a) (sizeof(a) / sizeof((a)[0]))

/* Operating system and architecture names, as System.Info reports them;
 * the same spellings as src/platform.h */
#if defined(_WIN32)
# define HOST_OS "mingw32"
#elif defined(__APPLE__)
# define HOST_OS "darwin"
#elif defined(__linux__)
# define HOST_OS "linux"
#elif defined(__FreeBSD__) || defined(__DragonFly__)
# define HOST_OS "freebsd"
#elif defined(__NetBSD__)
# define HOST_OS "netbsd"
#elif defined(__OpenBSD__)
# define HOST_OS "openbsd"
#else
# define HOST_OS "unknown"
#endif
#if defined(__x86_64__) || defined(_M_X64)
# define HOST_ARCH "x86_64"
#elif defined(__aarch64__) || defined(_M_ARM64)
# define HOST_ARCH "aarch64"
#else
# define HOST_ARCH "unknown"
#endif

/* -- HsBaseConfig.h -------------------------------------------------------- */

#define DEF(NAME)	printf("#define %s 1\n", #NAME)
#define UNDEF(NAME)	printf("/* #undef %s */\n", #NAME)

static void header(void)
{
    size_t i;
    puts("/* HsBaseConfig.h: generated by mksysconfig at build time; do not edit. */");
    puts("#ifndef HSBASECONFIG_H");
    puts("#define HSBASECONFIG_H");
    puts("");
    puts("/* Headers and functions, fixed per operating system */");
    DEF(STDC_HEADERS);
    DEF(HAVE_CTYPE_H);
    DEF(HAVE_ERRNO_H);
    DEF(HAVE_FCNTL_H);
    DEF(HAVE_LIMITS_H);
    DEF(HAVE_SIGNAL_H);
    DEF(HAVE_STDINT_H);
    DEF(HAVE_INTTYPES_H);
    DEF(HAVE_STDLIB_H);
    DEF(HAVE_STRING_H);
    DEF(HAVE_TIME_H);
    DEF(HAVE_SYS_STAT_H);
    DEF(HAVE_SYS_TYPES_H);
    DEF(HAVE_LONG_LONG);
    DEF(HAVE_WCTYPE_H);
    DEF(HAVE_ISWSPACE);
    DEF(HAVE_FTRUNCATE);
#if defined(_WIN32)
    DEF(HAVE_WINDOWS_H);
    DEF(HAVE_WINSOCK_H);
    DEF(HAVE__CHSIZE);
    DEF(HAVE_SYS_TIMEB_H);
    DEF(HAVE_FTIME);
    DEF(HAVE_TZNAME);
# if defined(__MINGW32__)
    DEF(HAVE_UNISTD_H);
    DEF(HAVE_DIRENT_H);
    DEF(HAVE_SYS_TIME_H);
    DEF(HAVE_UTIME_H);
    DEF(HAVE_GETTIMEOFDAY);
    DEF(TIME_WITH_SYS_TIME);
# endif
#else
    DEF(HAVE_UNISTD_H);
    DEF(HAVE_DIRENT_H);
    DEF(HAVE_STRINGS_H);
    DEF(HAVE_TERMIOS_H);
    DEF(HAVE_UTIME_H);
    DEF(HAVE_SYS_RESOURCE_H);
    DEF(HAVE_SYS_SELECT_H);
    DEF(HAVE_SYS_TIME_H);
    DEF(HAVE_SYS_TIMES_H);
    DEF(HAVE_SYS_UTSNAME_H);
    DEF(HAVE_SYS_WAIT_H);
    DEF(TIME_WITH_SYS_TIME);
    DEF(HAVE_FORK);
    DEF(HAVE_WORKING_FORK);
    DEF(HAVE_GETRUSAGE);
    DEF(HAVE_GETTIMEOFDAY);
    DEF(HAVE_GMTIME_R);
    DEF(HAVE_LOCALTIME_R);
    DEF(HAVE_LSTAT);
    DEF(HAVE_SETITIMER);
    DEF(HAVE_TIMES);
    DEF(HAVE_STRUCT_TM_TM_ZONE);
    DEF(HAVE_TM_ZONE);
#endif
    /* Deliberately off everywhere: vfork is deprecated and fork is fine,
     * readdir_r is deprecated in favour of readdir, altzone is Solaris. */
    UNDEF(HAVE_VFORK);
    UNDEF(HAVE_WORKING_VFORK);
    UNDEF(HAVE_VFORK_H);
    UNDEF(HAVE_READDIR_R);
    UNDEF(HAVE_GETCLOCK);
    UNDEF(HAVE_SYS_TIMERS_H);
    UNDEF(TM_IN_SYS_TIME);
    puts("#define HAVE_DECL_ALTZONE 0");
    puts("/* readdir() leaves errno alone at end of directory */");
    puts("#define READDIR_ERRNO_EOF 0");
    puts("");
    puts("/* Haskell types corresponding to C types */");
    for (i = 0; i < (size_t)nhtypes; i++)
	printf("#define HTYPE_%s %s\n", htypes[i].macro, htypes[i].hs);
    puts("");
    puts("/* errno values, -1 when the system does not define one */");
    for (i = 0; i < COUNT(errnos); i++)
	printf("#define CONST_%s %lld\n", errnos[i].name, errnos[i].value);
    puts("");
    puts("/* signal numbers and sigprocmask actions */");
    for (i = 0; i < COUNT(signals); i++)
	printf("#define CONST_%s %lld\n", signals[i].name, signals[i].value);
    printf("#define CONST_O_BINARY %lld\n", misc[0].value);
    puts("");
    puts("#endif /* HSBASECONFIG_H */");
}

/* -- Cuddles/SysConfig.hs ----------------------------------------------------- */

/* HTYPE_UNSIGNED_LONG -> HsType_unsigned_long */
static const char *hs_type_name(const char *macro)
{
    static char buf[64];
    char *p;
    strcpy(buf, "HsType_");
    p = buf + strlen(buf);
    for (; *macro; macro++)
	*p++ = (*macro >= 'A' && *macro <= 'Z') ? *macro - 'A' + 'a' : *macro;
    *p = 0;
    return buf;
}

static void haskell(void)
{
    size_t i;
    puts("-- Cuddles.SysConfig: generated by mksysconfig at build time; do not edit.");
    puts("--");
    puts("-- The platform-dependent facts the base library needs: which Haskell");
    puts("-- type each C type corresponds to, the errno and signal constants, and");
    puts("-- the operating system and architecture names.");
    puts("");
    puts("module Cuddles.SysConfig where");
    puts("");
    puts("import Cuddles.Prelude");
    puts("");
    puts("-- Haskell types corresponding to C types");
    for (i = 0; i < (size_t)nhtypes; i++)
	printf("type %s = %s\n", hs_type_name(htypes[i].macro), htypes[i].hs);
    puts("");
    puts("-- errno values; -1 where the system does not define one");
    for (i = 0; i < COUNT(errnos); i++)
	printf("const_%s :: Num a => a\nconst_%s = %lld\n",
	       errnos[i].name, errnos[i].name, errnos[i].value);
    puts("");
    puts("-- signal numbers, sigprocmask actions and signal dispositions");
    for (i = 0; i < COUNT(signals); i++)
	printf("const_%s :: Num a => a\nconst_%s = %lld\n",
	       signals[i].name, signals[i].name, signals[i].value);
    puts("");
    puts("-- miscellaneous constants");
    for (i = 0; i < COUNT(misc); i++)
	if (misc[i].defined)
	    printf("const_%s :: Num a => a\nconst_%s = %lld\n",
		   misc[i].name, misc[i].name, misc[i].value);
    puts("readdirErrnoEOF :: Num a => a");
    puts("readdirErrnoEOF = 0");
    puts("");
    puts("-- operating system and architecture, as System.Info reports them");
    printf("hostOS :: String\nhostOS = \"%s\"\n", HOST_OS);
    printf("hostArch :: String\nhostArch = \"%s\"\n", HOST_ARCH);
}

int main(int argc, char **argv)
{
    collect_htypes();
    if (argc == 2 && strcmp(argv[1], "--haskell") == 0)
	haskell();
    else if (argc == 2 && strcmp(argv[1], "--header") == 0)
	header();
    else {
	fprintf(stderr, "usage: mksysconfig --header | --haskell\n");
	return 2;
    }
    return 0;
}
