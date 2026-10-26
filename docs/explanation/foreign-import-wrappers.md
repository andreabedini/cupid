# Foreign import wrappers

A `foreign import "wrapper"` declaration turns a Haskell function into
a C function pointer, so that C code can call back into Haskell.  The
2006 release implemented it with a small piece of machine code
generated at run time, for three architectures none of which this
tree targets; that code is no longer in the tree.  This page explains
how the mechanism works, why it needs architecture-specific code, and
what happens when a program uses it today.

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

where `app` is the entry point that `fficupid` generated for the
declaration.  `app` is ordinary C compiled ahead of time; only the
step of adding `s` to the argument list must be created per closure.

## The solution the 2006 release used

`mkThunk` in `src/builtin.c` (it is now in `src/prims/builtin.c`)
allocated a small structure in memory that was both writable and
executable:

```c
struct thunk_data {
    struct thunk_data *next, *prev;   /* all thunks, for the collector */
    CupidStablePtr      stable;        /* the Haskell function */
    char               code[N];       /* machine code */
};
```

It filled `code` with a few instructions that insert `s` in front of the
incoming arguments and jump to `app`.  On 32-bit x86, where arguments
are on the stack, that is: push `s`, push the address of a small stub
that takes it off again, and jump to `app`, which took the original
return address as an extra unused argument.  The code is the same
whatever the function's type, because the C calling convention does not
depend on the argument types for the purpose of shifting them along.
The structure's address, offset to `code`, would be the function pointer
returned to Haskell.

The instruction sequence is different for every architecture and
calling convention, and hand-written as bytes.  The 2006 release
carried sequences for 32-bit x86 (stack arguments), PowerPC on macOS
(register arguments, up to seven) and SPARC - the only architectures
the 2006 tree ran on where `foreign import "wrapper"`
worked at all.  On x86-64 and ARM64 the first several arguments arrive
in registers, and the fix-up would have to shift them along a register
at a time, spill the last one to the stack, and handle floating-point
arguments, which travel in a separate register file.  It is the same
problem GHC's `Adjustor.c` solves for those targets, with considerable
care.

## What happens today

None of i386, PowerPC or SPARC is a supported target,
so that machine code was dead on every target this tree builds for,
and has been deleted along with the executable-memory allocator
(`mallocBytesRWX`) it depended on.  `mkThunk` now raises "Foreign
import wrapper is not supported on this architecture" as soon as it is
called - before allocating or linking anything into the list of live
thunks, unlike the 2006 code, which allocated and linked a thunk_data
first and only then discovered (via the same architecture check) that
it had no code to put in it, leaking a half-initialised thunk into the
list on every call.  `struct thunk_data`'s `code` field is now a
one-byte placeholder; `freeHaskellFunPtr` and the thunk list otherwise
work exactly as before, they just never have anything to free, since
no `wrapper` import ever produces a live thunk to begin with.  No
shipped library uses one.

## What works instead

- `foreign import "dynamic"` calls *out* through a `FunPtr` obtained
  from C, and works everywhere.
- A C library that takes a callback can often be driven with a
  C-side trampoline compiled by `fficupid` along with the module: a
  fixed C function that looks up what to do in a table the Haskell side
  fills through ordinary imports.

If wrappers are wanted on x86-64 or ARM64, `mkThunk` is the one
function to write, with `app`'s expectation of `s` as the first
argument as the contract; the alternative of passing `s` through a
global variable, with `app` adjusted to read it, avoids the register
shuffle at the cost of reentrancy.

## See also

- [The language Cupid
  accepts](../reference/language.md#foreign-function-interface)
- [Call C functions from Haskell](../how-to/call-c-functions.md)
