/* --------------------------------------------------------------------------
 * Implementation of the Haskell IO monad.
 *
 * The primitives below implement the standard IO monad for Haskell 1.3
 * using a continuation passing monad for sequencing.  The primitives are
 * believed to give a reasonably good implementation of the semantics
 * specified by the Haskell 1.3 report.  There are also some additional
 * primitives, particularly for dealing with IOError and Handle values
 * that are not included in the prelude, but are used by standard libraries.
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
#include "machdep.h"
#include "builtin.h"
#include "char.h"
#include <stdint.h> /* intptr_t, for the StablePtr conversions */
#include <sys/types.h>
#include <sys/stat.h>
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <errno.h>
#include <fcntl.h>
#if HAVE_IO_H
# include <io.h>
#endif
#if HAVE_WINDOWS_H
# include <windows.h>
#endif

Name nameIORun;  /* run IO code                     */
Name nameIOBind; /* bind IO code                    */
Name namePutStr; /* Prelude.putStr                  */

static Name namePass;        /* auxiliary:: \f b a -> f a b     */
static Name nameHreader;     /* auxiliary function                  */
static Cell cuddlesProgName; /* value of getProgName            */
static Cell cuddlesArgs;     /* value of getArgs                */

static char *toIOErrorDescr(int, bool);
static Name toIOError(int);
static int newHandle(Cell *, char *);
static char *modeString(int, bool);
static Cell openHandle(StackPtr, Cell *, int, bool, char *);
static Cell openFdHandle(StackPtr, int, int, bool, char *);
static Char hGetChar(int, char *);
static void hPutChar(Char, int, char *);
static void setRWState(int, int);
static void checkOpen(int, char *);
static void checkReadable(int, char *);
static void checkWritable(int, char *);

#define MAX_HANDLES num_handles

extern void pushString(char *);

/* --------------------------------------------------------------------------
 * IO monad control:
 * ------------------------------------------------------------------------*/

/* The statics that hold cells: registered as roots at INSTALL and marked
 * by the collector on every collection */
static Cell *const iomonadRoots[] = {
    &cuddlesProgName,
    &cuddlesArgs,
};

static void iomonadControl(int);
static void iomonadControl(int what) {
    switch (what) {
    case INSTALL:
        registerRoots(iomonadRoots);
        setCurrModule(modulePrelude);
        namePass = predefPrim("_pass", "passIO");
        nameHreader = predefPrim("_hreader", "hreader");
        nameIORun = predefName("cuddlesIORun");
        nameIOBind = predefName("primbindIO");
        namePutStr = predefName("putStr");
        break;
    default:
        break;
    }
}

PROTO_PRIM(primReturnIO);
PROTO_PRIM(primBindIO);
PROTO_PRIM(primPass);

PROTO_PRIM(primGC);
PROTO_PRIM(primGetEnv);
PROTO_PRIM(primSystem);
PROTO_PRIM(primGetRandomSeed);

PROTO_PRIM(primGetProgName);
PROTO_PRIM(primGetArgs);
PROTO_PRIM(primSetProgName);
PROTO_PRIM(primSetArgs);

static void fopenPrim(StackPtr, bool, char *);
static int getIOMode(Cell);

PROTO_PRIM(primHGetChar);
PROTO_PRIM(primHPutChar);
PROTO_PRIM(primHPutStr);
PROTO_PRIM(primHreader);
PROTO_PRIM(primHContents);
PROTO_PRIM(primOpenFile);
PROTO_PRIM(primOpenBinaryFile);
PROTO_PRIM(primStdin);
PROTO_PRIM(primStdout);
PROTO_PRIM(primStderr);
PROTO_PRIM(primOpenFd);
PROTO_PRIM(primHandleToFd);
PROTO_PRIM(primHIsEOF);
PROTO_PRIM(primHFlush);
PROTO_PRIM(primHClose);
PROTO_PRIM(primHGetPosn);
PROTO_PRIM(primHSetPosn);
PROTO_PRIM(primHSetBuffering);
PROTO_PRIM(primHGetBuffering);
PROTO_PRIM(primHSeek);
PROTO_PRIM(primHLookAhead);
PROTO_PRIM(primHIsOpen);
PROTO_PRIM(primHIsClosed);
PROTO_PRIM(primHIsReadable);
PROTO_PRIM(primHIsWritable);
PROTO_PRIM(primHIsSeekable);
PROTO_PRIM(primHFileSize);
PROTO_PRIM(primHWaitForInput);
PROTO_PRIM(primEqHandle);
PROTO_PRIM(primGetHandleNumber);

PROTO_PRIM(primHSetBinaryMode);
PROTO_PRIM(primHPutBuf);
PROTO_PRIM(primHGetBuf);

PROTO_PRIM(primHIsTerminalDevice);
PROTO_PRIM(primHGetEcho);
PROTO_PRIM(primHSetEcho);

PROTO_PRIM(primIOEql);
PROTO_PRIM(primIOHash);

PROTO_PRIM(primNewRef);
PROTO_PRIM(primDerefRef);
PROTO_PRIM(primAssignRef);
PROTO_PRIM(primEqRef);

PROTO_PRIM(primMakeSP);
PROTO_PRIM(primDerefSP);
PROTO_PRIM(primFreeSP);
PROTO_PRIM(primCastSPToP);
PROTO_PRIM(primCastPToSP);

PROTO_PRIM(primNewFP);
PROTO_PRIM(primAddFPF);
PROTO_PRIM(primAddFPFEnv);
PROTO_PRIM(primWriteFP);
PROTO_PRIM(primEqFP);
PROTO_PRIM(primTouchFP);
PROTO_PRIM(primFPToP);

PROTO_PRIM(primWeakPtrEq);
PROTO_PRIM(primMkWeak);
PROTO_PRIM(primDeRefWeak);
PROTO_PRIM(primReplaceFinalizer);
PROTO_PRIM(primFinalize);
PROTO_PRIM(primRunFinalizer);
PROTO_PRIM(primFinalizerWaiting);

PROTO_PRIM(primMakeSN);
PROTO_PRIM(primDerefSN);
PROTO_PRIM(primHashSN);
PROTO_PRIM(primEqSN);

static struct primitive iomonadPrimTable[] = {
    {"primretIO", 1 + IOArity, primReturnIO},
    {"primbindIO", 2 + IOArity, primBindIO},
    {"passIO", 2 + IOArity, primPass},

    {"primGC", 0 + IOArity, primGC},
    {"getEnv", 1 + IOArity, primGetEnv},
    {"primSystem", 1 + IOArity, primSystem},
    {"getRandomSeed", 0 + IOArity, primGetRandomSeed},

    {"primGetProgName", 0 + IOArity, primGetProgName},
    {"primGetArgs", 0 + IOArity, primGetArgs},
    {"primSetProgName", 1 + IOArity, primSetProgName},
    {"primSetArgs", 1 + IOArity, primSetArgs},

