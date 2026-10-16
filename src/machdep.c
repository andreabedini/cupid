/* --------------------------------------------------------------------------
 * Machine dependent code
 * RISCOS specific code provided by Bryan Scatergood, JBS
 * Macintosh specific code provided by Hans Aberg (haberg@matematik.su.se)
 * HaskellScript code and recursive directory search (now just one level)
 *  provided by Daan Leijen (leijen@fwi.uva.nl)
 *
 * The Hugs 98 system is Copyright (c) Mark P Jones, Alastair Reid, the
 * Yale Haskell Group, and the OGI School of Science & Engineering at OHSU,
 * 1994-2003, All rights reserved.  It is distributed as free software under
 * the license in the file "License", which is included in the distribution.
 * ------------------------------------------------------------------------*/
#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "opts.h"
#include "strutil.h"
#include "machdep.h"
#include "evaluator.h" /* installSubsystems() etc. proto only */

/*#define DEBUG_SEARCH*/

#include <signal.h>
#include <sys/types.h>
#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#include <limits.h>
#include <sys/stat.h>
#if HAVE_DIRENT_H
# include <dirent.h>
#endif
#include <locale.h>

/* Windows include files */
#if HAVE_IO_H
# include <io.h>
#endif
#if HAVE_WINDOWS_H
# include <windows.h>
#endif

#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#if IS_WINDOWS
# include <process.h>
#endif
#if defined(__APPLE__)
# include <mach-o/dyld.h> /* _NSGetExecutablePath */
#endif

/* --------------------------------------------------------------------------
 * Find information about a file:
 * ------------------------------------------------------------------------*/

/* Find time stamp and size of file */
void getFileInfo(char *f, Time *tm, long *sz) {
    struct stat scbuf;
    if (!stat(f, &scbuf)) {
        *tm = scbuf.st_mtime;
        *sz = (long)(scbuf.st_size);
    } else {
        *tm = 0;
        *sz = 0;
    }
}

/* Is f readable (and also, a regular file?) */
bool readable(char *f, bool isReg) {
    struct stat scbuf;
    return (!stat(f, &scbuf) && (scbuf.st_mode & S_IREAD) /* readable     */
            && (!isReg || S_ISREG(scbuf.st_mode))         /* regular file */
    );
}

/* Is f a directory? */
bool pathIsDirectory(char *f) {
    struct stat scbuf;
    return !stat(f, &scbuf) && S_ISDIR(scbuf.st_mode);
}

/* --------------------------------------------------------------------------
 * Search for script files on the CUPID path:
 * ------------------------------------------------------------------------*/

static char *homeDir(void);

static void searchChr(int);
static void searchStr(char *);
static bool tryEndings(void);
static bool find1(char *);
static bool find2(char *);
static char *expandVariable(char *);
static char *skipVariable(char *);
static char *nextPath(char *);
static bool samePath(char *, char *);
#if DOS_FILENAMES
static bool isPathSep(char *);
#endif
static bool scanSubDirs(char *);

#if DOS_FILENAMES
# define SLASH '\\'
# define isSLASH(c) ((c) == '\\' || (c) == '/')
# define PATHSEP ';'
# define isPATHSEP(x) isPathSep(x)
# define DLL_ENDING ".dll"
#else
# define SLASH '/'
# define isSLASH(c) ((c) == SLASH)
# define PATHSEP ':'
# define isPATHSEP(x) (*(x) == PATHSEP)
# define DLL_ENDING ".so"
#endif

#if HAVE_GETMODULEFILENAME
static HMODULE cupidModule = (HMODULE)0;
static char *cupidRoot = 0;

extern void setCupidModule(HMODULE);
extern bool setCupidRoot(char *);

void setCupidModule(HMODULE hmod) {
    cupidModule = hmod;
}

bool setCupidRoot(char *s) {
    char *newRoot = malloc(strlen(s) + 1);

    if (!newRoot)
        return false;

    strcpy(newRoot, s);

    if (cupidRoot)
        free(cupidRoot);
    cupidRoot = newRoot;
    return true;
}

#endif

#if !HAVE_GETMODULEFILENAME
/* The library directory of an installation that may have been moved:
 * CUPIDDIR_FROM_BINDIR, the library directory relative to the directory
 * of the executables, resolved against the directory that the running
 * executable really lives in, symbolic links followed.  NULL if there is
 * no packages directory there, as for an executable in the build tree, if
 * the build put the libraries outside the prefix, or on a system where an
 * executable cannot find itself.  Worked out once. */
static char *relocatedCupidDir(void) {
# if defined(CUPIDDIR_FROM_BINDIR) && (defined(__linux__) || defined(__APPLE__))
    static bool done = false;
    static char dir[PATH_MAX + 1];
    char exe[PATH_MAX + 1];
    char real[PATH_MAX + 1];
    char *slash;

    if (done)
        return dir[0] ? dir : NULL;
    done = true;
    dir[0] = '\0';
#  if defined(__linux__)
    ssize_t n = readlink("/proc/self/exe", exe, sizeof(exe) - 1);
    if (n < 0)
        return NULL;
    exe[n] = '\0';
#  else
    uint32_t size = sizeof(exe);
    if (_NSGetExecutablePath(exe, &size) != 0)
        return NULL;
#  endif
    if (realpath(exe, real) == NULL || (slash = strrchr(real, '/')) == NULL)
        return NULL;
    *slash = '\0';
    if (snprintf(exe, sizeof(exe), "%s/%s", real, CUPIDDIR_FROM_BINDIR) >=
            (int)sizeof(exe) ||
        realpath(exe, dir) == NULL) {
        dir[0] = '\0';
        return NULL;
    }
    if (snprintf(exe, sizeof(exe), "%s/packages", dir) >= (int)sizeof(exe) ||
        !pathIsDirectory(exe)) {
        dir[0] = '\0';
        return NULL;
    }
    return dir;
# else
    return NULL;
# endif
}
#endif

/* Directory containing libraries/Prelude.hs */
char *cupiddir(void) {
#if HAVE_GETMODULEFILENAME
    /* On Windows, we can find the binary we're running and it's
     * conventional to put the libraries in the same place.
     */
    static char dir[FILENAME_MAX + 1] = "";

    if (cupidRoot)
        return cupidRoot;

    if (dir[0] == '\0') { /* not initialised yet */
        char *slash = NIL;
        char *cupiddir = getenv("CUPIDDIR");

        if (cupiddir) {
            strncpy(dir, cupiddir, FILENAME_MAX);
        } else {
            GetModuleFileName(cupidModule, dir, FILENAME_MAX + 1);
            if (dir[0] == '\0') { /* GetModuleFileName must have failed */
                return CUPIDDIR;
            }
            if ((slash = strrchr(dir, SLASH)) != NULL) {
                /* truncate after directory name */
                *slash = '\0';
            }
        }
    }
    return dir;
#else
    /* The environment variable if it is set; then the libraries next to
     * the executable, for an installation that has been moved, such as a
     * release archive unpacked anywhere; then the directory the build
     * installed them in. */
    char *cupiddir = getenv("CUPIDDIR");
    char *moved;
    if (cupiddir)
        return cupiddir;
    moved = relocatedCupidDir();
    return moved ? moved : CUPIDDIR;
#endif
}

