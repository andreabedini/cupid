/*
 * Processing options/toggles.
 *
 *
 */
#include "prelude.h"
#include "storage.h"
#include "connect.h"
#include "errors.h"
#include "machdep.h"
#include "strutil.h"
#include "opts.h"
#include "char.h"

/* --------------------------------------------------------------------------
 * Flags and options:
 * ------------------------------------------------------------------------*/
Bool   preludeLoaded	 = FALSE;

Bool   showStats    = FALSE;     /* TRUE => print stats after eval  */
Bool   addType      = FALSE;     /* TRUE => print type with value   */
Bool   useShow      = TRUE;      /* TRUE => use Text/show printer   */
Bool   displayIO    = FALSE;     /* TRUE => use printer for IO result*/
Bool   useDots      = FALSE;     /* TRUE => use dots in progress    */
Bool   listScripts  = FALSE;     /* TRUE => list scripts after loading*/
Bool   quiet        = TRUE;      /* TRUE => don't show progress     */
Bool   printing     = FALSE;     /* TRUE => currently printing value*/
String  hugsEdit   = 0;	         /* String for editor command       */
String  prompt     = 0;          /* Prompt string                   */

#if EXPLAIN_INSTANCE_RESOLUTION
Bool showInstRes = FALSE;
#endif

Bool printTypeUseDefaults = FALSE;



/* --------------------------------------------------------------------------
 * Keeping track of current file:
 * ------------------------------------------------------------------------*/
static String  lastEdit   = 0;   /* Name of script to edit (if any) */
static Int     lastLine   = 0;   /* Editor line number (if possible)*/

Void setLastEdit(String fname, Int line) /* keep name of last file to edit */
{
    if (lastEdit) {
      free(lastEdit);
    }
    lastEdit = strCopy(fname);
    lastLine = line;
}

String getLastEdit(Int* pLastLine) {
  if (pLastLine)
    *pLastLine = lastLine;
  return lastEdit;
}

/* --------------------------------------------------------------------------
 * Setting of command line options:
 * ------------------------------------------------------------------------*/

#define Option(c,description,flag) { c, description, flag }

static Void   local toggleSet         (Char,Bool);
static Void   local togglesIn         (Bool);

struct options toggle[] = {     /* List of command line toggles    */
    Option('s', "Print no. reductions/cells after eval", &showStats),
    Option('t', "Print type after evaluation",           &addType),
    Option('g', "Print no. cells recovered after gc",    &gcMessages),
    Option('l', "Literate modules as default",           &literateScripts),
    Option('.', "Print dots to show progress",           &useDots),
    Option('q', "Print nothing to show progress",        &quiet),
    Option('Q', "Qualify names when printing",           &useQualifiedNames),
    Option('w', "Always show which modules are loaded",  &listScripts),
    Option('k', "Show kind errors in full",              &kindExpert),
    Option('o', "Allow overlapping instances",           &allowOverlap),
    Option('O', "Allow unsafe overlapping instances",    &allowUnsafeOverlap),
    Option('u', "Use \"show\" to display results",       &useShow),
    Option('I', "Display results of IO programs",        &displayIO),
    Option('T', "Apply 'defaulting' when printing types", &printTypeUseDefaults),
/* Conditional toggles: */
#if EXPLAIN_INSTANCE_RESOLUTION
    Option('x', "Explain instance resolution",           &showInstRes),
#endif
#if DEBUG_CODE
    Option('D', "Debug: show generated G code",          &debugCode),
#endif
    Option(0, 0, 0)
};

Void setOptions(Void)  /* change command line options from Hugs command line */
{
  String s;
  if ((s=readFilename())!=0) {
    do {
      if (!processOption(s)) {
	ERRMSG(0) "Option string must begin with `+' or `-'"
	  EEND;
      }
    } while ((s=readFilename())!=0);
#if USE_REGISTRY
    writeRegString("Options", optionsToStr());
#endif
  } else {
    optionInfo();
  }
}

static Void local toggleSet(Char c, Bool state)    /* Set command line toggle         */
{
    Int i;
    for (i=0; toggle[i].c; ++i)
	if (toggle[i].c == c) {
	    *toggle[i].flag = state;
	    return;
	}
    Printf("Warning: unknown toggle `%c'; ignoring.\n", c);
}

static Void local togglesIn(Bool state)      /* Print current list of toggles in*/
{                           /* given state                     */
    Int count = 0;
    Int i;
    for (i=0; toggle[i].c; ++i)
	if (*toggle[i].flag == state) {
	    if (count==0)
		Putchar((char)(state ? '+' : '-'));
	    Putchar(toggle[i].c);
	    count++;
	}
    if (count>0)
	Putchar(' ');
}