    {"hGetChar", 1 + IOArity, primHGetChar},
    {"hPutChar", 2 + IOArity, primHPutChar},
    {"hPutStr", 2 + IOArity, primHPutStr},
    {"hreader", 1, primHreader},
    {"hGetContents", 1 + IOArity, primHContents},
    {"openFile", 2 + IOArity, primOpenFile},
    {"openBinaryFile", 2 + IOArity, primOpenBinaryFile},
    {"openFd", 4 + IOArity, primOpenFd},
    {"handleToFd", 1 + IOArity, primHandleToFd},
    {"stdin", 0, primStdin},
    {"stdout", 0, primStdout},
    {"stderr", 0, primStderr},
    {"hIsEOF", 1 + IOArity, primHIsEOF},
    {"hFlush", 1 + IOArity, primHFlush},
    {"hClose", 1 + IOArity, primHClose},
    {"hGetPosnPrim", 1 + IOArity, primHGetPosn},
    {"hSetPosnPrim", 2 + IOArity, primHSetPosn},
    {"hSetBuff", 3 + IOArity, primHSetBuffering},
    {"hGetBuff", 1 + IOArity, primHGetBuffering},
    {"hSeekPrim", 3 + IOArity, primHSeek},
    {"hLookAhead", 1 + IOArity, primHLookAhead},
    {"hIsOpen", 1 + IOArity, primHIsOpen},
    {"hIsClosed", 1 + IOArity, primHIsClosed},
    {"hIsReadable", 1 + IOArity, primHIsReadable},
    {"hIsWritable", 1 + IOArity, primHIsWritable},
    {"hIsSeekable", 1 + IOArity, primHIsSeekable},
    {"hFileSize", 1 + IOArity, primHFileSize},
    {"hWaitForInput", 2 + IOArity, primHWaitForInput},
    {"primEqHandle", 2, primEqHandle},
    {"primGetHandleNumber", 1, primGetHandleNumber},
    {"hSetBinaryMode", 2 + IOArity, primHSetBinaryMode},
    {"hPutBuf", 3 + IOArity, primHPutBuf},
    {"hGetBuf", 3 + IOArity, primHGetBuf},
    {"hIsTerminalDevice", 1 + IOArity, primHIsTerminalDevice},
    {"hGetEcho", 1 + IOArity, primHGetEcho},
    {"hSetEcho", 2 + IOArity, primHSetEcho},

    {"newRef", 1 + IOArity, primNewRef},
    {"getRef", 1 + IOArity, primDerefRef},
    {"setRef", 2 + IOArity, primAssignRef},
    {"eqRef", 2, primEqRef},

    {"makeStablePtr", 1 + IOArity, primMakeSP},
    {"deRefStablePtr", 1 + IOArity, primDerefSP},
    {"freeStablePtr", 1 + IOArity, primFreeSP},
    {"castStablePtrToPtr", 1, primCastSPToP},
    {"castPtrToStablePtr", 1, primCastPToSP},

    {"writeForeignObj", 2 + IOArity, primWriteFP},
    {"eqForeignObj", 2, primEqFP},

    {"newForeignPtr_", 1 + IOArity, primNewFP},
    {"addForeignPtrFinalizer", 2 + IOArity, primAddFPF},
    {"addForeignPtrFinalizerEnv", 3 + IOArity, primAddFPFEnv},
    {"touchForeignPtr", 1 + IOArity, primTouchFP},
    {"unsafeForeignPtrToPtr", 1, primFPToP},

    {"weakPtrEq", 2, primWeakPtrEq},
    {"mkWeak", 3 + IOArity, primMkWeak},
    {"deRefWeak", 1 + IOArity, primDeRefWeak},
    {"replaceFinalizer", 2 + IOArity, primReplaceFinalizer},
    {"finalize", 1 + IOArity, primFinalize},
    {"runFinalizer", 0 + IOArity, primRunFinalizer},
    {"finalizerWaiting", 0 + IOArity, primFinalizerWaiting},

    {"makeStableName", 1 + IOArity, primMakeSN},
    {"deRefStableName", 1, primDerefSN},
    {"hashStableName", 1, primHashSN},
    {"eqStableName", 2, primEqSN},

    {"IOEql", 2 + IOArity, primIOEql},
    {"IOHash", 1 + IOArity, primIOHash},

    {0, 0, 0}};

struct primInfo iomonadPrims = {iomonadControl, iomonadPrimTable, 0};

/* --------------------------------------------------------------------------
 * The monad combinators:
 * ------------------------------------------------------------------------*/

/* IO monad unit */
primFun(primReturnIO) {
    IOReturn(IOArg(1));
}

/* IO monad bind bind 3 2 1 = 3 (pass 2 1) */
primFun(primBindIO) {
    push(ap(namePass, primArg(2)));
    toparg(primArg(1));
    updapRoot(primArg(3), top());
}

/* Auxiliary function pass 3 2 1 = 3 1 2 */
primFun(primPass) {
    push(ap(primArg(3), primArg(1)));
    updapRoot(top(), primArg(2));
}

/* --------------------------------------------------------------------------
 * Handle operations:
 * ------------------------------------------------------------------------*/

/* Return a free Handle or throw an IOError */
static int newHandle(Cell *sCell, char *loc) {
    int i;

    for (i = 0; i < (int)MAX_HANDLES && nonNull(handles[i].hcell); ++i)
        ;                        /* Search for unused handle*/
    if (i >= (int)MAX_HANDLES) { /* If at first we don't    */
        garbageCollect();        /* succeed, garbage collect*/
        for (i = 0; i < (int)MAX_HANDLES && nonNull(handles[i].hcell); ++i)
            ; /* and try again ...       */
        if (i >= (int)MAX_HANDLES) {
            int j;
            growDynTable(dynTabHandles);
            handles = (struct strHandle *)(dynTabHandles->data);
            num_handles = dynTabHandles->maxIdx;
            /* Nil out the new entries in the table */
            for (j = i; j < (int)num_handles; j++) {
                handles[j].hcell = NIL;
            }
        }
    }

    if (i >= (int)MAX_HANDLES) { /* ... before we give up   */
        IOFail(
            mkIOError(NULL, nameIllegal, loc, "too many handles open", sCell));
    }

    return i;
}

/* Return mode string for f(d)open */
static char *modeString(int hmode, bool binary) {
    if (binary) {
        return (hmode & HAPPEND)      ? "ab"
               : (hmode & HWRITE)     ? "wb"
               : (hmode & HREADWRITE) ? "rb+"
               : (hmode & HREAD)      ? "rb"
                                      : (char *)0;
    } else {
        return (hmode & HAPPEND)      ? "a"
               : (hmode & HWRITE)     ? "w"
               : (hmode & HREADWRITE) ? "r+"
               : (hmode & HREAD)      ? "r"
                                      : (char *)0;
    }
}