static char *homeDir(void) {
    return getenv("HOME");
}

/* Find absolute pathname of file */
char *RealPath(char *s) {
#if HAVE__FULLPATH /* Windows */
    static char path[FILENAME_MAX + 1];
    _fullpath(path, s, FILENAME_MAX + 1);
#elif HAVE_REALPATH /* eg Unix */
    static char path[PATH_MAX + 1];
    /* The name passed in may be this very buffer, returned by an earlier
     * call.  realpath(3) may write into its result before it has finished
     * reading its input -- macOS does, and resolves the overlap to "/" --
     * so resolve into a separate buffer. */
    char resolved[PATH_MAX + 1];
    if (strlen(s) > PATH_MAX) {
        return s;
    }
    if (realpath(s, resolved) == NULL) {
        return s;
    }
    strcpy(path, resolved);
#else
    static char path[FILENAME_MAX + 1];

    path[sizeof(path) - 1] = '\0';
    if (strlen(s) <= (sizeof(path) - 1)) {
        strcpy(path, s);
    } else {
        return s;
    }
#endif
    return path;
}

static char searchBuf[FILENAME_MAX + 1];
static int searchPos;

#define searchReset(n) searchBuf[searchPos = (n)] = '\0'

/* Add single character to search buffer */
static void searchChr(int c) {
    if (searchPos < FILENAME_MAX) {
        searchBuf[searchPos++] = (char)c;
        searchBuf[searchPos] = '\0';
    }
}

/* Add string to search buffer */
static void searchStr(char *s) {
    while (*s && searchPos < FILENAME_MAX)
        searchBuf[searchPos++] = *s++;
    searchBuf[searchPos] = '\0';
}

/* Try each of the listed endings */
static bool tryEndings(void) {
    int save;
    char *sp;

    save = searchPos;
    sp = cupidSuffixes;
    while (*sp) {
        for (; *sp && !isPATHSEP(sp); sp++)
            if (searchPos < FILENAME_MAX)
                searchBuf[searchPos++] = *sp;
        searchBuf[searchPos] = '\0';
        if (readable(searchBuf, true))
            return true;
        if (*sp)
            sp++;
        searchReset(save);
    }
    return false;
}

#if DOS_FILENAMES
/* Does 'sep' mark the end of a valid path? */
static bool isPathSep(char *sep) {
    /* ';' is always a separator */
    /* ':' is a separator iff it is not followed by a slash or backslash, */
    /* as in the drive letters of C:\cupid and C:/cupid                  */
    return *sep == ';' || (*sep == ':' && !isSLASH(sep[1]));
}
#endif

/* scandir, June 98 Daan Leijen
   searches the direct subdirectories of a directory for a file
   (excluding directories that start with a dot)

   input: searchbuf contains base directory (not SLASH terminated)
              argument name contains the module name
   output: true: searchBuf contains the full filename
           false: searchBuf is garbage, file not found
*/

#if HAVE_DIRENT_H

static bool scanSubDirs(char *s) {
    DIR *dir;
    struct dirent *entry;
    struct stat statb;
    int save;

    if ((dir = opendir(searchBuf)) == NULL)
        errno = 0;
    else {
        searchChr(SLASH);
        save = searchPos;
        while ((entry = readdir(dir)) != NULL)
            if (entry->d_name[0] != '.') {
                searchStr(entry->d_name);
                if (stat(searchBuf, &statb) == 0 && S_ISDIR(statb.st_mode)) {
                    searchChr(SLASH);
                    if (find2(s)) {
                        closedir(dir);
                        return true;
                    }
                }
                searchReset(save);
            }
        closedir(dir);
    }
    return false;
}

#elif HAVE_WINDOWS_H

static bool scanSubDirs(char *s) {
    struct _finddata_t findInfo;
    long handle;
    int save;

    searchChr(SLASH);
    save = searchPos;
    searchStr("*.*");

    /* initiate the search */
    handle = _findfirst(searchBuf, &findInfo);
    if (handle == -1) {
        errno = 0;
        return false;
    }

    /* search all subdirectories */
    do {
        /* if we have a valid sub directory */
        if (((findInfo.attrib & _A_SUBDIR) == _A_SUBDIR) &&
            (findInfo.name[0] != '.')) {
            searchReset(save);
            searchStr(findInfo.name);
            searchChr(SLASH);
            if (find2(s)) {
                _findclose(handle);
                return true;
            }
        }
    } while (_findnext(handle, &findInfo) == 0);

    _findclose(handle);
    return false;
}

#else

static bool scanSubDirs(char *name) {
    return false;
}

#endif /* HAVE_WINDOWS_H || HAVE_DIRENT_H || (__MWERKS__ && macintosh) */

/* Variables that may be substituted in the path */

struct shellVariable {
    char *var_name;
    char *(*var_value)(void);
};

static struct shellVariable shell_var[] = {
    {"Cupid", &cupiddir}, {"Home", &homeDir}, {0, 0}};

/*
    findPathname nm = [ nm ++ e | e <- "" : cupidSuffixes ]
*/

/* Look for a file, trying various extensions Return ***input name*** if no
 * file was found */
char *findPathname(char *filename) {
    searchReset(0);
    searchStr(filename);
    if (!readable(searchBuf, true) && !tryEndings())
        searchStr("");
    return searchBuf;
}

/* Finding the filename corresponding to a module name:

    find maybe_dir nm = [ d ++ map dot2slash nm ++ e | d <- dirs, e <- exts ]
      where 
        dirs          = maybeToList maybe_dir ++ cupidPath
        exts          = cupidSuffixes          -- default: [".hs"]

        -- the dir is added if the importing module was found there, or
        -- was specified as an explicit filename.

        dot2slash c   = if c=='.' then slash else c
*/

/* Look for a module in the suggested dir Return NULL if no file was found */
char *findMInDir(char *dir, char *nm) {
    searchReset(0);
    searchStr(dir);
    searchChr(SLASH);
    if (find2(nm)) {
        return searchBuf;
    } else {
        return NULL;
    }
}

/* Look for a module Return NULL if no file was found */
char *findMPathname(char *name) {
    if (find1(name)) {
        return searchBuf;
    } else {
        return NULL;
    }
}

