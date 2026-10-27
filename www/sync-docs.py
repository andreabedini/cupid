#!/usr/bin/env python3
"""Copy docs/ into the Zola content tree.

docs/ is the source of truth for the documentation.  This script mirrors it
into www/content/docs/, adding the TOML front matter Zola needs and rewriting
the relative markdown links into Zola's `@/...` internal-link form, so that
`zola build` fails loudly when a link goes stale.

Nothing here edits docs/.  Run it before `zola build` or `zola serve`; the
generated tree is not checked in.
"""

import re
import shutil
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
DOCS = ROOT / "docs"
ASSETS = ROOT / "assets"
WWW = Path(__file__).resolve().parent
CONTENT = WWW / "content" / "docs"
STATIC = WWW / "static"

# The documentation sections: the four Diátaxis quadrants, with Install in
# front of them and Contributing after.  The order here is the order they
# appear in the site navigation, and it has to match extra.doc_sections in
# config.toml.
SECTIONS = [
    ("install", "Install", "Get Cupid onto your machine: a release archive, or a build from source."),
    ("tutorials", "Tutorials", "Lessons that take you through a first session, start to finish."),
    ("how-to", "How-to guides", "Recipes for a specific task you already know you need."),
    ("explanation", "Explanation", "How the pieces fit, and why they are shaped the way they are."),
    ("reference", "Reference", "The dry description of commands, options and the accepted language."),
    ("contributing", "Contributing", "For people who change Cupid: how it is checked and released."),
]

# Page order within a section, where alphabetical is the wrong order.  Pages not
# listed sort alphabetically after the listed ones.
PAGE_ORDER = {
    "install": [
        "install-a-release.md",
        "build-on-linux-and-macos.md",
        "build-on-windows.md",
    ],
    "how-to": [
        "write-a-script.md",
        "call-c-functions.md",
        "configure-the-editor.md",
        "preprocess-sources.md",
        "add-a-library.md",
        "measure-time-and-memory.md",
    ],
    "contributing": [
        "check-every-configuration.md",
        "run-clang-tidy.md",
        "make-a-release.md",
    ],
    "reference": [
        "commands.md",
        "options.md",
        "environment-variables.md",
        "cupid.md",
        "build-options.md",
        "language.md",
        "haskell-2010.md",
        "extensions.md",
        "libraries.md",
    ],
    "explanation": [
        "design.md",
        "how-cupid-runs-a-program.md",
        "cells-text-and-tables.md",
        "how-source-is-read.md",
        "how-types-are-inferred.md",
        "how-constraints-are-solved.md",
        "how-code-is-compiled-and-run.md",
        "modules-and-the-search-path.md",
        "character-encoding.md",
        "foreign-import-wrappers.md",
        "history.md",
    ],
}

LINK = re.compile(r"\[([^\]]*)\]\(([^)\s]+)\)")
H1 = re.compile(r"^#\s+(.*?)\s*$", re.MULTILINE)
HEADING = re.compile(r"^#{1,6}\s+(.+?)\s*#*$")
INLINE_MARKS = re.compile(r"[`*_~]")
INLINE_LINK = re.compile(r"\[([^\]]*)\]\([^)]*\)")


def toml_escape(s: str) -> str:
    return s.replace("\\", "\\\\").replace('"', '\\"')


def split_title(text: str):
    """Take the first level-1 heading as the title and drop it from the body.

    The template renders the title itself, so leaving the heading in the body
    would print it twice.
    """
    m = H1.search(text)
    if not m or text[: m.start()].strip():
        return None, text
    return m.group(1), text[m.end() :].lstrip("\n")


# --- anchors ---------------------------------------------------------------
#
# GitHub and Zola slugify headings differently.  `### `-P`*str*` becomes
# `#-pstr` on GitHub and `#pstr` in Zola, because Zola drops the leading
# non-alphanumerics.  The docs are written to be readable on GitHub, so rather
# than change them, translate the anchors on the way in.  Both slugs are
# computed from the same heading text, so a link that works on GitHub works
# here; anything this fails to recognise is left alone and Zola's own link
# check will complain about it.

def heading_texts(md: str):
    """Every heading in the document, in order, ignoring fenced code."""
    out, fence = [], None
    for line in md.splitlines():
        stripped = line.strip()
        if fence is not None:
            if stripped.startswith(fence):
                fence = None
            continue
        if stripped.startswith("```") or stripped.startswith("~~~"):
            fence = stripped[:3]
            continue
        m = HEADING.match(line)
        if m:
            out.append(m.group(1))
    return out


def plain(text: str) -> str:
    return INLINE_MARKS.sub("", INLINE_LINK.sub(r"\1", text)).strip()


def github_slug(text: str) -> str:
    t = plain(text).lower()
    return re.sub(r"[^\w\- ]", "", t).replace(" ", "-")


def zola_slug(text: str) -> str:
    t = plain(text).lower()
    return re.sub(r"[^a-z0-9]+", "-", t).strip("-")


def anchor_map(md: str) -> dict:
    """GitHub anchor -> Zola anchor for one document."""
    seen_gh, seen_zola, out = {}, {}, {}
    for text in heading_texts(md):
        gh, zo = github_slug(text), zola_slug(text)
        n = seen_gh.get(gh, 0)
        seen_gh[gh] = n + 1
        gh_id = gh if n == 0 else "%s-%d" % (gh, n)
        n = seen_zola.get(zo, 0)
        seen_zola[zo] = n + 1
        zo_id = zo if n == 0 else "%s-%d" % (zo, n)
        out.setdefault(gh_id, zo_id)
    return out


