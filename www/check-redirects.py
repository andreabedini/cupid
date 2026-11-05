#!/usr/bin/env python3
"""Check that every address in old-urls.txt still leads somewhere in a built site.

usage: check-redirects.py PUBLIC

old-urls.txt lists the pages and heading anchors the site had before its sections
moved.  For each one this follows the redirect page, if there is one, as a browser
with JavaScript would, and checks that the page it lands on exists and has the
anchor.  It works on the files `zola build` produced, so it needs no server.
"""
import json
import pathlib
import posixpath
import re
import sys

public = pathlib.Path(sys.argv[1])
here = pathlib.Path(__file__).resolve().parent


def resolve(url):
    """(file, anchor) that a browser reaches from a site-relative address."""
    path, _, anchor = url.partition("#")
    for _ in range(3):
        page = public / path / "index.html"
        if not page.exists():
            return None, anchor
        text = page.read_text(encoding="utf-8")
        if "location.replace" not in text:
            return page, anchor
        table = json.loads(re.search(r"var moved = (\{.*?\});", text).group(1))
        target = table.get(anchor) or re.search(r'rel="canonical" href="([^"]*)"', text).group(1)
        target, _, new_anchor = target.partition("#")
        path = posixpath.normpath(posixpath.join(path, target))
        anchor = new_anchor or (anchor if anchor not in table else "")
    return None, anchor


bad = total = moved = 0
for line in (here / "old-urls.txt").read_text(encoding="utf-8").splitlines():
    if not line or line.startswith("#"):
        continue
    total += 1
    page, anchor = resolve(line)
    if page is None:
        print("no page for", line)
        bad += 1
    elif anchor and ('id="%s"' % anchor) not in page.read_text(encoding="utf-8"):
        print("no anchor for", line, "on", page.relative_to(public))
        bad += 1
    elif "/" + str(page.relative_to(public)) != "/" + line.partition("#")[0] + "index.html":
        moved += 1
print("%d old addresses, %d redirected, %d broken" % (total, moved, bad))
sys.exit(1 if bad else 0)
