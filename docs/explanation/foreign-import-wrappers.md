# Foreign import wrappers

A `foreign import "wrapper"` declaration turns a Haskell function into
a C function pointer, so that C code can call back into Haskell.  Hugs
implements it with a small piece of machine code generated at run
time, which exists for three architectures none of which this tree
targets.  This page explains how it works and why it is not simply
ported.

## The problem

A `wrapper` import has a type like

```haskell
foreign import ccall "wrapper" mkCallback :: (Int -> IO ()) -> IO (FunPtr (Int -> IO ()))
```

Each call to `mkCallback f` must produce a distinct C function pointer
that, when called from C with an `int`, runs `f` on it.  C has no way
to make a new function at run time, and invoking the C compiler and
linker for each one is not attractive.

What is needed, for a Haskell function `f` held behind a stable pointer
`s`, is the effect of this C function:

```c
rty thunk(ty1 a1, ..., tym am) {
    return app(s, a1, ..., am);
}
```

where `app` is the entry point that `ffihugs` generated for the
declaration.  `app` is ordinary C compiled ahead of time; only the
step of adding `s` to the argument list must be created per closure.

## The solution

`mkThunk` in `src/builtin.c` allocates a small structure in memory
that is both writable and executable:

```c
struct thunk_data {
    struct thunk_data *next, *prev;   /* all thunks, for the collector */
    HugsStablePtr      stable;        /* the Haskell function */
    char               code[N];       /* machine code */
};
```

It fills `code` with a few instructions that insert `s` in front of
the incoming arguments and jump to `app`.  On 32-bit x86, where
arguments are on the stack, that is: push the return address up one
slot, store `s` where it was, jump.  The code is the same whatever the
function's type, because the C calling convention does not depend on
the argument types for the purpose of shifting them along.  The
structure's address, offset to `code`, is the function pointer
returned to Haskell.

The list of thunks lets the garbage collector find every live
callback and keeps their stable pointers alive; `freeHaskellFunPtr`
unlinks and frees one.

## Why it exists for i386, PowerPC and SPARC only

The instruction sequence is different for every architecture and
calling convention, and hand-written as bytes.  The 2006 release
carried sequences for 32-bit x86 (stack arguments), PowerPC on macOS
(register arguments, up to seven) and SPARC.  On x86-64 and ARM64 the
first several arguments arrive in registers, and the fix-up would have
to shift them along a register at a time, spill the last one to the
stack, and handle floating-point arguments, which travel in a separate
register file.  It is the same problem GHC's `Adjustor.c` solves for
those targets, with considerable care.

In this tree `mkThunk` has no code for the supported targets.  A
program that evaluates a `wrapper` import gets a run-time error rather
than a crash, and no shipped library uses one.

## What works instead

- `foreign import "dynamic"` calls *out* through a `FunPtr` obtained
  from C, and works everywhere.
- A C library that takes a callback can often be driven with a
  C-side trampoline compiled by `ffihugs` along with the module: a
  fixed C function that looks up what to do in a table the Haskell side
  fills through ordinary imports.

If wrappers are wanted on x86-64 or ARM64, `mkThunk` is the one
function to write, with `app`'s expectation of `s` as the first
argument as the contract; the alternative of passing `s` through a
global variable, with `app` adjusted to read it, avoids the register
shuffle at the cost of reentrancy.

## See also

- [The language Hugs accepts](../reference/language.md#foreign-function-interface)
- [Call C functions from Haskell](../how-to/call-c-functions.md)
