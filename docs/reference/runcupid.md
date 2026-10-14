# runcupid

Run a Haskell program from the shell.

## Synopsis

```
runcupid [option ...] file [argument ...]
```

## Description

`runcupid` loads *file*, which must contain a `Main` module, runs
`main`, and exits with the exit code of the action.  The interpreter's
own output about loading is suppressed; compile errors go to standard
error and give an exit code of 1.

Modules that *file* imports are found as for `cupid`: first in the
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
runcupid Main.hs a b
```

Run a script with a bigger heap:

```sh
runcupid -h4M myscript
```

As the interpreter of an executable file, with both options in one
argument:

```
#! /usr/local/bin/runcupid -X+o -h4M
```

## Exit status

| Status | Condition |
|---|---|
| 0 | `main` completed |
| 1 | The file could not be loaded, or `main` raised an error, or no file was given |
| *n* | `main` called `exitWith (ExitFailure n)` |

## Environment

`CUPIDFLAGS`, `CUPIDDIR` and `HOME` as for `cupid`; see
[Environment variables](environment-variables.md).

## See also

- [Write a script that runs with runcupid](../how-to/write-a-script.md)
- [How Cupid runs a program](../explanation/how-cupid-runs-a-program.md)