Void optionInfo(Void) {                 /* Print information about command */
    static String fmts = "%-5s%s\n";    /* line settings                   */
    static String fmtc = "%-5c%s\n";
    Int    i;

    Printf("TOGGLES: groups begin with +/- to turn options on/off resp.\n");
    for (i=0; toggle[i].c; ++i) {
	Printf(fmtc,toggle[i].c,toggle[i].description);
    }

    Printf("\nOTHER OPTIONS: (leading + or - makes no difference)\n");
    Printf(fmts,"hnum","Set heap size (cannot be changed within Hugs)");
    Printf(fmts,"pstr","Set prompt string to str");
    Printf(fmts,"rstr","Set repeat last expression string to str");
    Printf(fmts,"Pstr","Set search path for modules to str");
    Printf(fmts,"Sstr","Set list of source file suffixes to str");
    Printf(fmts,"Estr","Use editor setting given by str");
    Printf(fmts,"cnum","Set constraint cutoff limit");
    Printf(fmts,"Fstr","Set preprocessor filter to str");
#if PROFILING
    Printf(fmts,"dnum","Gather profiling statistics every <num> reductions\n");
#endif

    Printf("\nCurrent settings: ");
    togglesIn(TRUE);
    togglesIn(FALSE);
    Printf("-h%d",heapSize);
    Printf(" -p");
    printString(prompt);
    Printf(" -r");
    printString(repeatStr);
    Printf(" -c%d",cutoff);
    Printf("\nSearch path     : -P");
    printString(hugsPath);
    Printf("\nSource suffixes : -S");
    printString(hugsSuffixes);
    Printf("\nEditor setting  : -E");
    printString(hugsEdit);
    Printf("\nPreprocessor    : -F");
    printString(preprocessor);
#if PROFILING
    Printf("\nProfile interval: -d%d", profiling ? profInterval : 0);
#endif
    Putchar('\n');
}

/* Get rid off superfluous trailing space(s) */
#define TRIMSPC() while (*(next-1) == ' ') { next--; charsLeft++; }

#define PUTC(c)                         \
    if (charsLeft > 1) {                \
      *next++=(c);charsLeft--;          \
    } else {                            \
      *next='\0';                       \
    }

#define PUTS(s)                         \
    do { String sref = (s);             \
         Int len = strlen(sref);        \
         if ( charsLeft > len ) {       \
            strcpy(next,sref);          \
            next+=len;                  \
            charsLeft -= len;           \
         } else {                       \
            *next = '\0';               \
	 }                              \
    } while(0)

#define PUTInt(optc,i)                  \
    if ( charsLeft > 20 /*conservative*/ ) { \
      Int n = sprintf(next,"-%c%d",optc,i); \
      next += n; charsLeft -= n;         \
    } else {                            \
      *next = '\0';                     \
    }

#define PUTStr(c,s)                     \
    next=PUTStr_aux(next,&charsLeft,c,s)

static String local PUTStr_aux (String,Int*,Char, String);

static String local PUTStr_aux(String next, Int*   chLeft, Char c, String s)
{
    Int charsLeft = *chLeft;
    Int len;
    if (s && (len = strlen(s)) > 0 ) {
	String t = 0;
	len = strlen(s);

	if ( (Int)(len + 10) > charsLeft ) {
	    *next = '\0';
	    /* optionsToStr() will not to break off immediately,
	     * but soon enough. 
	     */
	    return next;
	}

	*next++ = '-';
	*next++=c;
	*next++='"';
	charsLeft -= 3;
	/* 
	 * Subtlety if *s == '\0': for-loop below bails out right away,
	 * causing strlen(next) to report quite possibly an inappropriate 
	 * result. => always zero-terminate first.
	 *
	 * Do this even if we now explicitly test for 's' not being "" at
	 * the start.
	 */
	*next = '\0';
	for(t=s; *t; ) {
	    /* Explicitly bind result to a local to avoid 
	     * duplicating work within PUTS() macro. Ugly.
	     */
	    String strChar = unlexChar(ExtractChar(t),'"');
	    PUTS(strChar);
	}
	next+=strlen(next);
	PUTS("\" ");
    }
    *chLeft = charsLeft;
    return next;
}

String optionsToStr() {          /* convert options to string */
    static char buffer[2000];
    String next = buffer;
    Int charsLeft = 2000;

    Int i;
    for (i=0; toggle[i].c; ++i) {
	PUTC(*toggle[i].flag ? '+' : '-');
	PUTC(toggle[i].c);
	PUTC(' ');
    }
    PUTInt('h',hpSize);  PUTC(' ');
    PUTStr('p',prompt);
    PUTStr('r',repeatStr);
    PUTStr('P',hugsPath);
    PUTStr('S',hugsSuffixes);
    PUTStr('E',hugsEdit);
    PUTInt('c',cutoff);  PUTC(' ');
    PUTStr('F',preprocessor);
#if PROFILING
    PUTInt('d',profiling ? profInterval : 0);
#endif
    TRIMSPC();
    PUTC('\0');

    return buffer;
}


