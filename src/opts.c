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
bool preludeLoaded = false;

bool showStats = false;   /* true => print stats after eval  */
bool addType = false;     /* true => print type with value   */
bool useShow = true;      /* true => use Text/show printer   */
bool displayIO = false;   /* true => use printer for IO result*/
bool useDots = false;     /* true => use dots in progress    */
bool listScripts = false; /* true => list scripts after loading*/
bool quiet = true;        /* true => don't show progress     */
bool printing = false;    /* true => currently printing value*/
char *cupidEdit = 0;      /* String for editor command       */
char *prompt = 0;         /* Prompt string                   */

#if EXPLAIN_INSTANCE_RESOLUTION
bool showInstRes = false;
#endif

bool printTypeUseDefaults = false;

/* --------------------------------------------------------------------------
 * Keeping track of current file:
 * ------------------------------------------------------------------------*/
static char *lastEdit = 0; /* Name of script to edit (if any) */
static int lastLine = 0;   /* Editor line number (if possible)*/

/* Keep name of last file to edit */
void setLastEdit(char *fname, int line) {
    if (lastEdit) {
        free(lastEdit);
    }
    lastEdit = strCopy(fname);
    lastLine = line;
}

char *getLastEdit(int *pLastLine) {
    if (pLastLine)
        *pLastLine = lastLine;
    return lastEdit;
}

/* --------------------------------------------------------------------------
 * Setting of command line options:
 * ------------------------------------------------------------------------*/

static void toggleSet(Char, bool);
static void togglesIn(bool);

struct options toggle[] =
    {/* List of command line toggles    */
     {'s', "Print no. reductions/cells after eval", &showStats},
     {'t', "Print type after evaluation", &addType},
     {'g', "Print no. cells recovered after gc", &gcMessages},
     {'.', "Print dots to show progress", &useDots},
     {'q', "Print nothing to show progress", &quiet},
     {'Q', "Qualify names when printing", &useQualifiedNames},
     {'w', "Always show which modules are loaded", &listScripts},
     {'k', "Show kind errors in full", &kindExpert},
     {'o', "Allow overlapping instances", &allowOverlap},
     {'O', "Allow unsafe overlapping instances", &allowUnsafeOverlap},
     {'u', "Use \"show\" to display results", &useShow},
     {'I', "Display results of IO programs", &displayIO},
     {'T', "Apply 'defaulting' when printing types", &printTypeUseDefaults},
/* Conditional toggles: */
#if EXPLAIN_INSTANCE_RESOLUTION
     {'x', "Explain instance resolution", &showInstRes},
#endif
#if DEBUG_CODE
     {'D', "Debug: show generated G code", &debugCode},
#endif
     {0, 0, 0}};

/* Change command line options from Cupid command line */
void setOptions(void) {
    char *s;
    if ((s = readFilename()) != 0) {
        do {
            if (!processOption(s)) {
                errHead(0);
                errPrintf("Option string must begin with `+' or `-'");
                errFail();
            }
        } while ((s = readFilename()) != 0);
#if USE_REGISTRY
        writeRegString("Options", optionsToStr());
#endif
    } else {
        optionInfo();
    }
}

/* Set command line toggle */
static void toggleSet(Char c, bool state) {
    int i;
    for (i = 0; toggle[i].c; ++i)
        if (toggle[i].c == c) {
            *toggle[i].flag = state;
            return;
        }
    Printf("Warning: unknown toggle `%c'; ignoring.\n", c);
}

/* Print current list of toggles in given state */
static void togglesIn(bool state) {
    int count = 0;
    int i;
    for (i = 0; toggle[i].c; ++i)
        if (*toggle[i].flag == state) {
            if (count == 0)
                Putchar((char)(state ? '+' : '-'));
            Putchar(toggle[i].c);
            count++;
        }
    if (count > 0)
        Putchar(' ');
}

