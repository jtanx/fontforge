# Based on makeunicodedata.py from CPython

from typing import Iterator, List, Optional, Set, Tuple
from functools import partial

import dataclasses
import enum
import os
import sys

SCRIPT = sys.argv[0]
VERSION = "1.0"

UNIDATA_VERSION = "12.1.0"
UNICODE_DATA = "UnicodeData%s.txt"
DERIVED_CORE_PROPERTIES = "DerivedCoreProperties%s.txt"
PROP_LIST = "PropList%s.txt"
LINE_BREAK = "LineBreak%s.txt"
BIDI_MIRRORING = "BidiMirroring%s.txt"
UNICODE_MAX = 0x110000

DATA_DIR = "data"

MANDATORY_LINE_BREAKS = ["BK", "CR", "LF", "NL"]


class UcdTypeFlags(enum.IntFlag):  # rough character counts:
    FF_UNICODE_ISUNICODEPOINTASSIGNED = 0x1  # all
    FF_UNICODE_ISALPHA = 0x2  # 49471
    FF_UNICODE_ISIDEOGRAPHIC = 0x4  # 28044
    FF_UNICODE_ISLEFTTORIGHT = 0x10  # 57906
    FF_UNICODE_ISRIGHTTOLEFT = 0x20  # 1286
    FF_UNICODE_ISLOWER = 0x40  # 1434
    FF_UNICODE_ISUPPER = 0x80  # 1119
    FF_UNICODE_ISDIGIT = 0x100  # 370
    FF_UNICODE_ISLIGVULGFRAC = 0x200  # 615
    FF_UNICODE_ISCOMBINING = 0x400  # 2295
    FF_UNICODE_ISZEROWIDTH = 0x800  # 404 Really ISDEFAULTIGNORABLE now
    FF_UNICODE_ISEURONUMERIC = 0x1000  # 168
    FF_UNICODE_ISEURONUMTERM = 0x2000  # 76
    FF_UNICODE_ISARABNUMERIC = 0x8000  # 61


class CombiningClass(enum.IntFlag):
    ABOVE = 0x100
    BELOW = 0x200
    OVERSTRIKE = 0x400
    LEFT = 0x800
    RIGHT = 0x1000
    JOINS2 = 0x2000
    CENTERLEFT = 0x4000
    CENTERRIGHT = 0x8000
    CENTEREDOUTSIDE = 0x10000
    OUTSIDE = 0x20000
    RIGHTEDGE = 0x40000
    LEFTEDGE = 0x80000
    TOUCHING = 0x100000


def open_data(template, version):
    local = os.path.join(DATA_DIR, template % ("-" + version,))
    if not os.path.exists(local):
        import urllib.request

        url = ("https://www.unicode.org/Public/%s/ucd/" + template) % (version, "")
        print("Fetching", url)
        os.makedirs(DATA_DIR, exist_ok=True)
        urllib.request.urlretrieve(url, filename=local)
    if local.endswith(".txt"):
        return open(local, encoding="utf-8")
    else:
        # Unihan.zip
        return open(local, "rb")


def expand_range(char_range: str) -> Iterator[int]:
    """
    Parses ranges of code points, as described in UAX #44:
      https://www.unicode.org/reports/tr44/#Code_Point_Ranges
    """
    if ".." in char_range:
        first, last = [int(c, 16) for c in char_range.split("..")]
    else:
        first = last = int(char_range, 16)
    for char in range(first, last + 1):
        yield char


@dataclasses.dataclass
class UcdRecord:
    # 15 fields from UnicodeData.txt .  See:
    #   https://www.unicode.org/reports/tr44/#UnicodeData.txt
    codepoint: str
    name: str
    general_category: str
    canonical_combining_class: str
    bidi_class: str
    decomposition_type: str
    decomposition_mapping: str
    numeric_type: str
    numeric_value: str
    bidi_mirrored: str
    unicode_1_name: str  # obsolete
    iso_comment: str  # obsolete
    simple_uppercase_mapping: str
    simple_lowercase_mapping: str
    simple_titlecase_mapping: str

    # Binary properties, as a set of those that are true.
    # Taken from multiple files:
    #   https://www.unicode.org/reports/tr44/#DerivedCoreProperties.txt
    #   https://www.unicode.org/reports/tr44/#LineBreak.txt
    #   https://www.unicode.org/reports/tr44/#PropList.txt
    binary_properties: Set[str]

    # Used to get the mirrored character, if any. See:
    #   https://www.unicode.org/reports/tr44/#BidiMirroring.txt
    bidi_mirroring: str


