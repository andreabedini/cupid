# Character encoding

Haskell's `Char` is a Unicode code point.  Files, terminals and the
operating system deal in bytes.  This page is about where Hugs
converts between the two and how the choice is made.

## Where conversion happens

Hugs decodes bytes into characters when it reads program text and when
a program reads from a handle in text mode, and encodes characters
into bytes when a program writes to a text-mode handle, and at the
boundary with the system: file names, environment variables, command
arguments.  Handles opened in binary mode are exempt; they carry bytes
as `Char` values below 256.

Because program text is decoded, string literals and comments may
contain any character the encoding can represent.  Identifiers are an
exception: the lexer accepts only Latin-1 characters in names.

## The `char-encoding` build option

The encoding is chosen when Hugs is built, with `-Dchar-encoding=`:

| Value | Behaviour |
|---|---|
| `utf8` (default) | Bytes are UTF-8 everywhere, whatever the locale says |
| `locale` | The encoding of the `LC_CTYPE` category of the current locale, taken from `LC_ALL`, `LC_CTYPE` or `LANG`, converted through the C library's multibyte functions |
| `none` | No conversion: each byte is a `Char` below 256, and characters above 255 cannot be written |

The 2006 release used the locale, and only where the C library
advertised ISO 10646 support, which macOS and the BSDs did not.  On
those systems Unicode I/O was silently off.  Every supported target
today uses UTF-8 in practice, so the default is fixed UTF-8; `locale` remains for
an environment that genuinely uses another encoding.

## Consequences

- With the default, a file in Latin-1 is read as malformed UTF-8.
  Convert it, or build with `locale` and set the locale accordingly.
- Output to a terminal that is not in UTF-8 mode shows the raw bytes.
- `LC_ALL`, `LC_CTYPE` and `LANG` have no effect on Hugs itself unless
  it was built with `locale`.

## See also

- [Build options and install layout](../reference/build-options.md)
- [The language Hugs accepts](../reference/language.md#lexical-structure)
