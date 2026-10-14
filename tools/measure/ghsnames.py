"""Names in the two executables: Green Hills (the original) and Itanium (RedHills) mangling.

The Green Hills demangler is the cfront-style scheme the original was built with: `name__<scope>F<args>`,
`__ct`/`__dt` for constructors and destructors, `Q<n>_` for nested scopes, `__tm__` template arguments
and `__CPR` name compression. Itanium names are read with c++filt. Both are reduced to (scope, name,
parameters) so a function of one build can be paired with the same function of the other.
"""
import collections
import os
import re
import shutil
import subprocess
from pathlib import Path


BUILTIN = {
    "v": "void", "c": "char", "s": "short", "i": "int", "l": "long",
    "x": "long long", "f": "float", "d": "double", "b": "bool",
    "w": "wchar_t", "e": "...",
    # Green Hills mangles the 64-bit integer type as `L` (observed: layer
    # seeds `__ct__15OceanMixerLayerFLQ2_...` take the world seed, a 64-bit
    # value passed in a register pair).
    "L": "long long",
}

SPECIAL = {
    "__ct": "{ctor}", "__dt": "{dtor}", "__as": "operator=", "__eq": "operator==",
    "__ne": "operator!=", "__lt": "operator<", "__gt": "operator>",
    "__le": "operator<=", "__ge": "operator>=", "__pl": "operator+",
    "__mi": "operator-", "__ml": "operator*", "__dv": "operator/",
    "__md": "operator%", "__nw": "operator new", "__dl": "operator delete",
    "__vn": "operator new[]", "__vd": "operator delete[]",
    "__apl": "operator+=", "__ami": "operator-=", "__amu": "operator*=",
    "__adv": "operator/=", "__rf": "operator->", "__cl": "operator()",
    "__vc": "operator[]", "__nt": "operator!", "__aa": "operator&&",
    "__oo": "operator||", "__pp": "operator++", "__mm": "operator--",
    "__as__": "operator=",
}

_DIGITS = re.compile(r"(\d+)")

