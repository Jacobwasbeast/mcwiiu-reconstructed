"""Read Wii U RPL/RPX executables.

An RPX is a big-endian 32-bit ELF file. Sections flagged SHF_RPL_ZLIB store a
4-byte big-endian inflated size followed by a zlib stream. This module loads
every allocated section and answers reads by virtual address.
"""
import struct
import zlib

SHF_RPL_ZLIB = 0x08000000
SHT_NOBITS = 8
ELF_MAGIC = b"\x7fELF"
ELFCLASS32 = 1
ELFDATA2MSB = 2
SECTION_HEADER_SIZE = 40


class RplError(Exception):
    """The file is not a readable RPL/RPX or an address is not mapped."""


class Section(object):
    def __init__(self, index, name, kind, flags, address, data):
        self.index = index
        self.name = name
        self.kind = kind
        self.flags = flags
        self.address = address
        self.data = data

    @property
    def end(self):
        return self.address + len(self.data)


class Rpl(object):
    """An RPL/RPX image with its sections inflated."""

    def __init__(self, image):
        if len(image) < 52 or image[:4] != ELF_MAGIC:
            raise RplError("not an ELF file")
        if image[4] != ELFCLASS32 or image[5] != ELFDATA2MSB:
            raise RplError("not a 32-bit big-endian ELF file")
        (shoff,) = struct.unpack_from(">I", image, 0x20)
        shentsize, shnum, shstrndx = struct.unpack_from(">HHH", image, 0x2E)
        if shentsize != SECTION_HEADER_SIZE:
            raise RplError("unexpected section header size %d" % shentsize)
        if shoff + shnum * shentsize > len(image):
            raise RplError("section headers run past the end of the file")
        headers = []
        for index in range(shnum):
            fields = struct.unpack_from(">10I", image, shoff + index * shentsize)
            headers.append(fields)
        raw = [self._section_bytes(image, fields) for fields in headers]
        names = raw[shstrndx] if shstrndx < len(raw) else b""
        self.sections = []
        for index, fields in enumerate(headers):
            name_offset, kind, flags, address = fields[0], fields[1], fields[2], fields[3]
            end = names.find(b"\0", name_offset)
            name = names[name_offset:end if end >= 0 else len(names)].decode("ascii", "replace")
            self.sections.append(Section(index, name, kind, flags, address, raw[index]))
        self._mapped = sorted((s for s in self.sections if s.address and s.data), key=lambda s: s.address)

    @staticmethod
    def _section_bytes(image, fields):
        kind, flags, offset, size = fields[1], fields[2], fields[4], fields[5]
        if kind == SHT_NOBITS or size == 0 or offset == 0:
            return b""
        if offset + size > len(image):
            raise RplError("section data runs past the end of the file")
        data = image[offset:offset + size]
        if flags & SHF_RPL_ZLIB:
            (inflated,) = struct.unpack_from(">I", data, 0)
            data = zlib.decompress(data[4:])
            if len(data) != inflated:
                raise RplError("section inflated to %d bytes, expected %d" % (len(data), inflated))
        return data

    @classmethod
    def from_file(cls, path):
        with open(path, "rb") as handle:
            return cls(handle.read())

    def section(self, name):
        for section in self.sections:
            if section.name == name:
                return section
        raise RplError("no section named %s" % name)

    def section_at(self, address):
        for section in self._mapped:
            if section.address <= address < section.end:
                return section
        raise RplError("address 0x%08X is not in a loaded section" % address)

    def read(self, address, size):
        section = self.section_at(address)
        start = address - section.address
        if start + size > len(section.data):
            raise RplError("read of %d bytes at 0x%08X crosses the end of %s" % (size, address, section.name))
        return section.data[start:start + size]

    def u32(self, address):
        return struct.unpack(">I", self.read(address, 4))[0]

    def words(self, address, count):
        return list(struct.unpack(">%dI" % count, self.read(address, 4 * count)))

    def c_string(self, address, limit=4096):
        section = self.section_at(address)
        start = address - section.address
        end = section.data.find(b"\0", start, start + limit)
        if end < 0:
            raise RplError("unterminated string at 0x%08X" % address)
        return section.data[start:end].decode("ascii")

    def find_all(self, needle):
        """Every virtual address where `needle` occurs in a loaded section."""
        hits = []
        for section in self._mapped:
            position = section.data.find(needle)
            while position >= 0:
                hits.append(section.address + position)
                position = section.data.find(needle, position + 1)
        return hits
