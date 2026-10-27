# Make a release

Publish a new version of Cupid on GitHub, with binary archives for
Linux, macOS and Windows, the source distribution and the changelog.
The maintainer makes releases.

## Prerequisites

- [mise](https://mise.jdx.dev/), which installs the pinned git-cliff and
  jq from `mise.toml`
- zsh, which the release task is written in, and GNU sed (on macOS,
  install `gnu-sed` and put it first on the path)
- git and jj, with the repository a jj repository colocated with git
- gh, signed in to GitHub with push access to the repository
- every change to release merged into `main`, with CI green

## Steps

### 1. Open the release pull request

```sh
mise run release-pr
```

It reads the commits on `main` since the last release tag and picks the
next version from them (see [Versions](#versions)).  It sets that version
in `project()` in `meson.build`, regenerates `CHANGELOG.md` with
[git-cliff](https://git-cliff.org/), and opens a pull request titled
`chore(release): VERSION`.  If no commit since the last release is a
`feat`, `fix`, `perf` or breaking change, it says so and does nothing.

### 2. Review it

Edit `CHANGELOG.md` in the pull request if an entry needs rewording.  To
choose a different version, such as 1.0.0, change it in `meson.build`,
in the `CHANGELOG.md` heading and in the commit description, all three
the same.

### 3. Merge it

When `ci-ok` passes, merge the pull request with a rebase merge.

CI's `release` job sees the release commit at the head of `main`.  It
publishes a GitHub release, which creates the tag `vVERSION`, with:

- `cupid-VERSION-TARGET.tar.gz` for `x86_64-linux`, `aarch64-linux`,
  `x86_64-macos` and `aarch64-macos`: the archives that CI built and
  tested, nothing rebuilt
- `cupid-VERSION-x86_64-windows.zip`, likewise built and tested by CI
- `cupid-VERSION.tar.gz`, the source distribution that `meson dist`
  makes
- `SHA256SUMS` for all of them
- the version's section of `CHANGELOG.md` as the release notes

### 4. Verify it

```sh
gh release verify vVERSION
gh attestation verify cupid-VERSION-x86_64-linux.tar.gz --repo andreabedini/cupid --signer-workflow andreabedini/cupid/.github/workflows/ci.yml
```

The second command checks the build attestation that the `release` job
records for every file in `dist/` with `actions/attest-build-provenance`.
It is the check [Install a release](../install/install-a-release.md#2-check-that-ci-built-it)
gives users, and the one the install scripts run when `gh` is signed
in, so the signer workflow must stay `ci.yml` on `main` or that page
and the scripts need updating.

Releases are immutable: a published version cannot change.  If a
release is wrong, fix it and make a new one.  If the `release` job fails
before it publishes, rerun the failed jobs; it continues the draft
release.

## Versions

Versions follow [Semantic Versioning](https://semver.org/), `A.B.C`.
The release task works out the next one from the commit messages, which
CI checks follow [Conventional
Commits](https://www.conventionalcommits.org/):

| Since the last release      | While 0.x | From 1.0.0 |
|-----------------------------|-----------|------------|
| a breaking change (`type!:`) | 0.B+1.0   | A+1.0.0    |
| a `feat`                     | 0.B+1.0   | A.B+1.0    |
| only `fix` and `perf`        | 0.B.C+1   | A.B.C+1    |

Nothing leaves 0.x by itself: 1.0.0 is set by hand in step 2.
`cliff.toml` holds these rules and the grouping of the changelog.
