# Embed the interpreter in a C program

Drive Hugs from C: load a file, look up a Haskell function, apply it and
run the result.  This is how `runhugs` itself is written.

## Prerequisites

- a checkout of the Hugs sources with a working build
- some C

## Steps

### 1. Write the program against the server API

Include `HugsAPI.h` from `src/` and call `initHugsServer`.  It returns
a table of function pointers; everything else goes through that table.
This is a minimal `runhugs`, without error handling:

```c
#include "HugsAPI.h"

static char *hugs_argv[] = { "runhugs", "+l" };
static int   hugs_argc   = sizeof hugs_argv / sizeof hugs_argv[0];

int main(int argc, char **argv)
{
    HugsServerAPI *hugs = initHugsServer(hugs_argc, hugs_argv);
    hugs->setOutputEnable(0);
    argc--; argv++;
    hugs->setHugsArgs(argc, argv);
    hugs->loadFile(argv[0]);
    hugs->lookupName("Main", "main");
    return hugs->doIO();
}
```

Line by line: the `hugs_argv` array is the option string the
interpreter starts with, without any file names.  `setOutputEnable(0)`
silences the compiler's own output.  `setHugsArgs` sets what
`getProgName` and `getArgs` return.  `loadFile` loads and compiles a
file, `lookupName` pushes `Main.main` on the server's value stack, and
`doIO` runs the action on top of the stack and returns its exit code.

### 2. Check for errors after every call

Every function in the table does nothing while the error flag is set,
so a program that never checks cannot corrupt the server, but it also
cannot tell what went wrong.  `clearError` returns the pending error
message, or `NULL` if there is none, and clears the flag:

```c
static void check(HugsServerAPI *hugs)
{
    char *err = hugs->clearError();
    if (err) {
        fprintf(stderr, "Hugs error:\n%s\n", err);
        exit(1);
    }
}
```

Call it after `initHugsServer` (which returns `NULL` on failure),
after each `loadFile`, and after `doIO`.  When output is disabled the
message includes all the compiler output collected since the last
`clearError`.

### 3. Build it with the interpreter's objects

The interpreter is not built as a library.  Add your program to
`src/meson.build` next to `runhugs`, which links the same objects:

```meson
myhost = executable('myhost', files('myhost.c', 'server.c'),
  objects: core_objs, include_directories: src_inc, dependencies: deps)
```

`server.c` implements the API table; `core_objs` is the rest of the
interpreter.

## Applying functions and reading results

`lookupName`, `mkInt` and `mkString` push values; `apply` pops an
argument and a function and pushes the application; `evalInt` and
`evalString` evaluate the top of the stack.  To compute
`Foo.ackermann 4 2`:

```c
hugs->lookupName("Foo", "ackermann");
hugs->mkInt(4);
hugs->apply();
hugs->mkInt(2);
hugs->apply();
int r = hugs->evalInt();
```

Every application is type-checked using the `Typeable` class, so the
functions you look up must be monomorphic.  To use a polymorphic
function at a particular type, compile an expression with a signature:

```c
HVal len = hugs->compileExpr("Prelude", "length :: [Int] -> Int");
hugs->pushHVal(len);
```

`compileExpr` takes the module whose scope to use and the expression,
and returns a handle.  Handles from `popHVal` and `compileExpr` stay
valid across further loads and evaluations and are released with
`freeHVal`.  `reset(n)` unloads files down to the `n`th, as reported by
`getNumScripts`, and invalidates every handle.

## Loading several programs in one session

Load the shared modules once, remember the stack depth, and reset to it
before each program:

```c
hugs->loadFile("MyLibraries");
int base = hugs->getNumScripts();
for (int i = 1; i < argc; i++) {
    hugs->reset(base);
    hugs->loadFile(argv[i]);
    hugs->lookupName("Main", "main");
    hugs->doIO();
}
```

## See also

- [The server API](../reference/server-api.md) for every function in
  the table
- `src/runhugs.c` for the complete, error-checked version of step 1
