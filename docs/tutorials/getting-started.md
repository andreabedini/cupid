# Getting started with Hugs

Learn to use Hugs by building it, evaluating expressions at its prompt,
loading a module of your own and running a program from the shell.

**Time**: about 20 minutes, most of it the build.
**Level**: you have written a little Haskell before, or are following a
Haskell text alongside this.

## What you will learn

By the end you will be able to:

- build Hugs from source and run it without installing it
- evaluate expressions and ask for their types at the prompt
- write a module in a file, load it, and fix an error in it
- run a program's `main` from the shell with `runhugs`

## Prerequisites

You need a C compiler (gcc or clang), meson 1.1 or later, ninja and
python3.  On Fedora:

```sh
sudo dnf install gcc meson ninja-build python3
```

On Debian or Ubuntu:

```sh
sudo apt install gcc meson ninja-build python3
```

On macOS with Homebrew:

```sh
brew install meson ninja
```

Everything else in this tutorial is done from the top of the Hugs
source tree.

## Step 1: Build Hugs

Configure a build directory and build in it:

```sh
CC=gcc meson setup build
ninja -C build
```

The build compiles three programs into `build/src` and then assembles
the Haskell libraries into `build/libraries/hugsdir`.  The last lines
of output look like this:

```
Compiling FFI stubs for base/Data/ByteString/Base.hs
[43/43] Generating libraries/hugs-libraries with a custom command
```

Now tell the shell where the programs and the libraries are:

```sh
export PATH="$PWD/build/src:$PATH"
export HUGSDIR="$PWD/build/libraries/hugsdir"
```

These two settings last for the current shell session.  Every later
step assumes them.

## Step 2: Evaluate expressions

Start the interpreter:

```sh
hugs
```

You will see the banner and a prompt:

```
__   __ __  __  ____   ___      _________________________________________
||   || ||  || ||  || ||__      Hugs 98: Based on the Haskell 98 standard
||___|| ||__|| ||__||  __||     Copyright (c) 1994-2005
||---||         ___||           World Wide Web: http://haskell.org/hugs
||   ||                         Bugs: http://hackage.haskell.org/trac/hugs
||   || Version: September 2006 _________________________________________

Type :? for help
Hugs>
```

Type an expression and press Enter.  Hugs evaluates it and prints the
result:

```
Hugs> 1 + 2
3
Hugs> map (*2) [1..5]
[2,4,6,8,10]
Hugs> words "the quick brown fox"
["the","quick","brown","fox"]
```

Lines that begin with a colon are commands to the interpreter rather
than expressions.  Ask for the type of an expression with `:type`:

```
Hugs> :type map (*2)
map (flip (*) 2) :: Num a => [a] -> [a]
```

Expressions of type `IO` are run rather than shown:

```
Hugs> putStrLn "hello"
hello
```

Leave the interpreter with `:quit`.  You can also press Ctrl-D.

```
Hugs> :quit
[Leaving Hugs]
```

## Step 3: Write a module

The prompt accepts expressions, not definitions.  Definitions go in a
file.  Create a file called `Stats.hs` in the current directory with
this content:

```haskell
module Stats where

mean :: [Double] -> Double
mean xs = sum xs / fromIntegral (length xs)

variance :: [Double] -> Double
variance xs = mean [ (x - m) ^ 2 | x <- xs ]
```

The file is deliberately incomplete: `variance` uses a name `m` that is
never defined.  You will see how Hugs reports that next.

## Step 4: Load the module and fix the error

Start Hugs with the file name on the command line:

```sh
hugs Stats.hs
```

After the banner, Hugs tries to load the file and stops at the error:

```
ERROR "Stats.hs":7 - Undefined variable "m"
Hugs>
```

The message names the file and the line.  Leave Hugs running, open
`Stats.hs` in your editor, and add a `where` clause to `variance` so that
the function reads:

```haskell
variance :: [Double] -> Double
variance xs = mean [ (x - m) ^ 2 | x <- xs ]
  where m = mean xs
```

Save the file.  Back at the prompt, reload it:

```
Hugs> :reload
Stats>
```

The prompt now says `Stats>` because `Stats` is the module whose
definitions are in scope.  Use them:

```
Stats> mean [1, 2, 3, 4]
2.5
Stats> variance [1, 2, 3, 4]
1.25
Stats> :type variance
variance :: [Double] -> Double
```

Every time you change the file, `:reload` picks up the new version.

## Step 5: Run a program from the shell

A program is a module called `Main` that defines `main`.  Create
`Main.hs` next to `Stats.hs`:

```haskell
module Main where

import Stats
import System.Environment

main :: IO ()
main = do
  args <- getArgs
  let xs = map read args
  putStrLn ("mean:     " ++ show (mean xs))
  putStrLn ("variance: " ++ show (variance xs))
```

Leave the interpreter with `:quit` and run the program with `runhugs`,
giving it some numbers as arguments:

```sh
runhugs Main.hs 1 2 3 4
```

```
mean:     2.5
variance: 1.25
```

`runhugs` loads `Main.hs`, finds `Stats.hs` in the same directory
because `Main` imports it, runs `main`, and exits.  The words after
the file name are what `getArgs` returns.

You can run the same program from inside the interpreter with `:main`:

```sh
hugs Main.hs
```

```
Main> :main 10 20
mean:     15.0
variance: 25.0
```

## What you have done

You built Hugs, evaluated expressions at its prompt, wrote a module,
found and fixed an error in it with the help of Hugs's error message,
and ran a two-module program both from the shell and from the prompt.

## Next steps

- [Write a script that runs with runhugs](../how-to/write-a-script.md)
  shows how to make `Main.hs` an executable file.
- [Make `:edit` open your editor](../how-to/configure-the-editor.md)
  lets you jump to an error's line without leaving Hugs.
- [Interpreter commands](../reference/commands.md) lists everything the
  prompt understands.
- [The libraries](../reference/libraries.md) lists the modules you can
  import.
- [Build and install Hugs](../how-to/build-and-install.md) explains
  how to install so that the two `export` lines are no longer needed.