def from_row(row: List[str]) -> UcdRecord:
    return UcdRecord(*row, set(), "")


class UcdFile:
    """
    A file in the standard format of the UCD.

    See: https://www.unicode.org/reports/tr44/#Format_Conventions

    Note that, as described there, the Unihan data files have their
    own separate format.
    """

    def __init__(self, template: str, version: str) -> None:
        self.template = template
        self.version = version

    def records(self) -> Iterator[List[str]]:
        with open_data(self.template, self.version) as file:
            for line in file:
                line = line.split("#", 1)[0].strip()
                if not line:
                    continue
                yield [field.strip() for field in line.split(";")]

    def __iter__(self) -> Iterator[List[str]]:
        return self.records()

    def expanded(self) -> Iterator[Tuple[int, List[str]]]:
        for record in self.records():
            char_range, rest = record[0], record[1:]
            for char in expand_range(char_range):
                yield char, rest


# --------------------------------------------------------------------
# the following support code is taken from the unidb utilities
# Copyright (c) 1999-2000 by Secret Labs AB

# load a unicode-data file from disk


class UnicodeData:
    # table: List[Optional[UcdRecord]]  # index is codepoint; None means unassigned

    def __init__(self, version):
        table = [None] * UNICODE_MAX
        for s in UcdFile(UNICODE_DATA, version):
            char = int(s[0], 16)
            table[char] = from_row(s)

        # expand first-last ranges
        field = None
        for i in range(0, UNICODE_MAX):
            # The file UnicodeData.txt has its own distinct way of
            # expressing ranges.  See:
            #   https://www.unicode.org/reports/tr44/#Code_Point_Ranges
            s = table[i]
            if s:
                if s.name[-6:] == "First>":
                    s.name = ""
                    field = dataclasses.astuple(s)[:15]
                elif s.name[-5:] == "Last>":
                    s.name = ""
                    field = None
            elif field:
                table[i] = from_row(("%X" % i,) + field[1:])

        # public attributes
        self.filename = UNICODE_DATA % ""
        self.table = table
        self.chars = list(range(UNICODE_MAX))  # unicode 3.2

        for char, (p,) in UcdFile(DERIVED_CORE_PROPERTIES, version).expanded():
            if table[char]:
                # Some properties (e.g. Default_Ignorable_Code_Point)
                # apply to unassigned code points; ignore them
                table[char].binary_properties.add(p)

        for char, (p,) in UcdFile(PROP_LIST, version).expanded():
            if table[char]:
                table[char].binary_properties.add(p)

        for char_range, value in UcdFile(LINE_BREAK, version):
            if value not in MANDATORY_LINE_BREAKS:
                continue
            for char in expand_range(char_range):
                table[char].binary_properties.add("Line_Break")

        for data in UcdFile(BIDI_MIRRORING, version):
            c = int(data[0], 16)
            table[c].bidi_mirroring = data[1]


# --------------------------------------------------------------------
# unicode character type tables


