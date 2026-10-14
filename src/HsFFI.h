#ifndef __HSFFI_H__
#define __HSFFI_H__

typedef unsigned char cupid_uint8_t;
typedef unsigned short cupid_uint16_t;
typedef unsigned int cupid_uint32_t;
typedef signed char cupid_int8_t;
typedef signed short cupid_int16_t;
typedef signed int cupid_int32_t;
#ifdef _MSC_VER
typedef unsigned __int64 cupid_uint64_t;
typedef __int64 cupid_int64_t;
#else
typedef unsigned long long cupid_uint64_t;
typedef signed long long cupid_int64_t;
#endif

/* 
 * The test on __PRELUDE_H__ (the include guard of prelude.h) is a crude
 * way of testing whether this file is #included into Cupid.  Use it to
 * eliminate non-portable stuff.
 */

#ifdef __PRELUDE_H__ /* #included into Cupid */

typedef int HsInt;
typedef int8_t HsInt8;
typedef int16_t HsInt16;
typedef int HsInt32;
typedef unsigned int HsWord;
typedef unsigned char HsWord8;
typedef unsigned short HsWord16;
typedef unsigned int HsWord32;

#else /* #included into user-provided C code */

typedef int HsInt;
typedef cupid_int8_t HsInt8;
typedef cupid_int16_t HsInt16;
typedef cupid_int32_t HsInt32;
typedef unsigned int HsWord;
typedef cupid_uint8_t HsWord8;
typedef cupid_uint16_t HsWord16;
typedef cupid_uint32_t HsWord32;

#endif

/* 
 * Here we deviate from the FFI specification:
 * If we make them both float, then there's no way to pass a double
 * to C which means we can't call common C functions like sin.
 */
typedef float HsFloat;
typedef double HsDouble;

typedef cupid_int64_t HsInt64;
typedef cupid_uint64_t HsWord64;
typedef int HsChar;
typedef int HsBool;
typedef void *HsAddr;
typedef void *HsPtr;
typedef void (*HsFunPtr)(void);
typedef void *HsForeignPtr;
typedef void *HsStablePtr;

#define HS_CHAR_MIN 0
#define HS_CHAR_MAX 0x10FFFF
#define HS_BOOL_FALSE 0
#define HS_BOOL_TRUE 1
#define HS_BOOL_MIN HS_BOOL_FALSE
#define HS_BOOL_MAX HS_BOOL_TRUE
#define HS_INT_MIN __INT32_MIN
#define HS_INT_MAX __INT32_MAX
#define HS_INT8_MIN __INT8_MIN
#define HS_INT8_MAX __INT8_MAX
#define HS_INT16_MIN __INT16_MIN
#define HS_INT16_MAX __INT16_MAX
#define HS_INT32_MIN __INT32_MIN
#define HS_INT32_MAX __INT32_MAX
#define HS_INT64_MIN __INT64_MIN
#define HS_INT64_MAX __INT64_MAX
#define HS_WORD8_MAX __UINT8_MAX
#define HS_WORD16_MAX __UINT16_MAX
#define HS_WORD32_MAX __UINT32_MAX
#define HS_WORD64_MAX __UINT64_MAX

#ifndef __PRELUDE_H__

# include <float.h>

# define HS_FLOAT_RADIX FLT_RADIX
# define HS_FLOAT_ROUNDS FLT_ROUNDS
# define HS_FLOAT_EPSILON FLT_EPSILON
# define HS_FLOAT_DIG FLT_DIG
# define HS_FLOAT_MANT_DIG FLT_MANT_DIG
# define HS_FLOAT_MIN FLT_MIN
# define HS_FLOAT_MIN_EXP FLT_MIN_EXP
# define HS_FLOAT_MIN_10_EXP FLT_MIN_10_EXP
# define HS_FLOAT_MAX FLT_MAX
# define HS_FLOAT_MAX_EXP FLT_MAX_EXP
# define HS_FLOAT_MAX_10_EXP FLT_MAX_10_EXP

# define HS_DOUBLE_RADIX DBL_RADIX
# define HS_DOUBLE_ROUNDS DBL_ROUNDS
# define HS_DOUBLE_EPSILON DBL_EPSILON
# define HS_DOUBLE_DIG DBL_DIG
# define HS_DOUBLE_MANT_DIG DBL_MANT_DIG
# define HS_DOUBLE_MIN DBL_MIN
# define HS_DOUBLE_MIN_EXP DBL_MIN_EXP
# define HS_DOUBLE_MIN_10_EXP DBL_MIN_10_EXP
# define HS_DOUBLE_MAX DBL_MAX
# define HS_DOUBLE_MAX_EXP DBL_MAX_EXP
# define HS_DOUBLE_MAX_10_EXP DBL_MAX_10_EXP

