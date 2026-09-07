# Embed the interpreter in a C program

Drive Cuddles from C: load a file, look up a Haskell function, apply it and
run the result.  This is how `runcuddles` itself is written.

## Prerequisites

- a checkout of the Cuddles sources with a working build
- some C

## Steps

### 1. Write the program against the server API

Include `CuddlesAPI.h` from `src/` and call `initCuddlesServer`.  It returns
a table of function pointers; everything else goes through that table.
This is a minimal `runcuddles`, without error handling:

```c
#include "CuddlesAPI.h"

static char *cuddles_argv[] = { "runcuddles", "+l" };
static int   cuddles_argc   = sizeof cuddles_argv / sizeof cuddles_argv[0];

int main(int argc, char **argv)
{
    CuddlesServerAPI *cuddles = initCuddlesServer(cuddles_argc, cuddles_argv);
    cuddles->setOutputEnable(0);
    argc--; argv++;
    cuddles->setCuddlesArgs(argc, argv);
    cuddles->loadFile(argv[0]);
    cuddles->lookupName("Main", "main");
    return cuddles->doIO();
}
```

Line by line: the `cuddles_argv` array is the option string the
interpreter starts with, without any file names.  `setOutputEnable(0)`
silences the compiler's own output.  `setCuddlesArgs` sets what
`getProgName` and `getArgs` return.  `loadFile` loads and compiles a
file, `lookupName` pushes `Main.main` on the server's value stack, and
`doIO` runs the action on top of the stack and returns its exit code.

### 2. Check for errors after every call

Every function in the table does nothing while the error flag is set,
so a program that never checks cannot corrupt the server, but it also
cannot tell what went wrong.  `clearError` returns the pending error
message, or `NULL` if there is none, and clears the flag:

```c
static void check(CuddlesServerAPI *cuddles)
{
    char *err = cuddles->clearError();
    if (err) {
        fprintf(stderr, "Cuddles error:\n%s\n", err);
        exit(1);
    }
}
```

Call it after `initCuddlesServer` (which returns `NULL` on failure),
after each `loadFile`, and after `doIO`.  When output is disabled the
message includes all the compiler output collected since the last
`clearError`.

### 3. Build it with the interpreter's objects

The interpreter is not built as a library.  Add your program to
`src/meson.build` next to `runcuddles`, which links the same objects:

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
cuddles->lookupName("Foo", "ackermann");
cuddles->mkInt(4);
cuddles->apply();
cuddles->mkInt(2);
cuddles->apply();
int r = cuddles->evalInt();
```

Every application is type-checked using the `Typeable` class, so the
functions you look up must be monomorphic.  To use a polymorphic
function at a particular type, compile an expression with a signature:

```c
HVal len = cuddles->compileExpr("Prelude", "length :: [Int] -> Int");
cuddles->pushHVal(len);
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
cuddles->loadFile("MyLibraries");
int base = cuddles->getNumScripts();
for (int i = 1; i < argc; i++) {
    cuddles->reset(base);
    cuddles->loadFile(argv[i]);
    cuddles->lookupName("Main", "main");
    cuddles->doIO();
}
```

## See also

- [The server API](../reference/server-api.md) for every function in
  the table
- `src/runcuddles.c` for the complete, error-checked version of step 1
