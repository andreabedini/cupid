#!/usr/bin/env python3
"""A small traditional-style C preprocessor for Haskell sources.

The first stage of the library bootstrap has to preprocess the base,
haskell98 and Cabal packages before cpphs exists, since cpphs is itself
built with Cabal running under Hugs.  That stage used to run the C
compiler's preprocessor in traditional mode, which works with gcc but
not with clang, whose preprocessor rejects Haskell lines such as `#-}`.

This is the subset of cpp those sources actually use, with the lenient
behaviour of `gcc -E -traditional-cpp`:

  #include "file" / <file>      #define NAME body
  #define NAME(a,b) body        #undef NAME
  #if expr  #ifdef  #ifndef      #elif  #else  #endif   #error

Expressions understand defined(), integers, arithmetic, comparisons and
logical operators.  Object-like and function-like macros are expanded
outside double-quoted strings, C block comments are removed, and a `#`
line that is not a directive is passed through unchanged.  No `##` or
`#` stringification, which the Haskell sources do not use.

usage: hscpp.py [-Dname[=value]] [-Idir] file
"""
import os
import re
import sys

IDENT = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
DIRECTIVE = re.compile(r"^[ \t]*#[ \t]*([a-z]+)\b[ \t]*(.*)$")
DIRECTIVES = {"include", "define", "undef", "if", "ifdef", "ifndef",
              "elif", "else", "endif", "error", "warning", "line", "pragma"}


class Macro:
    def __init__(self, params, body):
        self.params = params        # None for object-like macros
        self.body = body


