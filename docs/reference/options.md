# Options

The options that change what the interpreter does, and where it reads
them from.

## Where options come from

Options start at their defaults and are then read, in this order, from:

1. the `CUPIDFLAGS` environment variable, if set
2. the command line

Later settings override earlier ones.  Most options can also be
changed while the interpreter is running with the `:set` command.

In `CUPIDFLAGS`, options are separated by whitespace.  To include a
space in an option, precede it with a backslash or wrap the option in
double quotes; inside double quotes `\ `, `\"` and `\\` are escapes
and other backslashes are literal.

## Two kinds of option

**Toggles** are a letter preceded by `+` to turn the setting on or
`-` to turn it off.  Several may be combined in one word: `+tk`
turns `t` and `k` on, `-sg` turns `s` and `g` off.

**Parameters** are a letter followed by a value, such as `-h4M` or
`-P.:lib`.  Whether the leading sign is `+` or `-` makes no
difference.

`:set` with no argument shows both kinds with their current values.

### `-X`*str*

Re-read *str* as an option string containing several options.  This
exists for `#!` scripts, where the kernel passes everything after the
program name as one argument.  See
[Write a script that runs with runcupid](../how-to/write-a-script.md).

## Language

The extensions beyond Haskell 2010 listed in
[Language extensions](extensions.md) are always enabled, except
overlapping instances, which need `+o` (or `+O`).  The `+98`
and `-98` options of earlier releases, which switched them off and
on, are accepted and ignored with a message.

### `-c`*num*

The constraint cutoff in the type checker.  Default: 40.  Range: 1 to
1024.  With flexible contexts and instances, constraint checking need
not terminate; Cupid gives up when the depth reaches this limit.
Hitting it usually means the type checker would not have terminated.

### `+o`

