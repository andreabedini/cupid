# Interpreter commands

What the `cupid` prompt accepts.

## Synopsis

```
cupid [option ...] [file-or-module ...]
```

Cupid reads its options (see [Options](options.md)), loads the `Prelude`
and any modules named on the command line, and shows a prompt.  The
prompt names the current module; at the start this is the empty module
`Cupid`, so the prompt is `Cupid>`.

At the prompt, a line is either a Haskell expression or a command.
Commands begin with `:` and may be abbreviated to their first letter:
`:l` for `:load`, `:t` for `:type`, and so on.

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
with `C` a standard class, provided at least one of those classes is
numeric or is `Show`, `Eq` or `Ord`.  This is wider than the Haskell
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

Forget everything and load again the modules that were loaded.  Use
this after editing a file.

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
Cupid.Prelude.map Cupid.Prelude.mapM Cupid.Prelude.mapM_ Cupid.Prelude.max
Cupid.Prelude.maxBound Cupid.Prelude.maximum Cupid.Prelude.maybe
(7 names listed)
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
instance Functor Maybe
instance Monad Maybe
...
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

Run a shell command, or with no argument start a shell.  The shell is
named by `SHELL`, or `/bin/sh` if that is unset.

### `:cd` *dir*

Change the working directory.  A leading `~/` is replaced by the home
directory.

### `:gc`

Force a garbage collection and report how many cells were recovered.

### `:xplain` *context*

Explain how the instances for *context* are resolved.  Only present
when Cupid was built with `-Dexplain-instance-resolution=true`.

## See also

- [Options](options.md)
- [Getting started with Cupid](../tutorials/getting-started.md)