/* Search each directory of the path */
static bool find1(char *name) {
    char *pathpt = cupidPath;
    char *value;

    searchReset(0); /* look along the CUPIDPATH */
    if (pathpt) {
        while (*pathpt) {
            searchReset(0);
            if ((value = expandVariable(pathpt)) != NULL) {
                searchStr(value);
                pathpt = skipVariable(pathpt);
            }
            while (*pathpt && !isPATHSEP(pathpt))
                searchChr(*pathpt++);
            /* If the path entry ends in SLASH '*', search immediate subdirs */
            if (searchPos >= 2 && isSLASH(pathpt[-2]) && pathpt[-1] == '*') {
                searchPos -= 2;
                searchBuf[searchPos] = '\0';
                if (scanSubDirs(name))
                    return true;
            } else {
                searchChr(SLASH);
                if (find2(name))
                    return true;
            }
            if (isPATHSEP(pathpt))
                pathpt++;
        }
    }
    return false;
}

/* Expansion of initial MPW-style "shell-variables" of the form {varname} */
static char *expandVariable(char *pathpt) {
    if (*pathpt == '{') {
        int i, len;

        for (i = 0; shell_var[i].var_name != NULL; i++) {
            len = (int)strlen(shell_var[i].var_name);
            if (strncmp(pathpt + 1, shell_var[i].var_name, len) == 0 &&
                pathpt[len + 1] == '}') {
                return (*shell_var[i].var_value)();
            }
        }
    }
    return NULL;
}

/* Assuming expandVariable(pathpt) succeeded, skip past the variable */
static char *skipVariable(char *pathpt) {
    return strchr(pathpt + 1, '}') + 1;
}

/* Turn module name into a filename */
static bool find2(char *s) {
    char *sp;

    /* replace all dots in the module name with slashes */
    for (sp = s; *sp; sp++) {
        if (searchPos >= FILENAME_MAX) { /* module name (plus whatever was  */
            searchBuf[searchPos] = '\0'; /* already in searchBuf) too long  */
            return false;                /* to fit: fail instead of         */
        } /* overrunning searchBuf           */
        searchBuf[searchPos++] = *sp == '.' ? SLASH : *sp;
    }
    searchBuf[searchPos] = '\0';
    return tryEndings();
}

/* Return the directory part of the filename or "." if no directory. */
char *dirname(char *filename) {

#if DOS_FILENAMES
    /* Allow both / and \\ as delimiters */
    /* So we cannot make use of strrchr() */
    char *slash;

    slash = filename + strlen(filename) - 1;
    while (slash > filename) {
        if (isSLASH(*slash))
            break;
        slash--;
    }
    if (slash <= filename) {
        return strCopy(".");
    } else {
        return strnCopy(filename, slash - filename);
    }
#else
    char *slash = strrchr(filename, SLASH);
    if (!slash) {
        return strCopy(".");
    } else {
        return strnCopy(filename, (int)(slash - filename));
    }
#endif
}

/* --------------------------------------------------------------------------
 * Substitute old value of path into empty entries in new path
 * eg substPath("a:b:c::d:e","x:y:z") = "a:b:c:x:y:z:d:e"
 * ------------------------------------------------------------------------*/

/* Substitute sub path into new path */
char *substPath(char *new, char *sub) {
    bool substituted = false; /*   only allow one replacement */
    int maxlen = (int)(strlen(sub) + strlen(new)); /* safe upper bound */
    char *r = (char *)malloc(maxlen + 1); /* result string                */
    char *t = r;                          /* pointer into r               */
    char *next = new;                     /* next uncopied char in new    */
    char *start = next;                   /* start of last path component */
    if (r == 0) {
        errHead(0);
        errPrintf("String storage space exhausted");
        errFail();
    }
    do {
        if (isPATHSEP(next) || *next == '\0') {
            if (!substituted && next == start) {
                char *s = sub;
                for (; *s != '\0'; ++s) {
                    *t++ = *s;
                }
                substituted = true;
            }
            start = next + 1;
        }
    } while ((*t++ = *next++) != '\0');
    return uniqPath(r);
}

/* Remove duplicates from the path */
char *uniqPath(char *path) {
    char *pp;
    for (pp = path; *pp;) {
        char *prev;
        char *next = nextPath(pp);
        for (prev = path; prev != pp; prev = nextPath(prev))
            if (samePath(prev, pp))
                break;
        if (prev == pp) /* not found: keep entry */
            pp = next;
        else if (*next) /* found in middle: delete entry */
            strcpy(pp, next);
        else { /* found at end: delete last entry */
            if (pp != path)
                pp--;
            *pp = '\0';
        }
    }
    return realloc(path, strlen(path) + 1);
}

/* Advance to the start of the next entry in the path list */
static char *nextPath(char *pp) {
    while (*pp && !isPATHSEP(pp))
        pp++;
    if (*pp)
        pp++;
    return pp;
}

static bool samePath(char *pp1, char *pp2) {
    char *ppsave1, *ppsave2;
    char *value;

    /* initial substitution variable? */
    if ((value = expandVariable(pp1)) != NULL) {
        ppsave1 = skipVariable(pp1);
        pp1 = value;
    } else
        ppsave1 = 0;
    if ((value = expandVariable(pp2)) != NULL) {
        ppsave2 = skipVariable(pp2);
        pp2 = value;
    } else
        ppsave2 = 0;

    while (*pp1 && !isPATHSEP(pp1) && *pp2 && !isPATHSEP(pp2)) {
        if (*pp1 != *pp2)
            return false;
        if (*++pp1 == '\0' && ppsave1 != 0) { /* end of substitution */
            pp1 = ppsave1;
            ppsave1 = 0;
        }
        if (*++pp2 == '\0' && ppsave2 != 0) { /* end of substitution */
            pp2 = ppsave2;
            ppsave2 = 0;
        }
    }
    return (*pp1 == '\0' || isPATHSEP(pp1)) && (*pp2 == '\0' || isPATHSEP(pp2));
}

/* --------------------------------------------------------------------------
 * Read value from environment variable:
 * ------------------------------------------------------------------------*/

/* Return value of: environment variable named by var or: default value given
 * by def */
char *fromEnv(char *var, char *def) {
    char *s = getenv(var);
    return (s ? s : def);
}

/* --------------------------------------------------------------------------
 * Get time/date stamp for inclusion in compiled files:
 * ------------------------------------------------------------------------*/

#if PROFILING
/* Return time&date string must end with '\n' character */
char *timeString(void) {
    time_t clock;
    time(&clock);
    return (ctime(&clock));
}
#endif

/* --------------------------------------------------------------------------
 * Garbage collection notification:
 * ------------------------------------------------------------------------*/

bool gcMessages = false; /* true => print GC messages       */

