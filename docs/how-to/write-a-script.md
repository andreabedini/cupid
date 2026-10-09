# Write a script that runs with runcuddles

Make a Haskell file executable so that it runs from the shell like any
other script.

## Prerequisites

- an installed `runcuddles` (this guide assumes `/usr/local/bin/runcuddles`)
- a file containing a `Main` module

## Steps

### 1. Write the program

The first line of a script must be the `#!` line.  Haskell has no
syntax for it, but Cuddles reads a line beginning with `#!` as a blank
line, so the rest of the file is an ordinary module.  Create a file
called `myecho`:

```
#! /usr/local/bin/runcuddles
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

The kernel runs `/usr/local/bin/runcuddles ./myecho Hello World`;
`runcuddles` loads the file, runs `main`, and `getArgs` returns the words
after the file name.

## Passing more than one option

The `#!` line passes everything after the program name as a single
argument.  With `#! /usr/local/bin/runcuddles +o -h4M` the first argument
would be the one string `+o -h4M`, which is not an option.  Use `-X` to
have the string split into options:

```
#! /usr/local/bin/runcuddles -X+o -h4M
```

Everything after `-X` is re-read as an option string, so both `+o` and
`-h4M` take effect.

## Troubleshooting

### `runcuddles: missing file argument`

**Cause**: every argument on the command line began with `+` or `-`,
so `runcuddles` found no file name.

**Fix**: put the script's name after the options.  In a `#!` line this
is done for you; from the shell write `runcuddles myecho`.

### The program runs but `getArgs` returns the wrong words

**Cause**: options were placed after the file name, where `runcuddles`
treats them as program arguments.

**Fix**: options go before the file name; arguments for the program go
after it.

## See also

- [runcuddles](../reference/runcuddles.md)
- [Options](../reference/options.md) for `+o`, `-h` and `-X`
