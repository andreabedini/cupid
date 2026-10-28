# Interpreter commands

What the `cupid` prompt accepts.

## Synopsis

```
cupid [option ...]
cupid [option ...] -i file-or-module ...
```

Cupid reads its options (see [Options](options.md)), loads the `Prelude`
and, with `-i`, the modules named on the command line, and shows a
prompt.  Without `-i`, a name on the command line is a program to run;
see [Running cupid](cupid.md).  The prompt names the current module; at
the start this is the empty module `Cupid`, so the prompt is `Cupid>`.

At the prompt, a line is either a Haskell expression or a command.
Commands begin with `:` and may be abbreviated to their first letter:
`:l` for `:load`, `:t` for `:type`, and so on.  Where two commands
share a first letter, the one listed first in `:?` wins: `:m` is
`:module`, so write `:main` in full.

Ctrl-C interrupts a load or an evaluation; Cupid prints
`{Interrupted!}` and returns to the prompt.  Ctrl-D at the prompt
leaves the interpreter.

## Evaluation

### *expr*

Evaluate a Haskell expression.  The expression must fit on one line.

The value is converted to a string with `show` and printed:

```
Cupid> 1+2
3
```

If the expression has type `IO t`, the action is performed and its
result discarded:

```
Cupid> print (1+2) >> putStrLn "bye"
3
bye
```

