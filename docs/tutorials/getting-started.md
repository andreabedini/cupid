# Getting started with Cupid

Learn to use Cupid by evaluating expressions at its prompt, loading a
module of your own and running a program from the shell.

**Time**: about 15 minutes.
**Level**: you have written a little Haskell before, or are following a
Haskell text alongside this.

## What you will learn

By the end you will be able to:

- evaluate expressions and ask for their types at the prompt
- write a module in a file, load it, and fix an error in it
- run a program's `main` from the shell with `cupid`

## Prerequisites

You need Cupid on your PATH.  [Install a release](../install/install-a-release.md)
does that in one command; if you would rather build it yourself,
[Build Cupid on Linux and macOS](../install/build-on-linux-and-macos.md)
and [Build Cupid on Windows](../install/build-on-windows.md) show how,
and how to run the result without installing it.

You also need a text editor and an empty directory to work in.  Every
step below is done from that directory.

## Step 1: Evaluate expressions

Start the interpreter:

```sh
cupid
```

You will see the banner and a prompt:

```
Cupid 0.1.0, a Haskell interpreter written in C
Derived from Hugs 98, Copyright (c) 1994-2005 Mark P Jones, Alastair Reid,
the Yale Haskell Group and the OGI School of Science & Engineering at OHSU

Type :? for help
Cupid>
```

Type an expression and press Enter.  Cupid evaluates it and prints the
result:

```
Cupid> 1 + 2
3
Cupid> map (*2) [1..5]
[2,4,6,8,10]
Cupid> words "the quick brown fox"
["the","quick","brown","fox"]
```

Lines that begin with a colon are commands to the interpreter rather
than expressions.  Ask for the type of an expression with `:type`:

```
Cupid> :type map (*2)
map (flip (*) 2) :: Num a => [a] -> [a]
```

An expression must fit on one line, but it can have local definitions
of its own, with `let` or `where`:

```
Cupid> let n = 10 in sum [1..n]
55
Cupid> take 5 squares where squares = [ x * x | x <- [1..] ]
[1,4,9,16,25]
```

Those names last only for the one expression.

Expressions of type `IO` are run rather than shown:

```
Cupid> putStrLn "hello"
hello
```

Leave the interpreter with `:quit`.  You can also press Ctrl-D.

```
Cupid> :quit
[Leaving Cupid]
```

## Step 2: Write a module

The prompt accepts expressions, not definitions.  If you are used to
GHCi, this is the first difference you will meet: `x = 1` and
`let x = 1` are syntax errors at Cupid's prompt.  Definitions that
last go in a file.  Create a file called `Stats.hs` in the current directory with
this content:

```haskell
module Stats where

mean :: [Double] -> Double
mean xs = sum xs / fromIntegral (length xs)

variance :: [Double] -> Double
variance xs = mean [ (x - m) ^ 2 | x <- xs ]
```

The file is deliberately incomplete: `variance` uses a name `m` that is
never defined.  You will see how Cupid reports that next.

## Step 3: Load the module and fix the error

Start Cupid with the `-i` option and the file name on the command
line.  The option asks Cupid to load the file at the prompt rather than
run it:

```sh
cupid -i Stats.hs
```

After the banner, Cupid tries to load the file and stops at the error:

```
ERROR "Stats.hs":7 - Undefined variable "m"
Cupid>
```

The message names the file and the line.  Leave Cupid running, open
`Stats.hs` in your editor, and add a `where` clause to `variance` so that
the function reads:

```haskell
variance :: [Double] -> Double
variance xs = mean [ (x - m) ^ 2 | x <- xs ]
  where m = mean xs
```

Save the file.  Back at the prompt, reload it:

```
Cupid> :reload
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

## Step 4: Run a program from the shell

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

Leave the interpreter with `:quit` and run the program with `cupid`,
giving it some numbers as arguments:

```sh
cupid Main.hs 1 2 3 4
```

```
mean:     2.5
variance: 1.25
```

Given a file and no `-i`, `cupid` loads `Main.hs`, finds `Stats.hs` in
the same directory because `Main` imports it, runs `main`, and exits.
The words after the file name are what `getArgs` returns.

You can run the same program from inside the interpreter by loading it
with `-i` and using `:main`:

```sh
cupid -i Main.hs
```

```
Main> :main 10 20
mean:     15.0
variance: 25.0
```

## What you have done

You evaluated expressions at Cupid's prompt, wrote a module, found and
fixed an error in it with the help of Cupid's error message, and ran a
two-module program both from the shell and from the prompt.

## Next steps

- [Write a script that runs with cupid](../how-to/write-a-script.md)
  shows how to make `Main.hs` an executable file.
- [Make `:edit` open your editor](../how-to/configure-the-editor.md)
  lets you jump to an error's line without leaving Cupid.
- [Interpreter commands](../reference/commands.md) lists everything the
  prompt understands.
- [The libraries](../reference/libraries.md) lists the modules you can
  import.