Allow overlapping instances where one instance type is a substitution
instance of the other.  Default: off.  See
[Overlapping instances](extensions.md#overlapping-instances).  While
this or `+O` is on, inferred contexts are not reduced through
instances, so `:type` may show a constraint such as `Show [a]` where
it would otherwise show `Show a`.

### `+O`

As `+o`, and also accept ambiguous uses of overlapping instances,
choosing the more general instance even where that may be wrong.
Default: off.

### `+x`

Explain instance resolution as it happens: print each entailment the
type checker tries and each instance it uses.  Default: off.  The
`:xplain` command shows the same for one context.

## Loading

### `+.`

Print a dot for each module as it loads.  Default: off.

### `+q`

Print nothing while loading.  Default: on.

### `+w`

After each load, list the files loaded.  Default: off.

### `-F`*cmd*

Run each source file through *cmd* before reading it: Cupid runs
*cmd* with the file name appended and reads its standard output.  An
empty *cmd* turns the filter off.  Default: unset.  *cmd* is split into
words on whitespace and run directly, without a shell, so shell syntax
in it is not interpreted and the file name needs no quoting.  See
[Preprocess source files before loading](../how-to/preprocess-sources.md).

### `-P`*str*

The search path for modules: a colon-separated list of directories
(semicolon-separated on Windows).

- An empty entry stands for the previous search path, so `-P:dir`
  appends and `-Pdir:` prepends.
- An empty *str* restores the default.
- `{Cupid}` is replaced by the library directory and `{Home}` by the
  home directory.
- An entry of the form `dir/*` means every immediate subdirectory of
  `dir`.
- Duplicate entries are dropped.

The default on Unix is

```
.:{Home}/lib/cupid/packages/*:/usr/local/lib/cupid/packages/*:{Cupid}/packages/*
```

A path from which the `Prelude` cannot be found is rejected with a
message and the old path kept.

### `-S`*str*

The suffixes tried when looking for a module's file, as a
colon-separated list (semicolons on Windows).  Default: `.hs`.
An empty entry stands for the previous list and an empty *str* restores
the default, as for `-P`.  The list must keep `.hs`, or the `Prelude`
could not be found.

## Editor

### `-E`*str*

The command that `:edit` and `:find` run.  Default: the `EDITOR`
environment variable.  *str* is a template:

- `%s` is replaced by the file name.
- `%d` is replaced by the line number of the last error.
- `%f` is replaced by the absolute file name.
- `%%` is a literal `%`.

If neither `%s` nor `%f` appears, the file name is appended.

The template is split into words on whitespace and the editor is run
directly, without a shell, so nothing needs quoting: a file name that
contains a space, a quote or a `$` is one argument whatever it holds.
By the same token the template's own words are separated on whitespace
and on nothing else - shell syntax in it is not interpreted.  A leading
`&` starts the editor without waiting for it.

```
-E"vi +%d %s"
```

## Evaluation and printing

### `-p`*str*

The prompt.  Default: `%s> `.  `%s` is replaced by the name of the
current module.

### `-r`*str*

The string that stands for the last expression evaluated.  Default:
`$$`.

### `+t`

After evaluating an expression, print its type as well as its value.
Default: off.  The type shown is the one after defaulting, so it can
differ from what `:type` reports:

```
Cupid> :set +t
Cupid> 1+2
3 :: Integer
Cupid> :type 1+2
1 + 2 :: Num a => a
```

### `+T`

Apply the defaulting rules to a type before `:type` prints it.
Default: off.

```
Cupid> :type 1
1 :: Num a => a
Cupid> :set +T
Cupid> :type 1
1 :: Integer
```

### `+u`

Display results with `show`.  Default: on.  A value whose type is not
an instance of `Show` is an error:

```
Cupid> id
ERROR - Cannot find "show" function for:
*** Expression : id
*** Of type    : a -> a
```

With `-u`, a built-in printer is used instead.  It works for every
type, and it prints the whole of a value in which some parts raise
exceptions:

```
Cupid> :set -u
Cupid> id
id
Cupid> [1, 2 + error "foo", 3]
[1,{error "foo"},3]
```

With `+u` the same list stops at the exception:

```
Cupid> [1, 2 + error "foo", 3]
[1,
Program error: foo
```

### `+I`

Print the result of an `IO` action, with `print`, after running it.
Default: off.  Evaluating `m` with `+I` is the same as evaluating
`do { x <- m; print x }` with `-I`.

### `+Q`

Print names qualified with their modules.  Default: off.  Useful when
an error message about a redefined name is otherwise ambiguous.

### `+k`

Report kind errors in full and show kinds in `:info`.  Default: off.
Without it a kind error is reported as an illegal type:

```
ERROR "Test.hs":4 - Illegal type in class constraint
```

With it:

```
ERROR "Test.hs":4 - Kind error in class constraint
*** constructor    : Int
*** kind           : *
*** does not match : * -> *
```

## Resources

### `-h`*num*

The heap size in cells.  Default: 1000000.  Minimum: 19000.  *num*
may carry a `k`, `M` or `G` suffix, in either case.  The heap is
allocated at start-up; setting `-h` with `:set` has no effect on the
running interpreter.

### `+s`

After each evaluation, print the number of reductions and the number of
cells allocated.  Default: off.

```
Cupid> :set +s
Cupid> True
True
(26 reductions, 51 cells)
```

### `+g`

Print the number of cells recovered by each garbage collection, as
`{{Gc:n}}`.  Default: off.  See
[Measure time and memory use](../how-to/measure-time-and-memory.md).

### `-d`*num*

Sample the heap every *num* reductions and write the profile to
`profile.hp`; `-d0` turns profiling off.  Only present when Cupid was
built with `-Dprofiling=true`.

## Options that only exist in some builds

| Option | Build option | Meaning |
|---|---|---|
| `+D` | `checks` | Show the generated bytecode |
| `-d`*num* | `profiling` | Heap profiling interval |

## Obsolete options

`+98`, `-98`, `-e`, `-f`, `-N`, `-W`, `-G` and `+L` from older
releases are accepted with a message and ignored.  The `+H`, `+R` and
`+m` toggles of the 2006 release are gone with the extensions they
controlled.

## See also

- [Interpreter commands](commands.md) for `:set`
- [Environment variables](environment-variables.md) for `CUPIDFLAGS`