/* Print information about command line settings */
void optionInfo(void) {
    static char *fmts = "%-5s%s\n";
    static char *fmtc = "%-5c%s\n";
    int i;

    Printf("TOGGLES: groups begin with +/- to turn options on/off resp.\n");
    for (i = 0; toggle[i].c; ++i) {
        Printf(fmtc, toggle[i].c, toggle[i].description);
    }

    Printf("\nOTHER OPTIONS: (leading + or - makes no difference)\n");
    Printf(fmts, "hnum", "Set heap size (cannot be changed within Cupid)");
    Printf(fmts, "pstr", "Set prompt string to str");
    Printf(fmts, "rstr", "Set repeat last expression string to str");
    Printf(fmts, "Pstr", "Set search path for modules to str");
    Printf(fmts, "Sstr", "Set list of source file suffixes to str");
    Printf(fmts, "Estr", "Use editor setting given by str");
    Printf(fmts, "cnum", "Set constraint cutoff limit");
    Printf(fmts, "Fstr", "Set preprocessor filter to str");
#if PROFILING
    Printf(fmts, "dnum",
           "Gather profiling statistics every <num> reductions\n");
#endif

    Printf("\nCurrent settings: ");
    togglesIn(true);
    togglesIn(false);
    Printf("-h%d", heapSize);
    Printf(" -p");
    printString(prompt);
    Printf(" -r");
    printString(repeatStr);
    Printf(" -c%d", cutoff);
    Printf("\nSearch path     : -P");
    printString(cupidPath);
    Printf("\nSource suffixes : -S");
    printString(cupidSuffixes);
    Printf("\nEditor setting  : -E");
    printString(cupidEdit);
    Printf("\nPreprocessor    : -F");
    printString(preprocessor);
#if PROFILING
    Printf("\nProfile interval: -d%d", profiling ? profInterval : 0);
#endif
    Putchar('\n');
}

/* Get rid off superfluous trailing space(s) */
#define TRIMSPC()                                                              \
    while (*(next - 1) == ' ') {                                               \
        next--;                                                                \
        charsLeft++;                                                           \
    }

#define PUTC(c)                                                                \
    if (charsLeft > 1) {                                                       \
        *next++ = (c);                                                         \
        charsLeft--;                                                           \
    } else {                                                                   \
        *next = '\0';                                                          \
    }

#define PUTS(s)                                                                \
    do {                                                                       \
        char *sref = (s);                                                      \
        int len = strlen(sref);                                                \
        if (charsLeft > len) {                                                 \
            strcpy(next, sref);                                                \
            next += len;                                                       \
            charsLeft -= len;                                                  \
        } else {                                                               \
            *next = '\0';                                                      \
        }                                                                      \
    } while (0)

#define PUTInt(optc, i)                                                        \
    if (charsLeft > 20 /*conservative*/) {                                     \
        int n = snprintf(next, charsLeft, "-%c%d", optc, i);                   \
        next += n;                                                             \
        charsLeft -= n;                                                        \
    } else {                                                                   \
        *next = '\0';                                                          \
    }

#define PUTStr(c, s) next = PUTStr_aux(next, &charsLeft, c, s)

static char *PUTStr_aux(char *, int *, Char, char *);

static char *PUTStr_aux(char *next, int *chLeft, Char c, char *s) {
    int charsLeft = *chLeft;
    int len;
    if (s && (len = (int)strlen(s)) > 0) {
        char *t = 0;
        len = (int)strlen(s);

        if ((int)(len + 10) > charsLeft) {
            *next = '\0';
            /* optionsToStr() will not to break off immediately,
             * but soon enough. 
             */
            return next;
        }

        *next++ = '-';
        *next++ = (char)c;
        *next++ = '"';
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
        for (t = s; *t;) {
            /* Explicitly bind result to a local to avoid 
             * duplicating work within PUTS() macro. Ugly.
             */
            char *strChar = unlexChar(ExtractChar(t), '"');
            PUTS(strChar);
        }
        next += strlen(next);
        PUTS("\" ");
    }
    *chLeft = charsLeft;
    return next;
}

