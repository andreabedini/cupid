# Character encoding

Haskell's `Char` is a Unicode code point.  Files, terminals and the
operating system deal in bytes.  This page is about where Cupid
converts between the two and how the choice is made.

## Where conversion happens

Cupid decodes bytes into characters when it reads program text and when
a program reads from a handle in text mode, and encodes characters into
bytes when a program writes to a text-mode handle.  It converts both
ways at the boundary with the system: file names, environment variables,
command arguments.  Handles opened in binary mode are exempt; they carry
bytes as `Char` values below 256.

Because program text is decoded, string literals and comments may
contain any character the encoding can represent.  Identifiers may use
any Unicode letter, classified through the Unicode tables in
`unitable.c`.

## The `char-encoding` build option

The encoding is chosen when Cupid is built, with `-Dchar-encoding=`:

| Value | Behaviour |
|---|---|
| `utf8` (default) | Bytes are UTF-8 everywhere, whatever the locale says |
| `locale` | The encoding of the `LC_CTYPE` category of the current locale, taken from `LC_ALL`, `LC_CTYPE` or `LANG`, converted through the C library's multibyte functions |
| `none` | No conversion: each byte is a `Char` below 256, and a character above 255 is written as its low byte |

The 2006 release used the locale, and only where the C library
advertised ISO 10646 support, which macOS and the BSDs did not.  On
those systems Unicode I/O was silently off.  Linux, macOS and the BSDs
use UTF-8 in practice, so the default is fixed UTF-8, on Windows too,
whatever the console's code page; `locale` remains for an environment
that uses another encoding.

## Consequences

- With the default, a Latin-1 byte is malformed UTF-8.  A program
  reading such a file gets an I/O error at that byte.  In a script, an
  accented letter also swallows the text after it, up to the next
  accented letter, with no message of its own: a stray `é` in a comment
  can end in `Undefined variable "main"`.  Convert the file, or build
  with `locale` and set the locale accordingly.
- Output to a terminal that is not in UTF-8 mode shows the raw bytes.
- `LC_ALL`, `LC_CTYPE` and `LANG` have no effect on Cupid itself unless
  it was built with `locale`.

## See also

- [Build options and install layout](../reference/build-options.md)
- [The language Cupid accepts](../reference/haskell-2010.md#2-lexical-structure)