The `-u` option switches to a built-in printer that works for any type
and shows exceptions inside partial results; the `+I` option prints the
result of an `IO` action.  See
[Evaluation and printing](options.md#evaluation-and-printing).

**Ambiguous types.**  Before evaluation, defaulting is applied to each
ambiguous type variable `v` whose constraints all have the form `C v`
with `C` one of the Haskell 2010 classes `Eq`, `Ord`, `Show`, `Read`,
`Enum`, `Bounded`, `Ix` or a numeric class, provided at least one of
those classes is numeric or is `Show`, `Eq` or `Ord`.  Classes such as
`Monoid` never default.  This is wider than the Haskell
rule for modules, which requires a numeric class.  It is what makes
`reverse []` printable: the `Show` constraint from printing defaults
the element type to `Integer`.  Any ambiguous variable not covered by
this rule is an error.

### `:type` *expr*

Print the type of *expr* without evaluating it:

```
Cupid> :type 1+2
1 + 2 :: Num a => a
```

Defaulting is not applied unless the `+T` option is set.

### `:main` *argument* ...

Run `main` from the current module with `getArgs` returning the
arguments.  The same effect is available from any module that imports
`System.Environment`:

```
Main> withArgs ["a", "b"] main
```

### `:set` [*option* ...]

Set options; see [Options](options.md).  With no arguments, list the
options and their current values.  Options set this way last until the
interpreter exits; put them in `CUPIDFLAGS` to make them permanent.

### `:quit`

Exit.

## Loading modules

Cupid keeps one module per file.  A file may be named by its path or,
if it is on the search path, by its module name.  See
[Modules, files and the search path](../explanation/modules-and-the-search-path.md)
for how names are resolved.

The **current module** is the one whose scope expressions are
evaluated in.  Unexported names of the current module are visible at
the prompt.

### `:load` [*file-or-module* ...]

Forget every module except the empty module `Cupid`, the `Prelude` and
what it uses, then load the named files or modules.  The last one
loaded becomes the current module.

A literal file name may contain a Haskell module of any name.  To
include a space in a file name, precede it with a backslash or wrap the
name in double quotes; inside double quotes `\ `, `\"` and `\\` are
escapes and other backslashes are literal.

With no arguments, `:load` just forgets everything.

### `:also` *file-or-module* ...

Load additional files or modules without forgetting the loaded ones.
The last one loaded becomes the current module.

### `:reload`

Reload the first loaded module whose file has changed since it was
read, and every module loaded after it.  If no file has changed,
nothing is read.  Use this after editing a file.

### `:module` *module*

Make *module* the current module.  It must be loaded.

### `:edit` [*file*]

Start the editor on *file*.  With no argument, edit the file last
edited or last named in an error message, at the line of the error;
failing that, the file of the last module loaded.  When the editor
exits, Cupid reloads the modules.  The editor is chosen by the `-E` option or
the `EDITOR` environment variable; see
[Make `:edit` open your editor](../how-to/configure-the-editor.md).

### `:find` *name*

Edit the module that defines *name*.

## Information

### `:?`

Print a summary of the commands.

### `:names` [*pattern* ...]

List the names in scope that match any of the patterns.  A pattern is
like a shell file pattern: `*` matches any string, `?` any character,
`[chars]` any of the characters (with `a-z` for a range) and `\c` the
character `c` itself.  With no pattern, list every name in scope.

```
Cupid> :names ma*
Cupid.Prelude.many Cupid.Prelude.map Cupid.Prelude.mapAccumL
Cupid.Prelude.mapAccumR Cupid.Prelude.mapM Cupid.Prelude.mapM_
Cupid.Prelude.mappend Cupid.Prelude.max Cupid.Prelude.maxBound
Cupid.Prelude.maximum Cupid.Prelude.maximumBy Cupid.Prelude.maybe
(12 names listed)
```

### `:info` [*name* ...]

Describe each name: its type, or for a class or type constructor its
definition, constructors and instances.  Qualified names may refer to
anything defined or imported in any loaded module.

```
Cupid> :info Maybe
-- type constructor
data Maybe a

-- constructors:
Nothing :: Maybe a
Just :: a -> Maybe a

-- instances:
instance Semigroup a => Semigroup (Maybe a)
instance Semigroup a => Monoid (Maybe a)
instance Foldable Maybe
instance Traversable Maybe
instance Functor Maybe
instance Applicative Maybe
instance Monad Maybe
instance MonadFail Maybe
instance Alternative Maybe
instance MonadPlus Maybe
instance Eq a => Eq (Maybe a)
instance Ord a => Ord (Maybe a)
instance Read a => Read (Maybe a)
instance Show a => Show (Maybe a)
```

With `+k` set, classes and type constructors are shown with their
kinds.  With no argument, `:info` lists the loaded files.

### `:browse` [`all`] [*module* ...]

List the names exported by each module, which must be loaded, with
their types.  With `all`, list every name in scope inside the module
rather than only the exports.  With no module, describe the current
module.

### `:version`

Print the version:

```
Cupid> :version
-- Cupid Version 0.1.0
```

## Other commands

### `:!` [*command*]

Run a shell command, or with no argument start a shell.  With no
argument Cupid starts the shell named by `SHELL`, or `/bin/sh` if that
is unset.  With an argument it runs the command with `/bin/sh -c`,
whatever `SHELL` says.

### `:cd` *dir*

Change the working directory.  A leading `~/` is replaced by the home
directory.

### `:gc`

Force a garbage collection and report how many cells were recovered.

### `:xplain` *context*

Explain how the instances for *context* are resolved, printing each
entailment tried and each instance used.

## Differences from GHCi

The commands look like GHCi's, and `:load`, `:reload`, `:type`,
`:info`, `:browse`, `:main`, `:cd` and `:!` do much the same.  These
are the places where a GHCi habit does not carry over:

| In GHCi | In Cupid |
|---|---|
| `x = 1` or `let x = 1` defines `x` for the session | A syntax error: the prompt takes expressions only.  Use `let x = 1 in ...` or `... where x = 1` inside one expression, or put the definition in a file and load it |
| `:{` and `:}` enter several lines | Not available: an expression must fit on one line |
| `it` is the last value | `$$` is the last expression, evaluated again; see [`-r`](options.md#-rstr) |
| An `IO` action's result is shown unless it is `()` | The result is discarded unless `+I` is set |
| `:module + A B` puts several modules in scope | `:module` takes one module, and the scope is that module's top level.  To use names from several, load a module that imports them all |
| `:kind T` | Not a command.  `:info T` describes `T`, and shows its kind when `+k` is set |
| `:set -XExtension` turns on an extension | Every extension is always on except overlapping instances, which need `+o`; `-X` re-reads an option string.  See [Language extensions](extensions.md) |
| `:set +s` shows time and bytes allocated | `+s` shows reductions and cells used |

## See also

- [Options](options.md)
- [Getting started with Cupid](../tutorials/getting-started.md)
- [Differences from Hugs 98](differences-from-hugs.md)