def makeutype(unicode, trace):

    FILE = "utype2.c"

    print("--- Preparing", FILE, "...")

    # extract unicode types
    dummy = (0, 0, 0, 0, 0)
    table = [dummy]
    cache = {dummy: 0}
    index = [0] * len(unicode.chars)
    # use switch statements for properties with low instance counts
    switches = {}

    for char in unicode.chars:
        record = unicode.table[char]
        if record:
            # extract database properties
            category = record.general_category
            bidirectional = record.bidi_class
            properties = record.binary_properties
            flags = UcdTypeFlags.FF_UNICODE_ISUNICODEPOINTASSIGNED
            if category in ["Lm", "Lt", "Lu", "Ll", "Lo"]:
                flags |= UcdTypeFlags.FF_UNICODE_ISALPHA
            if category in ["M", "Mn", "Mc", "Me"]:
                flags |= UcdTypeFlags.FF_UNICODE_ISCOMBINING
            if "Lowercase" in properties:
                flags |= UcdTypeFlags.FF_UNICODE_ISLOWER
            if "Ideographic" in properties:
                flags |= UcdTypeFlags.FF_UNICODE_ISIDEOGRAPHIC
            if category == "Zs" or bidirectional in ("WS", "B", "S"):
                switches.setdefault("space", []).append(char)

            # bidi flags
            if bidirectional in ("L", "LRE", "LRO"):
                flags |= UcdTypeFlags.FF_UNICODE_ISLEFTTORIGHT
            elif bidirectional in ("R", "AL", "RLE", "RLO"):
                flags |= UcdTypeFlags.FF_UNICODE_ISRIGHTTOLEFT
            elif bidirectional == "EN":
                flags |= UcdTypeFlags.FF_UNICODE_ISEURONUMERIC
            elif bidirectional == "AN":
                flags |= UcdTypeFlags.FF_UNICODE_ISARABNUMERIC
            elif bidirectional == "ES":
                switches.setdefault("euronumsep", []).append(char)
            elif bidirectional == "CS":
                switches.setdefault("commonsep", []).append(char)
            elif bidirectional == "ET":
                flags |= UcdTypeFlags.FF_UNICODE_ISEURONUMTERM

            if category == "Lt":
                switches.setdefault("title", []).append(char)
            if "Uppercase" in properties:
                flags |= UcdTypeFlags.FF_UNICODE_ISUPPER
            if "ASCII_Hex_Digit" in properties:
                switches.setdefault("hexdigit", []).append(char)
            if "Default_Ignorable_Code_Point" in properties:
                flags |= UcdTypeFlags.FF_UNICODE_ISZEROWIDTH

            # This is questionable... But ok
            if any(x in record.name for x in ("LIGATURE", "VULGAR", "FRACTION")):
                if char != 0x2044:  # FRACTION SLASH
                    flags |= UcdTypeFlags.FF_UNICODE_ISLIGVULGFRAC

            if record.simple_uppercase_mapping:
                upper = int(record.simple_uppercase_mapping, 16)
            else:
                upper = char
            if record.simple_lowercase_mapping:
                lower = int(record.simple_lowercase_mapping, 16)
            else:
                lower = char
            if record.simple_titlecase_mapping:
                title = int(record.simple_titlecase_mapping, 16)
            else:
                title = upper
            if record.bidi_mirroring:
                mirror = int(record.bidi_mirroring, 16)
            else:
                mirror = char

            # No special casing or case folding
            if upper == lower == title == mirror:
                upper = lower = title = mirror = 0
            else:
                upper = upper - char
                lower = lower - char
                title = title - char
                mirror = mirror - char
                assert (
                    abs(upper) <= 2147483647
                    and abs(lower) <= 2147483647
                    and abs(title) <= 2147483647
                    and abs(mirror) <= 2147483647
                )
            # integer digit
            digit = 0
            if record.numeric_type:
                flags |= UcdTypeFlags.FF_UNICODE_ISDIGIT
                digit = int(record.numeric_type)
            item = (upper, lower, title, mirror, flags)
            # add entry to index and item tables
            i = cache.get(item)
            if i is None:
                cache[item] = i = len(table)
                table.append(item)
            index[char] = i

    # You want to keep this generally below 255 so the index can fit in one
    # byte. Once it exceeds you double the size requirement for index2.
    print(len(table), "unique character type entries")
    for k, v in switches.items():
        print(len(v), k, "code points")

    print("--- Writing", FILE, "...")

    with open(FILE, "w") as fp:
        fprint = partial(print, file=fp)
        alignment = max(len(flag.name) for flag in UcdTypeFlags)

        fprint("/* this file was generated by %s %s */" % (SCRIPT, VERSION))
        fprint()
        fprint("#include <assert.h>")
        fprint("#include <utype2.h>")
        fprint()
        for flag in UcdTypeFlags:
            fprint("#define %-*s 0x%x" % (alignment, flag.name, flag.value))
        fprint()
        fprint("struct utyperecord {")
        fprint(
            "    int32_t upper, lower, title, mirror; /* delta from current character */"
        )
        fprint("    uint32_t flags; /* one or more of the above flags */")
        fprint("};")
        fprint()
        fprint("/* a list of unique character type descriptors */")
        fprint("static const struct utyperecord utype_records[] = {")
        for item in table:
            fprint("    {%d, %d, %d, %d, %d}," % item)
        fprint("};")
        fprint()

        # split decomposition index table
        # splitbins3(index)
        index1, index2, shift = splitbins(index, trace)

        fprint("/* type indexes */")
        fprint("#define SHIFT", shift)
        Array("index1", index1).dump(fp, trace)
        Array("index2", index2).dump(fp, trace)

        fprint("static const struct utyperecord* _GetRecord(unichar_t ch) {")
        fprint("    int index = 0;")
        fprint("    if (ch < 0x%x) {" % UNICODE_MAX)
        fprint("        index = index1[ch >> SHIFT];")
        fprint("        index = index2[(index << SHIFT) + (ch & ((1 << SHIFT) - 1))];")
        fprint("    }")
        fprint(
            "    assert(index >= 0 && index < sizeof(utype_records)/sizeof(utype_records[0]));"
        )
        fprint("    return &utype_records[index];")
        fprint("}")
        fprint()

        for flag in UcdTypeFlags:
            fprint("int %s(unichar_t ch) {" % flag.name.lower())
            fprint("    return _GetRecord(ch)->flags & %s;" % (flag.name,))
            fprint("}")
            fprint()

        for k in sorted(switches.keys()):
            Switch("ff_unicode_is" + k, switches[k]).dump(fp)

        conv_types = ("lower", "upper", "title", "mirror")
        for n in conv_types:
            fprint("unichar_t ff_unicode_to%s(unichar_t ch) {" % n)
            fprint("    const struct utyperecord* rec = _GetRecord(ch);")
            if n == "mirror":
                fprint("    if (rec->mirror == 0) {")
                fprint("        return 0;")  # special case...
                fprint("    }")
            fprint("    return (unichar_t)(((int32_t)ch) + rec->%s);" % n)
            fprint("}")
            fprint()

    return (
        [("int", x.name.lower()) for x in UcdTypeFlags]
        + [("int", "ff_unicode_is" + k) for k in switches]
        + [("unichar_t", "ff_unicode_to" + n) for n in conv_types]
    )