/* Open handle to file named s in the specified hmode */
static Cell openHandle(StackPtr root, Cell *sCell, int hmode, bool binary,
                       char *loc) {
    int i;
    char *s = evalName(*sCell);
    char *stmode = modeString(hmode, binary);

    /* openHandle() either returns a Handle or throws an IOError. */

    if (!s) { /* check for valid name              */
        IOFail(mkIOError(NULL, nameIllegal, loc, "illegal file name", NULL));
    }

    i = newHandle(sCell, loc);

    if (!stmode) {
        IOFail(mkIOError(NULL, nameIllegal, loc, "illegal mode", NULL));
    }
    /* prepare to open file    */
    handles[i].hfp = fopen(s, stmode);
    if (hmode == HREADWRITE && handles[i].hfp == NULL) /* try to create it */
        handles[i].hfp = fopen(s, binary ? "wb+" : "w+");
    if (!handles[i].hfp)
        throwErrno(loc, true, NO_HANDLE, sCell);

    handles[i].hmode = hmode;
    handles[i].hbufMode = HUNKNOWN_BUFFERING;
    handles[i].hbufSize = (-1);
    setRWState(i, RW_NEUTRAL);
#if CHAR_ENCODING
    handles[i].hBinaryMode = binary;
    handles[i].hLookAhead = -1;
#endif
    return (handles[i].hcell = ap(HANDCELL, i));
}

/* Open handle to file desc fd in the specified hmode */
static Cell openFdHandle(StackPtr root, int fd, int hmode, bool binary,
                         char *loc) {
    int i = newHandle(NIL, loc);
    char *stmode = modeString(hmode, binary);

    /* openFdHandle() either returns a Handle or throws an IOError. */

    if (!stmode) {
        IOFail(mkIOError(NULL, nameIllegal, loc, "illegal mode", NULL));
    }
    if (!(handles[i].hfp = fdopen(fd, stmode)))
        throwErrno(loc, true, NO_HANDLE, NULL);

    handles[i].hmode = hmode;
    handles[i].hbufMode = HANDLE_NOTBUFFERED;
    handles[i].hbufSize = 0;
    setRWState(i, RW_NEUTRAL);
#if CHAR_ENCODING
    handles[i].hBinaryMode = binary;
    handles[i].hLookAhead = -1;
#endif
    return (handles[i].hcell = ap(HANDCELL, i));
}

static Char hGetChar(int h, char *fname) {
    int c;
#if CHAR_ENCODING
    if (handles[h].hLookAhead >= 0) {
        c = handles[h].hLookAhead;
        handles[h].hLookAhead = -1;
    } else if (handles[h].hBinaryMode) {
        c = getc(handles[h].hfp);
    } else {
        c = FGetChar(handles[h].hfp);
    }
#else /* !CHAR_ENCODING */
    c = FGetChar(handles[h].hfp);
#endif
    if (c == EOF && !feof(handles[h].hfp))
        throwErrno(fname, true, h, NULL);
#if CHAR_ENCODING
    else if (c == BAD_CHAR) {
        IOFail(mkIOError(
            &handles[h].hcell, nameProtocolError, fname,
            "input contains non-character data - use binary I/O for binary data",
            NULL));
    }
#endif /* CHAR_ENCODING */
    return c;
}

static void hPutChar(Char c, int h, char *fname) {
    int retval;
#if CHAR_ENCODING
    retval = handles[h].hBinaryMode ? putc(c, handles[h].hfp)
                                    : FPutChar(c, handles[h].hfp);
#else
    retval = FPutChar(c, handles[h].hfp);
#endif
    if (retval == EOF)
        throwErrno(fname, true, h, NULL);
    if (h <= 2) { /* Only flush the standard handles */
        fflush(handles[h].hfp);
    }
}

/* If the stream is read-write, set the state, otherwise do nothing */
static void setRWState(int h, int newState) {
    if (handles[h].hmode & HREADWRITE) {
        /* ANSI C requires repositioning between writes and reads */
        if (newState == RW_READING) {
            if (handles[h].hRWState == RW_WRITING) {
                fflush(handles[h].hfp);
                fseek(handles[h].hfp, 0L, SEEK_CUR);
            }
        } else if (newState == RW_WRITING) {
            if (handles[h].hRWState == RW_READING)
                fseek(handles[h].hfp, 0L, SEEK_CUR);
        }
        handles[h].hRWState = newState;
    }
}

/* ensure that the handle is neither closed nor semi-closed */
static void checkOpen(int h, char *fname) {
    int mode = handles[h].hmode;
    if (mode == HCLOSED)
        IOFail(mkIOError(&handles[h].hcell, nameIllegal, fname,
                         "handle is closed", NULL));
    if (mode == HSEMICLOSED)
        IOFail(mkIOError(&handles[h].hcell, nameIllegal, fname,
                         "handle is semi-closed", NULL));
}

/* ensure that the handle is readable */
static void checkReadable(int h, char *fname) {
    checkOpen(h, fname);
    if (!(handles[h].hmode & (HREAD | HREADWRITE)))
        IOFail(mkIOError(&handles[h].hcell, nameIllegal, fname,
                         "handle is not readable", NULL));
}

/* ensure that the handle is writable */
static void checkWritable(int h, char *fname) {
    checkOpen(h, fname);
    if (!(handles[h].hmode & (HWRITE | HAPPEND | HREADWRITE)))
        IOFail(mkIOError(&handles[h].hcell, nameIllegal, fname,
                         "handle is not writeable", NULL));
}

/* --------------------------------------------------------------------------
 * Building strings:
 * ------------------------------------------------------------------------*/

/* Push pointer to string onto stack */
void pushString(char *s) {
    if (*s == '\0')
        push(nameNil);
    else {
        Cell l = ap(consChar(ExtractChar(s)), NIL);
        push(l);
        while (*s) {
            snd(l) = ap(consChar(ExtractChar(s)), NIL);
            l = snd(l);
        }
        snd(l) = nameNil;
    }
}

/* Helper function for constructing IOErrors (see Prelude defn of
 * IOError for explanation of what the the individual arguments
 * do.
 */

/* A Handle or NULL an IOErrorType a FilePath or NULL */
Cell mkIOError(Cell *mbH, Name kind, char *loc, char *desc, Cell *mbF) {
    Cell str;
    push(nameIOError);
    toparg(mbH == NULL ? nameNothing : ap(nameJust, *mbH));
    toparg(kind);
    pushString(loc);
    str = pop();
    toparg(str);
    pushString(desc);
    str = pop();
    toparg(str);
    toparg(mbF == NULL ? nameNothing : ap(nameJust, *mbF));
    return pop();
}

/* --------------------------------------------------------------------------
 * IO Errors (more defined for file ops)
 * ------------------------------------------------------------------------*/

void throwErrno(char *fname, bool isFile, int h, Cell *mbF) {
    IOFail(mkIOError(h == NO_HANDLE ? NULL : &handles[h].hcell,
                     toIOError(errno), fname, toIOErrorDescr(errno, isFile),
                     mbF));
}

/*
 * Map a libc error code to an IOError
 */
static Name toIOError(int errc) {
    switch (errc) {

    case EEXIST:
        return nameAlreadyExists;
    case ENOENT:
    case ENOTDIR:
        return nameDoesNotExist;
    case EPERM:
    case EACCES:
        return namePermDenied;
    case ENOSPC:
    case EFBIG:
        return nameIsFull;
    default:
        return nameIllegal;
    }
}