/* Convert options to string */
char *optionsToStr(void) {
    static char buffer[2000];
    char *next = buffer;
    int charsLeft = 2000;

    int i;
    for (i = 0; toggle[i].c; ++i) {
        PUTC(*toggle[i].flag ? '+' : '-');
        PUTC(toggle[i].c);
        PUTC(' ');
    }
    PUTInt('h', hpSize);
    PUTC(' ');
    PUTStr('p', prompt);
    PUTStr('r', repeatStr);
    PUTStr('P', cupidPath);
    PUTStr('S', cupidSuffixes);
    PUTStr('E', cupidEdit);
    PUTInt('c', cutoff);
    PUTC(' ');
    PUTStr('F', preprocessor);
#if PROFILING
    PUTInt('d', profiling ? profInterval : 0);
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

/* Read options from string */
void readOptions(char *options, bool freeUp) {
    if (!readOptions2(options)) {
        errHead(0);
        errPrintf("Option string must begin with `+' or `-'");
        errFail();
    }
    if (options && freeUp) {
        free(options);
    }
}

/* --------------------------------------------------------------------------
 * Tokenising option strings:
 *
 * Option strings come from the command line, from CUPIDFLAGS, from the
 * registry and from the argument of -X, and they are split into words here.
 * They used to be split by pointing the global input stream at them with
 * stringInput() and reading words back with readFilename(), but there is
 * only one input stream: a -X nested inside an option string reset it, and
 * everything after that -X was silently lost -- the rest of CUPIDFLAGS, or
 * the rest of a ":set" line, which was then read back as the next command.
 *
 * The word syntax is the one readFilename() accepted.  Words are separated
 * by white space.  A double quote begins a quoted section, which ends at the
 * next quote and may contain spaces; inside it \", \\ and "\ " stand for a
 * quote, a backslash and a space, and a backslash before anything else is an
 * ordinary backslash.  Outside quotes "\ " also stands for a space.  No
 * other escape is recognised, so that a backslash in a Windows path name
 * means itself.
 * ------------------------------------------------------------------------*/

#define MAX_OPTION_WORD 4000 /* longest word an option string may hold */

struct optScan { /* the state of a scan over an option string */
    char *rest;  /* the part of the string not read yet       */
    Char c;      /* the character at the scan position, or 0  */
                 /* at the end of the string                  */
    char *out;   /* where the next character of the word goes */
    char *word;  /* the buffer the word is built in           */
};

/* Advance one character */
static void optSkip(struct optScan *sc) {
    char *at = sc->rest;
    if (*at == '\0') { /* ExtractChar would step past the NUL */
        sc->c = '\0';
        return;
    }
    sc->c = ExtractChar(sc->rest);
    if (sc->rest == at) /* a malformed encoding decodes to nothing */
        sc->rest = at + 1;
}

/* Add a character to the word */
static void optSave(struct optScan *sc, Char c) {
    if (sc->out > sc->word + MAX_OPTION_WORD - MAX_CHAR_ENCODING) {
        errHead(0);
        errPrintf("Maximum option length (%d) exceeded", MAX_OPTION_WORD);
        errFail();
    }
    AddChar(c, sc->out);
}

/* The next word of the option string, or 0 when there are none left. The word
 * is in the scan's buffer until the next call */
static char *optWord(struct optScan *sc) {
    while (sc->c != '\0' && isIn(sc->c, SPACE))
        optSkip(sc);
    if (sc->c == '\0')
        return 0;

    sc->out = sc->word;
    while (sc->c != '\0' && !isIn(sc->c, SPACE)) {
        if (sc->c == '"') {
            optSkip(sc);
            while (sc->c != '\0' && sc->c != '"') {
                if (sc->c == '\\') {
                    optSkip(sc);
                    if (sc->c == '"' || sc->c == ' ' || sc->c == '\\') {
                        optSave(sc, sc->c);
                        optSkip(sc);
                    } else {
                        optSave(sc, '\\');
                    }
                } else {
                    optSave(sc, sc->c);
                    optSkip(sc);
                }
            }
            if (sc->c != '"') {
                errHead(0);
                errPrintf("a closing quote, '\"', was expected");
                errFail();
            }
            optSkip(sc);
        } else {
            Char saved = sc->c;
            optSkip(sc);
            if (saved == '\\' && sc->c == ' ') {
                optSave(sc, ' ');
                optSkip(sc);
            } else {
                optSave(sc, saved);
            }
        }
    }
    *sc->out = '\0';
    return sc->word;
}

/* Read options from string */
bool readOptions2(char *options) {
    struct optScan sc;
    char *s;
    bool ok = true;

    if (options) {
        if (!charTabBuilt) /* optWord classifies with isIn() */
            initCharTab();
        sc.word = (char *)malloc(MAX_OPTION_WORD + 1);
        if (!sc.word) {
            errHead(0);
            errPrintf("Out of memory reading options");
            errFail();
        }
        sc.rest = options;
        optSkip(&sc); /* prime the scan with the first character */
        while (ok && (s = optWord(&sc)) != 0)
            if (*s && !processOption(s))
                ok = false;
        free(sc.word);
    }
    return ok;
}

/* Process string s for options, return false if none found. */
bool processOption(char *s) {
    bool state;

    if (s[0] == '-')
        state = false;
    else if (s[0] == '+')
        state = true;
    else
        return false;

    while (*++s)
        switch (*s) {
        case 'p':
            if (s[1]) {
                if (prompt)
                    free(prompt);
                prompt = strCopy(s + 1);
            }
            return true;

        case 'r':
            if (s[1]) {
                if (repeatStr)
                    free(repeatStr);
                repeatStr = strCopy(s + 1);
            }
            return true;

#if PROFILING
        case 'd': { /* random choice of letter - ADR   */
            int i = argToInt(s + 1);
            if (i > 0) {
                profiling = true;
                profInterval = i;
            } else {
                profiling = false;
                /* To keep the profiling test efficient(?)
                                * we dont actually disable the gathering
                                * of profiling statistics - we just gather
                                * them very infrequently. ADR
                                */
                profInterval = MAXPOSINT;
            }
        }
            return true;
#endif

        case 'P': {
            char *prelLoc;
            char *savedPath;

            savedPath = cupidPath;
            if (*(s + 1) == '\0') {
                cupidPath = uniqPath(strCopy(CUPIDPATH));
            } else {
                cupidPath = substPath(s + 1, cupidPath ? cupidPath : "");
            }
            prelLoc = findMPathname(STD_PRELUDE);
            /* prelLoc points to static storage, don't free. */
            if (!prelLoc) {
                Printf(
                    "ERROR: unable to locate Prelude along new path: \"%s\" - ignoring it.\n",
                    cupidPath);
                if (cupidPath)
                    free(cupidPath);
                cupidPath = savedPath;
            } else {
                if (savedPath)
                    free(savedPath);
            }
            return true;
        }

        case 'S': {
            char *saveSuffixes = cupidSuffixes;
            if (*(s + 1) == '\0')
                cupidSuffixes = strCopy(CUPIDSUFFIXES);
            else
                cupidSuffixes = substPath(s + 1, cupidSuffixes);
            if (!findMPathname(STD_PRELUDE)) {
                Printf(
                    "ERROR: unable to locate Prelude with new suffix list: \"%s\" - ignoring it.\n",
                    cupidSuffixes);
                free(cupidSuffixes);
                cupidSuffixes = saveSuffixes;
            } else {
                free(saveSuffixes);
            }
            return true;
        }

        case 'E':
            if (cupidEdit)
                free(cupidEdit);
            cupidEdit = strCopy(s + 1);
            return true;

        case 'F':
            if (preprocessor)
                free(preprocessor);
            preprocessor = strCopy(s + 1);
            if (preprocessor && strlen(preprocessor) == 0) {
                free(preprocessor);
                preprocessor = NULL;
            }
            return true;

        case 'i':
            ffiAddCppInclude(s + 1);
            return true;

            /* re-parse options (useful with #!) */
        case 'X':
            return readOptions2(s + 1);

        case 'h':
            setHeapSize(s + 1);
            return true;

        case 'c': {
            int cutcand = argToInt(s + 1);
            if (cutcand >= 1 && cutcand <= 1024)
                cutoff = cutcand;
        }
            return true;

            /* warnings about obsolete options */

        case 'e':
        case 'f':
        case 'N':
        case 'W':
        case 'G':
            Printf("ERROR: ignoring obsolete %c%c option.\n", state ? '+' : '-',
                   *s);
            return true;

        case 'L':
            Printf(
                "ERROR: +L is no longer supported for fficupid - put the argument (without +L) *after* the module - ignoring it.\n");
            return true;

        default:
            if (strcmp("98", s) == 0) {
                /* the extensions are always enabled now */
                Printf("ERROR: ignoring obsolete %c98 option.\n",
                       state ? '+' : '-');
                return true;
            }
            toggleSet(*s, state);
            break;
        }
    return true;
}

/* Return true if 's' looks like an option */
bool isOption(char *s) {
    return (s && (s[0] == '-' || s[0] == '+'));
}

void setHeapSize(char *s) {
    if (s) {
        hpSize = argToInt(s);
        if (hpSize < MINIMUMHEAP)
            hpSize = MINIMUMHEAP;
        else if (MAXIMUMHEAP && hpSize > MAXIMUMHEAP)
            hpSize = MAXIMUMHEAP;
        if (heapBuilt() && hpSize != heapSize) {
#define HEAP_RESIZE_MSG                                                        \
    "Change to heap size will not take effect until you rerun Cupid"
#if USE_REGISTRY
            FPrintf(stderr, HEAP_RESIZE_MSG "\n");
#else
            FPrintf(stderr, "Cannot change heap size\n");
#endif
#undef HEAP_RESIZE_MSG
            FFlush(stderr);
        } else {
            heapSize = hpSize;
        }
    }
}

/* Read integer from argument str */
int argToInt(char *s) {
    int n = 0;
    char *t = s;

    if (*s == '\0' || !isascii(*s) || !isdigit(*s)) {
        errHead(0);
        errPrintf("Missing integer in option setting \"%s\"", t);
        errFail();
    }

    do {
        int d = (*s++) - '0';
        if (n > ((MAXPOSINT - d) / 10)) {
            errHead(0);
            errPrintf("Option setting \"%s\" is too large", t);
            errFail();
        }
        n = 10 * n + d;
    } while (isascii(*s) && isdigit(*s));

    if (*s == 'K' || *s == 'k') {
        if (n > (MAXPOSINT / 1000)) {
            errHead(0);
            errPrintf("Option setting \"%s\" is too large", t);
            errFail();
        }
        n *= 1000;
        s++;
    }

#if MAXPOSINT > 1000000 /* waste of time on 16 bit systems */
    if (*s == 'M' || *s == 'm') {
        if (n > (MAXPOSINT / 1000000)) {
            errHead(0);
            errPrintf("Option setting \"%s\" is too large", t);
            errFail();
        }
        n *= 1000000;
        s++;
    }
#endif

#if MAXPOSINT > 1000000000
    if (*s == 'G' || *s == 'g') {
        if (n > (MAXPOSINT / 1000000000)) {
            errHead(0);
            errPrintf("Option setting \"%s\" is too large", t);
            errFail();
        }
        n *= 1000000000;
        s++;
    }
#endif

    if (*s != '\0') {
        errHead(0);
        errPrintf("Unwanted characters after option setting \"%s\"", t);
        errFail();
    }

    return n;
}

/* --------------------------------------------------------------------------
 * Process the options entries in an argv-vector:
 * ------------------------------------------------------------------------*/
void processOptionVector(int argc, char *argv[]) {
    int i;

    for (i = 1; i < argc; ++i) { /* process command line arguments  */
        if (argv[i] && argv[i][0]) {
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
 *   - CUPIDFLAGS environment variable
 *   - Registry (machine-wide and user-specific portions.)
 *   - Preference files (MacOS<t>, where t < 10).
 *   - WinHugs GUI options.
 *
 */
void readOptionSettings(void) {

#if USE_REGISTRY
    readOptions(readRegString(HKEY_LOCAL_MACHINE, cupidRegRoot, "Options", ""),
                true);
    if (!fromEnv("IGNORE_USER_REGISTRY", NULL)) {
        /* If IGNORE_USER_REGISTRY exist as an env var, don't consult
       * the user portion of the Registry. Emergency workaround if it has
       * somehow become invalid.
       */
        readOptions(
            readRegString(HKEY_CURRENT_USER, cupidRegRoot, "Options", ""),
            true);
    }
#endif /* USE_REGISTRY */
    readOptions(fromEnv("CUPIDFLAGS", ""), false);
}