#undef TRIMSPC
#undef PUTC
#undef PUTS
#undef PUTInt
#undef PUTStr

/* --------------------------------------------------------------------------
 * Reading and processing option strings:
 * ------------------------------------------------------------------------*/

Void readOptions(String options, Bool freeUp)  /* read options from string */
{
    if (!readOptions2(options)) {
        ERRMSG(0) "Option string must begin with `+' or `-'"
	EEND;
    }
    if (options && freeUp) {
	free(options);
    }
}

Bool readOptions2(String options)         /* read options from string */
{
    String s;
    if (options) {
	stringInput(options);
	while ((s=readFilename())!=0) {
	    if (*s && !processOption(s))
		return FALSE;
	}
    }
    return TRUE;
}

Bool processOption(String s)    /* process string s for options,   */
{              /* return FALSE if none found.     */
    Bool state;

    if (s[0]=='-')
	state = FALSE;
    else if (s[0]=='+')
	state = TRUE;
    else
	return FALSE;

    while (*++s)
	switch (*s) {
	    case 'p' : if (s[1]) {
			   if (prompt) free(prompt);
			   prompt = strCopy(s+1);
		       }
		       return TRUE;

	    case 'r' : if (s[1]) {
			   if (repeatStr) free(repeatStr);
			   repeatStr = strCopy(s+1);
		       }
		       return TRUE;

#if PROFILING
	    case 'd' : {                /* random choice of letter - ADR   */
			   Int i = argToInt(s+1);
			   if (i > 0) {
			       profiling = TRUE;
			       profInterval = i;
			   } else {
			       profiling = FALSE;
			       /* To keep the profiling test efficient(?)
				* we dont actually disable the gathering
				* of profiling statistics - we just gather
				* them very infrequently. ADR
				*/
			       profInterval = MAXPOSINT;
			   }
		       }
		       return TRUE;
#endif

	    case 'P' : {
                           String prelLoc;
 	                   String savedPath;
			   
			   savedPath = hugsPath;
			   if (*(s+1) == '\0') {
			     hugsPath = uniqPath(strCopy(HUGSPATH));
			   } else {
			     hugsPath  = substPath(s+1,hugsPath ? hugsPath : "");
			   }
			   prelLoc = findMPathname(STD_PRELUDE);
			   /* prelLoc points to static storage, don't free. */
			   if (!prelLoc) {
			       Printf("ERROR: unable to locate Prelude along new path: \"%s\" - ignoring it.\n", hugsPath);
			       if (hugsPath) free(hugsPath);
			       hugsPath = savedPath;
			   } else {
			       if (savedPath) free(savedPath);
			   }
			   return TRUE;
		       }

	    case 'S' : {
			   String saveSuffixes = hugsSuffixes;
			   if (*(s+1) == '\0')
			       hugsSuffixes = strCopy(HUGSSUFFIXES);
			   else
			       hugsSuffixes = substPath(s+1,hugsSuffixes);
			   if ( !findMPathname(STD_PRELUDE) ) {
			       Printf("ERROR: unable to locate Prelude with new suffix list: \"%s\" - ignoring it.\n", hugsSuffixes);
			       free(hugsSuffixes);
			       hugsSuffixes = saveSuffixes;
			   } else {
			       free(saveSuffixes);
			   }
			   return TRUE;
		       }

	    case 'E' : if (hugsEdit) free(hugsEdit);
		       hugsEdit = strCopy(s+1);
		       return TRUE;

	    case 'F' : if (preprocessor) free(preprocessor);
		       preprocessor = strCopy(s+1);
		       if (preprocessor && strlen(preprocessor) == 0) {
			   free(preprocessor);
			   preprocessor = NULL;
		       }
		       return TRUE;

	    case 'i' : ffiAddCppInclude(s+1);
		       return TRUE;

		       /* re-parse options (useful with #!) */
	    case 'X' : return readOptions2(s+1);

	    case 'h' : setHeapSize(s+1);
		       return TRUE;

	    case 'c' : {   Int cutcand = argToInt(s+1);
			   if (cutcand>=1 && cutcand<=1024)
			       cutoff = cutcand;
		       }
		       return TRUE;

	    /* warnings about obsolete options */

	    case 'e':
	    case 'f':
	    case 'N':
	    case 'W':
	    case 'G' : Printf("ERROR: ignoring obsolete %c%c option.\n",
			  state ? '+' : '-', *s);
		       return TRUE;

	    case 'L' : Printf("ERROR: +L is no longer supported for ffihugs - put the argument (without +L) *after* the module - ignoring it.\n");
		       return TRUE;

	    default  :
		       if (strcmp("98",s)==0) {
			   /* the extensions are always enabled now */
			   Printf("ERROR: ignoring obsolete %c98 option.\n",
				  state ? '+' : '-');
			   return TRUE;
		       }
		       toggleSet(*s,state);
		       break;
	}
    return TRUE;
}