class Demangler:
    """A tolerant CodeWarrior/MetroWerks PPC demangler.

    Tolerant is the operative word: anything it cannot parse it returns
    verbatim rather than guessing, because a wrong signature here would be
    indistinguishable from a right one downstream.
    """

    def __init__(self, text):
        self.text = text
        self.pos = 0
        self.args = []

    def eof(self):
        return self.pos >= len(self.text)

    def peek(self):
        return self.text[self.pos] if not self.eof() else ""

    def take(self):
        char = self.text[self.pos]
        self.pos += 1
        return char

    def number(self):
        match = _DIGITS.match(self.text[self.pos:])
        if not match:
            return None
        self.pos += len(match.group(1))
        return int(match.group(1))

    def name(self):
        """A length-prefixed component, template suffix included."""
        length = self.number()
        if length is None:
            return None
        raw = self.text[self.pos:self.pos + length]
        self.pos += length
        return self.template(raw)

    @staticmethod
    def template(raw):
        """`basic_string__tm__58_c...` -> `basic_string<char, ...>`.

        The argument blob is decoded with the same type parser. It is often
        truncated by its own length prefix, so a blob that does not parse
        cleanly is left verbatim rather than half-decoded into a guess.
        """
        if "__tm__" not in raw:
            return raw
        head, _, tail = raw.partition("__tm__")
        inner = tail.split("_", 1)[1] if "_" in tail else tail
        probe = Demangler(inner)
        parts = []
        while not probe.eof():
            before = probe.pos
            one = probe.type()
            if one is None or probe.pos == before:
                parts = None
                break
            parts.append(one)
        if parts:
            return "%s<%s>" % (head, ", ".join(parts))
        return "%s<%s>" % (head, inner)

    def scope(self):
        """A possibly-nested class scope."""
        if self.peek() == "Q":
            self.take()
            count = self.number()
            if count is None:
                return None
            if self.peek() == "_":
                self.take()
            parts = []
            for _ in range(count):
                part = self.name()
                if part is None:
                    return None
                parts.append(part)
            return "::".join(parts)
        return self.name()

    def type(self):
        """One type, or None if unparseable.

        Modifiers nest rather than accumulate: `RC<T>` is a reference to a
        const T and prints `const T&`, not `T& const`. Parsing recursively is
        the only way to get that order right.
        """
        if self.eof():
            return None
        char = self.peek()
        if char == "P":
            self.take()
            if self.peek() == "F":
                function = self.function()
                return None if function is None else "%s(*)(%s)" % function
            inner = self.type()
            return None if inner is None else inner + "*"
        if char == "F":
            function = self.function()
            return None if function is None else "%s(%s)" % function
        if char == "R":
            self.take()
            inner = self.type()
            return None if inner is None else inner + "&"
        if char == "C":
            self.take()
            inner = self.type()
            return None if inner is None else "const " + inner
        if char == "V":
            self.take()
            inner = self.type()
            return None if inner is None else "volatile " + inner
        if char == "U":
            self.take()
            inner = self.type()
            return None if inner is None else "unsigned " + inner
        if char == "Z":
            # Template-parameter back-reference (`Z1Z`); kept symbolic.
            self.take()
            index = self.number()
            if index is None or self.eof() or self.peek() != "Z":
                return None
            self.take()
            return "Z%dZ" % index
        if char == "S":
            self.take()
            inner = self.type()
            return None if inner is None else "signed " + inner
        if char == "A":
            self.take()
            extent = self.number()
            if self.peek() == "_":
                self.take()
            inner = self.type()
            if inner is None:
                return None
            return "%s[%s]" % (inner, extent if extent is not None else "")
        if char == "M":
            self.take()
            owner = self.scope()
            if owner is None:
                return None
            if self.peek() == "C" and self.text[self.pos + 1:self.pos + 2] == "F":
                self.take()
                function = self.function()
                return None if function is None else "%s(%s::*)(%s) const" % (function[0], owner, function[1])
            if self.peek() == "F":
                function = self.function()
                return None if function is None else "%s(%s::*)(%s)" % (function[0], owner, function[1])
            inner = self.type()
            if inner is None:
                return None
            return "%s %s::*" % (inner, owner)
        if char in BUILTIN:
            self.take()
            return BUILTIN[char]
        if char == "Q" or char.isdigit():
            return self.scope()
        return None

    def function(self):
        """`F<args>_<return>` -> `(return, "args")`, or None if unparseable.

        Back-references inside the nested list index that list, so it is read
        with its own `arglist`.
        """
        if self.peek() != "F":
            return None
        self.take()
        args = self.arglist()
        if args is None or self.eof() or self.peek() != "_":
            return None
        self.take()
        result = self.type()
        if result is None:
            return None
        return result, ", ".join(a for a in args if a != "void")

    def arglist(self):
        """Every argument up to the return-type separator or end of string."""
        out = []
        while not self.eof():
            char = self.peek()
            if char == "_":
                # Return type separator; the caller decides whether to read it.
                break
            if char == "T":
                self.take()
                index = self.number()
                if index is None or index < 1 or index > len(out):
                    return None
                out.append(out[index - 1])
                continue
            if char == "N":
                self.take()
                start = self.pos
                count = self.number()
                if count is None:
                    return None
                # `N<count>_<index>` in long form; otherwise two packed single digits, and any
                # digits after them begin the next parameter's length prefix.
                if self.peek() == "_":
                    self.take()
                    index = self.number()
                else:
                    text = str(count)
                    if len(text) < 2:
                        return None
                    count, index = int(text[0]), int(text[1])
                    self.pos = start + 2
                if index is None or index < 1 or index > len(out):
                    return None
                out.extend([out[index - 1]] * count)
                continue
            before = self.pos
            one = self.type()
            if one is None or self.pos == before:
                return None
            out.append(one)
        return out