class Preprocessor:
    def __init__(self, includes):
        self.includes = includes
        self.macros = {}
        self.out = []

    # -- macro expansion --------------------------------------------------

    def expand(self, text, active=frozenset()):
        """Expand macros in TEXT.  ACTIVE holds the names being expanded,
        to stop self-referential macros from recursing."""
        result = []
        i = 0
        n = len(text)
        while i < n:
            c = text[i]
            if c == '"':
                j = i + 1
                while j < n and text[j] != '"':
                    j += 2 if text[j] == "\\" else 1
                result.append(text[i:j + 1])
                i = j + 1
                continue
            m = IDENT.match(text, i)
            if not m:
                result.append(c)
                i += 1
                continue
            name = m.group(0)
            i = m.end()
            macro = self.macros.get(name)
            if macro is None or name in active:
                result.append(name)
                continue
            if macro.params is None:
                result.append(self.expand(macro.body, active | {name}))
                continue
            # Function-like: only an invocation if a '(' follows
            j = i
            while j < n and text[j] in " \t":
                j += 1
            if j >= n or text[j] != "(":
                result.append(name)
                continue
            args, i = self.parse_args(text, j)
            if args is None:
                result.append(name)
                continue
            body = self.substitute(macro, args)
            result.append(self.expand(body, active | {name}))
        return "".join(result)

    def unfinished_call(self, text):
        """True if TEXT opens an argument list of a function-like macro
        that is not closed on this line."""
        i = 0
        n = len(text)
        while i < n:
            if text[i] == '"':
                j = i + 1
                while j < n and text[j] != '"':
                    j += 2 if text[j] == "\\" else 1
                i = j + 1
                continue
            m = IDENT.match(text, i)
            if not m:
                i += 1
                continue
            i = m.end()
            macro = self.macros.get(m.group(0))
            if macro is None or macro.params is None:
                continue
            j = i
            while j < n and text[j] in " \t":
                j += 1
            if j < n and text[j] == "(":
                args, end = self.parse_args(text, j)
                if args is None:
                    return True
                i = end
        return False

    @staticmethod
    def parse_args(text, start):
        """Split the argument list beginning at text[start] == '('.
        Returns (args, index after ')') or (None, start) if unterminated."""
        depth = 0
        args = []
        cur = []
        i = start
        n = len(text)
        while i < n:
            c = text[i]
            if c == '"':
                j = i + 1
                while j < n and text[j] != '"':
                    j += 2 if text[j] == "\\" else 1
                cur.append(text[i:j + 1])
                i = j + 1
                continue
            if c == "(":
                depth += 1
                if depth > 1:
                    cur.append(c)
            elif c == ")":
                depth -= 1
                if depth == 0:
                    args.append("".join(cur).strip())
                    return args, i + 1
                cur.append(c)
            elif c == "," and depth == 1:
                args.append("".join(cur).strip())
                cur = []
            else:
                cur.append(c)
            i += 1
        return None, start

    @staticmethod
    def substitute(macro, args):
        params = macro.params
        if len(args) == 1 and args[0] == "" and not params:
            args = []
        if len(args) != len(params):
            raise SystemExit("hscpp: macro expects %d arguments, got %d"
                             % (len(params), len(args)))
        table = dict(zip(params, args))
        # Traditional cpp substitutes parameters even inside strings
        return IDENT.sub(lambda m: table.get(m.group(0), m.group(0)), macro.body)

    # -- #if expressions --------------------------------------------------

    def evaluate(self, expr):
        expr = re.sub(r"defined[ \t]*\([ \t]*([A-Za-z_]\w*)[ \t]*\)",
                      lambda m: "1" if m.group(1) in self.macros else "0", expr)
        expr = re.sub(r"defined[ \t]+([A-Za-z_]\w*)",
                      lambda m: "1" if m.group(1) in self.macros else "0", expr)
        expr = self.expand(expr)
        tokens = re.findall(r"0[xX][0-9a-fA-F]+|\d+|[A-Za-z_]\w*|"
                            r"&&|\|\||==|!=|<=|>=|<<|>>|[-+*/%<>!&|^~()?:]", expr)
        self.tokens = tokens
        self.pos = 0
        value = self.ternary()
        if self.pos != len(tokens):
            raise SystemExit("hscpp: cannot parse #if expression: %s" % expr)
        return value

    def peek(self):
        return self.tokens[self.pos] if self.pos < len(self.tokens) else None

    def take(self):
        tok = self.peek()
        self.pos += 1
        return tok

    BINARY = [
        ("||",), ("&&",), ("|",), ("^",), ("&",), ("==", "!="),
        ("<", "<=", ">", ">="), ("<<", ">>"), ("+", "-"), ("*", "/", "%"),
    ]

    def ternary(self):
        cond = self.binary(0)
        if self.peek() == "?":
            self.take()
            a = self.ternary()
            if self.take() != ":":
                raise SystemExit("hscpp: bad ?: expression")
            b = self.ternary()
            return a if cond else b
        return cond

    def binary(self, level):
        if level == len(self.BINARY):
            return self.unary()
        left = self.binary(level + 1)
        while self.peek() in self.BINARY[level]:
            op = self.take()
            right = self.binary(level + 1)
            left = self.apply(op, left, right)
        return left

    @staticmethod
    def apply(op, a, b):
        if op == "||": return int(bool(a) or bool(b))
        if op == "&&": return int(bool(a) and bool(b))
        if op == "|": return a | b
        if op == "^": return a ^ b
        if op == "&": return a & b
        if op == "==": return int(a == b)
        if op == "!=": return int(a != b)
        if op == "<": return int(a < b)
        if op == "<=": return int(a <= b)
        if op == ">": return int(a > b)
        if op == ">=": return int(a >= b)
        if op == "<<": return a << b
        if op == ">>": return a >> b
        if op == "+": return a + b
        if op == "-": return a - b
        if op == "*": return a * b
        if op == "/": return int(a / b) if b else 0
        if op == "%": return a % b if b else 0
        raise SystemExit("hscpp: unknown operator " + op)

    def unary(self):
        tok = self.take()
        if tok is None:
            return 0                        # empty expression, e.g. #if X with X empty
        if tok == "!":
            return int(not self.unary())
        if tok == "-":
            return -self.unary()
        if tok == "+":
            return self.unary()
        if tok == "~":
            return ~self.unary()
        if tok == "(":
            value = self.ternary()
            if self.take() != ")":
                raise SystemExit("hscpp: missing ) in #if expression")
            return value
        if tok.startswith(("0x", "0X")):
            return int(tok, 16)
        if tok.isdigit():
            return int(tok)
        if IDENT.match(tok):
            return 0                        # undefined identifiers are 0
        raise SystemExit("hscpp: unexpected token %r in #if expression" % tok)

    # -- directives -------------------------------------------------------

    def define(self, arg):
        m = re.match(r"([A-Za-z_]\w*)(\(([^)]*)\))?[ \t]*(.*)$", arg, re.S)
        if not m:
            raise SystemExit("hscpp: bad #define " + arg)
        name, params, body = m.group(1), m.group(3), m.group(4)
        if params is not None:
            params = [p.strip() for p in params.split(",") if p.strip()]
        self.macros[name] = Macro(params, body.strip())

    def find_include(self, spec, current_dir):
        m = re.match(r'"([^"]+)"|<([^>]+)>', spec.strip())
        if not m:
            raise SystemExit("hscpp: bad #include " + spec)
        name = m.group(1) or m.group(2)
        dirs = ([current_dir] if m.group(1) else []) + self.includes
        for d in dirs:
            path = os.path.join(d, name)
            if os.path.isfile(path):
                return path
        raise SystemExit("hscpp: cannot find include file " + name)

    @staticmethod
    def strip_comments(text):
        """Remove C block comments outside double-quoted strings, as cpp
        does, keeping the newlines they span so line structure survives."""
        out = []
        i = 0
        n = len(text)
        while i < n:
            c = text[i]
            if c == '"':
                j = i + 1
                while j < n and text[j] != '"' and text[j] != "\n":
                    j += 2 if text[j] == "\\" else 1
                out.append(text[i:j + 1])
                i = j + 1
            elif c == "/" and text.startswith("/*", i):
                j = text.find("*/", i + 2)
                if j < 0:
                    j = n - 2
                out.append(" " + "\n" * text.count("\n", i, j + 2))
                i = j + 2
            else:
                out.append(c)
                i += 1
        return "".join(out)

    def process(self, path):
        with open(path, encoding="latin-1") as f:
            lines = self.strip_comments(f.read()).split("\n")
        current_dir = os.path.dirname(path) or "."
        # Stack of (this branch active, some branch already taken, parent active)
        stack = []
        active = True
        i = 0
        while i < len(lines):
            line = lines[i]
            i += 1
            m = DIRECTIVE.match(line)
            if m and m.group(1) in DIRECTIVES:
                name, arg = m.group(1), m.group(2)
                # Continuation lines belong to the directive
                while arg.endswith("\\") and i < len(lines):
                    arg = arg[:-1] + lines[i]
                    i += 1
                if name == "ifdef" or name == "ifndef":
                    taken = (arg.strip() in self.macros) == (name == "ifdef")
                    stack.append((active and taken, taken, active))
                    active = active and taken
                elif name == "if":
                    taken = bool(self.evaluate(arg)) if active else False
                    stack.append((active and taken, taken, active))
                    active = active and taken
                elif name == "elif":
                    _, done, parent = stack.pop()
                    taken = bool(self.evaluate(arg)) if (parent and not done) else False
                    stack.append((parent and taken, done or taken, parent))
                    active = parent and taken
                elif name == "else":
                    _, done, parent = stack.pop()
                    stack.append((parent and not done, True, parent))
                    active = parent and not done
                elif name == "endif":
                    if not stack:
                        raise SystemExit("hscpp: #endif without #if in " + path)
                    _, _, parent = stack.pop()
                    active = parent
                elif not active:
                    pass
                elif name == "define":
                    self.define(arg)
                elif name == "undef":
                    self.macros.pop(arg.strip(), None)
                elif name == "include":
                    self.process(self.find_include(arg, current_dir))
                elif name == "error":
                    raise SystemExit("%s: #error %s" % (path, arg))
                # warning, line and pragma are ignored
                continue
            if active:
                # A function-like macro invocation may continue on the
                # following lines; cpp joins them into one
                while self.unfinished_call(line) and i < len(lines):
                    line = line + " " + lines[i]
                    i += 1
                self.out.append(self.expand(line))
        if stack:
            raise SystemExit("hscpp: unterminated #if in " + path)


def main(argv):
    defines = []
    includes = []
    files = []
    for arg in argv:
        if arg.startswith("-D"):
            defines.append(arg[2:])
        elif arg.startswith("-I"):
            includes.append(arg[2:])
        elif arg.startswith("-"):
            raise SystemExit("hscpp: unknown option " + arg)
        else:
            files.append(arg)
    if len(files) != 1:
        raise SystemExit(__doc__)
    pp = Preprocessor(includes)
    for d in defines:
        name, _, value = d.partition("=")
        pp.macros[name] = Macro(None, value if "=" in d else "1")
    pp.process(files[0])
    sys.stdout.write("\n".join(pp.out))
    if pp.out:
        sys.stdout.write("\n")


if __name__ == "__main__":
    main(sys.argv[1:])
