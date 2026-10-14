# The Cupid website

A [Zola](https://www.getzola.org/) site.  The home page lives here; the
documentation does not — `../docs` is the source of truth for that, and
`sync-docs.py` mirrors it into `content/docs/` before each build.

```sh
./sync-docs.py     # mirror ../docs and ../assets into the content tree
zola serve         # http://127.0.0.1:1111
zola build         # into public/
```

`content/docs/` and the copied logos in `static/` are generated, so they are
not checked in.  Run `sync-docs.py` again after editing anything under
`../docs`; `zola serve` will not notice a change made outside `www/`.

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
- copies `../assets/*.svg` into `static/`.

Two lists need to stay in step by hand: `SECTIONS` in `sync-docs.py` and
`extra.doc_sections` in `config.toml`.  Adding a new documentation section
means adding it to both.  Page ordering within a section comes from
`PAGE_ORDER` in the script; anything not listed sorts alphabetically after the
listed pages.

## Publishing

`base_url` in `config.toml` is `https://andreabedini.github.io/cupid`, the
GitHub Pages URL for `andreabedini/cupid`.  Every URL on the site is built
from it, so it has to match wherever the site is actually served — change it
before pointing a custom domain at this.

There is no deploy workflow yet.
