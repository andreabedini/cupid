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
 *
 * $RCSfile: machdep.c,v $
 * $Revision: 1.140 $
 * $Date: 2006/05/11 15:14:11 $
 * ------------------------------------------------------------------------*/
#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "opts.h"
#include "strutil.h"
#include "machdep.h"
#include "char.h"
#include "evaluator.h" /* everybody() proto only */

/*#define DEBUG_SEARCH*/

#include <signal.h>
#include <sys/types.h>
#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif
#include <limits.h>
#include <sys/stat.h>
#if HAVE_DIRENT_H
#  include <dirent.h>
#endif
#include <locale.h>

/* Hack for systems with unlimited path length (e.g. the Hurd), which
 * will not define PATH_MAX or MAXPATHLEN.  The Right Thing would be
 * to dynamically allocate these buffers, and use functions like
 * get_current_dir_name() and canonicalize_file_name().
 */
#if HAVE_REALPATH && !defined(MAXPATHLEN)
#ifdef PATH_MAX
#define MAXPATHLEN PATH_MAX
#else
#define MAXPATHLEN 4096
#endif
#endif

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

/* --------------------------------------------------------------------------
 * Prototypes for registry reading
 * ------------------------------------------------------------------------*/
#if USE_REGISTRY

/* where have we hidden things in the registry? */

extern String readRegString        (HKEY, String, String, String);

static Bool   local createKey      (HKEY, String, PHKEY, REGSAM);
static Bool   local queryValue     (HKEY, String, String, LPDWORD, LPBYTE, DWORD);
static Bool   local queryString    (HKEY,String,String,String*);
static Bool   local setValue       (HKEY, String, String, DWORD, LPBYTE, DWORD);
#endif

/* --------------------------------------------------------------------------
 * Find information about a file:
 * ------------------------------------------------------------------------*/

Void getFileInfo(String f, Time*tm, Long*sz)  /* find time stamp and size of file*/
{
    struct stat scbuf;
    if (!stat(f,&scbuf)) {
	*tm = scbuf.st_mtime;
	*sz = (Long)(scbuf.st_size);
    } else {
	*tm = 0;
	*sz = 0;
    }
}


Bool readable(String f, Bool isReg)        /* is f readable (and also, a regular file?) */
{
    struct stat scbuf;
    return (  !stat(f,&scbuf) 
	   && (scbuf.st_mode & S_IREAD) /* readable     */
	   && ( !isReg || (scbuf.st_mode & S_IFREG)) /* regular file */
	   );
}


/* --------------------------------------------------------------------------
 * Search for script files on the CUDDLES path:
 * ------------------------------------------------------------------------*/

static String local homeDir	  (void);



static String local normPath      (String);
static Void   local searchChr     (Int);
static Void   local searchStr     (String);
static Bool   local tryEndings    (Void);
static Bool   local find1	  (String);
static Bool   local find2	  (String);
static String local expandVariable (String);
static String local skipVariable  (String);
static String local nextPath      (String);
static Bool   local samePath      (String,String);
#if DOS_FILENAMES
static Bool   local isPathSep     (String);
#endif
static Bool   local scanSubDirs   (String);


#if DOS_FILENAMES
# define SLASH                   '\\'
# define isSLASH(c)              ((c)=='\\' || (c)=='/')
# define PATHSEP                 ';'
# define isPATHSEP(x)            isPathSep(x)
# define DLL_ENDING              ".dll"
#else
# define SLASH                   '/'
# define isSLASH(c)              ((c)==SLASH)
# define PATHSEP                 ':'
# define isPATHSEP(x)            (*(x) == PATHSEP)
# define DLL_ENDING              ".so"
#endif

#if HAVE_GETMODULEFILENAME
static HMODULE cuddlesModule = (HMODULE)0;
static String  cuddlesRoot   = 0;

extern Void setCuddlesModule (HMODULE);
extern Bool setCuddlesRoot   (String);

Void setCuddlesModule(HMODULE hmod)
{
    cuddlesModule = hmod;
}

Bool setCuddlesRoot(String s)
{
    String newRoot = malloc(strlen(s) + 1);
    
    if (!newRoot) return FALSE;
    
    strcpy(newRoot,s);
    
    if (cuddlesRoot) free(cuddlesRoot);
    cuddlesRoot = newRoot;
    return TRUE;
}

#endif


String cuddlesdir() {		/* directory containing libraries/Prelude.hs */
#if HAVE_GETMODULEFILENAME
    /* On Windows, we can find the binary we're running and it's
     * conventional to put the libraries in the same place.
     */
    static char dir[FILENAME_MAX+1] = "";
   
    if (cuddlesRoot)
	return cuddlesRoot;

    if ( dir[0] == '\0' ) { /* not initialised yet */
 	String slash = NIL;
	char *cuddlesdir = getenv("CUDDLESDIR");
	
	if (cuddlesdir) {
	    strncpy(dir,cuddlesdir,FILENAME_MAX);
	} else {
	    GetModuleFileName(cuddlesModule,dir,FILENAME_MAX+1);
	    if ( dir[0] == '\0' ) { /* GetModuleFileName must have failed */
		return CUDDLESDIR;
	    }
	    if ( (slash = strrchr(dir,SLASH)) != NULL ) {
		/* truncate after directory name */
		*slash = '\0';
	    }
	}
    }
    return dir;
#else
    /* On Unix systems, data is not typically stored relative to a binary
     * (it's also harder for a binary to determine where it lives).
     * First, check for an environment var, then fall back to
     * a configuration-time constant (--datadir=...).
     */
    char *cuddlesdir = getenv("CUDDLESDIR");
    return cuddlesdir ? cuddlesdir : CUDDLESDIR;
#endif
}

static String homeDir() {
    return getenv("HOME");
}



String local RealPath(String s)         /* Find absolute pathname of file  */
{
#if HAVE__FULLPATH  /* Windows */
    static char path[FILENAME_MAX+1];
    _fullpath(path,s,FILENAME_MAX+1);
#elif HAVE_REALPATH /* eg Unix */
    static char path[MAXPATHLEN+1];
    path[sizeof(path)-1] = '\0';
    if (strlen(s) <= (sizeof(path)-1)) {
	realpath(s,path);                
    } else {
	return s;
    }
#else
    static char path[FILENAME_MAX+1];

    path[sizeof(path)-1] = '\0';
    if (strlen(s) <= (sizeof(path)-1)) {
        strcpy(path,s);
    } else {
	return s;
    }
#endif
    return path;
}


