# Make `:edit` open your editor

Tell Hugs how to start your editor, so that `:edit` and `:find` open the
right file at the right line.

## Prerequisites

- an editor that can be told a line number on its command line

## Steps

### 1. Choose the editor string

The `-E` option holds a template.  Hugs replaces `%s` with the file
name, `%d` with the line number of the last error, `%f` with the
absolute file name and `%%` with a percent sign.  If neither `%s` nor
`%f` appears, the file name is appended.

For vi and its clones:

```
-E"vi +%d %s"
```

For Emacs through emacsclient:

```
-E"emacsclient +%d %s"
```

For VS Code, which takes `file:line`:

```
-E"code --goto %s:%d"
```

When `-E` is not given, Hugs uses the `EDITOR` environment variable
without any line number.

### 2. Make it permanent

Put the option in `HUGSFLAGS` in your shell start-up file.  Quote the
option so that the shell passes the spaces through:

```sh
HUGSFLAGS='-E"vi +%d %s"'
export HUGSFLAGS
```

### 3. Use it

After a load error:

```
ERROR "Stats.hs":7 - Undefined variable "m"
Hugs> :edit
```

opens `Stats.hs` at line 7.  When the editor exits, Hugs reloads the
modules.  `:edit File.hs` edits a named file, and `:find name` opens the
module that defines `name`.

## Verify it worked

```
Hugs> :set
```

The line `Editor setting  :` shows the string in force.

## See also

- [Interpreter commands](../reference/commands.md) for `:edit` and
  `:find`
- [Options](../reference/options.md#-estr) for the template rules
- [Environment variables](../reference/environment-variables.md) for
  `HUGSFLAGS` and `EDITOR`
