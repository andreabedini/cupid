/*
 * Processing options/toggles.
 *
 *
 */
#ifndef __OPTS_H__
#define __OPTS_H__

#include "prelude.h"

/* --------------------------------------------------------------------------
 * Functions for processing and displaying Cupid options:
 * ------------------------------------------------------------------------*/
extern bool processOption(char *);
extern void readOptions(char *, bool);
extern bool readOptions2(char *);
extern void optionInfo(void);
extern bool isOption(char *);
extern void setOptions(void);
extern char *optionsToStr(void);
extern void processOptionVector(int, char *[]);
extern void readOptionSettings(void);

extern void setLastEdit(char *, int);
extern char *getLastEdit(int *);
extern int argToInt(char *);
extern void setHeapSize(char *);

/* --------------------------------------------------------------------------
 * Interpreter flags and options:
 * ------------------------------------------------------------------------*/
extern bool showStats;  /* true => print stats after eval  */
extern bool addType;    /* true => print type with value   */
extern bool gcMessages; /* true => print GC messages         */
extern bool useDots;    /* true => use dots in progress    */
extern bool quiet;      /* true => don't show progress     */
extern bool
    useQualifiedNames; /* true => qualify names when printing types and terms */
extern bool listScripts;        /* true => list scripts after loading*/
extern bool kindExpert;         /* true => display kind errors in    */
                                /*         full detail              */
extern bool allowOverlap;       /* true => allow overlapping insts   */
extern bool allowUnsafeOverlap; /* true => in addition, allow        */
                                /* potentially inconsistent          */
                                /* overlapping instances             */
extern bool useShow;            /* true => use Text/show printer     */
extern bool displayIO;          /* true => use printer for IO result */
extern bool
    printTypeUseDefaults; /* true => use 'default'ing when printing types */

extern int cutoff;          /* Constraint Cutoff depth          */
extern char *prompt;        /* Prompt string                   */
extern char *repeatStr;     /* Repeat last expr                */
extern char *cupidPath;     /* String for file search path     */
extern char *cupidSuffixes; /* Source filename suffixes        */
extern int hpSize;          /* Desired heap size               */
extern char *cupidEdit;     /* String for editor command       */

/* The rest are conditionally supported flags: */
#if PROFILING
extern bool profiling;   /* true => perform profiling.      */
extern int profInterval; /* interval between samples        */
#endif

extern bool showInstRes; /* true => show instance resolution */
#if DEBUG_CODE
extern bool debugCode; /* true => print G-code to screen  */
#endif

extern char *preprocessor; /* preprocessor command            */

extern bool printing; /* true => currently printing value*/

/* 
 * How to add a new flag / option:
 *
 *  - declare the variable which records the setting of the new option
 *    in the above block; add a definition for it at the start of opts.c,
 *    or local to whatever module the variable is being used.
 *
 *  - decide on what upper/lower-case letter to use for your new option
 *    (getting harder...option handling really ought to be extended with
 *     better support for option strings.)
 *
 *  - if the flag is a toggle, add an entry for it to the toggle[] array
 *    in opts.c. If it is an option that takes an argument, extend
 *    the switch statement in processOption() to cover your option. There's
 *    a decent chance that code for one of the other options supported can 
 *    be adapted to suit your needs..
 * 
 *  - if the code module which uses the new option doesn't already, add
 *    #include "opts.h" to it to bring the underlying variable declaration
 *    into scope.
 */

/*
 * Representing toggles -- exposed here so that UIs (such as winhugs/) can 
 * work off the toggle[] array when putting together an 'Options' dialog.
 *
 */
struct options { /* command line option toggles     */
    char c;      /* table defined in main app.      */
    char *description;
    bool *flag;
};

extern struct options toggle[];

#endif /* __OPTS_H__ */