/* Notify garbage collector start */
void gcStarted(void) {
    if (gcMessages) {
        Printf("{{Gc");
        FlushStdout();
    }
}

/* Notify garbage collector scans */
void gcScanning(void) {
    if (gcMessages) {
        Putchar(':');
        FlushStdout();
    }
}

/* Notify garbage collection done */
void gcRecovered(int recovered) {
    if (gcMessages) {
        Printf("%d}}", recovered);
        FlushStdout();
    }
}

Cell *CStackBase; /* Retain start of C control stack */

/* Garbage collect elements off */
void gcCStack(void) {
    Cell stackTop = NIL; /* C stack                         */
    Cell *ptr = &stackTop;
    if (((uintptr_t)ptr - (uintptr_t)CStackBase) & 3)
        fatal("gcCStack");

#define StackGrowsDown                                                         \
    while (ptr <= CStackBase)                                                  \
    markWithoutMove(*ptr++)
#define StackGrowsUp                                                           \
    while (ptr >= CStackBase)                                                  \
    markWithoutMove(*ptr--)
#define GuessDirection                                                         \
    if (ptr > CStackBase)                                                      \
        StackGrowsUp;                                                          \
    else                                                                       \
        StackGrowsDown

#if STACK_DIRECTION > 0
    StackGrowsUp;
#elif STACK_DIRECTION < 0
    StackGrowsDown;
#else
    GuessDirection;
#endif

#undef StackGrowsDown
#undef StackGrowsUp
#undef GuessDirection
}

/* --------------------------------------------------------------------------
 * Terminal dependent stuff:
 * ------------------------------------------------------------------------*/

#if HAVE_TERMIOS_H /* every POSIX target              */

/* grab the varargs prototype for ioctl */
# if HAVE_SYS_IOCTL_H
#  include <sys/ioctl.h>
# endif

# include <termios.h>
typedef struct termios TermParams;
# define getTerminal(fd, tp) tcgetattr(fd, &tp)
# define setTerminal(fd, tp) tcsetattr(fd, TCSAFLUSH, &tp)
# define getEcho(tp) ((tp.c_lflag | ECHO) != 0)
# define noEcho(tp) tp.c_lflag &= ~ECHO;
# define doEcho(tp) tp.c_lflag |= ECHO;
# define getBuff(tp) ((tp.c_lflag | ICANON) != 0)
# define noBuff(tp)                                                            \
     tp.c_lflag &= ~ICANON;                                                    \
     tp.c_cc[VMIN] = 1;                                                        \
     tp.c_cc[VTIME] = 0;
# define doBuff(tp)                                                            \
     tp.c_lflag |= ICANON;                                                     \
     tp.c_cc[VEOF] = '\04';                                                    \
     tp.c_cc[VEOL] = '\0';

static bool messedWithTerminal = false;
static TermParams originalSettings;

/* Restore terminal initial state */
void normalTerminal(void) {
    if (messedWithTerminal)
        setTerminal(fileno(stdin), originalSettings);
}

/* Restore terminal initial state from a signal handler.  tcsetattr is
 * async-signal-safe and fileno is not, so the descriptor is named and the
 * mode is applied at once, without waiting for output to drain. */
static void normalTerminalFromSignal(void) {
    if (messedWithTerminal)
        tcsetattr(0, TCSANOW, &originalSettings);
}

bool getEchoTerminal(int fd) {
    TermParams settings;

    getTerminal(fd, settings);
    return getEcho(settings);
}

void setEchoTerminal(int fd, bool echo) {
    TermParams settings;

    if (fd == 0 && !messedWithTerminal) {
        getTerminal(0, originalSettings);
        messedWithTerminal = true;
    }
    getTerminal(fd, settings);
    if (echo) {
        doEcho(settings);
    } else {
        noEcho(settings);
    }
    setTerminal(fd, settings);
}

bool getBuffTerminal(int fd) {
    TermParams settings;

    getTerminal(fd, settings);
    return getEcho(settings);
}

void setBuffTerminal(int fd, bool buffered) {
    TermParams settings;

    if (fd == 0 && !messedWithTerminal) {
        getTerminal(0, originalSettings);
        messedWithTerminal = true;
    }
    getTerminal(fd, settings);
    if (buffered) {
        doBuff(settings);
    } else {
        noBuff(settings);
    }
    setTerminal(fd, settings);
}

/* Determine width of terminal */
int getTerminalWidth(void) {
# ifdef TIOCGWINSZ
    static struct winsize terminalSize;
    ioctl(fileno(stdout), TIOCGWINSZ, &terminalSize);
    return (terminalSize.ws_col == 0) ? 80 : terminalSize.ws_col;
# else
    return 80;
# endif
}

#elif IS_WINDOWS

static bool messedWithTerminal = false;
static DWORD originalSettings;

int getTerminalWidth(void) {
    return 80;
}

/* Restore terminal initial state */
void normalTerminal(void) {
    if (messedWithTerminal) {
        HANDLE hIn;

        hIn = GetStdHandle(STD_INPUT_HANDLE);
        SetConsoleMode(hIn, originalSettings);
        messedWithTerminal = false;
    }
}

bool getEchoTerminal(int fd) {
    if (fd == 0) {
        DWORD mo;
        HANDLE hIn;

        hIn = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(hIn, &mo);
        return (mo & ENABLE_ECHO_INPUT) != 0;
    } else
        return false;
}

void setEchoTerminal(int fd, bool echo) {
    if (fd == 0) {
        DWORD mo;
        HANDLE hIn;

        hIn = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(hIn, &mo);
        if (!messedWithTerminal) {
            originalSettings = mo;
            messedWithTerminal = true;
        }
        if (echo)
            mo |= ENABLE_ECHO_INPUT;
        else
            mo &= ~ENABLE_ECHO_INPUT;
        SetConsoleMode(hIn, mo);
    }
}

bool getBuffTerminal(int fd) {
    if (fd == 0) {
        DWORD mo;
        HANDLE hIn;

        hIn = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(hIn, &mo);
        return (mo & ENABLE_LINE_INPUT) != 0;
    } else
        return false;
}

void setBuffTerminal(int fd, bool buffered) {
    if (fd == 0) {
        DWORD mo;
        HANDLE hIn;

        hIn = GetStdHandle(STD_INPUT_HANDLE);
        GetConsoleMode(hIn, &mo);
        if (!messedWithTerminal) {
            originalSettings = mo;
            messedWithTerminal = true;
        }
        if (buffered)
            mo |= ENABLE_LINE_INPUT;
        else
            mo &= ~ENABLE_LINE_INPUT;
        SetConsoleMode(hIn, mo);
    }
}

#endif /* HAVE_TERMIOS_H */

/* --------------------------------------------------------------------------
 * Interrupt handling:
 * ------------------------------------------------------------------------*/

