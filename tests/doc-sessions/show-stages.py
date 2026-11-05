# Show what total in Total.hs is made of at each stage of loading and running it.
# Run under gdb, in this directory, on a build with debug information:
#
#   gdb -q -batch -x show-stages.py --args cupid Total.hs
#
# It prints the tokens the lexer returns, then the second equation of total as cells:
# a pair as (fst . snd), a pair with a tag as its first component as TAG(snd) or, for
# a box, TAG value, a name by its text, and any other cell by what it is.  The numbers
# of heap cells and of table entries are left out, since they change between builds.
import gdb

for setting in ("pagination off", "confirm off", "breakpoint pending on"):
    gdb.execute("set " + setting)


def value(expr):
    return int(gdb.parse_and_eval(expr))


def text(t):
    return gdb.parse_and_eval("textToStr(%d)" % t).string()


def tag_name(c):
    return str(gdb.parse_and_eval("(CellTag)%d" % c))


class Ranges:
    """The cell range of each table, read from the table that whatIs consults."""

    def __init__(self):
        # Other files see a declaration without a size, so ask the file that defines it.
        size = value("sizeof('storage.c'::whatCode)") // value("sizeof(whatCode[0])")
        width = value("sizeof(whatCode[0])")
        memory = gdb.selected_inferior().read_memory(value("(long)&whatCode[0]"), size * width)
        self.codes = [int.from_bytes(memory[i * width:(i + 1) * width], "little")
                      for i in range(size)]
        self.start = {}
        for c in range(size - 1):
            kind = self.codes[c]
            # A tag's own code stands alone; a table's range is a run of them.
            if kind == self.codes[c + 1] and kind not in self.start:
                self.start[kind] = c
        self.int_kind = value("(int)INTCELL")
        self.kinds = {tag_name(k): k for k in set(self.codes)}

    def kind(self, c):
        return tag_name(self.codes[c])

    def index(self, c):
        return c - self.start[self.codes[c]]


def show(c, ranges):
    if c == 0:
        return "NIL"
    if c > 0:
        kind = ranges.kind(c)
        if kind == "NAME":
            return text(value("tabName[%d].text" % ranges.index(c)))
        if kind == "TYCON":
            return "Tycon:" + text(value("tabTycon[%d].text" % ranges.index(c)))
        if kind == "CLASS":
            return "Class:" + text(value("tabClass[%d].text" % ranges.index(c)))
        if kind == "OFFSET" and ranges.index(c) >= 0 and c != ranges.kinds["OFFSET"]:
            return "OFFSET %d" % ranges.index(c)
        if kind == "INTCELL" and c != ranges.int_kind:
            top = len(ranges.codes) - 1  # INTMAX; storage.h has the rest of this
            return "int %d" % (c - ((top - 4096) // 2 + top // 2))
        return kind
    head, tail = value("heapTopFst[%d]" % c), value("heapTopSnd[%d]" % c)
    if head > 0 and head == ranges.codes[head] and head not in ranges.start.values():
        tag = tag_name(head)
        if tag.endswith("CELL") or tag == "DICTVAR":
            if tag == "INTCELL":
                return "INTCELL %d" % tail
            return "%s %s" % (tag, text(tail))
        return "%s(%s)" % (tag, show(tail, ranges))
    return "(%s . %s)" % (show(head, ranges), show(tail, ranges))


def nth(cell, n):
    for _ in range(n):
        cell = value("heapTopSnd[%d]" % cell)
    return value("heapTopFst[%d]" % cell)


def fst(c):
    return value("heapTopFst[%d]" % c)


def snd(c):
    return value("heapTopSnd[%d]" % c)


# --- the tokens ------------------------------------------------------------

NAMES = {"WHERE": "where", "TMODULE": "module", "COCO": "::", "ARROW": "->", "IMPLIES": "=>"}
gdb.execute("break parseScript")
gdb.execute("run")
while not gdb.parse_and_eval("nm").string().endswith("Total.hs"):
    gdb.execute("continue")
gdb.execute("delete")
token_names = {f.enumval: f.name for f in gdb.lookup_type("enum yytokentype").fields()}
gdb.Breakpoint("yylex")
tokens = []
while True:
    gdb.execute("continue", to_string=True)
    gdb.execute("finish", to_string=True)
    t = int(gdb.parse_and_eval("$"))
    if t == 0:
        break
    tokens.append(NAMES.get(token_names.get(t, ""), token_names.get(t, chr(t))))
gdb.execute("delete")
print("--- tokens")
print(" ".join(tokens[1:]))  # the first is the marker SCRIPT, which is not in the file

# --- the second equation of total, stage by stage ---------------------------

ranges = Ranges()


def stage(name, cell):
    print("--- " + name)
    print(show(cell, ranges))


# Parsed: valDefns is a list of declarations, in reverse, and the equation is the
# third of them.  Its right-hand side is the pair (line, expression).
gdb.Breakpoint("checkDefns")
gdb.execute("continue", to_string=True)
gdb.execute("delete")
stage("parsed", snd(snd(snd(nth(value("valDefns"), 2)))))

# Resolved: valDefns is a list of groups; the first group is total, and the
# binding holds its alternatives after its name and its signature.
gdb.Breakpoint("typeCheckDefns")
gdb.execute("continue", to_string=True)
gdb.execute("delete")


def right_hand_side():
    binding = fst(fst(value("valDefns")))
    alternatives = snd(snd(binding))
    return snd(snd(nth(alternatives, 1)))


stage("resolved", right_hand_side())

gdb.Breakpoint("compileDefns")
gdb.execute("continue", to_string=True)
gdb.execute("delete")
binding = fst(fst(value("valDefns")))
stage("typed", snd(snd(nth(snd(binding), 1))))

# Compiled: what the code generator receives is a CASE with this as its second branch.
gdb.Breakpoint("codeGen")
while True:
    gdb.execute("continue", to_string=True)
    if value("n") and text(value("tabName[%d].text" % ranges.index(value("n")))) == "total":
        break
gdb.execute("delete")
alternatives = snd(snd(value("e")))
branch = next(snd(fst_alt) for fst_alt in (nth(alternatives, 0), nth(alternatives, 1))
              if ranges.kind(fst(fst_alt)) == "NAME"
              and text(value("tabName[%d].text" % ranges.index(fst(fst_alt)))) == ":")
stage("compiled", branch)

# Running: the root of the first call of total, after its code has overwritten it.
gdb.execute("finish", to_string=True)
code = int(gdb.parse_and_eval("$"))
gdb.Breakpoint("run", temporary=True).condition = "start == %d" % code
gdb.execute("continue", to_string=True)
root = value("root")
gdb.execute("finish", to_string=True)
result = value("cellStack[%d]" % root)
stage("running", result)
gdb.execute("kill")