static String local normPath(String s) /* Try, as much as possible, to normalize  */
{                     /* a pathname in some appropriate manner.  */
#if PATH_CANONICALIZATION
    String path = RealPath(s);
#if CASE_INSENSITIVE_FILENAMES
    strlwr(path);                       /* and convert to lowercase        */
#endif
    return path;
#else /* ! PATH_CANONICALIZATION */
    return s;
#endif /* ! PATH_CANONICALIZATION */
}

static char   searchBuf[FILENAME_MAX+1];
static Int    searchPos;

#define searchReset(n)          searchBuf[searchPos=(n)]='\0'

static Void local searchChr(Int c)  /* Add single character to search buffer   */
{
    if (searchPos<FILENAME_MAX) {
	searchBuf[searchPos++] = (char)c;
	searchBuf[searchPos]   = '\0';
    }
}

static Void local searchStr(String s)  /* Add string to search buffer             */
{
    while (*s && searchPos<FILENAME_MAX)
	searchBuf[searchPos++] = *s++;
    searchBuf[searchPos] = '\0';
}

static Bool local tryEndings()  /* Try each of the listed endings          */
{
    Int save;
    String sp;

    save = searchPos;
    sp = cuddlesSuffixes;
    while (*sp) {
	for ( ; *sp && ! isPATHSEP(sp); sp++)
	    if (searchPos<FILENAME_MAX)
		searchBuf[searchPos++] = *sp;
	searchBuf[searchPos] = '\0';
	if (readable(searchBuf,TRUE))
	    return TRUE;
	if (*sp)
	    sp++;
	searchReset(save);
    }
    return FALSE;
}

#if DOS_FILENAMES
static Bool local isPathSep(String sep)      /* does 'sep' mark the end of a valid path? */
{
    /* ';' is always a separator */
    /* ':' is a separator iff it is not followed by a backslash */
    /* (should test for <x>:\ but that seems too difficult) */
    return *sep == ';' || *sep == ':' && *(sep+1) != SLASH;
}
#endif

/* scandir, June 98 Daan Leijen
   searches the direct subdirectories of a directory for a file
   (excluding directories that start with a dot)

   input: searchbuf contains base directory (not SLASH terminated)
	      argument name contains the module name
   output: TRUE: searchBuf contains the full filename
	   FALSE: searchBuf is garbage, file not found
*/
	  

#if HAVE_DIRENT_H

static Bool scanSubDirs(String s)
{
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
		if (stat(searchBuf, &statb)==0 && S_ISDIR(statb.st_mode)) {
		    searchChr(SLASH);
		    if (find2(s)) {
			closedir(dir);
			return TRUE;
		    }
		}
		searchReset(save);
	    }
	closedir(dir);
    }
    return FALSE;
}

#elif HAVE_WINDOWS_H

static Bool scanSubDirs(String s)
{
    struct _finddata_t findInfo;
    long handle;
    int  save;
    
    searchChr(SLASH);
    save = searchPos;
    searchStr("*.*");
    
    /* initiate the search */
    handle = _findfirst( searchBuf, &findInfo );
    if (handle==-1) { errno = 0; return FALSE; }
    
    /* search all subdirectories */
    do {
	/* if we have a valid sub directory */
	if (((findInfo.attrib & _A_SUBDIR) == _A_SUBDIR) &&
	    (findInfo.name[0] != '.')) {
	    searchReset(save);
	    searchStr(findInfo.name);
	    searchChr(SLASH);
	    if (find2(s)) {
		_findclose( handle );
		return TRUE;
	    }
	}
    } while (_findnext( handle, &findInfo ) == 0);
    
    _findclose( handle );
    return FALSE;
}

#else

static Bool scanSubDirs(String name)
{   return FALSE;
}

#endif /* HAVE_WINDOWS_H || HAVE_DIRENT_H || (__MWERKS__ && macintosh) */

/* Variables that may be substituted in the path */

struct shellVariable {
    String var_name;
    String (*var_value) (Void);
};

static struct shellVariable shell_var[] = {
    { "Cuddles",		&cuddlesdir },
    { "Home",		&homeDir },
    { 0, 0 }
};

/*
    findPathname nm = [ nm ++ e | e <- "" : cuddlesSuffixes ]
*/

String findPathname(String filename)   /* Look for a file, trying various extensions */
{              /* Return ***input name*** if no file was found */
    searchReset(0);
    searchStr(filename);
    if (!readable(searchBuf,TRUE) && !tryEndings())
	searchStr("");
    return normPath(searchBuf);
}

/* Finding the filename corresponding to a module name:

    find maybe_dir nm = [ d ++ map dot2slash nm ++ e | d <- dirs, e <- exts ]
      where 
        dirs          = maybeToList maybe_dir ++ cuddlesPath
        exts          = cuddlesSuffixes		-- default: [".hs",".lhs"]

	-- the dir is added if the importing module was found there, or
	-- was specified as an explicit filename.

        dot2slash c   = if c=='.' then slash else c
*/

String findMInDir(String dir, String nm)       /* Look for a module in the suggested dir */
                                /* Return NULL if no file was found       */
{
    searchReset(0);
    searchStr(dir);
    searchChr(SLASH);
    if (find2(nm)) {
        return normPath(searchBuf);
    } else {
        return NULL;
    }
}

String findMPathname(String name)	    /* Look for a module                      */
{                      /* Return NULL if no file was found       */
    if (find1(name)) {
        return normPath(searchBuf);
    } else {
        return NULL;
    }
}

static Bool find1(String name)		/* Search each directory of the path */
{
    String pathpt = cuddlesPath;
    String value;

    searchReset(0);		/* look along the CUDDLESPATH */
    if (pathpt) {
	while (*pathpt) {
	    searchReset(0);
	    if ((value=expandVariable(pathpt)) != NULL) {
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
		    return TRUE;
	    } else {
		searchChr(SLASH);
		if (find2(name))
		    return TRUE;
	    }
	    if (isPATHSEP(pathpt))
		pathpt++;
	}
    } 
    return FALSE;    
}

/* Expansion of initial MPW-style "shell-variables" of the form {varname} */
static String local expandVariable(String pathpt)
{
    if (*pathpt=='{') {
	int i, len;

	for (i = 0; shell_var[i].var_name!=NULL; i++) {
	    len = strlen(shell_var[i].var_name);
	    if (strncmp(pathpt+1,shell_var[i].var_name,len)==0
		&& pathpt[len+1]=='}') {
		return (*shell_var[i].var_value)();
	    }
	}
    }
    return NULL;
}

/* Assuming expandVariable(pathpt) succeeded, skip past the variable */
static String local skipVariable(String pathpt)
{
    return strchr(pathpt+1,'}')+1;
}