/*
 * Map a libc error code to an IOError descriptive string
 */
static char *toIOErrorDescr(int errc, bool isFile) {
    switch (errc) {

    case EEXIST:
        return (isFile ? "file already exists" : "directory already exists");
    case ENOENT:
    case ENOTDIR:
        return (isFile ? "file does not exist" : "directory does not exist");
    case EPERM:
    case EACCES:
        return ""; /* No need to replicate the info conveyed by the IOErrorKind */
    case ENOSPC:
    case EFBIG:
        return "device is full";
    default:
        return "";
    }
}

/* --------------------------------------------------------------------------
 * Misc.
 * ------------------------------------------------------------------------*/

/* Force a GC right now */
primFun(primGC) {
    garbageCollect();
    IOReturn(nameUnit);
}

#include <time.h>

/* Generate a random seed */
primFun(primGetRandomSeed) {
    IOReturn(bigInt(clock()));
}

/* PrimGetEnv :: String -> IO String */
primFun(primGetEnv) {
    char *s = evalName(IOArg(1)); /* Eval name                    */
    char *r;
    if (!s) { /* check for valid name              */
        IOFail(mkIOError(NULL, nameIllegal, "System.getEnv",
                         "illegal environment variable name", &IOArg(1)));
    }
    if ((r = getenv(s)) == 0) {
        IOFail(mkIOError(NULL, nameDoesNotExist, "System.getEnv",
                         "environment variable not found", &IOArg(1)));
    }
    pushString(r);
    IOReturn(pop());
}

#include <sys/types.h>
#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
/* If it's not defined, return it verbatim. */
# define WEXITSTATUS(stat_val) (stat_val)
#endif

/* PrimSystem :: String -> IO Int */
primFun(primSystem) {
    char *s = evalName(IOArg(1)); /* Eval name                    */
    int r;
    if (!s) { /* check for valid string          */
        IOFail(mkIOError(NULL, nameIllegal, "System.system",
                         "illegal system command string", &IOArg(1)));
    }
    r = shellEsc(s);
    IOReturn(mkInt(WEXITSTATUS(r)));
}

void setCuddlesArgs(int argc, char *argv[]) {
    int i;
    Cell str;

    cuddlesArgs = nameNil;
    pushString(argv[0]);
    cuddlesProgName = pop();
    for (i = argc - 1; i > 0; i--) {
        pushString(argv[i]);
        str = pop();
        cuddlesArgs = ap2(nameCons, str, cuddlesArgs);
    }
}

/* PrimGetProgName :: IO String */
primFun(primGetProgName) {
    IOReturn(cuddlesProgName);
}

/* PrimGetArgs :: IO [String] */
primFun(primGetArgs) {
    IOReturn(cuddlesArgs);
}

/* PrimSetProgName :: String -> IO () */
primFun(primSetProgName) {
    cuddlesProgName = IOArg(1);
    IOReturn(nameUnit);
}

/* PrimSetArgs :: [String] -> IO () */
primFun(primSetArgs) {
    cuddlesArgs = IOArg(1);
    IOReturn(nameUnit);
}

/* --------------------------------------------------------------------------
 * File IO
 * ------------------------------------------------------------------------*/

/* Read character from handle */
primFun(primHGetChar) {
    int h;
    int c;
    HandleArg(h, 1 + IOArity);

    checkReadable(h, "IO.hGetChar");
    setRWState(h, RW_READING);
    c = hGetChar(h, "IO.hGetChar");
    if (c == EOF) {
        setRWState(h, RW_NEUTRAL);
        IOFail(mkIOError(&handles[h].hcell, nameEOFErr, "IO.hGetChar",
                         "end of file", NULL));
    }
    IOReturn(mkChar(c));
}

/* Print character on handle */
primFun(primHPutChar) {
    Char c = 0;
    int h;
    HandleArg(h, 2 + IOArity);
    CharArg(c, 1 + IOArity);

    checkWritable(h, "IO.hPutChar");
    setRWState(h, RW_WRITING);
    hPutChar(c, h, "IO.hPutChar");
    IOReturn(nameUnit);
}

/* Print string on handle */
primFun(primHPutStr) {
    int h;
    HandleArg(h, 2 + IOArity);
    push(primArg(1 + IOArity));
    primArg(1 + IOArity) = NIL;

    checkWritable(h, "IO.hPutStr");
    setRWState(h, RW_WRITING);
    blackHoleRoot();
    eval(pop());
    while (whnfHead == nameCons) {
        eval(pop());
        hPutChar(charOf(whnfHead), h, "IO.hPutStr");
        eval(pop());
    }
    IOReturn(nameUnit);
}

/* Read String from a handle */
primFun(primHreader) {
    int h; /* Handle -> String                */
    HandleArg(h, 1);
    if (handles[h].hmode & HSEMICLOSED) { /* read requires semi-closed handle*/
        int c = hGetChar(h, "IO.getContents");
        if (c != EOF && c >= 0 && c <= MAXCHARVAL) {
            updapRoot(consChar(c), ap(nameHreader, primArg(1)));
            return;
        }
        clearerr(handles[h].hfp);
    }
    updateRoot(nameNil);
}

/* HGetContents :: Handle -> IO Str */
primFun(primHContents) {
    int h;
    HandleArg(h, 1 + IOArity);

    checkReadable(h, "IO.hGetContents"); /* must have readable handle     */
    setRWState(h, RW_READING);
    handles[h].hmode = HSEMICLOSED; /* semi-close handle                   */
    IOReturn(ap(nameHreader, IOArg(1)));
}

/* From IOMode to internal form */
static int getIOMode(Cell mode) {
    int m = HCLOSED;

    eval(mode); /* Eval IOMode                     */
    if (isName(whnfHead) && isCfun(whnfHead))
        switch (cfunOf(whnfHead)) { /* we have to use numeric consts   */
        case 1:
            m = HREAD; /* here to avoid the need to put   */
            break;     /* IOMode in startup environment   */
        case 2:
            m = HWRITE;
            break;
        case 3:
            m = HAPPEND;
            break;
        case 4:
            m = HREADWRITE;
            break;
        default:
            break;
        }
    return m;
}

/* Auxiliary function for opening a file */
static void fopenPrim(StackPtr root, bool binary, char *loc) {
    int m;

    m = getIOMode(IOArg(1));
    if (m == HCLOSED) { /* Only accept legal modes         */
        IOFail(mkIOError(NULL, nameIllegal, loc, "unknown handle mode",
                         &IOArg(2)));
    }
    IOReturn(openHandle(root, &IOArg(2), m, binary, loc));
}

/* Open handle to a text file */
primFun(primOpenFile) {
    fopenPrim(root, false, "IO.openFile");
}

/* Open handle to a binary file */
primFun(primOpenBinaryFile) {
    fopenPrim(root, true, "System.IO.openBinaryFile");
}

/* Standard input handle */
primFun(primStdin) {
    push(handles[HSTDIN].hcell);
}

/* Standard output handle */
primFun(primStdout) {
    push(handles[HSTDOUT].hcell);
}

/* Standard error handle */
primFun(primStderr) {
    push(handles[HSTDERR].hcell);
}

