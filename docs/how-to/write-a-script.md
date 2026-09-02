# Write a script that runs with runhugs

Make a Haskell file executable so that it runs from the shell like any
other script.

## Prerequisites

- an installed `runhugs` (this guide assumes `/usr/local/bin/runhugs`)
- a file containing a `Main` module

## Steps

### 1. Write the program as a literate script

The first line of a script must be the `#!` line, and Haskell has no
syntax for it.  Literate scripts get around that: only lines beginning
with `>` are code, so the `#!` line is a comment.  Create a file called
`myecho`:

```
#! /usr/local/bin/runhugs +l

> module Main where
> import System.Environment
>
> main = do
>   args <- getArgs
>   putStrLn (unwords args)
```

The `+l` option tells `runhugs` that the file is a literate script
even though its name does not end in `.lhs`.

### 2. Make it executable

```sh
chmod 755 myecho
```

### 3. Run it

```sh
./myecho Hello World
```

```
Hello World
```

The kernel runs `/usr/local/bin/runhugs +l ./myecho Hello World`;
`runhugs` loads the file, runs `main`, and `getArgs` returns the words
after the file name.

## Passing more than one option

The `#!` line passes everything after the program name as a single
argument.  With `#! /usr/local/bin/runhugs +l -98` the first argument
would be the one string `+l -98`, which is not an option.  Use `-X` to
have the string split into options:

```
#! /usr/local/bin/runhugs -X+l -98
```

Everything after `-X` is re-read as an option string, so both `+l` and
`-98` take effect.

## Troubleshooting

### `runhugs: missing file argument`

**Cause**: every argument on the command line began with `+` or `-`,
so `runhugs` found no file name.

**Fix**: put the script's name after the options.  In a `#!` line this
is done for you; from the shell write `runhugs +l myecho`.

### The program runs but `getArgs` returns the wrong words

**Cause**: options were placed after the file name, where `runhugs`
treats them as program arguments.

**Fix**: options go before the file name; arguments for the program go
after it.

## See also

- [runhugs](../reference/runhugs.md)
- [Options](../reference/options.md) for `+l` and `-X`