#endif /* included into user code */

typedef int CupidStackPtr;
typedef void *CupidForeign;
typedef int CupidStablePtr;

typedef void (*CupidPrim)(CupidStackPtr); /* primitive function          */

#ifndef __PRELUDE_H__
/* The hooks a module offers the interpreter's lifecycle; a null hook
 * means the module has nothing to do in that phase.  Mirrors Subsystem
 * in connect.h, which is what this becomes when this header is
 * #included into Cupid itself. */
typedef struct {
    const char *name;
    void (*install)(void);
    void (*reset)(void);
    void (*gcMark)(void);
    void (*atExit)(void);
    void (*onBreak)(void);
} cupid_subsystem;

struct cupid_primitive { /* table of primitives                   */
    char *ref;           /* primitive reference string    */
    int arity;           /* primitive function arity      */
    CupidPrim imp;       /* primitive implementation      */
};

struct cupid_primInfo {
    const cupid_subsystem *hooks;
    struct cupid_primitive *primFuns;
    struct cupid_primInfo *nextPrimInfo;
};
#else
# define cupid_subsystem Subsystem
# define cupid_primInfo primInfo
#endif

/* API Version number.  6 because struct cupid_primInfo's first field
 * changed meaning: it used to be a control function taking an Int code,
 * and is now a pointer to a cupid_subsystem record of five hooks.  A
 * stub built against version 5's layout must not be loaded as if it
 * matched this one -- the field is the same size (a pointer) either
 * way, so nothing but the version number would catch the mismatch. */
#define CUPID_API_VERSION 6

typedef struct {

    /* evaluate next argument */
    HsBool (*getBool)(void);
    HsInt (*getInt)(void);
    HsWord (*getWord)(void);
    HsAddr (*getAddr)(void);
    HsFloat (*getFloat)(void);
    HsDouble (*getDouble)(void);
    HsChar (*getChar)(void);
    CupidForeign (*getForeign)(void);
    HsStablePtr (*getStablePtr4)(void);
    HsInt8 (*getInt8)(void);
    HsInt16 (*getInt16)(void);
    HsInt32 (*getInt32)(void);
    HsInt64 (*getInt64)(void);
    HsWord8 (*getWord8)(void);
    HsWord16 (*getWord16)(void);
    HsWord32 (*getWord32)(void);
    HsWord64 (*getWord64)(void);
    HsPtr (*getPtr)(void);
    HsFunPtr (*getFunPtr)(void);
    HsForeignPtr (*getForeignPtr)(void);

    /* push result   */
    void (*putBool)(HsBool);
    void (*putInt)(HsInt);
    void (*putWord)(HsWord);
    void (*putAddr)(HsAddr);
    void (*putFloat)(HsFloat);
    void (*putDouble)(HsDouble);
    void (*putChar)(HsChar);
    void (*putForeign)(CupidForeign, void (*)(CupidForeign));
    void (*putInt8)(HsInt8);
    void (*putInt16)(HsInt16);
    void (*putInt32)(HsInt32);
    void (*putInt64)(HsInt64);
    void (*putWord8)(HsWord8);
    void (*putWord16)(HsWord16);
    void (*putWord32)(HsWord32);
    void (*putWord64)(HsWord64);
    void (*putPtr)(HsPtr);
    void (*putFunPtr)(HsFunPtr);
    void (*putForeignPtr)(HsForeignPtr);
    void (*putStablePtr4)(HsStablePtr);

    /* return in IO monad or Id monad */
    void (*returnIO)(CupidStackPtr, int);
    int (*runIO)(int);
    void (*returnId)(CupidStackPtr, int);
    int (*runId)(int);

    /* register the prim table */
    void (*registerPrims)(struct cupid_primInfo *);

    CupidStablePtr (*lookupName)(char *, char *);
    void (*ap)(int);
    void (*getUnit)(void);
    void *(*mkThunk)(HsFunPtr, CupidStablePtr);
    void (*freeThunk)(void *);

    CupidStablePtr (*makeStablePtr4)(void);
    void (*derefStablePtr4)(CupidStablePtr);
    void (*freeStablePtr4)(HsStablePtr);
} CupidAPI5;

