"""Find code stock Cemu 2.6 runs wrongly or refuses to recompile, in a linked Cafe ELF.

    python tools/redhills/crscan.py IMAGE.elf [--show N]

Three checks; the exit status is 1 if any finds something:
- compares whose condition bits the recompiler drops while code still reads them (below);
- D-form loads and stores whose base register field is 0 (an absolute address): the recompiler aborts
  on one when it compiles the function, so the first call into it takes Cemu down;
- conditional traps (twi, tw other than the unconditional trap): the interpreter never advances past
  them, so the thread hangs even when the condition is false.

Cemu 2.6 (the current release) recompiles each guest function as segments that end at every branch; a
call, a return or an indirect branch ends a segment whose successor is "uncertain". For each instruction
that writes a condition register field (compares and record forms) it stores only the bits that its own
segment reads or that later segments may read (PPCRecompiler_removeRedundantCRUpdates). In functions of
five or more segments, a later segment that ends uncertainly is assumed to overwrite cr0's LT, GT and EQ
bits even when that segment reads them itself (_PPCRecompiler_getCROverwriteMask), so a compare whose
cr0 result is read in such a segment, and not in its own, loses the write and the read sees stale bits.
Green Hills code keeps compares next to their readers and never does this; a RedHills image must not
either. This scan replays that analysis over the ELF's functions and reports every compare whose dropped
bits some path reads before rewriting them. An mfcr reads the bits the code then extracts from its copy
(followed through branches, and through calls when the copy is in a callee-saved register). The Green
Hills build reports 1 (iswctype); a RedHills build must report 0.
"""
import argparse
import bisect
import struct
import sys
from collections import Counter

import capstone

UNCERTAIN = ('bl', 'bla', 'blr', 'blrl', 'bctr', 'bctrl')
BIT = {'lt': 0, 'gt': 1, 'eq': 2, 'so': 3, 'un': 3}
BRANCH_BIT = {'lt': 0, 'ge': 0, 'nl': 0, 'gt': 1, 'le': 1, 'ng': 1, 'eq': 2, 'ne': 2, 'so': 3, 'ns': 3,
              'un': 3, 'nu': 3}
CR0_LT_GT_EQ = 7
# lwz..stfdu: Cemu 2.6 aborts recompiling one whose base register field is 0 (an absolute address).
D_FORM_LOAD_STORE = (32, 55)
ALL_BITS = 0xFFFFFFFF
# decode()'s read mask for mfcr. Cemu 2.6 counts it as reading every bit of its own segment but adds
# nothing to the bits the segment expects from earlier segments; whether a stale bit matters depends on
# which bits the code then takes from the copied register (extracted_bits).
MFCR = -1


def read_elf(path):
    data = open(path, 'rb').read()
    shoff = struct.unpack('>I', data[0x20:0x24])[0]
    shentsize, shnum, shstrndx = struct.unpack('>HHH', data[0x2E:0x34])
    rows = [struct.unpack('>IIIIIIIIII', data[shoff + i * shentsize:shoff + i * shentsize + 40])
            for i in range(shnum)]
    names = rows[shstrndx][4]
    text = None
    for index, row in enumerate(rows):
        if data[names + row[0]:data.index(b'\0', names + row[0])] == b'.text':
            text = (index, row[3], data[row[4]:row[4] + row[5]])
    functions = []
    for row in rows:
        if row[1] != 2:
            continue
        strtab = rows[row[6]][4]
        for at in range(row[4], row[4] + row[5], 16):
            name, value, size, info, other, shndx = struct.unpack('>IIIBBH', data[at:at + 16])
            if info & 0xF == 2 and shndx == text[0] and size:
                label = data[strtab + name:data.index(b'\0', strtab + name)].decode('latin-1')
                functions.append((value, size, label))
    return text[1], text[2], sorted(set(functions))


def field_bits(field):
    return 0xF << (field * 4)


def cr_operand_bit(token):
    """'cr1eq' or 'eq' (cr0) -> bit number."""
    token = token.strip()
    field = 0
    if token.startswith('cr') and token[2:3].isdigit():
        field, token = int(token[2]), token[3:]
    return field * 4 + BIT[token] if token in BIT else None