def makeunicodedata(unicode, trace):
    FILE = "unicodedata_db.h"

    print("--- Preparing", FILE, "...")

    decomp_data = [0]
    decomp_prefix = [""]
    decomp_index = [0] * len(unicode.chars)
    decomp_size = 0

    for char in unicode.chars:
        record = unicode.table[char]
        if record:
            if record.decomposition_type:
                decomp = record.decomposition_type.split()
                if len(decomp) > 19:
                    raise Exception(
                        "character %x has a decomposition too large for nfd_nfkd" % char
                    )
                # prefix
                if decomp[0][0] == "<":
                    prefix = decomp.pop(0)
                else:
                    prefix = ""
                try:
                    i = decomp_prefix.index(prefix)
                except ValueError:
                    i = len(decomp_prefix)
                    decomp_prefix.append(prefix)
                prefix = i
                assert prefix < 256
                # content
                decomp = [prefix + (len(decomp) << 8)] + [int(s, 16) for s in decomp]
                try:
                    i = decomp_data.index(decomp)
                except ValueError:
                    i = len(decomp_data)
                    decomp_data.extend(decomp)
                    decomp_size = decomp_size + len(decomp) * 2
            else:
                i = 0
            decomp_index[char] = i

    print(len(decomp_prefix), "unique decomposition prefixes")
    print(len(decomp_data), "unique decomposition entries:", end=" ")
    print(decomp_size, "bytes")

    print("--- Writing", FILE, "...")

    with open(FILE, "w") as fp:
        fprint = partial(print, file=fp)

        fprint("/* this file was generated by %s %s */" % (SCRIPT, VERSION))
        fprint()
        fprint('#define UNIDATA_VERSION "%s"' % UNIDATA_VERSION)

        fprint("static const char *decomp_prefix[] = {")
        for name in decomp_prefix:
            fprint('    "%s",' % name)
        fprint("    NULL")
        fprint("};")

        # split decomposition index table
        index1, index2, shift = splitbins(decomp_index, trace)

        fprint("/* decomposition data */")
        Array("decomp_data", decomp_data).dump(fp, trace)

        fprint("/* index tables for the decomposition data */")
        fprint("#define DECOMP_SHIFT", shift)
        Array("decomp_index1", index1).dump(fp, trace)
        Array("decomp_index2", index2).dump(fp, trace)


def makeutypeheader(utype_funcs):
    FILE = "utype2.h"
    utype_funcs = [(t, n, n.replace("ff_unicode_", "")) for t, n in utype_funcs]

    print("--- Writing", FILE, "...")

    with open(FILE, "w") as fp:
        fprint = partial(print, file=fp)

        fprint(
            "#include <ctype.h>	/* Include here so we can control it. If a system header includes it later bad things happen */"
        )
        fprint(
            '#include "basics.h"	/* Include here so we can use pre-defined int types to correctly size constant data arrays. */'
        )
        fprint()

        for rettype, fn, _ in utype_funcs:
            fprint("extern %s %s(unichar_t ch);" % (rettype, fn))
        fprint()

        for _, fn, realfn in utype_funcs:
            fprint("#undef %s" % realfn)
        fprint()

        alignment = max(len(x) + 4 for _, _, x in utype_funcs)
        for _, fn, realfn in utype_funcs:
            fprint("#define %-*s %s((ch))" % (alignment, realfn + "(ch)", fn))
        fprint()


