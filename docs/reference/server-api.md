# The server API

The C interface through which a host program drives the interpreter.
`runcuddles` and `fficuddles` are written against it.

## Synopsis

```c
#include "CuddlesAPI.h"

CuddlesServerAPI *initCuddlesServer(int argc, char *argv[]);
void           shutdownCuddlesServer(CuddlesServerAPI *cuddles);
```

## Description

`initCuddlesServer` starts the interpreter with the options in `argv`
(`argv[0]` is the program name; file names are not allowed), loads the
`Prelude` and the `Cuddles.Dynamic` module, and returns a table of
function pointers.  It returns `NULL` if initialisation fails.  Every
other operation is a member of the table.

The server keeps a **value stack**.  Operations push Haskell values,
combine the top entries, or evaluate the top entry.  Values on the
stack are dynamically typed: every `apply` is checked with the
`Typeable` class, and a value whose type has no `Typeable` instance,
or a polymorphic value, is an error.

The server keeps an **error flag**.  When it is set, every operation
returns at once without doing anything.  `clearError` reads and clears
it.

## Functions

### Errors and output

| Function | Effect |
|---|---|
| `char *clearError(void)` | Return the pending error text, or `NULL` if there is none, and clear the flag.  When output is disabled the text includes the compiler output collected since the last call |
| `void setOutputEnable(unsigned on)` | With `0`, collect the interpreter's own output (progress, error messages) into a buffer instead of printing it.  Output from Haskell code is unaffected |

### Loading

| Function | Effect |
|---|---|
| `void loadFile(char *name)` | Load and compile a file or module, as `:also` would |
| `void loadFromBuffer(char *text)` | Load a module from a string |
| `int getNumScripts(void)` | The number of files currently loaded |
| `void reset(int n)` | Unload files until `n` remain.  Invalidates every `HVal` |
| `void changeDir(char *dir)` | Change the working directory |
| `void setOptions(char *opts)` | Process an option string, as `:set` would |
| `char *getOptions(void)` | The current options as a string |
| `void garbageCollect(void)` | Force a collection |
| `void setCuddlesArgs(int argc, char **argv)` | What `getProgName` and `getArgs` return; `argv[0]` is the program name |
| `void loadProject(char *)` | Obsolete; does nothing useful |

### Stack

| Function | Effect |
|---|---|
| `void lookupName(char *module, char *name)` | Push the value of `module.name` |
| `void mkInt(int)` | Push an `Int` |
| `void mkAddr(void *)` | Push an `Addr` |
| `void mkString(char *)` | Push a `String` |
| `void apply(void)` | Pop an argument, then a function, and push the application |
| `int evalInt(void)` | Pop and evaluate an `Int` |
| `void *evalAddr(void)` | Pop and evaluate an `Addr` |
| `char *evalString(void)` | Pop and evaluate a `String`; the result is a fresh C string |
| `int doIO(void)` | Pop and run an `IO ()`; return the exit code |
| `int doIO_Int(int *result)` | Pop and run an `IO Int`, storing the result |
| `int doIO_Addr(void **result)` | Pop and run an `IO Addr`, storing the result |

### Values

| Function | Effect |
|---|---|
| `HVal compileExpr(char *module, char *expr)` | Compile `expr` in the scope of `module` and return a handle.  A type annotation in `expr` fixes the type of a polymorphic name |
| `HVal popHVal(void)` | Pop the top of the stack into a handle |
| `void pushHVal(HVal)` | Push the value a handle refers to |
| `void freeHVal(HVal)` | Release a handle |

`HVal` is an `int`, a stable pointer into the heap.  Handles survive
further loads and evaluations but not `reset`.  Using a handle after
`reset` is not detected.

## Building against it

There is no installed library.  A host program is built inside the
tree, linking `server.c` and the interpreter objects as `runcuddles`
does; see [Embed the interpreter in a C program](../how-to/embed-cuddles.md).

## See also

- `src/CuddlesAPI.h` for the declarations
- `src/runcuddles.c` for a complete host program