_CPR = re.compile(r"^__CPR\d+__(.*)$", re.S)
_CPR_REF = re.compile(r"J(\d+)J")
_CPR_LEN = re.compile(r"(\d+)")


def decompress(mangled):
    """Expand Green Hills `__CPR` name compression.

    `__CPR<len>__<text>` where `J<n>J` in the text is a back-reference to the
    length-prefixed component that starts at offset <n> of the *decompressed*
    string (digits included). `JJ` encodes a literal `J`. Anything that does not
    resolve cleanly returns the original name untouched - a wrong expansion is
    worse than a mangled one.
    """
    match = _CPR.match(mangled)
    if not match:
        return mangled
    body = match.group(1)
    out = []
    pos = 0
    while pos < len(body):
        ref = _CPR_REF.match(body, pos)
        if ref:
            offset = int(ref.group(1))
            current = "".join(out)
            if offset >= len(current):
                return mangled
            length = _CPR_LEN.match(current, offset)
            if not length:
                return mangled
            end = length.end() + int(length.group(1))
            if end > len(current):
                return mangled
            out.append(current[offset:end])
            pos = ref.end()
        elif body.startswith("JJ", pos):
            out.append("J")
            pos += 2
        else:
            out.append(body[pos])
            pos += 1
    return "".join(out)


def demangle(mangled):
    """`(class, method, signature)` for a member function, else `(None, symbol, '')`.

    The name is everything before the `__` that introduces a scope followed by
    `F` or `C`; a leading `__ct`/`__dt`/`__as` is the special-name form.
    Static members carry `S` before the `F` (`allocate__8IntCacheSFi`).
    """
    mangled = decompress(mangled)
    at = mangled.find("__")
    while at != -1:
        head, rest = mangled[:at], mangled[at + 2:]
        probe = Demangler(rest)
        scope = probe.scope()
        if scope and not probe.eof() and probe.peek() in "FCS":
            const = False
            static = False
            if probe.peek() == "S":
                probe.take()
                static = True
            if probe.peek() == "C":
                probe.take()
                const = True
            if probe.eof() or probe.peek() != "F":
                at = mangled.find("__", at + 1)
                continue
            probe.take()
            args = probe.arglist()
            method = SPECIAL.get(head, head) if head.startswith("__") else head
            if not head:
                method = "{unnamed}"
            if args is None:
                return scope, method, ""
            shown = ", ".join(a for a in args if a != "void")
            suffix = (" const" if const else "") + (" [static]" if static else "")
            return scope, method, "(%s)%s" % (shown, suffix)
        at = mangled.find("__", at + 1)
    return None, mangled, ""



OPERATORS = dict((k, v) for k, v in SPECIAL.items() if v.startswith('operator'))
OPERATORS.update({'__nwa': 'operator new[]', '__dla': 'operator delete[]', '__aad': 'operator&=',
                  '__aor': 'operator|=', '__aer': 'operator^=', '__als': 'operator<<=', '__ars': 'operator>>=',
                  '__amd': 'operator%=', '__ls': 'operator<<', '__rs': 'operator>>', '__ad': 'operator&',
                  '__or': 'operator|', '__er': 'operator^', '__co': 'operator~', '__cm': 'operator,',
                  '__rm': 'operator->*'})
LIBRARY_ROOTS = ('std', 'boost', '__cxxabiv1', '__gnu_cxx', 'rapidxml')
CLONE = re.compile(r'\.(?:specialized|constprop|isra|part|cold|llvm)\.\d+$|\.cold$')
THUNK = re.compile(r'^_ZThn(\d+)_')
THUNK_TEXT = 'non-virtual thunk to '


def split_top(text, sep):
    """Split at sep where it is outside <> and ()."""
    parts, depth, start, i = [], 0, 0, 0
    while i < len(text):
        c = text[i]
        if c in '<(':
            depth += 1
        elif c in '>)':
            depth -= 1
        elif depth == 0 and text.startswith(sep, i):
            parts.append(text[start:i])
            i += len(sep)
            start = i
            continue
        i += 1
    parts.append(text[start:])
    return parts