volatile sig_atomic_t broken = false; /* pending break to be handled     */
                                      /* only ever set from the SIGINT   */
                                      /* handler, so it must stay        */
                                      /* sig_atomic_t, not Bool          */
static bool breakReqd = false;        /* currently trapping breaks       */
static bool trapBreak = false;        /* ever asked to trap breaks       */
static sigProto(ignoreBreak);

/* Set break trapping on if reqd, or off otherwise, returning old */
bool breakOn(bool reqd) {
    bool old = breakReqd;

    breakReqd = reqd;
    if (reqd) {
        trapBreak = true;
        if (broken) {       /* repond to break signal received */
            broken = false; /* whilst break trap disabled      */
            sigRaise(breakHandler);
            /* not reached */
        }
        ctrlbrk(ignoreBreak); /* install the flag-setting handler;*/
                              /* allowBreak() polls it and raises */
                              /* breakHandler() as a plain call.  */
    } else if (trapBreak) {   /* If we have been trapping breaks, */
        ctrlbrk(ignoreBreak); /* switch to deferring them.        */
    }
    return old;
}

/* Record but don't respond to break */
static sigHandler(ignoreBreak) {
    /* async-signal-safe: only sets the flag, nothing else.  ctrlbrk()   */
    /* installs this handler with sigaction() on POSIX, so (unlike the   */
    /* old signal()-based scheme) it does not need to be reinstalled     */
    /* here to survive delivery.                                        */
    broken = true;
    sigResume;
}

/* Fatal signals (including a stack-overflowing SIGSEGV) are handled on an
 * alternate signal stack: if we run out of C stack, the handler itself
 * still needs somewhere to run in order to report the problem and exit
 * cleanly instead of the process just dying with no message.  A fixed
 * size is used instead of SIGSTKSZ because on current glibc that is no
 * longer a compile-time constant.
 */
#if !IS_WINDOWS
# define PANIC_STACK_SIZE 65536
static char panicStack[PANIC_STACK_SIZE];
#endif

/* Is sig one the process raises against itself by faulting, rather than
 * one sent to ask it to stop? */
static bool isFaultSignal(int sig) {
    switch (sig) {
#ifdef SIGABRT
    case SIGABRT:
#endif
#ifdef SIGBUS
    case SIGBUS:
#endif
#ifdef SIGFPE
    case SIGFPE:
#endif
#ifdef SIGILL
    case SIGILL:
#endif
#ifdef SIGSEGV
    case SIGSEGV:
#endif
        return true;
    default:
        return false;
    }
}

static sigProto(panic);
/* Put the terminal back, report a fault, and let the signal end the
 * process.
 *
 * Only async-signal-safe calls are made here.  The signal can arrive in
 * the middle of malloc or stdio, and the interrupted code still holds
 * their locks, so the subsystems' exit hooks, fprintf and exit could all
 * deadlock or corrupt what they touch.  The handler is installed with
 * SA_RESETHAND, so the default disposition is already back in place;
 * raising the signal again leaves it pending, and it is delivered when
 * the handler returns.  The process then ends with the wait status and,
 * where the default asks for one, the core dump that the signal implies.
 * A signal sent to ask the process to stop is not unexpected and is not
 * reported. */
static sigHandler(panic) {
    static const char msg[] = "\nUnexpected signal\n";
    int savedErrno = errno;

#if HAVE_TERMIOS_H
    normalTerminalFromSignal();
#endif
    if (isFaultSignal(sig_arg)) {
        ssize_t written = write(2, msg, sizeof msg - 1);
        (void)written; /* nothing better to do if this fails */
    }
    raise(sig_arg);
    errno = savedErrno;
    sigResume;
}

#if IS_WINDOWS
BOOL WINAPI consoleHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {  /* Allows Cupid to be terminated    */
    case CTRL_CLOSE_EVENT: /* from the window's close menu.   */
        ExitProcess(0);
    }
    return false;
}
#endif

/* Install handlers for all fatal signals except SIGINT and SIGBREAK.
 *
 * What a signal does to the process is the program's decision, not the
 * interpreter's, so this is called by each main() and not by any
 * subsystem's install hook. */
void installSignalHandlers(void) {
#if IS_WINDOWS
    /* No alternate signal stack on Windows, and the C runtime's signal()
     * resets the handler before calling it, as SA_RESETHAND does. */
    SetConsoleCtrlHandler(consoleHandler, true);
    signal(SIGABRT, panic);
    signal(SIGFPE, panic);
    signal(SIGILL, panic);
    signal(SIGSEGV, panic);
    signal(SIGTERM, panic);
#else
    {
        stack_t ss;
        struct sigaction sa;

        ss.ss_sp = panicStack;
        ss.ss_size = sizeof(panicStack);
        ss.ss_flags = 0;
        sigaltstack(&ss, NULL); /* ok to fail (e.g. already smaller */
                                /* handler stack in place); panic() */
                                /* still runs, just without the     */
                                /* overflow protection then.        */

        sa.sa_handler = panic;
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_ONSTACK | SA_RESETHAND; /* see panic() */
# ifdef SIGABRT
        sigaction(SIGABRT, &sa, NULL);
# endif
# ifdef SIGBUS
        sigaction(SIGBUS, &sa, NULL);
# endif
# ifdef SIGFPE
        sigaction(SIGFPE, &sa, NULL);
# endif
# ifdef SIGHUP
        sigaction(SIGHUP, &sa, NULL);
# endif
# ifdef SIGILL
        sigaction(SIGILL, &sa, NULL);
# endif
# ifdef SIGQUIT
        sigaction(SIGQUIT, &sa, NULL);
# endif
# ifdef SIGSEGV
        sigaction(SIGSEGV, &sa, NULL);
# endif
# ifdef SIGTERM
        sigaction(SIGTERM, &sa, NULL);
# endif
    }
#endif
}

/* --------------------------------------------------------------------------
 * Shell escapes:
 * ------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Running an external program:
 *
 * The preprocessor filter, the editor and the FFI compile-and-link command
 * are built as an argv and handed straight to the operating system, so no
 * shell parses them.  A file name is module- or user-supplied and is not
 * trustworthy; as one element of an argv it is one argument whatever
 * punctuation it contains, which is why nothing here quotes anything.
 *
 * The commands that are meant to run a shell still do: the `:!' escape at
 * the prompt, primSystem, and an editor command prefixed with `!', all
 * through shellEsc below.
 *
 * A configured command (the -F filter, the editor template, MKDLL_CMD,
 * the FFI flags) is split into words on whitespace and on nothing else:
 * quotes, `$' and backticks in one are ordinary characters now.
 * ------------------------------------------------------------------------*/

void argvInit(Argv *a) {
    a->vec = NULL;
    a->used = 0;
    a->size = 0;
}