/* Note: the change in going from version 4 to 5 is that
   with 5 (and later), the DLLs specify the CupidAPI version
   assumed by the DLL primitives. The CupidAPI method table
   is _not_ identical to 4's; it has been re-orged and
   tidied up.
*/

extern CupidAPI5 *cupidAPI5(void);
typedef void (*InitModuleFun5)(CupidAPI5 *);
typedef int (*APIVersionFun)(void);

/* To ensure backward compatibility, CupidAPI4 is also supported:
 * (due to the CupidAPI4 vtbl being an extension of CupidAPI3's and
 *  CupidAPI2's, support for these two comes for free.)
 */
typedef struct {

    /* evaluate next argument */
    HsInt (*getInt)(void);
    HsWord (*getWord)(void);
    HsAddr (*getAddr)(void);
    HsFloat (*getFloat)(void);
    HsDouble (*getDouble)(void);
    char (*getChar)(void);
    CupidForeign (*getForeign)(void);
    CupidStablePtr (*getStablePtr)(void); /* deprecated */

    /* push part of result   */
    void (*putInt)(HsInt);
    void (*putWord)(HsWord);
    void (*putAddr)(HsAddr);
    void (*putFloat)(HsFloat);
    void (*putDouble)(HsDouble);
    void (*putChar)(char);
    void (*putForeign)(CupidForeign, void (*)(CupidForeign));
    void (*putStablePtr)(CupidStablePtr); /* deprecated */

    /* return n values in IO monad or Id monad */
    void (*returnIO)(CupidStackPtr, int);
    void (*returnId)(CupidStackPtr, int);
    int (*runIO)(int);

    /* free a stable pointer */
    void (*freeStablePtr)(CupidStablePtr); /* deprecated */

    /* register the prim table */
    void (*registerPrims)(struct cupid_primInfo *);

    /* garbage collect */
    void (*garbageCollect)(void);

    /* API3 additions follow */
    CupidStablePtr (*lookupName)(char *, char *);
    void (*ap)(int);
    void (*getUnit)(void);
    void *(*mkThunk)(HsFunPtr, CupidStablePtr);
    void (*freeThunk)(void *);
    HsBool (*getBool)(void);
    void (*putBool)(HsBool);

    /* API4 additions follow */
    HsInt8 (*getInt8)(void);
    HsInt16 (*getInt16)(void);
    HsInt32 (*getInt32)(void);
    HsInt64 (*getInt64)(void);
    HsWord8 (*getWord8)(void);
    HsWord16 (*getWord16)(void);
    HsWord32 (*getWord32)(void);
    HsWord64 (*getWord64)(void);
    HsPtr (*getPtr)(void);
    HsFunPtr (*getFunPtr)(void);
    HsForeignPtr (*getForeignPtr)(void);

    void (*putInt8)(HsInt8);
    void (*putInt16)(HsInt16);
    void (*putInt32)(HsInt32);
    void (*putInt64)(HsInt64);
    void (*putWord8)(HsWord8);
    void (*putWord16)(HsWord16);
    void (*putWord32)(HsWord32);
    void (*putWord64)(HsWord64);
    void (*putPtr)(HsPtr);
    void (*putFunPtr)(HsFunPtr);
    void (*putForeignPtr)(HsForeignPtr);

    CupidStablePtr (*makeStablePtr4)(void);
    void (*derefStablePtr4)(CupidStablePtr);

    void (*putStablePtr4)(HsStablePtr);
    HsStablePtr (*getStablePtr4)(void);
    void (*freeStablePtr4)(HsStablePtr);

    int (*runId)(int);
} CupidAPI4;

extern CupidAPI4 *cupidAPI4(void);
typedef void (*InitModuleFun4)(CupidAPI4 *);

extern void hs_perform_gc(void);
extern void hs_free_stable_ptr(HsStablePtr sp);
extern void hs_free_fun_ptr(HsFunPtr fp);

/* Copied verbatim from prelude.h */

#if defined(_WIN32) /* Microsoft Windows */
# define DLLIMPORT(rty) __declspec(dllimport) rty
# define DLLEXPORT(rty) __declspec(dllexport) rty
#else
# define DLLIMPORT(rty) rty
# define DLLEXPORT(rty) rty
#endif /* Don't need to declare DLL exports */

#endif /* __HSFFI_H__ */