Bool isOption(String s)
{                     /* return TRUE if 's' looks like an option */
  return ( s && (s[0] == '-' || s[0] == '+') );
}

Void setHeapSize(String s) 
{
    if (s) {
	hpSize = argToInt(s);
	if (hpSize < MINIMUMHEAP)
	    hpSize = MINIMUMHEAP;
	else if (MAXIMUMHEAP && hpSize > MAXIMUMHEAP)
	    hpSize = MAXIMUMHEAP;
	if (heapBuilt() && hpSize != heapSize) {
#define HEAP_RESIZE_MSG "Change to heap size will not take effect until you rerun Hugs"
#if USE_REGISTRY
	    FPrintf(stderr,HEAP_RESIZE_MSG "\n");
#else
	    FPrintf(stderr,"Cannot change heap size\n");
#endif
#undef HEAP_RESIZE_MSG
            FFlush(stderr);
	} else {
	    heapSize = hpSize;
	}
    }
}

Int argToInt(String s)            /* read integer from argument str  */
{
    Int    n = 0;
    String t = s;

    if (*s=='\0' || !isascii(*s) || !isdigit(*s)) {
	ERRMSG(0) "Missing integer in option setting \"%s\"", t
	EEND;
    }

    do {
	Int d = (*s++) - '0';
	if (n > ((MAXPOSINT - d)/10)) {
	    ERRMSG(0) "Option setting \"%s\" is too large", t
	    EEND;
	}
	n     = 10*n + d;
    } while (isascii(*s) && isdigit(*s));

    if (*s=='K' || *s=='k') {
	if (n > (MAXPOSINT/1000)) {
	    ERRMSG(0) "Option setting \"%s\" is too large", t
	    EEND;
	}
	n *= 1000;
	s++;
    }

#if MAXPOSINT > 1000000                 /* waste of time on 16 bit systems */
    if (*s=='M' || *s=='m') {
	if (n > (MAXPOSINT/1000000)) {
	    ERRMSG(0) "Option setting \"%s\" is too large", t
	    EEND;
	}
	n *= 1000000;
	s++;
    }
#endif

#if MAXPOSINT > 1000000000
    if (*s=='G' || *s=='g') {
	if (n > (MAXPOSINT/1000000000)) {
	    ERRMSG(0) "Option setting \"%s\" is too large", t
	    EEND;
	}
	n *= 1000000000;
	s++;
    }
#endif

    if (*s!='\0') {
	ERRMSG(0) "Unwanted characters after option setting \"%s\"", t
	EEND;
    }

    return n;
}

/* --------------------------------------------------------------------------
 * Process the options entries in an argv-vector:
 * ------------------------------------------------------------------------*/
Void processOptionVector(Int argc, String argv[])
{
    Int i;

    for (i=1; i<argc; ++i) {            /* process command line arguments  */
      if ( argv[i] && argv[i][0] ) {
	  /* Willfully ignore the bool returned by processOption();
	   * non-options (i.e., scripts) will be handled later on.
	   */
	  processOption(argv[i]);
      }
    }
}

/*
 * Read option settings from sources other than the command-line:
 *
 *   - HUGSFLAGS environment variable
 *   - Registry (machine-wide and user-specific portions.)
 *   - Preference files (MacOS<t>, where t < 10).
 *   - WinHugs GUI options.
 *
 */
Void readOptionSettings() {

#if USE_REGISTRY
    readOptions(readRegString(HKEY_LOCAL_MACHINE,hugsRegRoot,"Options",""), TRUE);
    if (!fromEnv("IGNORE_USER_REGISTRY",NULL)) {
      /* If IGNORE_USER_REGISTRY exist as an env var, don't consult
       * the user portion of the Registry. Emergency workaround if it has
       * somehow become invalid.
       */
      readOptions(readRegString(HKEY_CURRENT_USER, hugsRegRoot,"Options",""), TRUE);
    }
#endif /* USE_REGISTRY */
    readOptions(fromEnv("HUGSFLAGS",""),FALSE);
}