static void argvPush(Argv *a, char *w) {
    if (a->used + 2 > a->size) {
        size_t n = a->size ? 2 * a->size : 8;
        char **p = (char **)realloc(a->vec, n * sizeof(char *));
        if (!p) {
            errHead(0);
            errPrintf("Unable to build a command");
            errFail();
        }
        a->vec = p;
        a->size = n;
    }
    a->vec[a->used++] = w;
    a->vec[a->used] = NULL;
}

void argvWord(Argv *a, char *s) {
    argvPush(a, strCopy(s));
}

void argvWords(Argv *a, char *s) {
    for (;;) {
        char *start;
        while (*s && isspace((unsigned char)*s))
            s++;
        if (!*s)
            return;
        start = s;
        while (*s && !isspace((unsigned char)*s))
            s++;
        argvPush(a, strnCopy(start, (int)(s - start)));
    }
}

void argvFree(Argv *a) {
    size_t i;
    for (i = 0; i < a->used; i++)
        free(a->vec[i]);
    free(a->vec);
    argvInit(a);
}

#if !IS_WINDOWS

/* Wait for pid.  shield says to ignore SIGINT and SIGQUIT while waiting,
 * which is what system() did for us and what the editor needs: without it
 * a Ctrl-C meant for the editor runs the interpreter's break handler, which
 * longjmps out of here and leaves the child behind.  The filter does not
 * shield, because pclose() did not either.  Setting the dispositions after
 * the fork keeps them out of the child, and exec resets a handled signal to
 * its default there anyway. */
static int argvWait(ProcId pid, bool shield) {
    int status;
    void (*oldInt)(int) = NULL;
    void (*oldQuit)(int) = NULL;

    if (shield) {
        oldInt = signal(SIGINT, SIG_IGN);
        oldQuit = signal(SIGQUIT, SIG_IGN);
    }
    while (waitpid(pid, &status, 0) < 0) {
        if (errno != EINTR) {
            status = -1;
            break;
        }
    }
    if (shield) {
        signal(SIGINT, oldInt);
        signal(SIGQUIT, oldQuit);
    }
    if (status != -1 && WIFEXITED(status))
        return WEXITSTATUS(status);
    return -1;
}

/* Fork and exec a->vec.  outFd, when it is not -1, becomes the child's
 * standard output; shutFd, when it is not -1, is closed in the child so
 * that the program does not inherit the other end of a pipe. */
static ProcId argvStart(Argv *a, int outFd, int shutFd) {
    ProcId pid;
    if (a->used == 0)
        return -1;
    fflush(NULL); /* buffered output must not be duplicated by the fork */
    if ((pid = fork()) < 0)
        return -1;
    if (pid == 0) {
        if (shutFd >= 0)
            close(shutFd);
        if (outFd >= 0 && outFd != 1) {
            dup2(outFd, 1);
            close(outFd);
        }
        execvp(a->vec[0], a->vec);
        _exit(127); /* the shell's code for a command it cannot run */
    }
    return pid;
}

int argvRun(Argv *a, bool sync) {
    ProcId pid;
    /* sync is ignored, as it was when this went through system() */
    (void)sync;
    if ((pid = argvStart(a, -1, -1)) < 0)
        return -1;
    return argvWait(pid, true);
}

FILE *argvOpen(Argv *a, ProcId *pidp) {
    int fd[2];
    FILE *f;
    ProcId pid;
    if (pipe(fd) < 0)
        return NULL;
    pid = argvStart(a, fd[1], fd[0]);
    close(fd[1]);
    if (pid < 0) {
        close(fd[0]);
        return NULL;
    }
    if ((f = fdopen(fd[0], "r")) == NULL) {
        close(fd[0]);
        argvWait(pid, false);
        return NULL;
    }
    *pidp = pid;
    return f;
}

int argvClose(FILE *f, ProcId pid) {
    fclose(f);
    return argvWait(pid, false);
}

#else /* IS_WINDOWS: the same three entry points on _spawnvp.  Like the
       * rest of the Windows support here, this is written to the
       * documented interface but is not verified on a Windows host. */

int argvRun(Argv *a, bool sync) {
    intptr_t r;
    if (a->used == 0)
        return -1;
    fflush(NULL);
    r = _spawnvp(sync ? _P_WAIT : _P_NOWAIT, a->vec[0],
                 (const char *const *)a->vec);
    if (r < 0)
        return -1;
    return sync ? (int)r : 0;
}

FILE *argvOpen(Argv *a, ProcId *pidp) {
    int fd[2], saved;
    intptr_t pid;
    FILE *f;
    if (a->used == 0 || _pipe(fd, 4096, _O_BINARY) < 0)
        return NULL;
    fflush(NULL);
    saved = _dup(1); /* the child inherits our stdout, so aim it at the pipe */
    _dup2(fd[1], 1);
    _close(fd[1]);
    pid = _spawnvp(_P_NOWAIT, a->vec[0], (const char *const *)a->vec);
    _dup2(saved, 1);
    _close(saved);
    if (pid < 0) {
        _close(fd[0]);
        return NULL;
    }
    if ((f = _fdopen(fd[0], "r")) == NULL) {
        _close(fd[0]);
        return NULL;
    }
    *pidp = (ProcId)pid;
    return f;
}

int argvClose(FILE *f, ProcId pid) {
    int status;
    fclose(f);
    if (_cwait(&status, (intptr_t)pid, 0) < 0)
        return -1;
    return status;
}

#endif /* IS_WINDOWS */

/* Run cmd, or a shell, through the shell */
int shellEsc(char *cmd) {
#if HAVE_BIN_SH
    if (cmd[0] == '\0') {
        cmd = fromEnv("SHELL", "/bin/sh");
    }
#endif
    return system(cmd);
}

/* --------------------------------------------------------------------------
 * Floating point support:
 * ------------------------------------------------------------------------*/

static union {
    Float flVal;
    Cell clVal;
} fudgeCoerce;

Cell mkFloat(FloatPro fl) {
    fudgeCoerce.flVal = (Float)fl;
    return pair(FLOATCELL, fudgeCoerce.clVal);
}

FloatPro floatOf(Cell c) {
    fudgeCoerce.clVal = snd(c);
    return fudgeCoerce.flVal;
}

/* Make sure that floating point values print out in a form in which they could
 * also be entered as floats */
char *floatToString(FloatPro fl) {
    static char buffer1[32];
    static char buffer2[32];
    int i = 0, j = 0;

    snprintf(buffer1, sizeof buffer1, FloatFMT, fl);
    while (buffer1[i] && strchr("eE.", buffer1[i]) == 0)
        buffer2[j++] = buffer1[i++];
    if (buffer1[i] == '\0') {
        snprintf(buffer1, sizeof buffer1, "%.1f", fl);
        i = j = 0;
    } else if (buffer1[i] != '.') {
        buffer2[j++] = '.';
        buffer2[j++] = '0';
    }
    while ((buffer2[j++] = buffer1[i++]) != 0) {
    }
    return buffer2;
}