def decode(mnemonic, ops):
    """(read mask, written mask, writes-with-ignore field or None, branch kind) for one instruction.
    branch kind: None, 'cond' (conditional, in-function target), 'jump' (unconditional), 'uncertain'."""
    operands = [part.strip() for part in ops.split(',')] if ops else []
    if mnemonic.startswith(('cmp', 'fcmp')):
        field = int(operands[0][2]) if operands and operands[0].startswith('cr') else 0
        return 0, field_bits(field), field, None
    if mnemonic.endswith('.') and not mnemonic.startswith('st'):
        field = 1 if mnemonic.startswith('f') else 0
        return 0, field_bits(field), field, None
    if mnemonic == 'mfcr':
        return MFCR, 0, None, None
    if mnemonic in ('mtcrf', 'mtocrf'):
        mask = int(operands[0], 0)
        return 0, sum(field_bits(f) for f in range(8) if mask & (0x80 >> f)), None, None
    if mnemonic == 'mcrf':
        return field_bits(int(operands[1][2])), field_bits(int(operands[0][2])), None, None
    if mnemonic.startswith('cr'):
        bits = [cr_operand_bit(operand) for operand in operands]
        if None in bits or not bits:
            return 0, 0, None, None
        read = 0
        for bit in bits[1:]:
            read |= 1 << bit
        return read, 1 << bits[0], None, None
    if mnemonic in UNCERTAIN:
        return 0, 0, None, 'uncertain'
    if mnemonic in ('b', 'ba'):
        return 0, 0, None, 'jump'
    if mnemonic.startswith('b'):
        stem = mnemonic.rstrip('+-')
        to_register = stem.endswith(('lr', 'ctr', 'lrl', 'ctrl'))
        condition = stem[1:]
        for suffix in ('lrl', 'ctrl', 'lr', 'ctr', 'l', 'a'):
            if to_register or suffix in ('l', 'a'):
                if condition.endswith(suffix) and condition[:-len(suffix)] in BRANCH_BIT:
                    condition = condition[:-len(suffix)]
                    break
        read = 0
        if condition in BRANCH_BIT:
            field = int(operands[0][2]) if operands and operands[0].startswith('cr') else 0
            read = 1 << (field * 4 + BRANCH_BIT[condition])
        elif condition.startswith('d'):
            read = 0
        else:
            return 0, 0, None, None
        if to_register and read and not stem.endswith('l'):
            # Cemu lowers a conditional return or bctr to a conditional jump over the branch, which then
            # sits in a segment of its own.
            return read, 0, None, 'condret'
        return read, 0, None, 'uncertain' if to_register or stem.endswith('l') else 'cond'
    return 0, 0, None, None


def rotate_mask_bits(shift, first, last):
    """CR bits that rlwinm rD, rS, shift, first, last keeps from an mfcr copy, as a mask of 1 << n for CR bit
    n. mfcr puts CR bit n at register bit n counted from the most significant end, and result bit q of the
    rotate comes from source bit (q + shift) mod 32."""
    positions = range(first, last + 1) if first <= last else list(range(first, 32)) + list(range(0, last + 1))
    bits = 0
    for position in positions:
        bits |= 1 << ((position + shift) % 32)
    return bits


VOLATILE_GPRS = {'r0'} | {'r%d' % number for number in range(3, 13)}
CALLS = ('bl', 'bla', 'bctrl', 'blrl')


def use_bits(mnemonic, operands):
    """CR bits one instruction takes from an mfcr copy it reads, or ALL_BITS for a use that keeps them all."""
    name = mnemonic.rstrip('.')
    tail = operands[2:]
    if not all(part.lstrip('-').startswith('0x') or part.lstrip('-').isdigit() for part in tail):
        return ALL_BITS
    values = [int(part, 0) for part in tail]
    if name == 'rlwinm' and len(values) == 3:
        return rotate_mask_bits(*values)
    if name == 'clrlwi' and len(values) == 1:
        return rotate_mask_bits(0, values[0], 31)
    if name == 'srwi' and len(values) == 1:
        return rotate_mask_bits(32 - values[0], values[0], 31)
    if name == 'slwi' and len(values) == 1:
        return rotate_mask_bits(values[0], 0, 31 - values[0])
    if name == 'extrwi' and len(values) == 2:
        count, start = values
        return rotate_mask_bits(start + count, 32 - count, 31)
    return ALL_BITS


