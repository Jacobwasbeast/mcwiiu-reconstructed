"""Which functions the source tree defines, by (innermost class, name).

A function of the original with no copy in the rebuilt executable is either one the rebuilt compiler
inlined everywhere it was used or one the source does not have. This index answers which, from the
definitions in the source: `Class::name(...) {` out of line, a body inside a class or struct
declaration, and free functions. It reads text, not a parse, so templates and macros it cannot follow
are reported as not found rather than guessed.
"""
import re
from pathlib import Path

EXTENSIONS = ('.cpp', '.c', '.h', '.hpp', '.inl')
COMMENT = re.compile(r'//[^\n]*|/\*.*?\*/', re.S)
STRING = re.compile(r'"(?:\\.|[^"\\\n])*"|\'(?:\\.|[^\'\\\n])*\'')
QUALIFIED = re.compile(r'(\w+)\s*(?:<[^<>;{}()]*(?:<[^<>;{}()]*>[^<>;{}()]*)*>)?\s*::\s*(~?\s*\w+|operator\s*[^\s(]+)\s*\(')
CLASS_HEAD = re.compile(r'\b(?:class|struct|union)\s+(?:\w+\s+)*?(\w+)\s*(?:final\s*)?(?::[^{;]*)?\{')
MEMBER_BODY = re.compile(r'(~?\s*\w+|operator\s*[^\s(]+)\s*\(([^;{}]|\{\s*\})*?\)\s*(?:const\s*)?(?:throw\s*\([^)]*\)\s*)?'
                         r'(?::[^;{]*)?\{')
FREE_BODY = re.compile(r'(?:^|[;}\n])\s*(?:[\w:<>,*&\s]+?)\b(\w+)\s*\([^;{}()]*(?:\([^;{}()]*\)[^;{}()]*)*\)\s*\{')
SKIP_WORDS = {'if', 'for', 'while', 'switch', 'return', 'sizeof', 'catch', 'do', 'else', 'case'}


def strip(text):
    return COMMENT.sub(' ', STRING.sub('""', text))


def matching_brace(text, start):
    """Index just past the brace that closes the one at start."""
    depth = 0
    for i in range(start, len(text)):
        c = text[i]
        if c == '{':
            depth += 1
        elif c == '}':
            depth -= 1
            if depth == 0:
                return i + 1
    return len(text)


def norm(name):
    return re.sub(r'\s+', '', name)


def top_level(scope):
    """The `::` components of a scope, not splitting inside template arguments."""
    parts, depth, start = [], 0, 0
    for i, c in enumerate(scope):
        if c == '<':
            depth += 1
        elif c == '>':
            depth -= 1
        elif depth == 0 and scope.startswith('::', i):
            parts.append(scope[start:i])
            start = i + 2
    parts.append(scope[start:])
    return [p for p in parts if p]


class SourceIndex(object):
    def __init__(self, root):
        self.members = set()
        self.free = set()
        self.classes = set()
        for path in Path(root).rglob('*'):
            if path.suffix not in EXTENSIONS or not path.is_file():
                continue
            text = strip(path.read_text(encoding='utf-8', errors='replace'))
            for m in QUALIFIED.finditer(text):
                self.members.add((m.group(1), norm(m.group(2))))
            for m in CLASS_HEAD.finditer(text):
                self.classes.add(m.group(1))
                open_at = m.end() - 1
                body = text[open_at + 1:matching_brace(text, open_at) - 1]
                self._class_body(m.group(1), body)
            for m in FREE_BODY.finditer(text):
                if m.group(1) not in SKIP_WORDS:
                    self.free.add(m.group(1))

    def _class_body(self, name, body):
        """Member functions defined inside a class body, skipping nested bodies once seen."""
        i = 0
        while True:
            m = MEMBER_BODY.search(body, i)
            if not m:
                return
            member = norm(m.group(1))
            if member not in SKIP_WORDS:
                self.members.add((name, member))
            i = matching_brace(body, m.end() - 1)

    def defines(self, scope, method):
        """Whether the source defines a function of this (Green Hills) scope and name. A template
        member is found by its template's name; a constructor or destructor the source does not write
        is one the compiler generates for a class the source declares."""
        method = re.sub(r'__tm__.*$', '', method)
        if scope is None:
            return method in self.free
        leaf = re.sub(r'<.*$', '', top_level(re.sub(r'__L\d+__.*$', '', scope))[-1])
        if method in ('{ctor}', '{dtor}', 'operator='):
            return leaf in self.classes
        return (leaf, norm(method)) in self.members
