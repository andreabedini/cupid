# Write a script that runs with cupid

Make a Haskell file executable so that it runs from the shell like any
other script.

## Prerequisites

- an installed `cupid` (this guide assumes `/usr/local/bin/cupid`)
- a file containing a `Main` module

## Steps

### 1. Write the program

The first line of a script must be the `#!` line.  Haskell has no
syntax for it, but Cupid reads a `#!` first line as a blank line, so
the rest of the file is an ordinary module.  Create a file called
`myecho`:

```
#! /usr/local/bin/cupid
module Main where
import System.Environment

main = do
  args <- getArgs
  putStrLn (unwords args)
```

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

The kernel runs `/usr/local/bin/cupid ./myecho Hello World`; `cupid`
loads the file, runs `main`, and `getArgs` returns the words after the
file name.

If you do not know where `cupid` is installed, write the line as
`#!/usr/bin/env cupid` and the shell finds it on your PATH.

## Passing more than one option

The `#!` line passes everything after the program name as a single
argument.  With `#! /usr/local/bin/cupid +o -h4M` the first argument
would be the one string `+o -h4M`, which is not an option.  Use `-X` to
have the string split into options:

```
#! /usr/local/bin/cupid -X+o -h4M
```

Everything after `-X` is re-read as an option string, so both `+o` and
`-h4M` take effect.

## Troubleshooting

### The prompt appears instead of the program running

**Cause**: every argument on the command line began with `+` or `-`,
so `cupid` found no file name and started the prompt.

**Fix**: put the script's name after the options.  In a `#!` line this
is done for you; from the shell write `cupid myecho`.

### `cupid: can't find "Main" module`

**Cause**: the file's module header names a module other than `Main`.

**Fix**: make the file's module `Main`.  To load a file that is not a
program at the prompt, use `cupid -i FILE`.

### The program runs but `getArgs` returns the wrong words

**Cause**: options were placed after the file name, where `cupid`
treats them as program arguments.

**Fix**: options go before the file name; arguments for the program go
after it.

## See also

- [Running cupid](../reference/cupid.md)
- [Options](../reference/options.md) for `+o`, `-h` and `-X`
