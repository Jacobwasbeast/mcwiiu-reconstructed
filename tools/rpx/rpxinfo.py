"""Read an RPX/RPL: section table, decompressed contents, symbols and relocation statistics.

    python tools/rpx/rpxinfo.py <file.rpx> [--sections] [--imports] [--relocs] [--dump NAME]
"""
import argparse
import collections
import struct
import sys
import zlib
from pathlib import Path

SHF_RPL_ZLIB = 0x08000000
SHT_NAMES = {0: 'NULL', 1: 'PROGBITS', 2: 'SYMTAB', 3: 'STRTAB', 4: 'RELA', 8: 'NOBITS',
             0x80000001: 'RPL_EXPORTS', 0x80000002: 'RPL_IMPORTS', 0x80000003: 'RPL_CRCS',
             0x80000004: 'RPL_FILEINFO'}


class Rpl:
    def __init__(self, path):
        self.raw = Path(path).read_bytes()
        if self.raw[:4] != b'\x7fELF':
            raise ValueError('not an ELF file: %s' % path)
        header = struct.unpack('>16sHHIIIIIHHHHHH', self.raw[:52])
        self.type, self.machine, self.entry = header[1], header[2], header[4]
        shoff, shentsize, shnum, shstrndx = header[6], header[11], header[12], header[13]
        self.abi = self.raw[7:9]
        self.sections = []
        for index in range(shnum):
            fields = struct.unpack('>IIIIIIIIII', self.raw[shoff + index * shentsize:shoff + index * shentsize + 40])
            self.sections.append(dict(zip(('name_off', 'type', 'flags', 'addr', 'offset', 'size', 'link', 'info',
                                           'align', 'entsize'), fields), index=index))
        strings = self.data(self.sections[shstrndx])
        for section in self.sections:
            section['name'] = strings[section['name_off']:strings.index(b'\0', section['name_off'])].decode()

    def data(self, section):
        if section['type'] == 8 or section['offset'] == 0:
            return b''
        blob = self.raw[section['offset']:section['offset'] + section['size']]
        if section['flags'] & SHF_RPL_ZLIB:
            size = struct.unpack('>I', blob[:4])[0]
            blob = zlib.decompress(blob[4:])
            if len(blob) != size:
                raise ValueError('bad inflated size in ' + section['name'])
        return blob

    def by_name(self, name):
        for section in self.sections:
            if section['name'] == name:
                return section
        raise KeyError(name)

    def symbols(self):
        rows = []
        for section in self.sections:
            if section['type'] != 2:
                continue
            table = self.data(section)
            strings = self.data(self.sections[section['link']])
            for offset in range(0, len(table), 16):
                name, value, size, info, other, shndx = struct.unpack('>IIIBBH', table[offset:offset + 16])
                label = strings[name:strings.index(b'\0', name)].decode(errors='replace')
                rows.append((label, value, size, info, other, shndx))
        return rows

    def relocations_in(self, low, high):
        """(offset, type, symbol name, addend) for every relocation whose target address is in [low, high)."""
        names = None
        rows = []
        for section in self.sections:
            if section['type'] != 4:
                continue
            target = self.sections[section['info']]
            if target['addr'] + target['size'] < low and not (target['flags'] & SHF_RPL_ZLIB):
                continue
            table = self.data(section)
            for offset in range(0, len(table), 12):
                where, info, addend = struct.unpack('>IIi', table[offset:offset + 12])
                if low <= where < high:
                    if names is None:
                        names = self.symbols()
                    rows.append((where, info & 0xff, names[info >> 8][0], addend))
        return sorted(rows)

    def relocation_types(self):
        counts = collections.Counter()
        for section in self.sections:
            if section['type'] != 4:
                continue
            table = self.data(section)
            target = self.sections[section['info']]['name']
            for offset in range(0, len(table), 12):
                _, info, _ = struct.unpack('>IIi', table[offset:offset + 12])
                counts[(target, info & 0xff)] += 1
        return counts


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('path')
    parser.add_argument('--sections', action='store_true')
    parser.add_argument('--imports', action='store_true')
    parser.add_argument('--relocs', action='store_true')
    parser.add_argument('--dump')
    parser.add_argument('--relocs-at', nargs=2, metavar=('LOW', 'HIGH'), help='list relocations in an address range')
    args = parser.parse_args()
    rpl = Rpl(args.path)
    print('type %d machine %d abi %s entry 0x%08x sections %d' % (rpl.type, rpl.machine, rpl.abi.hex(), rpl.entry,
                                                                  len(rpl.sections)))
    if args.sections:
        for s in rpl.sections:
            print('%3d %-24s %-12s flags %08x addr %08x size %8x link %3d info %3d align %3d entsize %d' % (
                s['index'], s['name'], SHT_NAMES.get(s['type'], hex(s['type'])), s['flags'], s['addr'], s['size'],
                s['link'], s['info'], s['align'], s['entsize']))
    if args.imports:
        for s in rpl.sections:
            if s['type'] == 0x80000002:
                blob = rpl.data(s)
                print(s['name'], hex(s['addr']), len(blob), blob[:32].hex())
    if args.relocs:
        for (target, kind), count in sorted(rpl.relocation_types().items()):
            print('%-24s type %3d  %d' % (target, kind, count))
    if args.relocs_at:
        low, high = (int(value, 16) for value in args.relocs_at)
        for where, kind, name, addend in rpl.relocations_in(low, high):
            print('%08x type %3d %s%+d' % (where, kind, name, addend))
    if args.dump:
        sys.stdout.buffer.write(rpl.data(rpl.by_name(args.dump)))


if __name__ == '__main__':
    main()
