# runhugs

Run a Haskell program from the shell.

## Synopsis

```
runhugs [option ...] file [argument ...]
```

## Description

`runhugs` loads *file*, which must contain a `Main` module, runs
`main`, and exits with the exit code of the action.  The interpreter's
own output about loading is suppressed; compile errors go to standard
error and give an exit code of 1.

Modules that *file* imports are found as for `hugs`: first in the
directory of *file*, then on the search path.

## Arguments

| Position | Meaning |
|---|---|
| Before *file*, beginning with `+` or `-` | Options for the interpreter, as in [Options](options.md) |
| *file* | The first argument that does not begin with `+` or `-` |
| After *file* | Arguments for the program; `getArgs` returns them |

`getProgName` returns *file*.

## Examples

Run a program with two arguments:

```sh
runhugs Main.hs a b
```

Run a literate script with a bigger heap:

```sh
runhugs -h4M +l myscript
```

As the interpreter of an executable file, with both options in one
argument:

```
#! /usr/local/bin/runhugs -X+l -h4M
```

## Exit status

| Status | Condition |
|---|---|
| 0 | `main` completed |
| 1 | The file could not be loaded, or `main` raised an error, or no file was given |
| *n* | `main` called `exitWith (ExitFailure n)` |

## Environment

`HUGSFLAGS`, `HUGSDIR` and `HOME` as for `hugs`; see
[Environment variables](environment-variables.md).

## See also

- [Write a script that runs with runhugs](../how-to/write-a-script.md)
- [The server API](server-api.md), which `runhugs` is written against
