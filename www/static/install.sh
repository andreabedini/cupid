#!/bin/sh
# Install the latest release of Cupid on Linux or macOS.
#
#   curl -fsSL https://andreabedini.github.io/cupid/install.sh | sh
#
# Downloads the archive for this machine from the GitHub releases of
# andreabedini/cupid, checks it against the SHA256SUMS of the same release
# and, when gh is installed and signed in, against its build attestation,
# then unpacks it under ~/.local/opt and links the programs into
# ~/.local/bin.  Nothing needs root.  Settings, all optional:
#
#   CUPID_VERSION      the version to install, such as 0.2.0; default the latest
#   CUPID_INSTALL_DIR  where the release directory goes; default ~/.local/opt
#   CUPID_BIN_DIR      where the links to the programs go; default ~/.local/bin
#
# The same steps by hand, and building from source, are described at
# https://andreabedini.github.io/cupid/docs/install/

set -eu

repo=andreabedini/cupid
site=https://andreabedini.github.io/cupid
install_dir=${CUPID_INSTALL_DIR:-$HOME/.local/opt}
bin_dir=${CUPID_BIN_DIR:-$HOME/.local/bin}

say() { printf '%s\n' "$*"; }
die() { printf 'install.sh: %s\n' "$*" >&2; exit 1; }

# -- which archive -----------------------------------------------------------

system=$(uname -s)
machine=$(uname -m)
case $system in
  Linux)  os=linux ;;
  Darwin) os=macos ;;
  *) die "there is no release archive for $system; build from source instead: $site/docs/install/" ;;
esac
case $machine in
  x86_64 | amd64)  arch=x86_64 ;;
  aarch64 | arm64) arch=aarch64 ;;
  *) die "there is no release archive for $machine; build from source instead: $site/docs/install/" ;;
esac
target=$arch-$os

# -- downloading -------------------------------------------------------------

if command -v curl >/dev/null 2>&1; then
  fetch() { curl -fsSL -o "$2" "$1"; }
  # The URL that GitHub's "latest release" link ends up at.
  resolve() { curl -fsSLI -o /dev/null -w '%{url_effective}' "$1"; }
elif command -v wget >/dev/null 2>&1; then
  fetch() { wget -q -O "$2" "$1"; }
  resolve() { wget -q -S -O /dev/null "$1" 2>&1 | sed -n 's/^ *Location: *//p' | tail -n 1; }
else
  die "neither curl nor wget is installed"
fi

if [ -n "${CUPID_VERSION:-}" ]; then
  version=${CUPID_VERSION#v}
else
  url=$(resolve "https://github.com/$repo/releases/latest") || die "cannot reach github.com"
  case $url in
    */releases/tag/v*) version=${url##*/releases/tag/v} ;;
    *) die "no release of Cupid has been published yet; build from source instead: $site/docs/install/" ;;
  esac
fi

archive=cupid-$version-$target.tar.gz
base=https://github.com/$repo/releases/download/v$version

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT INT HUP TERM

say "Downloading $archive"
fetch "$base/$archive" "$tmp/$archive" || die "cannot download $base/$archive"
fetch "$base/SHA256SUMS" "$tmp/SHA256SUMS" || die "cannot download $base/SHA256SUMS"

# -- checking ----------------------------------------------------------------

if command -v sha256sum >/dev/null 2>&1; then
  check() { sha256sum -c - >/dev/null; }
elif command -v shasum >/dev/null 2>&1; then
  check() { shasum -a 256 -c - >/dev/null; }
else
  die "neither sha256sum nor shasum is installed"
fi

expected=$(grep " $archive\$" "$tmp/SHA256SUMS") || die "SHA256SUMS has no line for $archive"
( cd "$tmp" && printf '%s\n' "$expected" | check ) || die "$archive does not match SHA256SUMS"

# -- provenance --------------------------------------------------------------
#
# CI records a build attestation for every release file.  gh can check it,
# but only when signed in, so the check is made when it can be and said to
# be skipped otherwise.

attested=no
if command -v gh >/dev/null 2>&1 && gh auth token >/dev/null 2>&1; then
  say "Checking the build attestation with gh"
  if ! gh attestation verify "$tmp/$archive" --repo "$repo" \
       --signer-workflow "$repo/.github/workflows/ci.yml" >"$tmp/gh.out" 2>&1; then
    cat "$tmp/gh.out" >&2
    die "$archive has no valid build attestation from $repo's CI"
  fi
  attested=yes
fi

# -- installing --------------------------------------------------------------

dir=$install_dir/cupid-$version-$target
mkdir -p "$install_dir" "$bin_dir"
rm -rf "$dir"
tar -xzf "$tmp/$archive" -C "$install_dir"
[ -x "$dir/bin/cupid" ] || die "$archive does not contain bin/cupid"

programs=
for exe in "$dir"/bin/*; do
  name=$(basename "$exe")
  ln -sf "$exe" "$bin_dir/$name"
  programs="$programs $name"
done

say "Installed Cupid $version in $dir"
say "Linked$programs into $bin_dir"
if [ "$attested" = no ]; then
  say "The checksum matched; the build attestation was not checked because gh is not installed or not signed in."
fi
case ":$PATH:" in
  *":$bin_dir:"*) ;;
  *)
    say ""
    say "$bin_dir is not on your PATH.  Add it in your shell's startup file, for example:"
    say "  export PATH=\"$bin_dir:\$PATH\""
    ;;
esac