static Bool local find2(String s)	/* Turn module name into a filename */
{
    String sp;

    /* replace all dots in the module name with slashes */
    for (sp = s; *sp; sp++) {
	if (searchPos>=FILENAME_MAX) {  /* module name (plus whatever was  */
	    searchBuf[searchPos] = '\0'; /* already in searchBuf) too long  */
	    return FALSE;                /* to fit: fail instead of         */
	}                                 /* overrunning searchBuf           */
	searchBuf[searchPos++] = *sp == '.' ? SLASH : *sp;
    }
    searchBuf[searchPos] = '\0';
    return tryEndings();
}

String dirname(String filename)	/* Return the directory part of the filename */
{		/* or "." if no directory.                   */
    
#if DOS_FILENAMES
    /* Allow both / and \\ as delimiters */
    /* So we cannot make use of strrchr() */
    String slash;

    slash = filename + strlen(filename) - 1;
    while (slash > filename) {
	if (isSLASH(*slash)) break;
	slash--;
    }
    if (slash <= filename) {
	return strCopy(".");
    } else {
	return strnCopy(filename, slash - filename);
    }
#else
    String slash = strrchr(filename,SLASH);
    if (!slash) {
	return strCopy(".");
    } else {
	return strnCopy(filename, slash - filename);
    }
#endif
}

/* --------------------------------------------------------------------------
 * Substitute old value of path into empty entries in new path
 * eg substPath("a:b:c::d:e","x:y:z") = "a:b:c:x:y:z:d:e"
 * ------------------------------------------------------------------------*/

String local substPath(String new, String sub) /* substitute sub path into new path*/
{
    Bool   substituted = FALSE;            /*   only allow one replacement */
    Int    maxlen      = strlen(sub) + strlen(new);    /* safe upper bound */
    String r = (String) malloc(maxlen+1);  /* result string                */
    String t = r;                          /* pointer into r               */
    String next = new;                     /* next uncopied char in new    */
    String start = next;                   /* start of last path component */
    if (r == 0) {
	errHead(0); errPrintf("String storage space exhausted");
	errFail();
    }
    do {
	if (isPATHSEP(next) || *next == '\0') {
	    if (!substituted && next == start) {
		String s = sub;
		for(; *s != '\0'; ++s) {
		    *t++ = *s;
		}
		substituted = TRUE;
	    }
	    start = next+1;
	}
    } while ((*t++ = *next++) != '\0');
    return uniqPath(r);
}

/* Remove duplicates from the path */
String uniqPath(String path)
{
    char *pp;
    for (pp = path; *pp; ) {
	char *prev;
	char *next = nextPath(pp);
	for (prev = path; prev != pp; prev = nextPath(prev))
	    if (samePath(prev,pp))
		break;
	if (prev == pp)		/* not found: keep entry */
	    pp = next;
	else if (*next)		/* found in middle: delete entry */
	    strcpy(pp, next);
	else {			/* found at end: delete last entry */
	    if (pp != path)
		pp--;
	    *pp = '\0';
	}
    }
    return realloc(path, strlen(path)+1);
}

/* Advance to the start of the next entry in the path list */
static String local nextPath(String pp)
{
    while (*pp && !isPATHSEP(pp))
	pp++;
    if (*pp)
	pp++;
    return pp;
}

static Bool local samePath(String pp1, String pp2)
{
    char *ppsave1, *ppsave2;
    char *value;

    /* initial substitution variable? */
    if ((value=expandVariable(pp1)) != NULL) {
	ppsave1 = skipVariable(pp1);
	pp1 = value;
    } else
	ppsave1 = 0;
    if ((value=expandVariable(pp2)) != NULL) {
	ppsave2 = skipVariable(pp2);
	pp2 = value;
    } else
	ppsave2 = 0;

    while (*pp1 && !isPATHSEP(pp1) && *pp2 && !isPATHSEP(pp2)) {
	if (*pp1 != *pp2)
	    return FALSE;
	if (*++pp1 == '\0' && ppsave1 != 0) {	/* end of substitution */
	    pp1 = ppsave1;
	    ppsave1 = 0;
	}
	if (*++pp2 == '\0' && ppsave2 != 0) {	/* end of substitution */
	    pp2 = ppsave2;
	    ppsave2 = 0;
	}
    }
    return (*pp1=='\0' || isPATHSEP(pp1)) && (*pp2=='\0' || isPATHSEP(pp2));
}

/* --------------------------------------------------------------------------
 * Read value from environment variable or registry:
 * ------------------------------------------------------------------------*/

String fromEnv(String var, String def)         /* return value of:                        */
                                /*     environment variable named by var   */
{                   /* or: default value given by def          */
    String s = getenv(var);     
    return (s ? s : def);
}

/* --------------------------------------------------------------------------
 * Get time/date stamp for inclusion in compiled files:
 * ------------------------------------------------------------------------*/

#if PROFILING
String timeString() {                   /* return time&date string         */
    time_t clock;                       /* must end with '\n' character    */
    time(&clock);
    return(ctime(&clock));
}
#endif

/* --------------------------------------------------------------------------
 * Garbage collection notification:
 * ------------------------------------------------------------------------*/

Bool gcMessages = FALSE;                /* TRUE => print GC messages       */

Void gcStarted() {                      /* Notify garbage collector start  */
    if (gcMessages) {
	Printf("{{Gc");
	FlushStdout();
    }
}

Void gcScanning() {                     /* Notify garbage collector scans  */
    if (gcMessages) {
	Putchar(':');
	FlushStdout();
    }
}

Void gcRecovered(Int recovered)		/* Notify garbage collection done  */
{
    if (gcMessages) {
	Printf("%d}}",recovered);
	FlushStdout();
    }
}

Cell *CStackBase;                       /* Retain start of C control stack */


Void gcCStack() {                       /* Garbage collect elements off    */
    Cell stackTop = NIL;                /* C stack                         */
    Cell *ptr = &stackTop;
    if (((long)(ptr) - (long)(CStackBase))&3)
	fatal("gcCStack");

#define StackGrowsDown  while (ptr<=CStackBase) markWithoutMove(*ptr++)
#define StackGrowsUp    while (ptr>=CStackBase) markWithoutMove(*ptr--)
#define GuessDirection  if (ptr>CStackBase) StackGrowsUp; else StackGrowsDown

#if STACK_DIRECTION > 0
    StackGrowsUp;
#elif STACK_DIRECTION < 0
    StackGrowsDown;
#else
    GuessDirection;
#endif


#undef  StackGrowsDown
#undef  StackGrowsUp
#undef  GuessDirection
}

/* --------------------------------------------------------------------------
 * Terminal dependent stuff:
 * ------------------------------------------------------------------------*/

#if HAVE_TERMIOS_H                      /* every POSIX target              */

/* grab the varargs prototype for ioctl */
#if HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif

#include <termios.h>
typedef  struct termios  TermParams;
#define  getTerminal(fd,tp) tcgetattr(fd, &tp)
#define  setTerminal(fd,tp) tcsetattr(fd, TCSAFLUSH, &tp)
#define  getEcho(tp)     ((tp.c_lflag | ECHO) != 0)
#define  noEcho(tp)      tp.c_lflag    &= ~ECHO;
#define  doEcho(tp)      tp.c_lflag    |= ECHO;
#define  getBuff(tp)     ((tp.c_lflag | ICANON) != 0)
#define  noBuff(tp)      tp.c_lflag    &= ~ICANON; \
			 tp.c_cc[VMIN]  = 1; \
			 tp.c_cc[VTIME] = 0;
#define  doBuff(tp)      tp.c_lflag    |= ICANON; \
			 tp.c_cc[VEOF]  = '\04'; \
			 tp.c_cc[VEOL]  = '\0';


static Bool messedWithTerminal = FALSE;
static TermParams originalSettings;

Void normalTerminal() {                 /* restore terminal initial state  */
    if (messedWithTerminal)
	setTerminal(fileno(stdin), originalSettings);
}

Bool getEchoTerminal(Int fd) {
    TermParams settings;

    getTerminal(fd, settings);
    return getEcho(settings);
}

Void setEchoTerminal(Int fd, Bool echo) {
    TermParams settings;

    if (fd==0 && !messedWithTerminal) {
	getTerminal(0, originalSettings);
	messedWithTerminal = TRUE;
    }
    getTerminal(fd, settings);
    if (echo) {
	doEcho(settings);
    } else {
	noEcho(settings);
    }
    setTerminal(fd, settings);
}

Bool getBuffTerminal(Int fd) {
    TermParams settings;

    getTerminal(fd, settings);
    return getEcho(settings);
}

Void setBuffTerminal(Int fd, Bool buffered) {
    TermParams settings;

    if (fd==0 && !messedWithTerminal) {
	getTerminal(0, originalSettings);
	messedWithTerminal = TRUE;
    }
    getTerminal(fd, settings);
    if (buffered) {
	doBuff(settings);
    } else {
	noBuff(settings);
    }
    setTerminal(fd, settings);
}

Int getTerminalWidth() {                /* determine width of terminal     */
#ifdef TIOCGWINSZ
    static struct winsize terminalSize;
    ioctl(fileno(stdout),TIOCGWINSZ,&terminalSize);
    return (terminalSize.ws_col==0)? 80 : terminalSize.ws_col;
#else
    return 80;
#endif
}

#elif IS_WINDOWS

static Bool messedWithTerminal = FALSE;
static DWORD originalSettings;

Int getTerminalWidth() {
    return 80;
}

Void normalTerminal() {                 /* restore terminal initial state  */
    if (messedWithTerminal) {
 	HANDLE hIn;

 	hIn = GetStdHandle(STD_INPUT_HANDLE);
 	SetConsoleMode(hIn, originalSettings);
	messedWithTerminal = FALSE;
    }
}

Bool getEchoTerminal(Int fd) {
    if (fd==0) {
 	DWORD mo;
 	HANDLE hIn;

 	hIn = GetStdHandle(STD_INPUT_HANDLE);
 	GetConsoleMode(hIn, &mo);
	return (mo & ENABLE_ECHO_INPUT)!=0;
    } else
	return FALSE;
}

Void setEchoTerminal(Int fd, Bool echo) {
    if (fd==0) {
 	DWORD mo;
 	HANDLE hIn;

 	hIn = GetStdHandle(STD_INPUT_HANDLE);
 	GetConsoleMode(hIn, &mo);
	if (!messedWithTerminal) {
	    originalSettings = mo;
	    messedWithTerminal = TRUE;
	}
	if (echo)
	    mo |= ENABLE_ECHO_INPUT;
	else
	    mo &= ~ENABLE_ECHO_INPUT;
 	SetConsoleMode(hIn, mo);
    }
}

Bool getBuffTerminal(Int fd) {
    if (fd==0) {
 	DWORD mo;
 	HANDLE hIn;

 	hIn = GetStdHandle(STD_INPUT_HANDLE);
 	GetConsoleMode(hIn, &mo);
	return (mo & ENABLE_LINE_INPUT)!=0;
    } else
	return FALSE;
}