def extracted_bits(insns, index, position_of):
    """The CR bits the code takes from the register an mfcr at insns[index] (the function's instructions in
    address order; position_of maps an address to its index) copies the CR into, on every path until the
    register is redefined: rotate-and-mask extractions keep their bits, any other use keeps every bit. A
    copy in a callee-saved register lives across calls; a call ends a volatile one; leaving the function
    with the copy live keeps every bit."""
    register = insns[index][3].strip()
    bits = 0
    seen = set()
    work = [index + 1]
    while work:
        position = work.pop()
        while position < len(insns) and position not in seen:
            seen.add(position)
            address, _, mnemonic, ops = insns[position]
            operands = [part.strip() for part in ops.split(',')] if ops else []
            stores = mnemonic in ('mtcrf', 'mtocrf', 'stw', 'stwu', 'stwx')
            sources = operands if stores else operands[1:]
            if register in sources:
                bits |= use_bits(mnemonic, operands)
                if bits == ALL_BITS:
                    return bits
            if operands and operands[0] == register and not stores and not mnemonic.startswith('cmp'):
                break
            if mnemonic in CALLS:
                if register in VOLATILE_GPRS:
                    break
                position += 1
                continue
            kind = decode(mnemonic, ops)[3]
            if kind == 'uncertain':
                return ALL_BITS
            if kind in ('cond', 'jump'):
                target = operands[-1] if operands else ''
                destination = position_of.get(int(target, 16)) if target.startswith('0x') else None
                if destination is None:
                    return ALL_BITS
                work.append(destination)
                if kind == 'jump':
                    break
            position += 1
    return bits or ALL_BITS


class Segment:
    def __init__(self, start):
        self.start = start
        self.insns = []
        self.read = self.written = self.input = 0
        self.taken = self.not_taken = None
        self.uncertain = False


def segments_of(insns, start, end):
    targets = set()
    for address, _, mnemonic, ops in insns:
        _, _, _, kind = decode(mnemonic, ops)
        if kind in ('cond', 'jump'):
            target = ops.split(',')[-1].strip()
            if target.startswith('0x') and start <= int(target, 16) < end:
                targets.add(int(target, 16))
    segments, current = [], None
    for insn in insns:
        if current is None or insn[0] in targets:
            current = Segment(insn[0])
            segments.append(current)
        current.insns.append(insn)
        if decode(insn[2], insn[3])[3]:
            current = None
    by_start = {segment.start: index for index, segment in enumerate(segments)}
    returns = []
    for index, segment in enumerate(segments):
        following = segments[index + 1] if index + 1 < len(segments) else None
        for address, _, mnemonic, ops in segment.insns:
            read, written, _, _ = decode(mnemonic, ops)
            if read == MFCR:
                segment.read = ALL_BITS
            else:
                segment.input |= read & ~segment.written
                segment.read |= read
            segment.written |= written
        address, _, mnemonic, ops = segment.insns[-1]
        kind = decode(mnemonic, ops)[3]
        if kind in ('cond', 'jump'):
            target = ops.split(',')[-1].strip()
            destination = by_start.get(int(target, 16)) if target.startswith('0x') else None
            if destination is None:
                segment.uncertain = True
            else:
                segment.taken = segments[destination]
                if kind == 'cond':
                    segment.not_taken = following
        elif kind == 'condret':
            branch = Segment(address)
            branch.uncertain = True
            returns.append(branch)
            segment.taken = following
            segment.not_taken = branch
            if following is None:
                segment.uncertain = True
        elif kind == 'uncertain':
            segment.uncertain = True
        elif following is not None:
            segment.not_taken = following
        else:
            segment.uncertain = True
    return segments + returns