_anchor_cache = {}


def anchors_for(rel: str) -> dict:
    if rel not in _anchor_cache:
        path = DOCS / rel
        _anchor_cache[rel] = anchor_map(path.read_text(encoding="utf-8")) if path.exists() else {}
    return _anchor_cache[rel]


# --- links -----------------------------------------------------------------

def rewrite_link(target: str, source_rel: Path) -> str:
    """Turn a docs-relative link into a Zola internal link.

    `source_rel` is the source file's path relative to docs/.  A link to a
    directory means that section's index.
    """
    if "://" in target or target.startswith(("#", "mailto:", "/")):
        return target

    path, _, anchor = target.partition("#")
    if not path:
        return target

    if path.endswith("/"):
        path += "README.md"
    if not path.endswith(".md"):
        return target

    parts = []
    for part in (source_rel.parent / path).as_posix().split("/"):
        if part in (".", ""):
            continue
        if part == ".." and parts and parts[-1] != "..":
            parts.pop()
        else:
            parts.append(part)
    resolved = "/".join(parts)

    if anchor:
        anchor = anchors_for(resolved).get(anchor, anchor)

    # docs/README.md and docs/*/README.md become Zola section indexes.
    if resolved.endswith("README.md"):
        resolved = resolved[: -len("README.md")] + "_index.md"

    return "@/docs/" + resolved + (("#" + anchor) if anchor else "")


def rewrite_links(text: str, source_rel: Path) -> str:
    return LINK.sub(
        lambda m: "[%s](%s)" % (m.group(1), rewrite_link(m.group(2), source_rel)),
        text,
    )


# --- writing ---------------------------------------------------------------

def weight_of(section: str, name: str, names: list) -> int:
    order = PAGE_ORDER.get(section)
    if order and name in order:
        return order.index(name)
    return 100 + names.index(name)


def raw_wrap(body: str) -> str:
    """Hide the body from Zola's shortcode pass.

    The docs are plain markdown, not templates, and sample output in them
    contains things like `{{Gc:986865}}` that Tera would otherwise try to
    render.  `{% raw %}` suppresses only shortcodes; markdown still renders.
    """
    if not body.strip():
        return body
    return "{% raw %}\n" + body + "\n{% endraw %}\n"


def write_page(dest: Path, front: dict, body: str) -> None:
    lines = ["+++"]
    for key, value in front.items():
        if isinstance(value, bool):
            lines.append("%s = %s" % (key, "true" if value else "false"))
        elif isinstance(value, int):
            lines.append("%s = %d" % (key, value))
        elif isinstance(value, dict):
            lines.append("[%s]" % key)
            for k, v in value.items():
                lines.append('%s = "%s"' % (k, toml_escape(str(v))))
        else:
            lines.append('%s = "%s"' % (key, toml_escape(str(value))))
    lines.append("+++")
    lines.append("")
    dest.parent.mkdir(parents=True, exist_ok=True)
    dest.write_text("\n".join(lines) + raw_wrap(body), encoding="utf-8")


def main() -> int:
    if not DOCS.is_dir():
        print("no docs/ next to www/ -- run this from a Cupid checkout", file=sys.stderr)
        return 1

    if CONTENT.exists():
        shutil.rmtree(CONTENT)
    CONTENT.mkdir(parents=True)

    # The docs index becomes the /docs/ landing page.
    title, body = split_title((DOCS / "README.md").read_text(encoding="utf-8"))
    write_page(
        CONTENT / "_index.md",
        {
            "title": title or "Documentation",
            "sort_by": "weight",
            "template": "docs-home.html",
            "page_template": "docs-page.html",
        },
        rewrite_links(body, Path("README.md")),
    )

    count = 1
    for weight, (name, label, blurb) in enumerate(SECTIONS, start=1):
        src_dir = DOCS / name
        if not src_dir.is_dir():
            print("skipping missing section docs/%s" % name, file=sys.stderr)
            continue

        pages = sorted(p for p in src_dir.glob("*.md") if p.name != "README.md")
        names = [p.name for p in pages]

        # A section README, where there is one, is the section's own page.
        readme = src_dir / "README.md"
        if readme.exists():
            sec_title, sec_body = split_title(readme.read_text(encoding="utf-8"))
            sec_body = rewrite_links(sec_body, Path(name) / "README.md")
        else:
            sec_title, sec_body = label, ""

        write_page(
            CONTENT / name / "_index.md",
            {
                "title": sec_title or label,
                "weight": weight,
                "sort_by": "weight",
                "template": "docs-section.html",
                "page_template": "docs-page.html",
                "extra": {"blurb": blurb, "nav": label},
            },
            sec_body,
        )

        for page in pages:
            page_title, page_body = split_title(page.read_text(encoding="utf-8"))
            write_page(
                CONTENT / name / page.name,
                {
                    "title": page_title or page.stem,
                    "weight": weight_of(name, page.name, names),
                },
                rewrite_links(page_body, Path(name) / page.name),
            )
            count += 1

    # The logo lives in assets/ so the README can use it too.
    STATIC.mkdir(parents=True, exist_ok=True)
    for svg in sorted(ASSETS.glob("*.svg")):
        shutil.copy2(svg, STATIC / svg.name)

    print("synced %d pages from docs/ into content/docs/" % count)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