Void setBuffTerminal(Int fd, Bool buffered) {
    if (fd==0) {
 	DWORD mo;
 	HANDLE hIn;

 	hIn = GetStdHandle(STD_INPUT_HANDLE);
 	GetConsoleMode(hIn, &mo);
	if (!messedWithTerminal) {
	    originalSettings = mo;
	    messedWithTerminal = TRUE;
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

volatile sig_atomic_t broken = FALSE;		/* pending break to be handled     */
						/* only ever set from the SIGINT   */
						/* handler, so it must stay        */
						/* sig_atomic_t, not Bool          */
static  Bool breakReqd = FALSE;		/* currently trapping breaks       */
static  Bool trapBreak = FALSE;		/* ever asked to trap breaks       */
static  sigProto(ignoreBreak);
static  Void local installHandlers (Void);

Bool breakOn(Bool reqd)                      /* set break trapping on if reqd,  */
{                            /* or off otherwise, returning old */
    Bool old  = breakReqd;

    breakReqd = reqd;
    if (reqd) {
	trapBreak = TRUE;
	if (broken) {                   /* repond to break signal received */
	    broken = FALSE;             /* whilst break trap disabled      */
	    sigRaise(breakHandler);
	    /* not reached */
	}
	ctrlbrk(ignoreBreak);		/* install the flag-setting handler;*/
					/* allowBreak() polls it and raises */
					/* breakHandler() as a plain call.  */
    } else if (trapBreak) {		/* If we have been trapping breaks, */
	ctrlbrk(ignoreBreak);		/* switch to deferring them.        */
    }
    return old;
}

static sigHandler(ignoreBreak) {        /* record but don't respond to break*/
    /* async-signal-safe: only sets the flag, nothing else.  ctrlbrk()   */
    /* installs this handler with sigaction() on POSIX, so (unlike the   */
    /* old signal()-based scheme) it does not need to be reinstalled     */
    /* here to survive delivery.                                        */
    broken = TRUE;
    sigResume;
}

#if !DONT_PANIC
/* Fatal signals (including a stack-overflowing SIGSEGV) are handled on an
 * alternate signal stack: if we run out of C stack, the handler itself
 * still needs somewhere to run in order to report the problem and exit
 * cleanly instead of the process just dying with no message.  A fixed
 * size is used instead of SIGSTKSZ because on current glibc that is no
 * longer a compile-time constant.
 */
#define PANIC_STACK_SIZE 65536
static char panicStack[PANIC_STACK_SIZE];

static sigProto(panic);
static sigHandler(panic) {              /* exit in a panic, on receipt of  */
    everybody(EXIT);                    /* an unexpected signal            */
    fprintf(stderr,"\nUnexpected signal\n");
    exit(1);
    sigResume;/*NOTREACHED*/
}
#endif /* !DONT_PANIC */

#if IS_WINDOWS
BOOL WINAPI consoleHandler(DWORD dwCtrlType) {
    switch (dwCtrlType) {		/* Allows Cuddles to be terminated    */
	case CTRL_CLOSE_EVENT :		/* from the window's close menu.   */
	    ExitProcess(0);
    }
    return FALSE;
}
#endif

static Void local installHandlers() { /* Install handlers for all fatal    */
				      /* signals except SIGINT and SIGBREAK*/
#if IS_WINDOWS
    SetConsoleCtrlHandler(consoleHandler,TRUE);
#endif
#if !DONT_PANIC
    {
	stack_t ss;
	struct sigaction sa;

	ss.ss_sp    = panicStack;
	ss.ss_size  = sizeof(panicStack);
	ss.ss_flags = 0;
	sigaltstack(&ss,NULL);          /* ok to fail (e.g. already smaller */
					 /* handler stack in place); panic() */
					 /* still runs, just without the     */
					 /* overflow protection then.        */

	sa.sa_handler = panic;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_ONSTACK;
# ifdef SIGABRT
	sigaction(SIGABRT,&sa,NULL);
# endif
# ifdef SIGBUS
	sigaction(SIGBUS,&sa,NULL);
# endif
# ifdef SIGFPE
	sigaction(SIGFPE,&sa,NULL);
# endif
# ifdef SIGHUP
	sigaction(SIGHUP,&sa,NULL);
# endif
# ifdef SIGILL
	sigaction(SIGILL,&sa,NULL);
# endif
# ifdef SIGQUIT
	sigaction(SIGQUIT,&sa,NULL);
# endif
# ifdef SIGSEGV
	sigaction(SIGSEGV,&sa,NULL);
# endif
# ifdef SIGTERM
	sigaction(SIGTERM,&sa,NULL);
# endif
    }
#endif /* !DONT_PANIC */

}

/* --------------------------------------------------------------------------
 * Shell escapes:
 * ------------------------------------------------------------------------*/

/* Append the quoted form of s to dst, so that a shell (cmd.exe on Windows,
 * a POSIX shell everywhere else) sees s as a single word no matter what
 * punctuation it contains - a file name is not trustworthy input, and both
 * popen() and system() (see shellEsc below) pass their argument straight
 * to a shell.  If dst is NULL, nothing is written, but the number of
 * characters shellQuote(dst,s) would write (not counting the terminating
 * NUL, which is written whenever dst is not NULL) is still returned, so a
 * caller can size (or grow) a buffer first, e.g.
 *
 *     char *buf = malloc((size_t)shellQuote(NULL,s) + 1);
 *     shellQuote(buf,s);
 *
 * On POSIX, s is wrapped in single quotes, with each embedded single quote
 * replaced by '\'' (close the quote, an escaped literal quote, reopen the
 * quote) - the standard way to quote arbitrary bytes for a POSIX shell.
 * On Windows, s is wrapped in double quotes instead, with embedded double
 * quotes backslash-escaped; this is the convention the MSVC runtime (and
 * hence cmd.exe / CreateProcess argv parsing) expects.
 */
Int shellQuote(String dst, String s)
{
    Int n = 0;
#if !IS_WINDOWS
#define QUOTE_CHAR   '\''
#define ESCAPED_QUOTE  "'\\''"		/* close, escaped quote, reopen    */
#else
#define QUOTE_CHAR   '"'
#define ESCAPED_QUOTE  "\\\""		/* backslash-escaped quote         */
#endif
    if (dst) dst[n] = QUOTE_CHAR;
    n++;
    for ( ; *s; s++) {
	if (*s == QUOTE_CHAR) {
	    if (dst) strcpy(dst+n, ESCAPED_QUOTE);
	    n += (Int)strlen(ESCAPED_QUOTE);
	} else {
	    if (dst) dst[n] = *s;
	    n++;
	}
    }
    if (dst) dst[n] = QUOTE_CHAR;
    n++;
    if (dst) dst[n] = '\0';
#undef QUOTE_CHAR
#undef ESCAPED_QUOTE
    return n;
}

Int
shellEsc(String cmd, Bool sync, Bool useShell) /* run a shell command (or shell)  */
{
#ifndef HAVE_WINDOWS_H

  /* currently ignore the 'useShell' and 'sync' flags */
#  if HAVE_BIN_SH
    if (cmd[0]=='\0') {
	cmd = fromEnv("SHELL","/bin/sh");
    }
# endif
    return system(cmd);
#else
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  BOOL bStatus;
  DWORD dwResult;

  if (useShell) {
    return system(cmd);
  }

  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);
  si.dwFlags = STARTF_USESHOWWINDOW;
  si.wShowWindow = SW_SHOW;
  
  bStatus = 
    CreateProcess(NULL,  /* app name is the first component of the command line string */
		  cmd, 
		  NULL,  /* default process security attributes */
		  NULL,  /* default prim. thread security attributes */
		  FALSE, /* don't inherit */
		  CREATE_NEW_CONSOLE,
		  NULL,  /* environment; same block as parent */
		  NULL,  /* current directory; same as parent */
		  &si,
		  &pi);
		    
  if (!bStatus) {
    return 1;
  } else {
    CloseHandle(pi.hThread);
    /*
     * Wait for the editor process to complete, or not. If we
     * don't wait for the editor process to complete, the user
     * will have to manually :(re)load the sources after having
     * save them within the editor. The default is to wait.
     */
    if (!sync) {
      CloseHandle(pi.hProcess);
      return 0;
    } else {
      dwResult = WaitForSingleObject(pi.hProcess, INFINITE);
      return (dwResult == WAIT_OBJECT_0 ? 0 : 1);
    }
  }
#endif
}



/* --------------------------------------------------------------------------
 * Floating point support:
 * ------------------------------------------------------------------------*/

static union {
    Float flVal;
    Cell  clVal;
} fudgeCoerce;

Cell mkFloat(FloatPro fl)
{
    fudgeCoerce.flVal = (Float)fl;
    return pair(FLOATCELL,fudgeCoerce.clVal);
}

FloatPro floatOf(Cell c)
{
    fudgeCoerce.clVal = snd(c);
    return fudgeCoerce.flVal;
}

String floatToString(FloatPro fl)                     /* Make sure that floating    */
{                               /* point values print out in  */
    static char buffer1[32];                 /* a form in which they could */
    static char buffer2[32];                 /* also be entered as floats  */
    Int i=0, j=0;

    sprintf(buffer1,FloatFMT,fl);
    while (buffer1[i] && strchr("eE.",buffer1[i])==0)
	buffer2[j++] = buffer1[i++];
    if (buffer1[i]=='\0') {
        sprintf(buffer1,"%.1f",fl);
        i = j = 0;
    } else if (buffer1[i]!='.') {
	buffer2[j++] = '.';
	buffer2[j++] = '0';
    }
    while ((buffer2[j++]=buffer1[i++])!=0) {
    }
    return buffer2;
}

static union {
    Double  dblVal;
    struct {
	Cell dblPart1,dblPart2;
    }      cdVal;
} fudgeDCoerce;

Cell part1Double(DoublePro dbl)
{
    fudgeDCoerce.dblVal = dbl;
    return fudgeDCoerce.cdVal.dblPart1;
}

Cell part2Double(DoublePro dbl)
{
    fudgeDCoerce.dblVal = dbl;
    return fudgeDCoerce.cdVal.dblPart2;
}

DoublePro doubleFromParts(Cell c1, Cell c2)
{
    fudgeDCoerce.cdVal.dblPart1 = c1;
    fudgeDCoerce.cdVal.dblPart2 = c2;
    return fudgeDCoerce.dblVal;
}

Cell bdTemp = NIL;

Cell mkDouble(DoublePro dbl)
{
    Cell p1,p2;
    fudgeDCoerce.dblVal = dbl;
    bdTemp = mkInt(fudgeDCoerce.cdVal.dblPart1);
    p2     = mkInt(fudgeDCoerce.cdVal.dblPart2);
    p1     = bdTemp;
    bdTemp = NIL;
    return pair(DOUBLECELL,pair(p1,p2));
}

DoublePro doubleOf(Cell c)
{
    fudgeDCoerce.cdVal.dblPart1 = intOf(fst(snd(c)));
    fudgeDCoerce.cdVal.dblPart2 = intOf(snd(snd(c)));
    return fudgeDCoerce.dblVal;
}

String doubleToString(DoublePro dbl)                   /* Make sure that floating    */
{                             /* point values print out in  */
    static char buffer1[32];                 /* a form in which they could */
    static char buffer2[32];                 /* also be entered as doubles */
    Int i=0, j=0;

    sprintf(buffer1,DoubleFMT,dbl);
    while (buffer1[i] && strchr("eE.",buffer1[i])==0)
	buffer2[j++] = buffer1[i++];
    if (buffer1[i]=='\0') {
        sprintf(buffer1,"%.1f",dbl);
        i = j = 0;
    } else if (buffer1[i]!='.') {
	buffer2[j++] = '.';
	buffer2[j++] = '0';
    }
    while ((buffer2[j++]=buffer1[i++])!=0) {
    }
    return buffer2;
}

DoublePro stringToDouble(String s)
{
    return atof(s);
}

/*---------------------------------------------------------------------------
 * Int64-related operations:
 *-------------------------------------------------------------------------*/

Int part1Int64(HsInt64 i)
{
    return (Int)(i >> 32);
}

Int part2Int64(HsInt64 i)
{
    return (Int)(i);
}

HsInt64 int64FromParts(Int c1, Int c2)
{
    return ((HsInt64)c1 << 32) | (HsInt64)((HsWord32)c2);
}

/*---------------------------------------------------------------------------
 * Printf-related operations:
 *-------------------------------------------------------------------------*/



/* --------------------------------------------------------------------------
 * Dynamic loading:
 * ------------------------------------------------------------------------*/

static void* local getModDLL (String);
static void* local getDLL (String);
static void* local getDLLSymbol (void*,String);

#if HAVE_DLFCN_H /* eg LINUX, SOLARIS, ULTRIX */

#include <stdio.h>
#include <dlfcn.h>

static void* local getDLL(String dll)  /* load dll */
{
    void *instance = dlopen(dll,
			    0
#if defined(RTLD_LAZY) /* eg SunOS4 doesn't have RTLD_NOW */
			    | RTLD_LAZY 
# if defined(RTLD_GLOBAL)
			    | RTLD_GLOBAL
# endif
#elif defined(RTLD_NOW)
			    | RTLD_NOW
#else /* eg FreeBSD doesn't have RTLD_LAZY */
			    | 1
#endif
			    );

    if (NULL == instance) {
	errHead(0); errPrintf("Error while importing DLL \"%s\":\n%s\n", dll, dlerror());
	errFail();
    }
    
    return instance;
}

static void* local getDLLSymbol(void*  instance, String symbol)  /* lookup dll symbol */
{
    void *sym;

    if ((sym = dlsym(instance,symbol)) == 0) {
	errHead(0); errPrintf("Error loading sym:\n%s\n", dlerror());
	errFail();
    }

    return sym;
}

void freeDLL(void* dll) /* free up DLL */
{
  if (dll) {
    /* No error checking done. */
    dlclose(dll);
  }
  return;
}

#elif HAVE_WINDOWS_H

static void* local getDLL(String dll)  /* load dll */
{
    HINSTANCE instance = LoadLibrary(dll);
    if (NULL == instance) {
	/* GetLastError allegedly provides more detail - in practice,
	 * it tells you nothing more.
	 */
	errHead(0); errPrintf("Error while importing DLL \"%s\"", dll);
	errFail();
    }
    /* fprintf(stderr, "Loaded DLL 0x%p\n",instance); fflush(stderr); */
    return instance;
}

static void* local getDLLSymbol(void* instance, String symbol)  /* lookup dll symbol */
{
    return (void*)GetProcAddress((HINSTANCE)instance,symbol);
}

Void freeDLL(void* dll)
{
  if (dll) {
    /* fprintf(stderr, "Freeing DLL 0x%p\n",dll); fflush(stderr); */
    FreeLibrary(dll);
  }
  return;
}


#else /* Dynamic loading not available */

static void* local getDLL(String dll)  /* load dll */
{
    return 0;
}
static void* local getDLLSymbol(void* dll, String symbol)  /* load dll and lookup symbol */
{
    return 0;
}

Void freeDLL(void* dll)
{
}

#endif /* Dynamic loading not available */

static String local lastExtDot(String path)  /* strrchr(path,'.'), but only  */
{                                        /* within the last path component:   */
    String base = path;                 /* a '.' in a directory name (e.g.   */
    String s;                           /* "../dir/Foo" or "./Foo") must not */
    for (s = path; *s; s++)             /* be mistaken for a file extension  */
	if (isSLASH(*s))
	    base = s+1;
    return strrchr(base,'.');
}

static void* local getModDLL(String file)		/* load DLL for module     */
{
#if HAVE_REALPATH && !HAVE__FULLPATH
    char dllPath[MAXPATHLEN+1];
#else
    char dllPath[FILENAME_MAX+1];
#endif
    String dot;
    String s;
    for (s = file; *s && !isSLASH(*s); s++)
	;
    if (*s)
	strcpy(dllPath, file);			/* pathname for module     */
    else /* if in this directory, prefix ./ so search won't use the path   */
	sprintf(dllPath, ".%c%s", SLASH, file);
    dot = lastExtDot(dllPath);        		/* patch file extension    */
    if (dot == NULL || dot == file) {
	dot = dllPath + strlen(dllPath);
    }
    strcpy(dot,DLL_ENDING);
    return getDLL(dllPath);
}

String mkFFIFilename2(String file)    
{
#if HAVE_REALPATH && !HAVE__FULLPATH
    static char path[MAXPATHLEN+1];
#else
    static char path[FILENAME_MAX+1];
#endif
    String dot = lastExtDot(file);              /* patch file extension    */
    if (dot==NULL)
        dot = file + strlen(file);
    strcpy(path, file);
    strcpy(path + (dot - file),DLL_ENDING);
    return path;
}

String mkFFIFilename(String file)                      /* get DLL path for module */
{
#if HAVE_REALPATH && !HAVE__FULLPATH
    static char path[MAXPATHLEN+1];
#else
    static char path[FILENAME_MAX+1];
#endif
    String dot = lastExtDot(file);              /* patch file extension    */
    if (dot==NULL)
        dot = file + strlen(file);
    strcpy(path, file);
    strcpy(path + (dot - file),".c");
    return path;
}

#  define INIT_MODULE_FUN  "initModule"
#  define API_VERSION_FUN  "CuddlesAPIVersion"

Void needPrims(Int version, void*  dll)   /* Load dll containing prims for current module */
{
    /* Version 2-5: the Haskell module specifies what module to expect
     *              (via a needPrims_cuddles decl).
     *
     * Version 0:   the extension DLL specifies the API version it assumes.
     */
    switch (version) { 
    case 2 :
    case 3 :
    case 4 :
	{ 
	    InitModuleFun4 initModule;
	    if (!dll) dll = getModDLL(scriptFile);
	    initModule = (InitModuleFun4)getDLLSymbol(dll,INIT_MODULE_FUN);
	    if (initModule) {
	        Bool flg = setOldDLLFlag(TRUE);
		(*initModule)(cuddlesAPI4()); 
		setScriptPrims(setPrimInfoDll(dll));
		setOldDLLFlag(flg);
		return;
	    }
	    break;
	}
    case 5 : 
	{ 
	    InitModuleFun5 initModule;
	    if (!dll) dll = getModDLL(scriptFile);
	    initModule = (InitModuleFun5)getDLLSymbol(dll,INIT_MODULE_FUN);
	    if (initModule) {
	        Bool flg = setOldDLLFlag(FALSE);
		(*initModule)(cuddlesAPI5()); 
		setScriptPrims(setPrimInfoDll(dll));
		setOldDLLFlag(flg);
		return;
	    }
	    break;
	}
    case 0 : 
        {
	    APIVersionFun versionFun;
	    Int   version = 5;

	    dll = getModDLL(scriptFile);
	    versionFun = (APIVersionFun)getDLLSymbol(dll,API_VERSION_FUN);
	    if (versionFun) {
	      version = (*versionFun)();
	    } 
	    needPrims(version, dll);
	    return;
	}
    default: 
	{
	    errHead(0); errPrintf("This version of Cuddles does not support FFI version %d\n", version);
	    errFail();
	}
    }
    errHead(0); errPrintf("Unable to load FFI primitives\n");
    errFail();
}


/* --------------------------------------------------------------------------
 * Compile and link an ffi file which can be dynamically loaded using
 * the above mechanisms.
 * ------------------------------------------------------------------------*/

/* The command is assembled in a buffer that grows as needed: with a long
 * build directory the absolute paths of the include directories and the
 * extra C files alone run to well over a kilobyte. */
static char*  buffer  = NULL;
static size_t bufsize = 0;
static size_t used    = 0;
static Void local reserve      (size_t);
static Void local insert       (String);
static Void local insertPath   (String);
static Void local insertChar   (Char);
static Void local insertQuoted (String);

static Void local reserve(size_t extra)
{
    if (used + extra + 1 > bufsize) {
        size_t n = bufsize ? bufsize : 1024;
        while (used + extra + 1 > n)
            n *= 2;
        char* p = realloc(buffer, n);
        if (!p) {
            errHead(0); errPrintf("Unable to build compilation command");
            errFail();
        }
        buffer  = p;
        bufsize = n;
    }
}

static Void local insert(String s)
{
    size_t l = strlen(s);
    reserve(l);
    memcpy(buffer+used, s, l+1);
    used += l;
}

/* Convert backslashes, because they can cause problems with system() */
static Void local insertPath(String s)
{
    reserve(strlen(s));
    while (*s) {
        buffer[used++] = *s == SLASH ? '/' : *s;
        s++;
    }
    buffer[used] = '\0';
}

/* Append the shell-quoted form of s (see shellQuote above): unlike the
 * "\"...\"" wrapping this used to do, this is safe even if a file name
 * (ultimately module- or user-supplied) contains a quote, '$' or backtick.
 */
static Void local insertQuoted(String s)
{
    Int l = shellQuote(NULL,s);
    reserve((size_t)l);
    shellQuote(buffer+used,s);
    used += (size_t)l;
}

static Void local insertChar(Char c)
{
    char s[2];
    s[0] = c;
    s[1] = '\0';
    insert(s);
}

Void compileAndLink(String fn, String flags)
{
    char* i = fn;
 
    used    = 0;

#if defined(MKDLL_VISUAL_STUDIO)
    /* find the location of fficuddles.bat */
    /* is in the same directory */
    {
	char Buffer[MAX_PATH+1];
	char Buffer2[MAX_PATH+1];
	GetModuleFileName(GetModuleHandle(NULL), Buffer, MAX_PATH);
	strcpy(strrchr(Buffer, '.'), ".bat");
	GetShortPathName(Buffer, Buffer2, MAX_PATH);

	insert(Buffer2);
	insert(MKDLL_CMD);
    }
#elif defined(MKDLL_CMD)
    /* The compile and link command */
    insert(MKDLL_CMD);
#endif
    
    /* Identify ourselves */
    insert(" -D__CUDDLES__");

    /* the path to HsFFI.h */
    insert(" \"-I");
    insertPath(cuddlesdir());
    insertChar('/');
    insert("include\"");

    /* the output file */
    insert(" -o ");
    insertQuoted(mkFFIFilename2(i));

    /* the file to compile */
    insert(" ");
    insertQuoted(mkFFIFilename(i));

    /* compiler and linker flags specified on Cuddles command line */
    if (flags) {
        insert(" ");
        insert(flags);
    }

    if (shellEsc(buffer,TRUE,TRUE) != 0) {
        errHead(0); errPrintf("Error while running compilation command '%s'", buffer);
        errFail();
    }
    used = 0;
}

/* --------------------------------------------------------------------------
 * Read/write values from/to the registry
 * ------------------------------------------------------------------------*/

#if USE_REGISTRY

static Bool local createKey(HKEY hKey, String regPath, PHKEY phRootKey, REGSAM samDesired)
{
    DWORD  dwDisp;
    return RegCreateKeyEx(hKey, regPath,
			  0, "", REG_OPTION_NON_VOLATILE,
			  samDesired, NULL, phRootKey, &dwDisp) 
	   == ERROR_SUCCESS;
}

static Bool local queryValue(HKEY hKey, String regPath, String var, LPDWORD type, LPBYTE buf, DWORD bufSize)
{
    HKEY hRootKey;

    if (!createKey(hKey, regPath, &hRootKey, KEY_READ)) {
	return FALSE;
    } else {
	LONG res = RegQueryValueEx(hRootKey, var, NULL, type, buf, &bufSize);
	RegCloseKey(hRootKey);
	return (res == ERROR_SUCCESS);
    }
}

/* Specialised version of queryValue(), which doesn't require
 * you to guess the length of a REG_SZ value. Allocates a big
 * enough buffer (using malloc()) to hold the key's value, which
 * is then returned to the callee (along with the resp. to free the
 * buffer.)
 */
static Bool local queryString(HKEY hKey, String regPath, String var, String* pString)
{
    HKEY  hRootKey;
    LONG  rc;
    DWORD bufSize;
    DWORD valType = REG_SZ;
    Bool  res = FALSE;

    if (!createKey(hKey, regPath, &hRootKey, KEY_READ)) {
	return FALSE;
    } else {
        /* Determine the length of the entry */
	rc = RegQueryValueEx(hRootKey, var, NULL, &valType, NULL, &bufSize);
	
	if (rc == ERROR_SUCCESS && valType == REG_SZ) {
	  /* Got the length, now allocate the buffer and retrieve the string. */
	  if ((*pString = (String)malloc(sizeof(char) * (bufSize + 1))) != NULL) {
	    rc = RegQueryValueEx(hRootKey, var, NULL, &valType, (LPBYTE)*pString, &bufSize);
	    res = (rc == ERROR_SUCCESS);
	  }
	}

	RegCloseKey(hRootKey);
	return (res);
    }
}

static Bool local setValue(HKEY hKey, String regPath, String var, DWORD type, LPBYTE buf, DWORD bufSize)
{
    HKEY hRootKey;

    if (!createKey(hKey, regPath, &hRootKey, KEY_WRITE)) {
	return FALSE;
    } else {
	LONG res = RegSetValueEx(hRootKey, var, 0, type, buf, bufSize);
	RegCloseKey(hRootKey);
	return (res == ERROR_SUCCESS);
    }
}

String readRegString(HKEY key, String regPath, String var, String def) /* read String from registry */
{
    char* stringVal;
    
    if (queryString(key, regPath, var, &stringVal)) {
        /* The callee is responsible for freeing the returned string */
	return (String)stringVal;
    } else {
        /* Create a *copy* of the default string, so that it can be freed
	   without worry. */
      if ((stringVal = malloc(sizeof(char) * (strlen(def) + 1))) == NULL) {
	return NULL;
      } else {
	strcpy(stringVal, def);
	return (String)stringVal;
      }
    }
}

Bool writeRegString(String var, String val)      /* write String to registry */
{
    String realVal = ( (NULL == val) ? "" : val);

    return setValue(HKEY_CURRENT_USER, cuddlesRegRoot, var,
		    REG_SZ, (LPBYTE)realVal, lstrlen(realVal)+1);
}


/* concatenate together all strings from registry of the form regPath\\*\\var,
 * separated by PATHSEP.
 */
String readRegChildStrings(HKEY key, String regPath, String var, String def)
{
  HKEY baseKey;
  ULONG ulResult;
  int done = 0;
  DWORD dwIndex = 0;
  char subKeyName[256];
  DWORD subKeyLen;
  BOOL addedPath = FALSE;

  char* resPath = NULL; /* result path, returned to caller */
  String component;
  FILETIME ft; /* just to satisfy RegEnumKeyEx() */
  char sepString[2];
  StringBuilder* builder = newStringBuilder(0);
  
  sepString[0] = PATHSEP;
  sepString[1] = '\0';
  
  ulResult = RegOpenKeyEx(key, regPath, 0, KEY_READ, &baseKey);
  if (ulResult != ERROR_SUCCESS) {
      freeStringBuilder(builder);
      resPath = strCopy(def);
      return resPath;
  }

  appendString(builder, def);
  while (!done) {
    subKeyLen = sizeof(subKeyName);
    ulResult = RegEnumKeyEx(baseKey, dwIndex, subKeyName, &subKeyLen,
			    NULL, NULL, NULL, &ft);
    if (ulResult == ERROR_SUCCESS) {
      /* read next component of path */
      component = readRegString(baseKey, subKeyName, var, "");
      
      if (addedPath) {
	  appendString(builder,sepString);
      }
      addedPath = TRUE;
      appendString(builder,component);
      free(component); /* readRegString() dynamically allocated it, so let go. */
    } else {
      if (ulResult == ERROR_NO_MORE_ITEMS) {
	done = 1;
      }
    }

    dwIndex++;
  }

  RegCloseKey(baseKey);
  resPath = strCopy(toString(builder));
  freeStringBuilder(builder);
  return resPath;
}
#endif /* USE_REGISTRY */

/* --------------------------------------------------------------------------
 * Platform initialisation 
 * ------------------------------------------------------------------------*/
extern Bool initSystem  (Void);
Bool local
initSystem()
{
  /* Called right away by main()  */
    setlocale(LC_CTYPE, "");
    return TRUE;
}

/* --------------------------------------------------------------------------
 * Machine dependent control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const machdepRoots[] = {
    &bdTemp,
};

Void machdep(Int what)                      /* Handle machine specific         */
{                             /* initialisation etc..            */
    switch (what) {
	case INSTALL : registerRoots(machdepRoots);
		       installHandlers();
		       break;
	case RESET   :
		       bdTemp = NIL;
	case BREAK   :
	case EXIT    : normalTerminal();
		       break;
    }
}

/*-------------------------------------------------------------------------*/