/* Open handle to file descriptor. */
primFun(primOpenFd) {
    int m; /* :: Int{-Fd-} -> Bool -> IOMode -> Bool -> IO Handle */
    int fd;
    bool binary;
    [[maybe_unused]] bool isSock;

    IntArg(fd, 4 + IOArity);
    BoolArg(isSock, 3 + IOArity);
    BoolArg(binary, 1 + IOArity);

    m = getIOMode(IOArg(2));
    if (m == HCLOSED) { /* Only accept legal modes         */
        IOFail(mkIOError(NULL, nameIllegal, "openFd", "unknown handle mode",
                         NULL));
    }

#if defined(_WIN32) && !cygwin32_HOST_OS
    if (isSock) {
        /* fd is a SOCKET, convert it to an FD.
         * Note: _open_osfhandle() will fail under
         * Win9x. ToDo: better on those plats.
         */
        fd = _open_osfhandle(fd, (m & HREAD ? O_RDONLY : 0) |
                                     (m & HWRITE ? O_WRONLY : 0) |
                                     (m & HREADWRITE ? O_RDWR : 0) |
                                     (m & HAPPEND ? O_APPEND : 0) |
                                     (binary ? O_BINARY : O_TEXT));
    }
#endif

    IOReturn(openFdHandle(root, fd, m, binary, "openFd"));
}

/* Extract the file descriptor from a Handle, discarding the Handle */
primFun(primHandleToFd) {
    int h;
    HandleArg(h, 1 + IOArity);
    if (IS_STANDARD_HANDLE(h)) {
        IOFail(mkIOError(NULL, nameIllegal, "System.Posix.IO.handleToFd",
                         "invalid handle", NULL));
    }
    checkOpen(h, "System.Posix.IO.handleToFd");
    {
        int fd = dup(fileno(handles[h].hfp));
        fclose(handles[h].hfp);
        handles[h].hfp = 0;
        handles[h].hmode = HCLOSED;
        IOReturn(mkInt(fd));
    }
}

/* Test for end of file on handle */
primFun(primHIsEOF) {
    /* :: Handle -> IO Bool */
    int h;
    FILE *fp;
    bool isEOF;
    HandleArg(h, 1 + IOArity);
    checkReadable(h, "IO.hIsEOF");
    fp = handles[h].hfp;
    isEOF = feof(fp);
    if (isEOF) { /* If the EOF flag is already signalled,
                    peeking at the next char isn't likely
                    to produce a different outcome! */
        IOBoolResult(isEOF);
#if CHAR_ENCODING
    } else if (handles[h].hLookAhead >= 0) {
        IOReturn(nameFalse);
#endif
    } else {
        int c;

        setRWState(h, RW_READING);
        c = fgetc(fp);
        isEOF = feof(fp);
        if (isEOF)
            setRWState(h, RW_NEUTRAL);
        /* Put the char back and clear any flags. */
        ungetc(c, fp);
        clearerr(fp);

        IOBoolResult(isEOF);
    }
}

/* Flush handle */
primFun(primHFlush) {
    int h;
    HandleArg(h, 1 + IOArity);
    checkWritable(h, "IO.hFlush");
    fflush(handles[h].hfp);
    IOReturn(nameUnit);
}

/* Close handle */
primFun(primHClose) {
    int h;
    HandleArg(h, 1 + IOArity);

    /* Disallow closing any of the standard handles */
    if (!IS_STANDARD_HANDLE(h) && handles[h].hmode != HCLOSED) {
        if (h > HSTDERR && handles[h].hfp)
            fclose(handles[h].hfp);
        handles[h].hfp = 0;
        handles[h].hmode = HCLOSED;
    }
    /* closing an already closed handle is the identity
       (i.e., not an error.) */
    IOReturn(nameUnit);
}

/* Get file position */
primFun(primHGetPosn) {
    int h;
    long pos;
    HandleArg(h, 1 + IOArity);
    checkOpen(h, "IO.hGetPosn");
    pos = ftell(handles[h].hfp);
    IOReturn(mkInt((int)pos));
}

/* Set file position */
primFun(primHSetPosn) {
    long pos = 0;
    int h;
    HandleArg(h, 2 + IOArity);
    IntArg(pos, 1 + IOArity);
    checkOpen(h, "IO.hSetPosn");
    setRWState(h, RW_NEUTRAL);
    fflush(handles[h].hfp);
    if (fseek(handles[h].hfp, pos, SEEK_SET) == 0) {
        IOReturn(nameUnit);
    }
}

/* Seek to new file posn */
primFun(primHSeek) {
    /* :: Handle -> Int -> Int -> IO () */
    int h;
    int sMode;
    int off;

    HandleArg(h, 3 + IOArity);
    IntArg(sMode, 2 + IOArity);
    IntArg(off, 1 + IOArity);

    if (sMode == 0)
        sMode = SEEK_SET;
    else if (sMode == 1)
        sMode = SEEK_CUR;
    else
        sMode = SEEK_END;

    checkOpen(h, "IO.hSeek");
    if (fseek(handles[h].hfp, off, sMode) != 0)
        throwErrno("IO.hSeek", true, h, NULL);

    setRWState(h, RW_NEUTRAL);

    IOReturn(nameUnit);
}

/* Peek at the next char */
primFun(primHLookAhead) {
    /* :: Handle -> IO Char  */
    int h;
    int c;

    HandleArg(h, 1 + IOArity);

    checkReadable(h, "IO.hLookAhead");
#if CHAR_ENCODING
    if (handles[h].hLookAhead >= 0) {
        IOReturn(mkChar(handles[h].hLookAhead));
        return;
    }
#endif
    setRWState(h, RW_READING);
    if (feof(handles[h].hfp) || (c = hGetChar(h, "IO.hLookAhead")) == EOF) {
        setRWState(h, RW_NEUTRAL);
        IOFail(mkIOError(&handles[h].hcell, nameEOFErr, "IO.hLookAhead",
                         "end of file", NULL));
    }
#if CHAR_ENCODING
    if (handles[h].hBinaryMode)
        ungetc(c, handles[h].hfp);
    else
        handles[h].hLookAhead = c;
#else
    ungetc(c, handles[h].hfp);
#endif
    IOReturn(mkChar(c));
}