static union {
    Double dblVal;
    struct {
        Cell dblPart1, dblPart2;
    } cdVal;
} fudgeDCoerce;

Cell part1Double(DoublePro dbl) {
    fudgeDCoerce.dblVal = dbl;
    return fudgeDCoerce.cdVal.dblPart1;
}

Cell part2Double(DoublePro dbl) {
    fudgeDCoerce.dblVal = dbl;
    return fudgeDCoerce.cdVal.dblPart2;
}

DoublePro doubleFromParts(Cell c1, Cell c2) {
    fudgeDCoerce.cdVal.dblPart1 = c1;
    fudgeDCoerce.cdVal.dblPart2 = c2;
    return fudgeDCoerce.dblVal;
}

Cell bdTemp = NIL;

Cell mkDouble(DoublePro dbl) {
    Cell p1, p2;
    fudgeDCoerce.dblVal = dbl;
    bdTemp = mkInt(fudgeDCoerce.cdVal.dblPart1);
    p2 = mkInt(fudgeDCoerce.cdVal.dblPart2);
    p1 = bdTemp;
    bdTemp = NIL;
    return pair(DOUBLECELL, pair(p1, p2));
}

DoublePro doubleOf(Cell c) {
    fudgeDCoerce.cdVal.dblPart1 = intOf(fst(snd(c)));
    fudgeDCoerce.cdVal.dblPart2 = intOf(snd(snd(c)));
    return fudgeDCoerce.dblVal;
}

/* Make sure that floating point values print out in a form in which they could
 * also be entered as doubles */
char *doubleToString(DoublePro dbl) {
    static char buffer1[32];
    static char buffer2[32];
    int i = 0, j = 0;

    snprintf(buffer1, sizeof buffer1, DoubleFMT, dbl);
    while (buffer1[i] && strchr("eE.", buffer1[i]) == 0)
        buffer2[j++] = buffer1[i++];
    if (buffer1[i] == '\0') {
        snprintf(buffer1, sizeof buffer1, "%.1f", dbl);
        i = j = 0;
    } else if (buffer1[i] != '.') {
        buffer2[j++] = '.';
        buffer2[j++] = '0';
    }
    while ((buffer2[j++] = buffer1[i++]) != 0) {
    }
    return buffer2;
}

DoublePro stringToDouble(char *s) {
    return atof(s);
}

/*---------------------------------------------------------------------------
 * Int64-related operations:
 *-------------------------------------------------------------------------*/

int part1Int64(HsInt64 i) {
    return (int)(i >> 32);
}

int part2Int64(HsInt64 i) {
    return (int)(i);
}

HsInt64 int64FromParts(int c1, int c2) {
    return ((HsInt64)c1 << 32) | (HsInt64)((HsWord32)c2);
}

/*---------------------------------------------------------------------------
 * Printf-related operations:
 *-------------------------------------------------------------------------*/

/* --------------------------------------------------------------------------
 * Dynamic loading:
 * ------------------------------------------------------------------------*/

static void *getModDLL(char *);
static void *getDLL(char *);
static void *getDLLSymbol(void *, char *);

#if HAVE_DLFCN_H /* eg LINUX, SOLARIS, ULTRIX */

# include <stdio.h>
# include <dlfcn.h>

/* Load dll */
static void *getDLL(char *dll) {
    void *instance = dlopen(dll, 0
# if defined(RTLD_LAZY) /* eg SunOS4 doesn't have RTLD_NOW */
                                     | RTLD_LAZY
#  if defined(RTLD_GLOBAL)
                                     | RTLD_GLOBAL
#  endif
# elif defined(RTLD_NOW)
                                     | RTLD_NOW
# else /* eg FreeBSD doesn't have RTLD_LAZY */
                                     | 1
# endif
    );

    if (NULL == instance) {
        errHead(0);
        errPrintf("Error while importing DLL \"%s\":\n%s\n", dll, dlerror());
        errFail();
    }

    return instance;
}

/* Lookup dll symbol */
static void *getDLLSymbol(void *instance, char *symbol) {
    void *sym;

    if ((sym = dlsym(instance, symbol)) == 0) {
        errHead(0);
        errPrintf("Error loading sym:\n%s\n", dlerror());
        errFail();
    }

    return sym;
}

/* Free up DLL */
void freeDLL(void *dll) {
    if (dll) {
        /* No error checking done. */
        dlclose(dll);
    }
    return;
}

#elif HAVE_WINDOWS_H

/* Load dll */
static void *getDLL(char *dll) {
    HINSTANCE instance = LoadLibrary(dll);
    if (NULL == instance) {
        /* GetLastError allegedly provides more detail - in practice,
         * it tells you nothing more.
         */
        errHead(0);
        errPrintf("Error while importing DLL \"%s\"", dll);
        errFail();
    }
    /* fprintf(stderr, "Loaded DLL 0x%p\n",instance); fflush(stderr); */
    return instance;
}

/* Lookup dll symbol */
static void *getDLLSymbol(void *instance, char *symbol) {
    return (void *)GetProcAddress((HINSTANCE)instance, symbol);
}

void freeDLL(void *dll) {
    if (dll) {
        /* fprintf(stderr, "Freeing DLL 0x%p\n",dll); fflush(stderr); */
        FreeLibrary(dll);
    }
    return;
}

#else /* Dynamic loading not available */

/* Load dll */
static void *getDLL(char *dll) {
    return 0;
}
/* Load dll and lookup symbol */
static void *getDLLSymbol(void *dll, char *symbol) {
    return 0;
}

void freeDLL(void *dll) {
}

#endif /* Dynamic loading not available */

/* Strrchr(path,'.'), but only within the last path component: a '.' in a
 * directory name (e.g. "../dir/Foo" or "./Foo") must not be mistaken for a
 * file extension */
static char *lastExtDot(char *path) {
    char *base = path;
    char *s;
    for (s = path; *s; s++)
        if (isSLASH(*s))
            base = s + 1;
    return strrchr(base, '.');
}

/* Load DLL for module */
static void *getModDLL(char *file) {
#if HAVE_REALPATH && !HAVE__FULLPATH
    char dllPath[PATH_MAX + 1];
#else
    char dllPath[FILENAME_MAX + 1];
#endif
    char *dot;
    char *s;
    for (s = file; *s && !isSLASH(*s); s++)
        ;
    if (*s)
        strcpy(dllPath, file); /* pathname for module     */
    else /* if in this directory, prefix ./ so search won't use the path   */
        snprintf(dllPath, sizeof dllPath, ".%c%s", SLASH, file);
    dot = lastExtDot(dllPath); /* patch file extension    */
    if (dot == NULL || dot == file) {
        dot = dllPath + strlen(dllPath);
    }
    strcpy(dot, DLL_ENDING);
    return getDLL(dllPath);
}