# stuff to deal with arrays of unsigned integers


class Array:
    def __init__(self, name, data):
        self.name = name
        self.data = data

    def dump(self, file, trace=0):
        # write data to file, as a C array
        size = getsize(self.data)
        if trace:
            print(self.name + ":", size * len(self.data), "bytes", file=sys.stderr)
        file.write("static const ")
        if size == 1:
            file.write("unsigned char")
        elif size == 2:
            file.write("unsigned short")
        else:
            file.write("unsigned int")
        file.write(" " + self.name + "[] = {\n")
        if self.data:
            s = "    "
            for item in self.data:
                i = str(item) + ", "
                if len(s) + len(i) > 78:
                    file.write(s.rstrip() + "\n")
                    s = "    " + i
                else:
                    s = s + i
            if s.strip():
                file.write(s.rstrip() + "\n")
        file.write("};\n\n")


class Switch:
    def __init__(self, name, data):
        self.name = name
        self.data = data

    def dump(self, file):
        fprint = partial(print, file=file)
        fprint("int %s(unichar_t ch) {" % self.name)
        fprint("    switch (ch) {")

        for codepoint in sorted(self.data):
            fprint("    case 0x%04X:" % (codepoint,))
        fprint("        return 1;")

        fprint("    }")
        fprint("    return 0;")
        fprint("}")
        fprint()


def getsize(data: List[int]) -> int:
    # return smallest possible integer size for the given array
    maxdata = max(data)
    if maxdata < 256:
        return 1
    elif maxdata < 65536:
        return 2
    else:
        return 4


def splitbins(t: List[int], trace=0) -> Tuple[List[int], List[int], int]:
    """t, trace=0 -> (t1, t2, shift).  Split a table to save space.

    t is a sequence of ints.  This function can be useful to save space if
    many of the ints are the same.  t1 and t2 are lists of ints, and shift
    is an int, chosen to minimize the combined size of t1 and t2 (in C
    code), and where for each i in range(len(t)),
        t[i] == t2[(t1[i >> shift] << shift) + (i & mask)]
    where mask is a bitmask isolating the last "shift" bits.

    If optional arg trace is non-zero (default zero), progress info
    is printed to sys.stderr.  The higher the value, the more info
    you'll get.
    """

    if trace:

        def dump(t1, t2, shift, bytes):
            print(
                "%d+%d bins at shift %d; %d bytes" % (len(t1), len(t2), shift, bytes),
                file=sys.stderr,
            )

        print("Size of original table:", len(t) * getsize(t), "bytes", file=sys.stderr)
    n = len(t) - 1  # last valid index
    maxshift = 0  # the most we can shift n and still have something left
    if n > 0:
        while n >> 1:
            n >>= 1
            maxshift += 1
    del n
    bytes = sys.maxsize  # smallest total size so far
    t = tuple(t)  # so slices can be dict keys
    for shift in range(maxshift + 1):
        t1 = []
        t2 = []
        size = 2 ** shift
        bincache = {}
        for i in range(0, len(t), size):
            bin = t[i : i + size]
            index = bincache.get(bin)
            if index is None:
                index = len(t2)
                bincache[bin] = index
                t2.extend(bin)
            t1.append(index >> shift)
        # determine memory size
        b = len(t1) * getsize(t1) + len(t2) * getsize(t2)
        if trace > 1:
            dump(t1, t2, shift, b)
        if b < bytes:
            best = t1, t2, shift
            bytes = b
    t1, t2, shift = best
    if trace:
        print("Best:", end=" ", file=sys.stderr)
        dump(t1, t2, shift, bytes)
    if __debug__:
        # exhaustively verify that the decomposition is correct
        mask = ~((~0) << shift)  # i.e., low-bit mask of shift bits
        for i in range(len(t)):
            assert t[i] == t2[(t1[i >> shift] << shift) + (i & mask)]
    return best


def maketables(trace=0):
    print("--- Reading", UNICODE_DATA % "", "...")

    unicode = UnicodeData(UNIDATA_VERSION)
    funcs = makeutype(unicode, trace)
    makeunicodedata(unicode, trace)
    makeutypeheader(funcs)


if __name__ == "__main__":
    maketables(1)