/* Change a Handle's buffering */
primFun(primHSetBuffering) {
    /* :: Handle -> Int -> Int -> IO () */
    int h;
    int ty;
    int sz;
    int rc;
    HandleArg(h, 3 + IOArity);
    IntArg(ty, 2 + IOArity);
    IntArg(sz, 1 + IOArity);

    checkOpen(h, "IO.hSetBuffering");
    switch (ty) {
    case 0:
        ty = _IONBF;
        handles[h].hbufMode = HANDLE_NOTBUFFERED;
        handles[h].hbufSize = 0;
        break;
    case 1:
        ty = _IOLBF;
        sz = BUFSIZ;
        handles[h].hbufMode = HANDLE_LINEBUFFERED;
        handles[h].hbufSize = 0;
        break;
    case 2:
        ty = _IOFBF;
        handles[h].hbufMode = HANDLE_BLOCKBUFFERED;
        if (sz == 0) {
            sz = BUFSIZ;
        }
        handles[h].hbufSize = sz;
        break;
    default:
        IOFail(mkIOError(&handles[h].hcell, nameIllegal, "IO.hSetBuffering",
                         "illegal buffer mode", NULL));
    }

    /* Change the buffering mode; setvbuf() flushes the old buffer. */
    /* Let setvbuf() allocate the buffer for us. */
    rc = setvbuf(handles[h].hfp, NULL, ty, sz);
    if (rc != 0)
        throwErrno("IO.hSetBuffering", true, h, NULL);
    if ((handles[h].hmode & (HWRITE | HAPPEND | HREADWRITE)) &&
        isatty(fileno(handles[h].hfp)))
        setBuffTerminal(fileno(handles[h].hfp), ty != 0);
    IOReturn(nameUnit);
}

/* Return buffering info of a handle. */
primFun(primHGetBuffering) {
    /*  Handle :: IO (Int,Int)            */
    int h;
    HandleArg(h, 1 + IOArity);

    checkOpen(h, "IO.hGetBuffering");
    if (handles[h].hbufMode == HUNKNOWN_BUFFERING) {
        /* figure out buffer mode and size. */
        if (isatty(fileno(handles[h].hfp))) {
            /* TTY connected handles are normally linebuffered. */
            handles[h].hbufMode =
                (handles[h].hmode & (HWRITE | HAPPEND | HREADWRITE)) == 0 ||
                        getBuffTerminal(fileno(handles[h].hfp))
                    ? HANDLE_LINEBUFFERED
                    : HANDLE_NOTBUFFERED;
            handles[h].hbufSize = 0;
        } else {
            /* ..if not, block buffered. */
            handles[h].hbufMode = HANDLE_BLOCKBUFFERED;
            handles[h].hbufSize = BUFSIZ;
        }
    }
    IOReturn(ap(ap(mkTuple(2), mkInt((int)handles[h].hbufMode)),
                mkInt((int)handles[h].hbufSize)));
}

/* Test is handle open */
primFun(primHIsOpen) {
    int h;
    HandleArg(h, 1 + IOArity);
    IOBoolResult(handles[h].hmode != HCLOSED &&
                 handles[h].hmode != HSEMICLOSED);
}

/* Test is handle closed */
primFun(primHIsClosed) {
    int h;
    HandleArg(h, 1 + IOArity);
    IOBoolResult(handles[h].hmode == HCLOSED);
}

/* Test is handle readable */
primFun(primHIsReadable) {
    int h;
    HandleArg(h, 1 + IOArity);
    IOBoolResult(handles[h].hmode & (HREAD | HREADWRITE));
}

/* Test is handle writable */
primFun(primHIsWritable) {
    int h;
    HandleArg(h, 1 + IOArity);
    IOBoolResult(handles[h].hmode & (HWRITE | HREADWRITE | HAPPEND));
}

#if defined(IS_WINDOWS) && !defined(S_ISREG)
# define S_ISREG(x) ((x) & _S_IFREG)
#endif

/* Test if handle is writable */
primFun(primHIsSeekable) {
    int h;
    bool okHandle;
    struct stat sb;

    HandleArg(h, 1 + IOArity);

    okHandle = (handles[h].hmode & (HREAD | HWRITE | HREADWRITE | HAPPEND));
    if (okHandle && (fstat(fileno(handles[h].hfp), &sb) == 0)) {
        okHandle = S_ISREG(sb.st_mode);
    }
    IOBoolResult(okHandle);
}

/* If handle points to a regular file, return the size of the file
 * :: Handle -> IO Integer */
primFun(primHFileSize) {
    int h;
    struct stat sb;

    HandleArg(h, 1 + IOArity);

    checkOpen(h, "IO.hFileSize");
    if (handles[h].hmode & (HWRITE | HREADWRITE | HAPPEND))
        fflush(handles[h].hfp);
    if (fstat(fileno(handles[h].hfp), &sb) != 0 || !S_ISREG(sb.st_mode)) {
        IOFail(mkIOError(&handles[h].hcell, nameIllegal, "IO.hFileSize",
                         "not a regular file", NULL));
    }
    IOReturn(bigInt64(sb.st_size));
}

/* Test for handle equality */
primFun(primEqHandle) {
    int h1, h2;
    HandleArg(h1, 1);
    HandleArg(h2, 2);
    BoolResult(h1 == h2);
}

primFun(primGetHandleNumber) {
    int h;
    HandleArg(h, 1);
    IntResult(h);
}

primFun(primHSetBinaryMode) {
    int h;
    bool binary;
    HandleArg(h, 2 + IOArity);
    BoolArg(binary, 1 + IOArity);

    checkOpen(h, "System.IO.hSetBinaryMode");
#if defined(mingw32_HOST_OS) || defined(_MSC_VER)
    setmode(fileno(handles[h].hfp), binary ? _O_BINARY : _O_TEXT);
#endif
#if CHAR_ENCODING
    handles[h].hBinaryMode = binary;
#endif
    IOReturn(nameUnit);
}

/* Write binary data from a buffer */
primFun(primHPutBuf) {
    int h, size;
    void *buf;

    HandleArg(h, 3 + IOArity);
    PtrArg(buf, 2 + IOArity);
    IntArg(size, 1 + IOArity);

    /* argument checks */
    checkWritable(h, "System.IO.hPutBuf");
    if (size < 0) {
        IOFail(mkIOError(&handles[h].hcell, nameIllegal, "System.IO.hPutBuf",
                         "illegal buffer size", NULL));
    }
#if CHAR_ENCODING
    if (!handles[h].hBinaryMode) {
        IOFail(mkIOError(&handles[h].hcell, nameIllegal, "System.IO.hPutBuf",
                         "not a binary handle", NULL));
    }
#endif
    setRWState(h, RW_WRITING);

    errno = 0;
    while (size > 0) {
        size -= (int)fwrite(buf, 1, size, handles[h].hfp);
        if (ferror(handles[h].hfp)) /* a successful fwrite may still  */
            throwErrno("System.IO.hPutBuf", true, h, NULL); /* set errno */
    }

    IOReturn(nameUnit);
}

/* Read binary data into a buffer */
primFun(primHGetBuf) {
    int h, size, numRead;
    void *buf;

    HandleArg(h, 3 + IOArity);
    PtrArg(buf, 2 + IOArity);
    IntArg(size, 1 + IOArity);

    /* argument checks */
    checkReadable(h, "System.IO.hGetBuf");
    if (size < 0) {
        IOFail(mkIOError(&handles[h].hcell, nameIllegal, "System.IO.hGetBuf",
                         "illegal buffer size", NULL));
    }
#if CHAR_ENCODING
    if (!handles[h].hBinaryMode) {
        IOFail(mkIOError(&handles[h].hcell, nameIllegal, "System.IO.hGetBuf",
                         "not a binary handle", NULL));
    }
#endif
    setRWState(h, RW_READING);

    numRead = (int)fread(buf, 1, size, handles[h].hfp);
    if (numRead < size && ferror(handles[h].hfp))
        throwErrno("System.IO.hGetBuf", true, h, NULL);
    IOReturn(mkInt(numRead));
}

