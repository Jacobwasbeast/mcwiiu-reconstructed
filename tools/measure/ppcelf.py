"""Read big-endian 32-bit PowerPC ELF files: Wii U RPX/RPL executables and relocatable objects.

Both kinds keep their symbol table and their relocations, which is what the measurement reads: a
relocation names the function a call reaches and the data an instruction loads. RPX sections flagged
SHF_RPL_ZLIB store a 4-byte big-endian inflated size followed by a zlib stream.
"""
import struct
import zlib

SHT_SYMTAB = 2
SHT_STRTAB = 3
SHT_RELA = 4
SHT_NOBITS = 8
SHF_EXECINSTR = 0x4
SHF_RPL_ZLIB = 0x08000000
STT_FUNC = 2
STT_SECTION = 3
STB_LOCAL = 0
STB_WEAK = 2

R_PPC_ADDR32 = 1
R_PPC_ADDR16_LO = 4
R_PPC_ADDR16_HI = 5
R_PPC_ADDR16_HA = 6
R_PPC_REL24 = 10
R_PPC_REL14 = 11


class ElfError(Exception):
    """The file is not a readable big-endian 32-bit ELF file."""


class Section(object):
    def __init__(self, index, name, kind, flags, address, data, link, info, entsize, size):
        self.index = index
        self.name = name
        self.kind = kind
        self.flags = flags
        self.address = address
        self.data = data
        self.link = link
        self.info = info
        self.entsize = entsize
        self.size = size

    @property
    def executable(self):
        return bool(self.flags & SHF_EXECINSTR) and self.kind != SHT_NOBITS


class Symbol(object):
    __slots__ = ('index', 'name', 'value', 'size', 'kind', 'bind', 'shndx')

    def __init__(self, index, name, value, size, kind, bind, shndx):
        self.index = index
        self.name = name
        self.value = value
        self.size = size
        self.kind = kind
        self.bind = bind
        self.shndx = shndx


class Elf(object):
    """Sections, symbols and relocations of one file. `relocatable` objects address code by section
    offset; executables by virtual address. Offsets below are always section offsets."""

    def __init__(self, image):
        if len(image) < 52 or image[:4] != b'\x7fELF':
            raise ElfError('not an ELF file')
        if image[4] != 1 or image[5] != 2:
            raise ElfError('not a 32-bit big-endian ELF file')
        (self.type,) = struct.unpack_from('>H', image, 0x10)
        self.relocatable = self.type == 1
        (shoff,) = struct.unpack_from('>I', image, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from('>HHH', image, 0x2E)
        if shentsize != 40 or shoff + shnum * shentsize > len(image):
            raise ElfError('bad section header table')
        headers = [struct.unpack_from('>10I', image, shoff + i * 40) for i in range(shnum)]
        raw = [self._bytes(image, h) for h in headers]
        names = raw[shstrndx] if shstrndx < len(raw) else b''
        self.sections = []
        for index, h in enumerate(headers):
            end = names.find(b'\0', h[0])
            name = names[h[0]:end if end >= 0 else len(names)].decode('ascii', 'replace')
            size = len(raw[index]) if raw[index] else h[5]
            self.sections.append(Section(index, name, h[1], h[2], h[3], raw[index], h[6], h[7], h[9], size))
        self.symbols = []
        self._relocs = {}
        for section in self.sections:
            if section.kind == SHT_SYMTAB:
                self._read_symbols(section)
        for section in self.sections:
            if section.kind == SHT_RELA:
                self._read_relocs(section)

    @staticmethod
    def _bytes(image, h):
        kind, flags, offset, size = h[1], h[2], h[4], h[5]
        if kind == SHT_NOBITS or size == 0 or offset == 0:
            return b''
        if offset + size > len(image):
            raise ElfError('section data runs past the end of the file')
        data = image[offset:offset + size]
        if flags & SHF_RPL_ZLIB:
            (inflated,) = struct.unpack_from('>I', data, 0)
            data = zlib.decompress(data[4:])
            if len(data) != inflated:
                raise ElfError('section inflated to %d bytes, expected %d' % (len(data), inflated))
        return data

    def _read_symbols(self, section):
        strings = self.sections[section.link].data
        data = section.data
        for i in range(len(data) // 16):
            name, value, size, info, _other, shndx = struct.unpack_from('>IIIBBH', data, i * 16)
            end = strings.find(b'\0', name)
            text = strings[name:end if end >= 0 else len(strings)].decode('latin-1')
            self.symbols.append(Symbol(i, text, value, size, info & 0xf, info >> 4, shndx))

    def _read_relocs(self, section):
        target = self.sections[section.info] if section.info < len(self.sections) else None
        if target is None:
            return
        table = self._relocs.setdefault(target.index, {})
        data = section.data
        base = 0 if self.relocatable else target.address
        for i in range(len(data) // 12):
            offset, info, addend = struct.unpack_from('>IIi', data, i * 12)
            table[offset - base] = (info & 0xff, info >> 8, addend)

    def relocs(self, shndx):
        """{section offset: (type, symbol index, addend)} for one section."""
        return self._relocs.get(shndx, {})

    def symbol_offset(self, symbol):
        """A symbol's offset inside its section."""
        if self.relocatable or symbol.shndx >= len(self.sections):
            return symbol.value
        return symbol.value - self.sections[symbol.shndx].address

    def functions(self):
        """(symbol, section, offset) for every sized function symbol in an executable section."""
        for symbol in self.symbols:
            if symbol.kind != STT_FUNC or not symbol.size or not (0 < symbol.shndx < len(self.sections)):
                continue
            section = self.sections[symbol.shndx]
            if section.executable:
                yield symbol, section, self.symbol_offset(symbol)

    def target(self, symbol_index, addend):
        """(section, offset) a relocation points at, or None for an undefined or absolute symbol."""
        if symbol_index >= len(self.symbols):
            return None
        symbol = self.symbols[symbol_index]
        if not (0 < symbol.shndx < len(self.sections)):
            return None
        if self.relocatable:
            return self.sections[symbol.shndx], symbol.value + addend
        return self.section_at(symbol.value + addend)

    def section_at(self, address):
        """(section, offset) of a virtual address in an executable; RPL relocations name segment
        symbols ($TEXT, $DATA) whose value is the segment base, not their section's start."""
        mapped = getattr(self, '_mapped', None)
        if mapped is None:
            mapped = sorted((s for s in self.sections if s.address and s.size and not s.name.startswith('.rela')
                             and s.kind in (1, SHT_NOBITS)), key=lambda s: s.address)
            self._mapped = mapped
        for section in mapped:
            if section.address <= address < section.address + section.size:
                return section, address - section.address
        return None

    def read(self, section, offset, size):
        if offset < 0 or offset + size > len(section.data):
            return None
        return section.data[offset:offset + size]


def load(path):
    with open(path, 'rb') as handle:
        return Elf(handle.read())
