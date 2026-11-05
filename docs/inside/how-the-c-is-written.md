# How the C is written

The C is meant to be read, so it is written in a small number of
consistent ways.  None of what follows is unusual taken alone.  The point
is that once you have learned it for one file it holds for all of them.

**Standard C23, using little of what is new in it.**  The GNU
extensions are computed-goto dispatch in `machine.c`, with a `switch`
fallback for compilers that don't have it, and a few function attributes
in `prelude.h`, each with a fallback for other compilers.  The sources
rely on `bool` as a keyword and on `[[maybe_unused]]`; `nullptr`,
`typeof` and `constexpr` are not used.

**One file is one translation unit.**  No `.c` file includes another.
A fragment included into several files shares their static state and
macros, and can't be compiled, checked or read on its own.  Where a
fragment used to be a macro template, it is now a function that takes
callbacks.

**Every header compiles on its own.**  A header includes what it uses,
so it can be the first line of a translation unit.  In practice most
need only `prelude.h` and `storage.h`.  The include block
of a `.c` file lists what the file calls, and its order doesn't matter.

**A function unless C cannot express it.**  A macro evaluates its
arguments more than once, can hide an assignment, and can't be stepped
into with a debugger.  So anything that can be a function is one,
marked `CUPID_inline` (`static inline` and always inlined, so it costs
nothing even in the files built at `-O0`).  The macros that remain are
those C has no other way to write: lvalue projections such as `fst`
and `hd`, the `map` family, the primitive templates, and anything that
needs `sizeof` of the caller's array.

**Errors are function calls.**  A message is built with `errHead`,
`errPrintf` and the `errExpr`/`errType`/... helpers, and raised with
`errFail`.  Because these are ordinary calls, editors and analysers
understand them, and the compiler checks every format string.

**Plain C spelling.**  Standard types are used (`bool`, `int8_t` and
so on).  `Char` and the domain types (`Cell`, `Text`, `Name`, `Type`
and so on) are the aliases kept, because they carry meaning.  Output
goes through `snprintf`, never `sprintf`.  Each function's description
is a block comment above its definition.

**Switches over cell tags say what they mean.**  Cell tags are a
`typedef enum CellTag`.  Every `switch (whatIs(c))` has a `default`.
`default : break;` says on purpose that the other tags are ignored,
and `default : internal(...)` that they cannot occur; the rest handle
the remaining tags there.

**The state a function uses is in its signature, where that has been
done.**  The code generator's state is a `CodeGen` record that its entry
points create and pass on, and the type checker returns a `Typing` record,
so those signatures say what the functions read and write.  The rest of
`type.c`'s working state is still file-scope, and so is much of the front
end's.

**Formatting is clang-format's.**  `.clang-format` formats the sources
in `src/` and `src/prims/`, and generated files are left alone.  It
never splits a string literal, so reformatting can't break a search for
a message; some long messages are split by hand.  New code is formatted
before it is committed.

**Static analysis is a short list, reviewed by hand.**  `.clang-tidy`
enables only checks whose findings are worth a person's time here; it
says why each broader group was left out.  Findings are read and fixed
by hand, never auto-fixed.  A finding is checked against a build with
every feature option on before anyone acts on it, because one
configuration is not the whole tree.  An `#include` that only a
feature option needs carries an `IWYU pragma: keep` comment naming
that option.  See [Run the static analysis](../contributing/run-clang-tidy.md).