/* Check whether a character can be read from a handle within x msecs
 * :: Handle -> Int -> IO Bool */
primFun(primHWaitForInput) {
    int h;
    int msecs;

    HandleArg(h, 2 + IOArity);
    IntArg(msecs, 1 + IOArity);

#if HAVE_SELECT
    checkReadable(h, "IO.hWaitForInput");
    {
        /* Implementation is a rip-off of GHC's inputReady.c */
        int maxfd, fd;
        int ready;
        fd_set rfd;
        struct timeval tv;

        FD_ZERO(&rfd);
        fd = fileno(handles[h].hfp);
        FD_SET(fd, &rfd);

        maxfd = fd + 1;
        tv.tv_sec = msecs / 1000;
        /* tv_usec counts microseconds; the product is at most 999000, but
         * widen before multiplying so the arithmetic is in the field's type */
        tv.tv_usec = (suseconds_t)(msecs % 1000) * 1000;

        while ((ready = select(maxfd, &rfd, NULL, NULL, &tv)) < 0) {
            if (errno != EINTR) {
                IOFail(mkIOError(&handles[h].hcell, nameIllegal,
                                 "IO.hWaitForInput",
                                 "input waiting terminated by signal", NULL));
            }
        }
        IOBoolResult(ready > 0);
    }
#else
    /* For now, punt on implementing async IO under Win32 */
    /* For other platforms that don't support select() on file
       file descs, please insert code that'll work. */
    IOFail(mkIOError(&handles[h].hcell, nameIllegal, "IO.hWaitForInput",
                     "unsupported operation", NULL));
#endif
}

/* Does the handle refer to a terminal? */
primFun(primHIsTerminalDevice) {
    int h;
    HandleArg(h, 1 + IOArity);
    checkOpen(h, "System.IO.hIsTerminalDevice");
    IOBoolResult(isatty(fileno(handles[h].hfp)));
}

primFun(primHGetEcho) {
    int h;
    int fd;
    HandleArg(h, 1 + IOArity);
    checkOpen(h, "System.IO.hGetEcho");
    fd = fileno(handles[h].hfp);
    IOBoolResult(isatty(fd) && getEchoTerminal(fd));
}

primFun(primHSetEcho) {
    int h;
    bool echo;
    int fd;
    HandleArg(h, 2 + IOArity);
    BoolArg(echo, 1 + IOArity);
    checkOpen(h, "System.IO.hSetEcho");
    fd = fileno(handles[h].hfp);
    if (isatty(fd))
        setEchoTerminal(fd, echo);
    IOReturn(nameUnit);
}

/* --------------------------------------------------------------------------
 * Mutable variables
 * ------------------------------------------------------------------------*/

#if CHECK_TAGS
# define checkRef()                                                            \
     if (MUTVAR != whatIs(whnfHead))                                           \
     internal("Ref expected")
#else
# define checkRef() /* do nothing */
#endif

/* A -> IO (Ref a) */
primFun(primNewRef) {
    IOReturn(ap(MUTVAR, IOArg(1)));
}

/* Ref a -> IO a */
primFun(primDerefRef) {
    eval(pop());
    checkRef();
    IOReturn(snd(whnfHead));
}

/* Ref a -> a -> IO () */
primFun(primAssignRef) {
    eval(IOArg(2));
    checkRef();
    snd(whnfHead) = IOArg(1);
    IOReturn(nameUnit);
}

/* Ref a -> Ref a -> Bool */
primFun(primEqRef) {
    eval(primArg(2));
    checkRef();
    push(whnfHead);
    eval(primArg(1));
    checkRef();
    updateRoot(pop() == whnfHead ? nameTrue : nameFalse);
}

/* --------------------------------------------------------------------------
 * Stable Pointers
 * ------------------------------------------------------------------------*/

#if CHECK_TAGS
# define checkSP() checkInt()
#else
# define checkSP() /* do nothing */
#endif

#define SPArg(nm, offset)                                                      \
    eval(primArg(offset));                                                     \
    checkSP();                                                                 \
    nm = (CuddlesStablePtr)whnfInt

/* nm should be a variable in which result is stored.
   If you use an expression, reevaluation might occur */
#define SPResult(nm) updateRoot(mkInt((int)(nm)))

/* A -> IO (StablePtr a) */
primFun(primMakeSP) {
    CuddlesStablePtr sp = mkStablePtr(IOArg(1));
    if (sp == 0) {
        IOFail(mkIOError(NULL, nameIsFull, "Foreign.makeStablePtr",
                         "too many StablePtrs", NULL));
    }
    IOReturn(mkInt(sp));
}

/* StablePtr a -> IO a */
primFun(primDerefSP) {
    CuddlesStablePtr x;
    SPArg(x, 1 + IOArity);

    IOReturn(derefStablePtr(x));
}

/* StablePtr a -> IO () */
primFun(primFreeSP) {
    CuddlesStablePtr x;
    SPArg(x, 1 + IOArity);
    freeStablePtr(x);
    IOReturn(nameUnit);
}

/* StablePtr a -> Ptr () */
primFun(primCastSPToP) {
    CuddlesStablePtr x;
    SPArg(x, 1);
    PtrResult((void *)(intptr_t)x);
}

/* Ptr () -> StablePtr a */
primFun(primCastPToSP) {
    void *x;
    CuddlesStablePtr idx; /* not sp: that is the machine stack pointer */
    PtrArg(x, 1);
    idx = (CuddlesStablePtr)(intptr_t)x;
    SPResult(idx);
}

/* --------------------------------------------------------------------------
 * Foreign Objects
 * ------------------------------------------------------------------------*/

#if CHECK_TAGS
# define checkForeign()                                                        \
     if (MPCELL != whatIs(whnfHead))                                           \
     internal("ForeignObj expected")
#else
# define checkForeign() /* do nothing */
#endif

/* Ptr a -> IO (ForeignPtr a) */
primFun(primNewFP) {
    void *addr = 0;
    eval(IOArg(1));
    addr = ptrOf(whnfHead);
    IOReturn(newMallocPtr(addr));
}

/* FunPtr (Ptr a -> IO ()) -> ForeignPtr a -> IO () */
primFun(primAddFPF) {
    int mp;
    eval(IOArg(1));
    mp = mpOf(whnfHead);
    eval(IOArg(2));
    mallocPtrs[mp].finalizers = cons(whnfHead, mallocPtrs[mp].finalizers);
    IOReturn(nameUnit);
}

/* FunPtr (Ptr env -> Ptr a -> IO ()) -> Ptr Env -> ForeignPtr a -> IO () */
primFun(primAddFPFEnv) {
    int mp;
    eval(IOArg(1));
    mp = mpOf(whnfHead);
    eval(IOArg(2));
    push(whnfHead);
    eval(IOArg(3));
    mallocPtrs[mp].finalizers =
        cons(pair(whnfHead, pop()), mallocPtrs[mp].finalizers);
    IOReturn(nameUnit);
}