char *mkFFIFilename2(char *file) {
#if HAVE_REALPATH && !HAVE__FULLPATH
    static char path[PATH_MAX + 1];
#else
    static char path[FILENAME_MAX + 1];
#endif
    char *dot = lastExtDot(file); /* patch file extension    */
    if (dot == NULL)
        dot = file + strlen(file);
    strcpy(path, file);
    strcpy(path + (dot - file), DLL_ENDING);
    return path;
}

/* Get DLL path for module */
char *mkFFIFilename(char *file) {
#if HAVE_REALPATH && !HAVE__FULLPATH
    static char path[PATH_MAX + 1];
#else
    static char path[FILENAME_MAX + 1];
#endif
    char *dot = lastExtDot(file); /* patch file extension    */
    if (dot == NULL)
        dot = file + strlen(file);
    strcpy(path, file);
    strcpy(path + (dot - file), ".c");
    return path;
}

#define INIT_MODULE_FUN "initModule"
#define API_VERSION_FUN "CupidAPIVersion"

/* Load dll containing prims for current module */
void needPrims(int version, void *dll) {
    /* Version 2-6: the Haskell module specifies what module to expect
     *              (via a needPrims_cupid decl).
     *
     * Version 0:   the extension DLL specifies the API version it assumes.
     *
     * Version 5 is refused below, in default: a stub built against it
     * has struct cupid_primInfo's first field as a control function
     * taking an Int code, not the cupid_subsystem pointer version 6
     * expects, and the two cannot be told apart by size or content.
     */
    switch (version) {
    case 2:
    case 3:
    case 4: {
        InitModuleFun4 initModule;
        if (!dll)
            dll = getModDLL(scriptFile);
        initModule = (InitModuleFun4)getDLLSymbol(dll, INIT_MODULE_FUN);
        if (initModule) {
            bool flg = setOldDLLFlag(true);
            (*initModule)(cupidAPI4());
            setScriptPrims(setPrimInfoDll(dll));
            setOldDLLFlag(flg);
            return;
        }
        break;
    }
    case 6: {
        InitModuleFun5 initModule;
        if (!dll)
            dll = getModDLL(scriptFile);
        initModule = (InitModuleFun5)getDLLSymbol(dll, INIT_MODULE_FUN);
        if (initModule) {
            bool flg = setOldDLLFlag(false);
            (*initModule)(cupidAPI5());
            setScriptPrims(setPrimInfoDll(dll));
            setOldDLLFlag(flg);
            return;
        }
        break;
    }
    case 0: {
        APIVersionFun versionFun;
        int version = 6;

        dll = getModDLL(scriptFile);
        versionFun = (APIVersionFun)getDLLSymbol(dll, API_VERSION_FUN);
        if (versionFun) {
            version = (*versionFun)();
        }
        needPrims(version, dll);
        return;
    }
    default: {
        errHead(0);
        errPrintf("This version of Cupid does not support FFI version %d\n",
                  version);
        errFail();
    }
    }
    errHead(0);
    errPrintf("Unable to load FFI primitives\n");
    errFail();
}

/* --------------------------------------------------------------------------
 * Compile and link an ffi file which can be dynamically loaded using
 * the above mechanisms.
 * ------------------------------------------------------------------------*/

/* "-I<cupiddir>/include", with the separators turned into the '/' every
 * supported compiler accepts. */
static char *includeArg(void) {
    char *dir = cupiddir();
    char *s = (char *)malloc(strlen(dir) + sizeof("-I/include"));
    char *p;
    if (!s) {
        errHead(0);
        errPrintf("Unable to build compilation command");
        errFail();
    }
    strcpy(s, "-I");
    p = s + 2;
    while (*dir) {
        *p++ = (*dir == SLASH) ? '/' : *dir;
        dir++;
    }
    strcpy(p, "/include");
    return s;
}

void compileAndLink(char *fn, char *flags) {
    Argv cmd;
    char *inc;

    argvInit(&cmd);

#if defined(MKDLL_VISUAL_STUDIO)
    /* find the location of fficupid.bat */
    /* is in the same directory */
    {
        char Buffer[MAX_PATH + 1];
        char Buffer2[MAX_PATH + 1];
        GetModuleFileName(GetModuleHandle(NULL), Buffer, MAX_PATH);
        strcpy(strrchr(Buffer, '.'), ".bat");
        GetShortPathName(Buffer, Buffer2, MAX_PATH);

        argvWord(&cmd, Buffer2);
        argvWords(&cmd, MKDLL_CMD);
    }
#elif defined(MKDLL_CMD)
    /* The compile and link command */
    argvWords(&cmd, MKDLL_CMD);
#endif

    /* Identify ourselves */
    argvWord(&cmd, "-D__CUPID__");

    /* the path to HsFFI.h */
    inc = includeArg();
    argvWord(&cmd, inc);
    free(inc);

    /* the output file, then the file to compile */
    argvWord(&cmd, "-o");
    argvWord(&cmd, mkFFIFilename2(fn));
    argvWord(&cmd, mkFFIFilename(fn));

    /* compiler and linker flags specified on Cupid command line */
    if (flags)
        argvWords(&cmd, flags);

    if (argvRun(&cmd, true) != 0) {
        size_t i;
        errHead(0);
        errPrintf("Error while running compilation command");
        for (i = 0; i < cmd.used; i++)
            errPrintf(" %s", cmd.vec[i]);
        argvFree(&cmd);
        errFail();
    }
    argvFree(&cmd);
}

/* --------------------------------------------------------------------------
 * Platform initialisation 
 * ------------------------------------------------------------------------*/
extern bool initSystem(void);
bool initSystem(void) {
    /* Called right away by main()  */
    setlocale(LC_CTYPE, "");
    return true;
}

/* --------------------------------------------------------------------------
 * Machine dependent control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const machdepRoots[] = {
    &bdTemp,
};

/* Register the roots */
void installMachdep(void) {
    registerRoots(machdepRoots);
}

/* Put the terminal back into its normal (non-raw) mode; shared by
 * RESET, BREAK and EXIT, which all did this before the codes went. */
void exitMachdep(void) {
    normalTerminal();
}

/* Clear the pending breakpoint buffer, and put the terminal back to
 * normal as RESET always also did. */
void resetMachdep(void) {
    bdTemp = NIL;
    exitMachdep();
}

/*-------------------------------------------------------------------------*/
