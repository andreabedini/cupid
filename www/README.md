# The Cupid website

A [Zola](https://www.getzola.org/) site.  The home page lives here; the
documentation does not — `../docs` is the source of truth for that, and
`sync-docs.py` mirrors it into `content/docs/` before each build.

```sh
mise run site      # http://127.0.0.1:1111, mirroring ../docs and ../assets as they change
./sync-docs.py     # mirror them once
zola build         # into public/
```

`content/docs/` and the copied logos in `static/` are generated, so they are
not checked in.  `static/install.sh` and `static/install.ps1` are written by
hand and checked in: they are the one-line installers the home page and the
install guide quote, served at `BASE_URL/install.sh` and
`BASE_URL/install.ps1`.

`zola serve` on its own will not notice a change made outside `www/`, so
`mise run site` runs it alongside a watcher that re-runs `sync-docs.py`.  The
script writes only the files whose contents changed, so a one-page edit costs
one rebuild.

## What sync-docs.py does

- copies every `docs/**/*.md` into `content/docs/`, adding the TOML front
  matter Zola needs;
- takes each page's title from its first `#` heading and drops that heading
  from the body, because the template renders the title itself;
- turns `docs/README.md`, and the README of any section that has one, into
  a Zola section index (`_index.md`);
- rewrites relative markdown links (`../reference/options.md`) into Zola's
  `@/docs/reference/options.md` form.  Zola resolves those at build time, so
  `zola build` fails on a link that has gone stale — which is the main reason
  for doing it this way rather than serving the markdown as-is;
- makes a link to the published site, such as the one to `install.sh`, relative
  to its page;
- copies `../assets/*.svg` into `static/`;
- writes a redirect page into `static/docs/` for each old address in its
  `MOVED_PAGES` and `MOVED_ANCHORS` tables.

`check-redirects.py public` checks a built site against `old-urls.txt`, every
page and anchor the site had before its sections moved.  That check is separate
from `zola build`, which knows nothing about those addresses, and CI doesn't run
it yet.

Two lists need to stay in step by hand: `SECTIONS` in `sync-docs.py` and
`extra.doc_sections` in `config.toml`.  Adding a new documentation section
means adding it to both.  Page ordering within a section comes from
`PAGE_ORDER` in the script; anything not listed sorts alphabetically after the
listed pages.

## Publishing

`base_url` in `config.toml` is `https://andreabedini.github.io/cupid`, the
GitHub Pages URL for `andreabedini/cupid`.  `base.html` makes every internal link
in a page relative, so a build works wherever it is served, with `zola serve` or
from `public/`.  `base_url` still decides the sitemap and the URLs quoted in text,
such as the install command, so change it before pointing a custom domain at this.

CI deploys it.  The `site` job in `.github/workflows/ci.yml` runs
`sync-docs.py` and `zola build` on every pull request and push, so a
stale link fails CI, and uploads `public/` as the Pages artifact.  The
`pages` job deploys that artifact after each release, so the site
documents the latest release rather than `main`.  To deploy `main`
without a release, run CI by hand on it:

```sh
gh workflow run ci.yml --ref main
```

The zola version is pinned in `../mise.toml`; the `site` job pins the
same one, with the checksum of its Linux archive.  Change them together.
