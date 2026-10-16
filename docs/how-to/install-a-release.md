# Install a release

Install Cupid from a binary archive on the
[releases page](https://github.com/andreabedini/cupid/releases),
without building it.

## Prerequisites

- Linux with glibc 2.35 or later (Ubuntu 22.04, Debian 12, Fedora 36 or
  later), or macOS 13 or later; on x86-64 or ARM64
- on Linux, libedit, for line editing at the prompt: the mainstream
  distributions install it with OpenSSH; otherwise install `libedit2`
  (Debian, Ubuntu) or `libedit` (Fedora)
- a C compiler, only to use `fficupid`

## Steps

### 1. Download and check the archive

Pick the archive for your system, `cupid-VERSION-TARGET.tar.gz` with
TARGET one of `x86_64-linux`, `aarch64-linux`, `x86_64-macos` and
`aarch64-macos`, and check it against `SHA256SUMS` from the same
release:

```sh
sha256sum --ignore-missing -c SHA256SUMS
```

On macOS, use `shasum -a 256 --ignore-missing -c SHA256SUMS`.  With gh,
you can also check that CI built it:

```sh
gh attestation verify cupid-VERSION-TARGET.tar.gz --repo andreabedini/cupid
```

### 2. Unpack it anywhere

```sh
tar -xzf cupid-VERSION-TARGET.tar.gz -C ~/.local/opt
```

The archive has one directory, `cupid-VERSION-TARGET`, with `bin`,
`lib/cupid` and `share/man` inside.  The programs find their libraries
next to themselves, so the directory can go anywhere and move later.

### 3. Put the programs on your PATH

```sh
ln -s ~/.local/opt/cupid-VERSION-TARGET/bin/* ~/.local/bin/
```

## Verify it worked

```sh
cupid
```

```
Cupid VERSION, a Haskell interpreter written in C
```

If it stops with `FATAL ERROR: Unable to load Prelude`, `CUPIDDIR` is
probably set in your environment and points somewhere else; unset it.