POSTFIX_CONST = re.compile(r'([\w:]+(?:<[^<>]*(?:<[^<>]*>[^<>]*)*>)?) const\b')


def norm_type(text):
    """A parameter type in one spelling: `T const` as `const T` (c++filt writes the first, the Green
    Hills demangler the second), without `volatile`, elaborated keywords or spaces."""
    text = POSTFIX_CONST.sub(r'const \1', text)
    text = re.sub(r'\bvolatile\b|\bstruct\b|\bclass\b|\benum\b', '', text)
    text = text.replace('std::__1::', 'std::').replace('std::__cxx11::', 'std::')
    return re.sub(r'\s+', '', text)


def norm_scope(scope):
    return None if scope is None else norm_type(scope)


def scope_leaf(scope):
    """The innermost class of a scope; a local class is named by its own name on both sides."""
    if scope is None:
        return None
    scope = re.sub(r'__L\d+__.*$', '', scope)
    return split_top(scope, '::')[-1]


def ghs_key(mangled):
    """(scope, name, parameters) of a Green Hills name, parameters None when they do not parse; None
    for a name that is not mangled."""
    scope, method, sig = demangle(mangled)
    if scope is not None:
        params = None
        if sig.startswith('('):
            params = tuple(norm_type(p) for p in split_top(sig[1:sig.rfind(')')], ',') if p.strip())
        return scope, OPERATORS.get(method, method), params
    text = decompress(mangled)
    at = text.find('__F')
    while at > 0:
        probe = Demangler(text[at + 3:])
        args = probe.arglist()
        if args is not None and probe.eof():
            head = text[:at]
            return None, OPERATORS.get(head, head), tuple(norm_type(a) for a in args if a != 'void')
        at = text.find('__F', at + 1)
    return None


def itanium_key(text):
    """(scope, name, parameters) from demangled Itanium text, or None for a plain (C) name."""
    text = re.sub(r'\[abi:\w+\]', '', text)
    if '(' not in text:
        return None
    depth, open_at = 0, None
    for i in range(len(text) - 1, -1, -1):
        if text[i] == ')':
            depth += 1
        elif text[i] == '(':
            depth -= 1
            if depth == 0:
                open_at = i
                break
    if open_at is None:
        return None
    head, args = text[:open_at], text[open_at + 1:]
    args = args[:args.rfind(')')]
    head = head.replace('(anonymous namespace)::', '')
    parts = split_top(head, '::')
    name, scope = parts[-1], '::'.join(parts[:-1]) or None
    leaf = re.sub(r'<.*>$', '', parts[-2]) if len(parts) > 1 else None
    if leaf and name == leaf:
        name = '{ctor}'
    elif leaf and name == '~' + leaf:
        name = '{dtor}'
    params = [norm_type(p) for p in split_top(args, ',') if p.strip() and p.strip() != 'void']
    return scope, name, tuple(params)


def library_scope(scope):
    """Whether a scope belongs to the standard library or boost, whose code each compiler brings."""
    if not scope:
        return False
    root = re.sub(r'<.*$', '', split_top(norm_type(scope), '::')[0])
    return root in LIBRARY_ROOTS


BUILD_IMAGE = 'minecraft-wiiu-redhills'
_CXXFILT = []


def cxxfilt_command():
    """A command that demangles Itanium names line by line: CXXFILT, llvm-cxxfilt or c++filt on the
    machine, else c++filt inside the toolchain image tools/build.py builds."""
    if _CXXFILT:
        return _CXXFILT[0]
    for candidate in (os.environ.get('CXXFILT'), shutil.which('llvm-cxxfilt'), shutil.which('c++filt'),
                      shutil.which('powerpc-eabi-c++filt'), r'C:\Program Files\LLVM\bin\llvm-cxxfilt.exe'):
        if candidate and Path(candidate).exists():
            _CXXFILT.append([candidate])
            return _CXXFILT[0]
    if shutil.which('docker'):
        listed = subprocess.run(['docker', 'images', '--format', '{{.Repository}}:{{.Tag}}', BUILD_IMAGE],
                                capture_output=True, text=True)
        tags = [line for line in listed.stdout.split() if line.startswith(BUILD_IMAGE + ':')]
        if listed.returncode == 0 and tags:
            _CXXFILT.append(['docker', 'run', '--rm', '-i', tags[0], 'c++filt'])
            return _CXXFILT[0]
    raise SystemExit('no C++ demangler found: install llvm-cxxfilt (LLVM) or c++filt (binutils), set CXXFILT, '
                     'or build once with Docker so its toolchain image can be used')