def overwrite(segment, count, depth):
    """_PPCRecompiler_getCROverwriteMask as Cemu 2.6 ships it."""
    mask = segment.written & ~segment.input
    if not segment.uncertain and depth < 3:
        following = 0
        if segment.taken and segment.not_taken:
            following = overwrite(segment.taken, count, depth + 1) & overwrite(segment.not_taken, count, depth + 1)
        elif segment.not_taken:
            following = overwrite(segment.not_taken, count, depth + 1)
        mask |= following & ~segment.read
    elif segment.uncertain and count >= 5:
        return CR0_LT_GT_EQ
    return mask


def top_overwrite(segment, count):
    if segment.uncertain:
        return 0
    if segment.taken and segment.not_taken:
        return overwrite(segment.taken, count, 0) & overwrite(segment.not_taken, count, 0)
    if segment.not_taken:
        return overwrite(segment.not_taken, count, 0)
    return 0


def stale_reader(segment, index, dropped, insns, position_of):
    """First instruction that reads a dropped bit before it is rewritten, on any path that stays in
    straight-line code or follows branches (calls and returns end a path: the callee owns the CR)."""
    seen = set()
    work = [(segment, index + 1, dropped)]
    while work:
        current, position, live = work.pop()
        for offset, (address, _, mnemonic, ops) in enumerate(current.insns[position:], position):
            read, written, _, _ = decode(mnemonic, ops)
            if read == MFCR:
                read = extracted_bits(insns, position_of[address], position_of)
            if read & live:
                return address, mnemonic, ops
            live &= ~written
            if not live:
                break
        if not live or current.uncertain:
            continue
        for following in (current.taken, current.not_taken):
            if following is not None and (following.start, live) not in seen:
                seen.add((following.start, live))
                work.append((following, 0, live))
    return None


def main():
    parser = argparse.ArgumentParser(description=__doc__.split('\n\n')[0])
    parser.add_argument('image')
    parser.add_argument('--show', type=int, default=0)
    args = parser.parse_args()
    base, code, functions = read_elf(args.image)
    md = capstone.Cs(capstone.CS_ARCH_PPC, capstone.CS_MODE_32 | capstone.CS_MODE_BIG_ENDIAN)
    md.skipdata = True
    counts, shown = Counter(), 0
    for start, size, name in functions:
        insns = list(md.disasm_lite(code[start - base:start - base + size], start))
        if not insns:
            continue
        segments = segments_of(insns, start, start + size)
        position_of = {insn[0]: number for number, insn in enumerate(insns)}
        for segment in segments:
            ignore = None
            for index, (address, _, mnemonic, ops) in enumerate(segment.insns):
                field = decode(mnemonic, ops)[2]
                if field is None:
                    continue
                if ignore is None:
                    ignore = top_overwrite(segment, len(segments)) & ~segment.read
                dropped = ignore & field_bits(field)
                if not dropped:
                    continue
                reader = stale_reader(segment, index, dropped, insns, position_of)
                if reader:
                    counts['stale cr%d read' % field] += 1
                    if shown < args.show:
                        shown += 1
                        print('%08x %s %s -> %08x %s %s  in %s' % (address, mnemonic, ops, reader[0], reader[1],
                                                                   reader[2], name[:90]))
    for start, size, name in functions:
        for offset in range(start - base, start - base + size - 3, 4):
            word = struct.unpack_from('>I', code, offset)[0]
            primary, to = word >> 26, (word >> 21) & 31
            kind = None
            if D_FORM_LOAD_STORE[0] <= primary <= D_FORM_LOAD_STORE[1] and (word >> 16) & 31 == 0:
                kind = 'load/store with a zero base register'
            elif primary == 3 or (primary == 31 and (word >> 1) & 1023 == 4 and to != 31):
                kind = 'conditional trap'
            if kind:
                counts[kind] += 1
                if shown < args.show:
                    shown += 1
                    print('%08x %s  in %s' % (base + offset, kind, name[:90]))
    for key, count in sorted(counts.items()):
        print('%7d  %s' % (count, key))
    total = sum(counts.values())
    print('%7d  total' % total)
    return 1 if total else 0


if __name__ == '__main__':
    sys.exit(main())