/* ForeignPtr a -> Ptr a -> IO () */
primFun(primWriteFP) {
    Cell mp = NIL;
    eval(IOArg(2));
    checkForeign();
    mp = whnfHead;
    eval(IOArg(1));
    derefMP(mp) = ptrOf(whnfHead);
    IOReturn(nameUnit);
}

/* ForeignPtr a -> ForeignPtr a -> Bool */
primFun(primEqFP) {
    eval(primArg(2));
    checkForeign();
    push(whnfHead);
    eval(primArg(1));
    checkForeign();
    updateRoot(pop() == whnfHead ? nameTrue : nameFalse);
}

/* ForeignPtr a -> IO () */
primFun(primTouchFP) {
    eval(IOArg(1));
    checkForeign();
    IOReturn(nameUnit);
}

/* ForeignPtr a -> Ptr a */
primFun(primFPToP) {
    eval(primArg(1));
    checkForeign();
    PtrResult(derefMP(whnfHead));
}

/* --------------------------------------------------------------------------
 * Stable Names
 * ------------------------------------------------------------------------*/

/* A -> IO (StableName a) */
primFun(primMakeSN) {
    IOReturn(ap(STABLENAME, IOArg(1)));
}

/* StableName a -> a */
primFun(primDerefSN) {
    eval(primArg(1));
    updateRoot(snd(whnfHead));
}

/* StableName a -> Int */
primFun(primHashSN) {
    eval(primArg(1));
    updateRoot(mkInt(whnfHead));
}
/* StableName a -> StableName a -> Bool */
primFun(primEqSN) {
    eval(primArg(2));
    push(whnfHead);
    eval(primArg(1));
    updateRoot(pop() == whnfHead ? nameTrue : nameFalse);
}

/* --------------------------------------------------------------------------
 * Weak Pointers
 * ------------------------------------------------------------------------*/

#if CHECK_TAGS
# define checkWeak()                                                           \
     if (WEAKCELL != whatIs(whnfHead))                                         \
         internal("weakPtr expected");
#else
# define checkWeak() /* do nothing */
#endif

/* Weak a -> Weak a -> Bool */
primFun(primWeakPtrEq) {
    eval(primArg(2));
    push(whnfHead);
    eval(primArg(1));
    updateRoot(pop() == whnfHead ? nameTrue : nameFalse);
}

/* K -> v -> Maybe (IO ()) */
primFun(primMkWeak) {
    Cell w = NIL; /*                    -> IO (Weak v) */
    eval(IOArg(1));
    if (whnfHead == nameJust) { /* Look for finalizer              */
        w = pop();
    }
    w = ap(NIL, ap(NIL, ap(NIL, w)));
    fst(snd(w)) = IOArg(3);
    fst(snd(snd(w))) = IOArg(2);
    liveWeakPtrs = cons(w, liveWeakPtrs);
    fst(w) = WEAKFIN;
    IOReturn(w);
}

/* Weak v -> IO (Maybe v) */
primFun(primDeRefWeak) {
    eval(IOArg(1));
    if (whatIs(whnfHead) != WEAKFIN) {
        internal("primDeRefWeak");
    }
    if (nonNull(snd(whnfHead))) {
        IOReturn(ap(nameJust, fst(snd(snd(whnfHead)))));
    } else {
        IOReturn(nameNothing);
    }
}

/* Weak v -> Maybe (IO ()) */
primFun(primReplaceFinalizer) {
    /*      -> IO (Maybe (IO ()))      */
    eval(IOArg(1)); /* Grab new finalizer ...          */
    if (whnfHead != nameJust) {
        push(NIL);
    }
    eval(IOArg(2)); /* Get weak pointer ...                */
    if (whatIs(whnfHead) != WEAKFIN) {
        internal("primReplaceFinalizer");
    } else if (nonNull(snd(whnfHead))) { /* ... and replace finalizer      */
        Cell oldfin = snd(snd(snd(whnfHead)));
        snd(snd(snd(whnfHead))) = pop();
        if (nonNull(oldfin)) {
            IOReturn(ap(nameJust, oldfin));
        }
    }
    IOReturn(nameNothing);
}

/* Weak v -> IO () */
primFun(primFinalize) {
    eval(IOArg(1));                    /* Bring weak pointer to an early  */
    if (whatIs(whnfHead) != WEAKFIN) { /* end ...                          */
        internal("primFinalize");
    } else if (nonNull(snd(whnfHead))) {
        Cell wp = whnfHead;
        Cell vf = snd(snd(wp));
        if (isPair(vf)) {
            if (nonNull(snd(vf))) {
                fst(vf) = snd(vf);
                snd(vf) = finalizers;
                finalizers = vf;
            }
            fst(snd(wp)) = NIL;
            snd(snd(wp)) = NIL;
            snd(wp) = NIL;
        }
        liveWeakPtrs = removeCell(wp, liveWeakPtrs);
    }
    IOReturn(nameUnit);
}

/* IO () */
primFun(primRunFinalizer) {
    if (isNull(finalizers)) {
        IOReturn(nameUnit);
    } else {
        updapRoot(ap(hd(finalizers), primArg(2)), primArg(1));
        finalizers = tl(finalizers);
        return;
    }
}

/* IO Boolean */
primFun(primFinalizerWaiting) {
    IOBoolResult(!isNull(finalizers));
}

/* --------------------------------------------------------------------------
 * Primitives for implementing disposable memo functions
 * Byron Cook -- byron@cse.ogi.edu
 *
 * IOEql :: Eval a => a -> a -> IO Bool 
 *   if argument is an Int or Char
 *   then use ==
 *   else use pointer identity
 *
 * IOHash :: Eval a => a -> IO Int
 *   if a is an Int or Char
 *   then use value cast as an Int
 *   else use pointer identity
 *
 * (Earlier versions made Float a special case too - but that's not very
 *  portable since it assumes that sizeof(FloatPro) == sizeof(Int).)
 * ------------------------------------------------------------------------*/

/* :: Eval a => a -> a -> ST Mem Bool */
primFun(primIOEql) {
    Cell x = IOArg(1);
    Cell y = IOArg(2);
    eval(x);
    eval(y);
    x = followInd(IOArg(1));
    y = followInd(IOArg(2));

    if (whatIs(x) == whatIs(y)) {
        switch (whatIs(x)) {
        case INTCELL:
            IOBoolResult(intOf(x) == intOf(y));
        case CHARCELL:
            IOBoolResult(charOf(x) == charOf(y));
        /* deliberate fall through to end */
        default:
            break;
        }
    }
    IOBoolResult(x == y);
}

/* :: Eval a => a -> ST Mem Int */
primFun(primIOHash) {
    Cell x = IOArg(1);
    eval(x);
    x = followInd(IOArg(1));

    switch (whatIs(x)) {
    case INTCELL:
        IOReturn(x);
    case CHARCELL:
        IOReturn(mkInt(charOf(x)));
    default:
        break;
    }
    IOReturn(mkInt((int)x));
}

/*-------------------------------------------------------------------------*/