def demangle_itanium(names):
    """{mangled: demangled text} through c++filt."""
    names = sorted(set(names))
    if not names:
        return {}
    out = subprocess.run(cxxfilt_command(), input='\n'.join(names) + '\n', capture_output=True, text=True,
                         check=True).stdout.splitlines()
    if len(out) != len(names):
        raise SystemExit('c++filt returned %d lines for %d names' % (len(out), len(names)))
    return dict(zip(names, out))


def keys(scope, method, params):
    """Lookup keys from most to least specific: full scope with parameters, full scope with the
    parameter count, full scope alone, then the innermost class with parameters and with the count."""
    full, leaf = norm_scope(scope), scope_leaf(norm_scope(scope))
    known = params is not None
    return [(full, method, params) if known else None,
            (full, method, len(params)) if known else None,
            (full, method),
            (leaf, method, params) if known else None,
            (leaf, method, len(params)) if known else None]


class NameMap(object):
    """Pairs Itanium names of the rebuilt executable with Green Hills names of the original. A name
    maps only when one original is the unique candidate at the most specific key that has any."""

    def __init__(self, originals):
        self.originals = set(originals)
        self.tables = [collections.defaultdict(set) for _ in range(5)]
        for name in self.originals:
            key = ghs_key(name)
            if key is None:
                continue
            for table, k in zip(self.tables, keys(*key)):
                if k is not None:
                    table[k].add(name)
        self.mapped = {}
        self.texts = {}

    def prime(self, names):
        todo = [n for n in set(names) if n not in self.mapped]
        bare = {n: CLONE.sub('', n) for n in todo}
        self.texts.update(demangle_itanium(b for b in bare.values() if b.startswith('_Z')))
        for name in todo:
            self.mapped[name] = self._lookup(bare[name])

    def _lookup(self, name):
        if name in self.originals:
            return name
        text = self.texts.get(name)
        if text is None:
            return None
        thunk = THUNK.match(name)
        if thunk:
            if not text.startswith(THUNK_TEXT):
                return None
            text = text[len(THUNK_TEXT):]
        key = itanium_key(text)
        if key is None:
            return text if text in self.originals else None
        scope, method, params = key
        if thunk:
            method = '__ghs_thunk__0x%08x__%s' % (-int(thunk.group(1)) & 0xffffffff, method)
        for table, k in zip(self.tables, keys(scope, method, params)):
            hits = table.get(k)
            if hits:
                return next(iter(hits)) if len(hits) == 1 else None
        return None

    def __call__(self, name):
        if name not in self.mapped:
            self.prime([name])
        return self.mapped[name]

    def library(self, name):
        """Whether an Itanium name belongs to the standard library or boost."""
        text = self.texts.get(CLONE.sub('', name))
        if text is None:
            return False
        if text.startswith(THUNK_TEXT):
            text = text[len(THUNK_TEXT):]
        key = itanium_key(text)
        if key is not None and library_scope(key[0]):
            return True
        return re.match(r'^(?:[\w:<>,* ]+ )?(?:std|__gnu_cxx|boost|__cxxabiv1|rapidxml)::', text) is not None


def ghs_library(name):
    """Whether a Green Hills name belongs to the standard library, boost or the compiler runtime."""
    if name.startswith(('__ghs', '__sti__', '__gh_')):
        return True
    key = ghs_key(name)
    return key is not None and library_scope(key[0])
