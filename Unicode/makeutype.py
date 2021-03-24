# Based on makeunicodedata.py from CPython

from typing import Iterator, List, Optional, Set, Tuple
from functools import partial
from itertools import chain

import dataclasses
import enum
import os
import sys
import zipfile

SCRIPT = sys.argv[0]
VERSION = "1.0"

UNIDATA_VERSION = "13.0.0"
UNICODE_DATA = "UnicodeData%s.txt"
UNIHAN = "Unihan%s.zip"
DERIVED_CORE_PROPERTIES = "DerivedCoreProperties%s.txt"
PROP_LIST = "PropList%s.txt"
LINE_BREAK = "LineBreak%s.txt"
BIDI_MIRRORING = "BidiMirroring%s.txt"
UNICODE_MAX = 0x110000

DATA_DIR = "data"

MANDATORY_LINE_BREAKS = ["BK", "CR", "LF", "NL"]

LICENSE = f"""/* This is a GENERATED file - from {SCRIPT} {VERSION} with Unicode {UNIDATA_VERSION} */

/* Copyright (C) 2000-2012 by George Williams */
/* Contributions: Werner Lemberg, Khaled Hosny, Joe Da Silva */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

 * The name of the author may not be used to endorse or promote products
 * derived from this software without specific prior written permission.

 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
"""

# These non-normative decompositions allow display algorithems to
# pick something that looks right, even if the character doesn't mean
# what it should. For example Alpha LOOKS LIKE A so if we don't have
# an Alpha character available we can draw it with an A. But this decomp
# is not normative and should not be used for ordering purposes
VISUAL_ALTS = {
    #  ligatures
    #  I don't bother with AE, ae because they are in latin1 and so common
    0x152: (0x4F, 0x45),  # Œ -> OE
    0x153: (0x6F, 0x65),  # œ -> oe
    #  Things which look alike to my eyes
    0x110: (0xD0,),  # Đ -> Ð
    0x138: (0x3BA,),  # ĸ -> κ
    0x182: (0x402,),  # Ƃ -> Ђ
    0x189: (0xD0,),  # Ɖ -> Ð
    0x19E: (0x3B7,),  # ƞ -> η
    0x19F: (0x398,),  # Ɵ -> Θ
    0x1A9: (0x3A3,),  # Ʃ -> Σ
    0x1C0: (0x7C,),  # ǀ -> |
    0x1C1: (0x7C, 0x7C),  # ǁ -> ||
    0x269: (0x3B9,),  # ɩ -> ι
    #  IPA
    0x278: (0x3A6,),  # ɸ -> Φ
    0x299: (0x432,),  # ʙ -> в
    0x292: (0x1B7,),  # ʒ -> Ʒ
    0x29C: (0x43D,),  # ʜ -> н
    0x2B9: (0x27,),  # ʹ -> '
    0x2BA: (0x22,),  # ʺ -> "
    0x2BC: (0x27,),  # ʼ -> '
    0x2C4: (0x5E,),  # ˄ -> ^
    0x2C6: (0x5E,),  # ˆ -> ^
    0x2C8: (0x27,),  # ˈ -> '
    0x2DC: (0x7E,),  # ˜ -> ~
    0x2E0: (0x263,),  # ˠ -> ɣ
    0x2E1: (0x6C,),  # ˡ -> l
    0x2E2: (0x73,),  # ˢ -> s
    0x2E3: (0x78,),  # ˣ -> x
    0x2E4: (0x2E4,),  # ˤ -> ˤ
    0x301: (0xB4,),  # ́ -> ´
    0x302: (0x5E,),  # ̂ -> ^
    0x303: (0x7E,),  # ̃ -> ~
    0x308: (0xA8,),  # ̈ -> ¨
    0x30A: (0xB0,),  # ̊ -> °
    0x30B: (0x27,),  # ̋ -> '
    0x30E: (0x27,),  # ̎ -> '
    0x327: (0xB8,),  # ̧ -> ¸
    #  Greek
    0x374: (0x27,),  # ʹ -> '
    0x375: (0x2CF,),  # ͵ -> ˏ
    0x37A: (0x345,),  # ͺ -> ͅ
    0x37E: (0x3B,),  # ; -> ;
    0x391: (0x41,),  # Α -> A
    0x392: (0x42,),  # Β -> B
    0x393: (0x413,),  # Γ -> Г
    0x395: (0x45,),  # Ε -> E
    0x396: (0x5A,),  # Ζ -> Z
    0x397: (0x48,),  # Η -> H
    0x399: (0x49,),  # Ι -> I
    0x39A: (0x4B,),  # Κ -> K
    0x39C: (0x4D,),  # Μ -> M
    0x39D: (0x4E,),  # Ν -> N
    0x39F: (0x4F,),  # Ο -> O
    0x3A1: (0x50,),  # Ρ -> P
    0x3A4: (0x54,),  # Τ -> T
    0x3A5: (0x59,),  # Υ -> Y
    0x3A7: (0x58,),  # Χ -> X
    0x3BA: (0x138,),  # κ -> ĸ
    0x3BF: (0x6F,),  # ο -> o
    0x3C1: (0x70,),  # ρ -> p
    0x3C7: (0x78,),  # χ -> x
    #  Cyrillic
    0x405: (0x53,),  # Ѕ -> S
    0x406: (0x49,),  # І -> I
    0x408: (0x4A,),  # Ј -> J
    0x410: (0x41,),  # А -> A
    0x412: (0x42,),  # В -> B
    0x413: (0x393,),  # Г -> Γ
    0x415: (0x45,),  # Е -> E
    0x41A: (0x4B,),  # К -> K
    0x41C: (0x4D,),  # М -> M
    0x41D: (0x48,),  # Н -> H
    0x41E: (0x4F,),  # О -> O
    0x41F: (0x3A0,),  # П -> Π
    0x420: (0x50,),  # Р -> P
    0x421: (0x43,),  # С -> C
    0x422: (0x54,),  # Т -> T
    0x424: (0x3A6,),  # Ф -> Φ
    0x425: (0x58,),  # Х -> X
    0x430: (0x61,),  # а -> a
    0x435: (0x65,),  # е -> e
    0x43A: (0x3BA,),  # к -> κ
    0x43E: (0x6F,),  # о -> o
    0x43F: (0x3C0,),  #  Not quite right, but close # п -> π
    0x440: (0x70,),  # р -> p
    0x441: (0x63,),  # с -> c
    0x443: (0x79,),  # у -> y
    0x445: (0x78,),  # х -> x
    0x455: (0x73,),  # ѕ -> s
    0x456: (0x69,),  # і -> i
    0x458: (0x6A,),  # ј -> j
    #  extended Cyrillic
    0x470: (0x3A8,),  # Ѱ -> Ψ
    0x471: (0x3C8,),  # ѱ -> ψ
    0x4AE: (0x59,),  # Ү -> Y
    0x4C0: (0x49,),  # Ӏ -> I
    0x4D4: (0xC6,),  # Ӕ -> Æ
    0x4D5: (0xE6,),  # ӕ -> æ
    0x4E0: (0x1B7,),  # Ӡ -> Ʒ
    0x4E1: (0x292,),  # ӡ -> ʒ
    0x4E8: (0x398,),  # Ө -> Θ
    0x4E9: (0x3B8,),  # ө -> θ
    #  Armenian
    0x54F: (0x53,),  # Տ -> S
    0x555: (0x4F,),  # Օ -> O
    0x570: (0x26F,),  # հ -> ɯ
    0x570: (0x68,),  # հ -> h
    0x578: (0x6E,),  # ո -> n
    0x57A: (0x270,),  # պ -> ɰ
    0x57D: (0x75,),  # ս -> u
    0x581: (0x261,),  # ց -> ɡ
    0x582: (0x269,),  # ւ -> ɩ
    0x584: (0x66,),  # ք -> f
    0x585: (0x6F,),  # օ -> o
    0x589: (0x3A,),  # ։ -> :
    #  Yiddish ligs
    0x5F0: (0x5D5, 0x5D5),  # װ -> וו
    0x5F1: (0x5D5, 0x5D9),  #  0x5d9 should be drawn first (r to l,) # ױ -> וי
    0x5F2: (0x5D9, 0x5D9),  # ײ -> יי
    #  Arabic
    0x60C: (0x2018,),  # ، -> ‘
    0x66A: (0x25,),  # ٪ -> %
    0x66C: (0x2C),  # ٬ -> ,
    0x66D: (0x22C6,),  # ٭ -> ⋆
    0x6D4: (0xB7,),  # ۔ -> ·
    #  Many of the Korean Jamo are ligatures of other Jamo
    #  0x110b often, but not always, rides underneath (0x1135 it's left,)
    #  Chosung
    0x1101: (0x1100, 0x1100),  # ᄁ -> ᄀᄀ
    0x1104: (0x1103, 0x1103),  # ᄄ -> ᄃᄃ
    0x1108: (0x1107, 0x1107),  # ᄈ -> ᄇᄇ
    0x110A: (0x1109, 0x1109),  # ᄊ -> ᄉᄉ
    0x110D: (0x110C, 0x110C),  # ᄍ -> ᄌᄌ
    0x1113: (0x1102, 0x1100),  # ᄓ -> ᄂᄀ
    0x1114: (0x1102, 0x1102),  # ᄔ -> ᄂᄂ
    0x1115: (0x1102, 0x1103),  # ᄕ -> ᄂᄃ
    0x1116: (0x1102, 0x1107),  # ᄖ -> ᄂᄇ
    0x1117: (0x1103, 0x1100),  # ᄗ -> ᄃᄀ
    0x1118: (0x1105, 0x1102),  # ᄘ -> ᄅᄂ
    0x1119: (0x1105, 0x1105),  # ᄙ -> ᄅᄅ
    0x111A: (0x1105, 0x1112),  # ᄚ -> ᄅᄒ
    0x111B: (0x1105, 0x110B),  # ᄛ -> ᄅᄋ
    0x111C: (0x1106, 0x1107),  # ᄜ -> ᄆᄇ
    0x111D: (0x1106, 0x110B),  # ᄝ -> ᄆᄋ
    0x111E: (0x1107, 0x1100),  # ᄞ -> ᄇᄀ
    0x111F: (0x1107, 0x1102),  # ᄟ -> ᄇᄂ
    0x1120: (0x1107, 0x1103),  # ᄠ -> ᄇᄃ
    0x1121: (0x1107, 0x1109),  # ᄡ -> ᄇᄉ
    0x1122: (0x1107, 0x1109, 0x1100),  # ᄢ -> ᄇᄉᄀ
    0x1123: (0x1107, 0x1109, 0x1103),  # ᄣ -> ᄇᄉᄃ
    0x1124: (0x1107, 0x1109, 0x1107),  # ᄤ -> ᄇᄉᄇ
    0x1125: (0x1107, 0x1109, 0x1109),  # ᄥ -> ᄇᄉᄉ
    0x1126: (0x1107, 0x1109, 0x110C),  # ᄦ -> ᄇᄉᄌ
    0x1127: (0x1107, 0x110C),  # ᄧ -> ᄇᄌ
    0x1128: (0x1107, 0x110E),  # ᄨ -> ᄇᄎ
    0x1129: (0x1107, 0x1110),  # ᄩ -> ᄇᄐ
    0x112A: (0x1107, 0x1111),  # ᄪ -> ᄇᄑ
    0x112B: (0x1107, 0x110B),  # ᄫ -> ᄇᄋ
    0x112C: (0x1107, 0x1107, 0x110B),  # ᄬ -> ᄇᄇᄋ
    0x112D: (0x1109, 0x1100),  # ᄭ -> ᄉᄀ
    0x112E: (0x1109, 0x1102),  # ᄮ -> ᄉᄂ
    0x112F: (0x1109, 0x1103),  # ᄯ -> ᄉᄃ
    0x1130: (0x1109, 0x1105),  # ᄰ -> ᄉᄅ
    0x1131: (0x1109, 0x1106),  # ᄱ -> ᄉᄆ
    0x1132: (0x1109, 0x1107),  # ᄲ -> ᄉᄇ
    0x1133: (0x1109, 0x1107, 0x1100),  # ᄳ -> ᄉᄇᄀ
    0x1134: (0x1109, 0x1109, 0x1109),  # ᄴ -> ᄉᄉᄉ
    0x1135: (0x1109, 0x110B),  # ᄵ -> ᄉᄋ
    0x1136: (0x1109, 0x110C),  # ᄶ -> ᄉᄌ
    0x1137: (0x1109, 0x110E),  # ᄷ -> ᄉᄎ
    0x1138: (0x1109, 0x110F),  # ᄸ -> ᄉᄏ
    0x1139: (0x1109, 0x1110),  # ᄹ -> ᄉᄐ
    0x113A: (0x1109, 0x1111),  # ᄺ -> ᄉᄑ
    0x113B: (0x1109, 0x1112),  # ᄻ -> ᄉᄒ
    0x113D: (0x113C, 0x113C),  # ᄽ -> ᄼᄼ
    0x113F: (0x113E, 0x113E),  # ᄿ -> ᄾᄾ
    0x1141: (0x110B, 0x1100),  # ᅁ -> ᄋᄀ
    0x1142: (0x110B, 0x1103),  # ᅂ -> ᄋᄃ
    0x1143: (0x110B, 0x1106),  # ᅃ -> ᄋᄆ
    0x1144: (0x110B, 0x1107),  # ᅄ -> ᄋᄇ
    0x1145: (0x110B, 0x1109),  # ᅅ -> ᄋᄉ
    0x1146: (0x110B, 0x1140),  # ᅆ -> ᄋᅀ
    0x1147: (0x110B, 0x110B),  # ᅇ -> ᄋᄋ
    0x1148: (0x110B, 0x110C),  # ᅈ -> ᄋᄌ
    0x1149: (0x110B, 0x110E),  # ᅉ -> ᄋᄎ
    0x114A: (0x110B, 0x1110),  # ᅊ -> ᄋᄐ
    0x114B: (0x110B, 0x1111),  # ᅋ -> ᄋᄑ
    0x114D: (0x110C, 0x110B),  # ᅍ -> ᄌᄋ
    0x114F: (0x114E, 0x114E),  # ᅏ -> ᅎᅎ
    0x1151: (0x1150, 0x1150),  # ᅑ -> ᅐᅐ
    0x1152: (0x110E, 0x110F),  # ᅒ -> ᄎᄏ
    0x1153: (0x110E, 0x1112),  # ᅓ -> ᄎᄒ
    0x1156: (0x1111, 0x1107),  # ᅖ -> ᄑᄇ
    0x1157: (0x1111, 0x110B),  # ᅗ -> ᄑᄋ
    0x1158: (0x1112, 0x1112),  # ᅘ -> ᄒᄒ
    #  Jungsung
    0x1162: (0x1161, 0x1175),  # ᅢ -> ᅡᅵ
    0x1164: (0x1163, 0x1175),  # ᅤ -> ᅣᅵ
    0x1166: (0x1165, 0x1175),  # ᅦ -> ᅥᅵ
    0x1168: (0x1167, 0x1175),  # ᅨ -> ᅧᅵ
    0x116A: (0x1169, 0x1161),  # ᅪ -> ᅩᅡ
    0x116B: (0x1169, 0x1162),  # ᅫ -> ᅩᅢ
    0x116C: (0x1169, 0x1175),  # ᅬ -> ᅩᅵ
    0x116F: (0x116E, 0x1165),  # ᅯ -> ᅮᅥ
    0x1170: (0x116E, 0x1166),  # ᅰ -> ᅮᅦ
    0x1171: (0x116E, 0x1175),  # ᅱ -> ᅮᅵ
    0x1174: (0x1173, 0x1175),  # ᅴ -> ᅳᅵ
    0x1176: (0x1161, 0x1169),  # ᅶ -> ᅡᅩ
    0x1177: (0x1161, 0x116E),  # ᅷ -> ᅡᅮ
    0x1178: (0x1163, 0x1169),  # ᅸ -> ᅣᅩ
    0x1179: (0x1163, 0x116D),  # ᅹ -> ᅣᅭ
    0x117A: (0x1165, 0x1169),  # ᅺ -> ᅥᅩ
    0x117B: (0x1165, 0x116E),  # ᅻ -> ᅥᅮ
    0x117C: (0x1165, 0x1173),  # ᅼ -> ᅥᅳ
    0x117D: (0x1167, 0x1169),  # ᅽ -> ᅧᅩ
    0x117E: (0x1167, 0x116E),  # ᅾ -> ᅧᅮ
    0x117F: (0x1169, 0x1165),  # ᅿ -> ᅩᅥ
    0x1180: (0x1169, 0x1166),  # ᆀ -> ᅩᅦ
    0x1181: (0x1169, 0x1168),  # ᆁ -> ᅩᅨ
    0x1182: (0x1169, 0x1169),  # ᆂ -> ᅩᅩ
    0x1183: (0x1169, 0x116E),  # ᆃ -> ᅩᅮ
    0x1184: (0x116D, 0x1163),  # ᆄ -> ᅭᅣ
    0x1185: (0x116D, 0x1164),  # ᆅ -> ᅭᅤ
    0x1186: (0x116D, 0x1167),  # ᆆ -> ᅭᅧ
    0x1187: (0x116D, 0x1169),  # ᆇ -> ᅭᅩ
    0x1188: (0x116D, 0x1175),  # ᆈ -> ᅭᅵ
    0x1189: (0x116E, 0x1161),  # ᆉ -> ᅮᅡ
    0x118A: (0x116E, 0x1162),  # ᆊ -> ᅮᅢ
    0x118B: (0x116E, 0x1165, 0x1173),  # ᆋ -> ᅮᅥᅳ
    0x118C: (0x116E, 0x1168),  # ᆌ -> ᅮᅨ
    0x118D: (0x116E, 0x116E),  # ᆍ -> ᅮᅮ
    0x118E: (0x1172, 0x1161),  # ᆎ -> ᅲᅡ
    0x118F: (0x1172, 0x1165),  # ᆏ -> ᅲᅥ
    0x1190: (0x1172, 0x1166),  # ᆐ -> ᅲᅦ
    0x1191: (0x1172, 0x1167),  # ᆑ -> ᅲᅧ
    0x1192: (0x1172, 0x1168),  # ᆒ -> ᅲᅨ
    0x1193: (0x1172, 0x116E),  # ᆓ -> ᅲᅮ
    0x1194: (0x1172, 0x1175),  # ᆔ -> ᅲᅵ
    0x1195: (0x1173, 0x116E),  # ᆕ -> ᅳᅮ
    0x1196: (0x1173, 0x1173),  # ᆖ -> ᅳᅳ
    0x1197: (0x1174, 0x116E),  # ᆗ -> ᅴᅮ
    0x1198: (0x1175, 0x1161),  # ᆘ -> ᅵᅡ
    0x1199: (0x1175, 0x1163),  # ᆙ -> ᅵᅣ
    0x119A: (0x1175, 0x1169),  # ᆚ -> ᅵᅩ
    0x119B: (0x1175, 0x116E),  # ᆛ -> ᅵᅮ
    0x119C: (0x1175, 0x1173),  # ᆜ -> ᅵᅳ
    0x119D: (0x1175, 0x119E),  # ᆝ -> ᅵᆞ
    0x119F: (0x119E, 0x1165),  # ᆟ -> ᆞᅥ
    0x11A0: (0x119E, 0x116E),  # ᆠ -> ᆞᅮ
    0x11A1: (0x119E, 0x1175),  # ᆡ -> ᆞᅵ
    0x11A2: (0x119E, 0x119E),  # ᆢ -> ᆞᆞ
    #  Jongsung
    0x11A8: (0x1100,),  # ᆨ -> ᄀ
    0x11A9: (0x11A8, 0x11A8),  # ᆩ -> ᆨᆨ
    0x11AA: (0x11A8, 0x11BA),  # ᆪ -> ᆨᆺ
    0x11AB: (0x1102,),  # ᆫ -> ᄂ
    0x11AC: (0x11AB, 0x11BD),  # ᆬ -> ᆫᆽ
    0x11AD: (0x11AB, 0x11C2),  # ᆭ -> ᆫᇂ
    0x11AE: (0x1103,),  # ᆮ -> ᄃ
    0x11AF: (0x1105,),  # ᆯ -> ᄅ
    0x11B0: (0x11AF, 0x11A8),  # ᆰ -> ᆯᆨ
    0x11B1: (0x11AF, 0x11B7),  # ᆱ -> ᆯᆷ
    0x11B2: (0x11AF, 0x11B8),  # ᆲ -> ᆯᆸ
    0x11B3: (0x11AF, 0x11BA),  # ᆳ -> ᆯᆺ
    0x11B4: (0x11AF, 0x11C0),  # ᆴ -> ᆯᇀ
    0x11B5: (0x11AF, 0x11C1),  # ᆵ -> ᆯᇁ
    0x11B6: (0x11AF, 0x11C2),  # ᆶ -> ᆯᇂ
    0x11B7: (0x1106,),  # ᆷ -> ᄆ
    0x11B8: (0x1107,),  # ᆸ -> ᄇ
    0x11B9: (0x11B8, 0x11BA),  # ᆹ -> ᆸᆺ
    0x11BA: (0x1109,),  # ᆺ -> ᄉ
    0x11BB: (0x11BA, 0x11BA),  # ᆻ -> ᆺᆺ
    0x11BC: (0x110B,),  # ᆼ -> ᄋ
    0x11BD: (0x110C,),  # ᆽ -> ᄌ
    0x11BE: (0x110E,),  # ᆾ -> ᄎ
    0x11BF: (0x110F,),  # ᆿ -> ᄏ
    0x11C0: (0x1110,),  # ᇀ -> ᄐ
    0x11C1: (0x1111,),  # ᇁ -> ᄑ
    0x11C2: (0x1112,),  # ᇂ -> ᄒ
    0x11C3: (0x11A8, 0x11AF),  # ᇃ -> ᆨᆯ
    0x11C4: (0x11A8, 0x11BA, 0x11A8),  # ᇄ -> ᆨᆺᆨ
    0x11C5: (0x11AB, 0x11A8),  # ᇅ -> ᆫᆨ
    0x11C6: (0x11AB, 0x11AE),  # ᇆ -> ᆫᆮ
    0x11C7: (0x11AB, 0x11BA),  # ᇇ -> ᆫᆺ
    0x11C8: (0x11AB, 0x11EB),  # ᇈ -> ᆫᇫ
    0x11C9: (0x11AB, 0x11C0),  # ᇉ -> ᆫᇀ
    0x11CA: (0x11AE, 0x11A8),  # ᇊ -> ᆮᆨ
    0x11CB: (0x11AE, 0x11AF),  # ᇋ -> ᆮᆯ
    0x11CC: (0x11AF, 0x11A8, 0x11BA),  # ᇌ -> ᆯᆨᆺ
    0x11CD: (0x11AF, 0x11AB),  # ᇍ -> ᆯᆫ
    0x11CE: (0x11AF, 0x11AE),  # ᇎ -> ᆯᆮ
    0x11CF: (0x11AF, 0x11AE, 0x11C2),  # ᇏ -> ᆯᆮᇂ
    0x11D0: (0x11AF, 0x11AF),  # ᇐ -> ᆯᆯ
    0x11D1: (0x11AF, 0x11B7, 0x11A8),  # ᇑ -> ᆯᆷᆨ
    0x11D2: (0x11AF, 0x11B7, 0x11BA),  # ᇒ -> ᆯᆷᆺ
    0x11D3: (0x11AF, 0x11B8, 0x11BA),  # ᇓ -> ᆯᆸᆺ
    0x11D4: (0x11AF, 0x11B8, 0x11C2),  # ᇔ -> ᆯᆸᇂ
    # 0x11d5: (0x11af , 0x11b8 , 0x11bc), # ᇕ -> ᆯᆸᆼ
    0x11D5: (0x11AF, 0x11E6),  # ᇕ -> ᆯᇦ
    0x11D6: (0x11AF, 0x11BA, 0x11BA),  # ᇖ -> ᆯᆺᆺ
    0x11D7: (0x11AF, 0x11EB),  # ᇗ -> ᆯᇫ
    0x11D8: (0x11AF, 0x11BF),  # ᇘ -> ᆯᆿ
    0x11D9: (0x11AF, 0x11F9),  # ᇙ -> ᆯᇹ
    0x11DA: (0x11B7, 0x11A8),  # ᇚ -> ᆷᆨ
    0x11DB: (0x11B7, 0x11AF),  # ᇛ -> ᆷᆯ
    0x11DC: (0x11B7, 0x11B8),  # ᇜ -> ᆷᆸ
    0x11DD: (0x11B7, 0x11BA),  # ᇝ -> ᆷᆺ
    0x11DE: (0x11B7, 0x11BA, 0x11BA),  # ᇞ -> ᆷᆺᆺ
    0x11DF: (0x11B7, 0x11EB),  # ᇟ -> ᆷᇫ
    0x11E0: (0x11B7, 0x11BE),  # ᇠ -> ᆷᆾ
    0x11E1: (0x11B7, 0x11C2),  # ᇡ -> ᆷᇂ
    0x11E2: (0x11B7, 0x11BC),  # ᇢ -> ᆷᆼ
    0x11E3: (0x11B8, 0x11AF),  # ᇣ -> ᆸᆯ
    0x11E4: (0x11B8, 0x11C1),  # ᇤ -> ᆸᇁ
    0x11E5: (0x11B8, 0x11C2),  # ᇥ -> ᆸᇂ
    0x11E6: (0x11B8, 0x11BC),  # ᇦ -> ᆸᆼ
    0x11E7: (0x11BA, 0x11A8),  # ᇧ -> ᆺᆨ
    0x11E8: (0x11BA, 0x11AE),  # ᇨ -> ᆺᆮ
    0x11E9: (0x11BA, 0x11AF),  # ᇩ -> ᆺᆯ
    0x11EA: (0x11BA, 0x11B8),  # ᇪ -> ᆺᆸ
    0x11EB: (0x1140,),  # ᇫ -> ᅀ
    0x11EC: (0x11BC, 0x11A8),  # ᇬ -> ᆼᆨ
    0x11ED: (0x11BC, 0x11A8, 0x11A8),  # ᇭ -> ᆼᆨᆨ
    0x11EE: (0x11BC, 0x11BC),  # ᇮ -> ᆼᆼ
    0x11EF: (0x11BC, 0x11BF),  # ᇯ -> ᆼᆿ
    0x11F0: (0x114C,),  # ᇰ -> ᅌ
    0x11F1: (0x11F0, 0x11BA),  # ᇱ -> ᇰᆺ
    0x11F2: (0x11F0, 0x11EB),  # ᇲ -> ᇰᇫ
    0x11F3: (0x11C1, 0x11B8),  # ᇳ -> ᇁᆸ
    0x11F4: (0x11C1, 0x11BC),  # ᇴ -> ᇁᆼ
    0x11F5: (0x11C2, 0x11AB),  # ᇵ -> ᇂᆫ
    0x11F6: (0x11C2, 0x11AF),  # ᇶ -> ᇂᆯ
    0x11F7: (0x11C2, 0x11B7),  # ᇷ -> ᇂᆷ
    0x11F8: (0x11C2, 0x11B8),  # ᇸ -> ᇂᆸ
    0x11F9: (0x1159,),  # ᇹ -> ᅙ
    #  Cherokee
    0x13A0: (0x44,),  # Ꭰ -> D
    0x13A1: (0x52,),  # Ꭱ -> R
    0x13A2: (0x54,),  # Ꭲ -> T
    0x13A2: (0x54,),  # Ꭲ -> T
    0x13A9: (0x423,),  # Ꭹ -> У
    0x13AA: (0x41,),  # Ꭺ -> A
    0x13AB: (0x4A,),  # Ꭻ -> J
    0x13AC: (0x45,),  # Ꭼ -> E
    0x13B1: (0x393,),  # Ꮁ -> Γ
    0x13B3: (0x57,),  # Ꮃ -> W
    0x13B7: (0x4D,),  # Ꮇ -> M
    0x13BB: (0x48,),  # Ꮋ -> H
    0x13BE: (0x398,),  # Ꮎ -> Θ
    0x13C0: (0x47,),  # Ꮐ -> G
    0x13C2: (0x68,),  # Ꮒ -> h
    0x13C3: (0x5A,),  # Ꮓ -> Z
    0x13CF: (0x42C,),  # Ꮟ -> Ь
    0x13D9: (0x56,),  # Ꮩ -> V
    0x13DA: (0x53,),  # Ꮪ -> S
    0x13DE: (0x4C,),  # Ꮮ -> L
    0x13DF: (0x43,),  # Ꮯ -> C
    0x13E2: (0x50,),  # Ꮲ -> P
    0x13E6: (0x4B,),  # Ꮶ -> K
    0x13F4: (0x42,),  # Ᏼ -> B
    #  punctuation
    0x2000: (0x20,),  #   ->
    0x2001: (0x20,),  #   ->
    0x2010: (0x2D,),  # ‐ -> -
    0x2011: (0x2D,),  # ‑ -> -
    0x2012: (0x2D,),  # ‒ -> -
    0x2013: (0x2D,),  # – -> -
    0x2014: (0x2D,),  # — -> -
    0x2015: (0x2D,),  # ― -> -
    0x2016: (0x7C, 0x7C),  # ‖ -> ||
    0x2018: (0x60,),  # ‘ -> `
    0x2019: (0x27,),  # ’ -> '
    0x201C: (0x27,),  # “ -> '
    0x201D: (0x27,),  # ” -> '
    0x2024: (0x2E,),  # ․ -> .
    0x2025: (0x2E, 0x2E),  # ‥ -> ..
    0x2026: (0x2E, 0x2E, 0x2E),  # … -> ...
    0x2032: (0x27,),  # ′ -> '
    0x2033: (0x27,),  # ″ -> '
    0x2035: (0x60,),  # ‵ -> `
    0x2036: (0x27,),  # ‶ -> '
    0x2039: (0x3C,),  # ‹ -> <
    0x203A: (0x3E,),  # › -> >
    0x203C: (0x21, 0x21),  # ‼ -> !!
    0x2048: (0x3F, 0x21),  # ⁈ -> ?!
    0x2049: (0x21, 0x3F),  # ⁉ -> !?
    0x2126: (0x3A9,),  # Ω -> Ω
    #  Mathematical operators
    0x2205: (0xD8,),  # ∅ -> Ø
    0x2206: (0x394,),  # ∆ -> Δ
    0x220F: (0x3A0,),  # ∏ -> Π
    0x2211: (0x3A3,),  # ∑ -> Σ
    0x2212: (0x2D,),  # − -> -
    0x2215: (0x2F,),  # ∕ -> /
    0x2216: (0x5C,),  # ∖ -> \
    0x2217: (0x2A,),  # ∗ -> *
    0x2218: (0xB0,),  # ∘ -> °
    0x2219: (0xB7,),  # ∙ -> ·
    0x2223: (0x7C,),  # ∣ -> |
    0x2225: (0x7C, 0x7C),  # ∥ -> ||
    0x2236: (0x3A,),  # ∶ -> :
    0x223C: (0x7E,),  # ∼ -> ~
    0x226A: (0xAB,),  # ≪ -> «
    0x226B: (0xBB,),  # ≫ -> »
    0x2299: (0x298,),  # ⊙ -> ʘ
    0x22C4: (0x25CA,),  # ⋄ -> ◊
    0x22C5: (0xB7,),  # ⋅ -> ·
    0x22EF: (0xB7, 0xB7, 0xB7),  # ⋯ -> ···
    #  Misc Technical
    0x2303: (0x5E,),  # ⌃ -> ^
    #  APL greek
    0x2373: (0x3B9,),  # ⍳ -> ι
    0x2374: (0x3C1,),  # ⍴ -> ρ
    0x2375: (0x3C9,),  # ⍵ -> ω
    0x237A: (0x3B1,),  # ⍺ -> α
    #  names of control chars
    0x2400: (0x4E, 0x55, 0x4C),  # ␀ -> NUL
    0x2401: (0x53, 0x4F, 0x48),  # ␁ -> SOH
    0x2402: (0x53, 0x54, 0x58),  # ␂ -> STX
    0x2403: (0x45, 0x54, 0x58),  # ␃ -> ETX
    0x2404: (0x45, 0x4F, 0x54),  # ␄ -> EOT
    0x2405: (0x45, 0x4E, 0x41),  # ␅ -> ENA
    0x2406: (0x41, 0x43, 0x4B),  # ␆ -> ACK
    0x2407: (0x42, 0x45, 0x4C),  # ␇ -> BEL
    0x2408: (0x42, 0x53),  # ␈ -> BS
    0x2409: (0x48, 0x54),  # ␉ -> HT
    0x240A: (0x4C, 0x46),  # ␊ -> LF
    0x240B: (0x56, 0x54),  # ␋ -> VT
    0x240C: (0x46, 0x46),  # ␌ -> FF
    0x240D: (0x43, 0x52),  # ␍ -> CR
    0x240E: (0x53, 0x4F),  # ␎ -> SO
    0x240F: (0x53, 0x49),  # ␏ -> SI
    0x2410: (0x44, 0x4C, 0x45),  # ␐ -> DLE
    0x2411: (0x44, 0x43, 0x31),  # ␑ -> DC1
    0x2412: (0x44, 0x43, 0x32),  # ␒ -> DC2
    0x2413: (0x44, 0x43, 0x33),  # ␓ -> DC3
    0x2414: (0x44, 0x43, 0x34),  # ␔ -> DC4
    0x2415: (0x4E, 0x41, 0x4B),  # ␕ -> NAK
    0x2416: (0x53, 0x59, 0x4E),  # ␖ -> SYN
    0x2417: (0x45, 0x54, 0x42),  # ␗ -> ETB
    0x2418: (0x43, 0x41, 0x4E),  # ␘ -> CAN
    0x2419: (0x45, 0x4D),  # ␙ -> EM
    0x241A: (0x53, 0x55, 0x42),  # ␚ -> SUB
    0x241B: (0x45, 0x53, 0x43),  # ␛ -> ESC
    0x241C: (0x46, 0x53),  # ␜ -> FS
    0x241D: (0x47, 0x53),  # ␝ -> GS
    0x241E: (0x52, 0x53),  # ␞ -> RS
    0x241F: (0x55, 0x53),  # ␟ -> US
    0x2420: (0x53, 0x50),  # ␠ -> SP
    0x2421: (0x44, 0x45, 0x4C),  # ␡ -> DEL
    0x2422: (0x180,),  # ␢ -> ƀ
    0x2500: (0x2014,),  # ─ -> —
    0x2502: (0x7C,),  # │ -> |
    0x25B3: (0x2206,),  # △ -> ∆
    0x25B8: (0x2023,),  # ▸ -> ‣
    0x25BD: (0x2207,),  # ▽ -> ∇
    0x25C7: (0x25CA,),  # ◇ -> ◊
    0x25E6: (0xB0,),  # ◦ -> °
    0x2662: (0x25CA,),  # ♢ -> ◊
    0x2731: (0x2A,),  # ✱ -> *
    0x2758: (0x7C,),  # ❘ -> |
    0x2762: (0x21,),  # ❢ -> !
    #  Idiographic symbols
    0x3001: (0x2C),  # 、 -> ,
    0x3008: (0x3C,),  # 〈 -> <
    0x3009: (0x3E,),  # 〉 -> >
    0x300A: (0xAB,),  # 《 -> «
    0x300B: (0xBB,),  # 》 -> »
    #  The Hangul Compatibility Jamo are just copies of the real Jamo
    #   (different spacing semantics though,)
    0x3131: (0x1100,),  # ㄱ -> ᄀ
    0x3132: (0x1101,),  # ㄲ -> ᄁ
    0x3133: (0x11AA,),  # ㄳ -> ᆪ
    0x3134: (0x1102,),  # ㄴ -> ᄂ
    0x3135: (0x11AC,),  # ㄵ -> ᆬ
    0x3136: (0x11AD,),  # ㄶ -> ᆭ
    0x3137: (0x1103,),  # ㄷ -> ᄃ
    0x3138: (0x1104,),  # ㄸ -> ᄄ
    0x3139: (0x1105,),  # ㄹ -> ᄅ
    0x313A: (0x11B0,),  # ㄺ -> ᆰ
    0x313B: (0x11B1,),  # ㄻ -> ᆱ
    0x313C: (0x11B2,),  # ㄼ -> ᆲ
    0x313D: (0x11B3,),  # ㄽ -> ᆳ
    0x313E: (0x11B4,),  # ㄾ -> ᆴ
    0x313F: (0x11B5,),  # ㄿ -> ᆵ
    0x3140: (0x111A,),  # ㅀ -> ᄚ
    0x3141: (0x1106,),  # ㅁ -> ᄆ
    0x3142: (0x1107,),  # ㅂ -> ᄇ
    0x3143: (0x1108,),  # ㅃ -> ᄈ
    0x3144: (0x1121,),  # ㅄ -> ᄡ
    0x3145: (0x1109,),  # ㅅ -> ᄉ
    0x3146: (0x110A,),  # ㅆ -> ᄊ
    0x3147: (0x110B,),  # ㅇ -> ᄋ
    0x3148: (0x110C,),  # ㅈ -> ᄌ
    0x3149: (0x110D,),  # ㅉ -> ᄍ
    0x314A: (0x110E,),  # ㅊ -> ᄎ
    0x314B: (0x110F,),  # ㅋ -> ᄏ
    0x314C: (0x1110,),  # ㅌ -> ᄐ
    0x314D: (0x1111,),  # ㅍ -> ᄑ
    0x314E: (0x1112,),  # ㅎ -> ᄒ
    0x314F: (0x1161,),  # ㅏ -> ᅡ
    0x3150: (0x1162,),  # ㅐ -> ᅢ
    0x3151: (0x1163,),  # ㅑ -> ᅣ
    0x3152: (0x1164,),  # ㅒ -> ᅤ
    0x3153: (0x1165,),  # ㅓ -> ᅥ
    0x3154: (0x1166,),  # ㅔ -> ᅦ
    0x3155: (0x1167,),  # ㅕ -> ᅧ
    0x3156: (0x1168,),  # ㅖ -> ᅨ
    0x3157: (0x1169,),  # ㅗ -> ᅩ
    0x3158: (0x116A,),  # ㅘ -> ᅪ
    0x3159: (0x116B,),  # ㅙ -> ᅫ
    0x315A: (0x116C,),  # ㅚ -> ᅬ
    0x315B: (0x116D,),  # ㅛ -> ᅭ
    0x315C: (0x116E,),  # ㅜ -> ᅮ
    0x315D: (0x116F,),  # ㅝ -> ᅯ
    0x315E: (0x1170,),  # ㅞ -> ᅰ
    0x315F: (0x1171,),  # ㅟ -> ᅱ
    0x3160: (0x1172,),  # ㅠ -> ᅲ
    0x3161: (0x1173,),  # ㅡ -> ᅳ
    0x3162: (0x1174,),  # ㅢ -> ᅴ
    0x3163: (0x1175,),  # ㅣ -> ᅵ
    0x3164: (0x1160,),  # ㅤ -> ᅠ
    0x3165: (0x1114,),  # ㅥ -> ᄔ
    0x3166: (0x1115,),  # ㅦ -> ᄕ
    0x3167: (0x11C7,),  # ㅧ -> ᇇ
    0x3168: (0x11C8,),  # ㅨ -> ᇈ
    0x3169: (0x11CC,),  # ㅩ -> ᇌ
    0x316A: (0x11CE,),  # ㅪ -> ᇎ
    0x316B: (0x11D3,),  # ㅫ -> ᇓ
    0x316C: (0x11D7,),  # ㅬ -> ᇗ
    0x316D: (0x11D9,),  # ㅭ -> ᇙ
    0x316E: (0x111C,),  # ㅮ -> ᄜ
    0x316F: (0x11DD,),  # ㅯ -> ᇝ
    0x3170: (0x11DF,),  # ㅰ -> ᇟ
    0x3171: (0x111D,),  # ㅱ -> ᄝ
    0x3172: (0x111E,),  # ㅲ -> ᄞ
    0x3173: (0x1120,),  # ㅳ -> ᄠ
    0x3174: (0x1122,),  # ㅴ -> ᄢ
    0x3175: (0x1123,),  # ㅵ -> ᄣ
    0x3176: (0x1127,),  # ㅶ -> ᄧ
    0x3177: (0x1129,),  # ㅷ -> ᄩ
    0x3178: (0x112B,),  # ㅸ -> ᄫ
    0x3179: (0x112C,),  # ㅹ -> ᄬ
    0x317A: (0x112D,),  # ㅺ -> ᄭ
    0x317B: (0x112E,),  # ㅻ -> ᄮ
    0x317C: (0x112F,),  # ㅼ -> ᄯ
    0x317D: (0x1132,),  # ㅽ -> ᄲ
    0x317E: (0x1136,),  # ㅾ -> ᄶ
    0x317F: (0x1140,),  # ㅿ -> ᅀ
    0x3180: (0x1147,),  # ㆀ -> ᅇ
    0x3181: (0x114C,),  # ㆁ -> ᅌ
    0x3182: (0x11F1,),  # ㆂ -> ᇱ
    0x3183: (0x11F2,),  # ㆃ -> ᇲ
    0x3184: (0x1157,),  # ㆄ -> ᅗ
    0x3185: (0x1158,),  # ㆅ -> ᅘ
    0x3186: (0x1159,),  # ㆆ -> ᅙ
    0x3187: (0x1184,),  # ㆇ -> ᆄ
    0x3188: (0x1185,),  # ㆈ -> ᆅ
    0x3189: (0x1188,),  # ㆉ -> ᆈ
    0x318A: (0x1191,),  # ㆊ -> ᆑ
    0x318B: (0x1192,),  # ㆋ -> ᆒ
    0x318C: (0x1194,),  # ㆌ -> ᆔ
    0x318D: (0x119E,),  # ㆍ -> ᆞ
    0x318E: (0x11A1,),  # ㆎ -> ᆡ
    #  similar double brackets
    0xFF5F: (0x2E28,),  # ｟ -> ⸨
    0x2E28: (0xFF5F,),  # ⸨ -> ｟
    0xFF60: (0x2E29,),  # ｠ -> ⸩
    0x2E29: (0xFF60,),  # ⸩ -> ｠
}


class UcdTypeFlags(enum.IntFlag):  # rough character counts:
    FF_UNICODE_ISUNICODEPOINTASSIGNED = 0x1  # all
    FF_UNICODE_ISALPHA = 0x2  # 49471
    FF_UNICODE_ISIDEOGRAPHIC = 0x4  # 28044
    FF_UNICODE_ISLEFTTORIGHT = 0x10  # 57906
    FF_UNICODE_ISRIGHTTOLEFT = 0x20  # 1286
    FF_UNICODE_ISLOWER = 0x40  # 1434
    FF_UNICODE_ISUPPER = 0x80  # 1119
    FF_UNICODE_ISDIGIT = 0x100  # 758
    FF_UNICODE_ISNUMERIC = 0x200  # 1835
    FF_UNICODE_ISLIGVULGFRAC = 0x400  # 615
    FF_UNICODE_ISCOMBINING = 0x800  # 2295
    FF_UNICODE_ISZEROWIDTH = 0x1000  # 404 Really ISDEFAULTIGNORABLE now
    FF_UNICODE_ISEURONUMERIC = 0x2000  # 168
    FF_UNICODE_ISEURONUMTERM = 0x4000  # 76


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

        with open_data(UNIHAN, version) as file:
            zip = zipfile.ZipFile(file)
            if version == "3.2.0":
                data = zip.open("Unihan-3.2.0.txt").read()
            else:
                data = zip.open("Unihan_NumericValues.txt").read()
        for line in data.decode("utf-8").splitlines():
            if not line.startswith("U+"):
                continue
            code, tag, value = line.split(None, 3)[:3]
            if tag not in ("kAccountingNumeric", "kPrimaryNumeric", "kOtherNumeric"):
                continue
            value = value.strip().replace(",", "")
            i = int(code[2:], 16)
            # Patch the numeric field
            if table[i] is not None:
                table[i].numeric_value = value


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
                switches.setdefault("arabnumeric", []).append(char)
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
            if record.numeric_value:
                flags |= UcdTypeFlags.FF_UNICODE_ISNUMERIC
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

        for name, flag in [
            ("ff_unicode_isideoalpha", "FF_UNICODE_ISALPHA | FF_UNICODE_ISIDEOGRAPHIC"),
            ("ff_unicode_isalnum", "FF_UNICODE_ISALPHA | FF_UNICODE_ISNUMERIC"),
        ]:
            fprint("int %s(unichar_t ch) {" % name)
            fprint("    return _GetRecord(ch)->flags & (%s);" % flag)
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
        + [
            ("int", "ff_unicode_is" + k)
            for k in chain(["ideoalpha", "alnum"], switches)
        ]
        + [("unichar_t", "ff_unicode_to" + n) for n in conv_types]
    )


def makeunicodedata(unicode, trace):
    FILE = "unicodedata_db.h"

    print("--- Preparing", FILE, "...")

    decomp_data = [0]
    decomp_prefix = [""]
    decomp_index = [0] * len(unicode.chars)
    decomp_lookup = {}
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
                decomp_lookup[char] = decomp[1:]
                try:
                    i = decomp_data.index(decomp)
                except ValueError:
                    i = len(decomp_data)
                    decomp_data.extend(decomp)
                    decomp_size = decomp_size + len(decomp) * 2
            else:
                i = 0
            decomp_index[char] = i
    return decomp_lookup

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
        # splitbins3(decomp_index)
        index1, index2, shift = splitbins(decomp_index, trace)

        fprint("/* decomposition data */")
        Array("decomp_data", decomp_data).dump(fp, trace)

        fprint("/* index tables for the decomposition data */")
        fprint("#define DECOMP_SHIFT", shift)
        Array("decomp_index1", index1).dump(fp, trace)
        Array("decomp_index2", index2).dump(fp, trace)


def makearabicforms(unicode, trace):
    FILE = "ArabicForm2.c"

    print("--- Preparing", FILE, "...")

    # Could compress this a little with a single stage lookup but oh well
    table = []
    lookup = {x.name: i for i, x in enumerate(unicode.table) if x and x.name}

    for char in range(0x600, 0x700):
        record = unicode.table[char]
        initial, medial, final, isolated, isletter, joindual = 0, 0, 0, 0, 0, 0
        required_lig_with_alef = char == 0x644  # 0x644 == LAM

        if record is not None:
            if not record.name.startswith("ARABIC LETTER "):
                # No op (not a letter, no fancy forms)
                initial = medial = final = isolated = char
            else:
                isletter = 1
                initial = lookup.get(record.name + " INITIAL FORM", char)
                medial = lookup.get(record.name + " MEDIAL FORM", char)
                final = lookup.get(record.name + " FINAL FORM", char)
                isolated = lookup.get(record.name + " ISOLATED FORM", char)
                joindual = int(initial != char and medial != char)
        table.append(
            (
                initial,
                medial,
                final,
                isolated,
                isletter,
                joindual,
                required_lig_with_alef,
            )
        )

    print("--- Writing", FILE, "...")

    with open(FILE, "w") as fp:
        fprint = partial(print, file=fp)
        fprint(LICENSE)
        fprint("#include <utype.h>")
        fprint()
        fprint("struct arabicforms ArabicForms[] = {")
        fprint(
            "\t/* initial, medial, final, isolated, isletter, joindual, required_lig_with_alef */"
        )
        for i, form in enumerate(table):
            fp.write("\t{ 0x%04x, 0x%04x, 0x%04x, 0x%04x, %d, %d, %d }," % form)
            fprint(f"\t/* 0x{0x600+i:04x} */" if (i % 32) == 0 else "")
        fprint("};")


def makeutypeheader(utype_funcs):
    FILE = "utype2.h"
    utype_funcs = [(t, n, n.replace("ff_unicode_", "")) for t, n in utype_funcs]

    print("--- Writing", FILE, "...")

    with open(FILE, "w") as fp:
        fprint = partial(print, file=fp)

        fprint(LICENSE)
        fprint("#ifndef FONTFORGE_UNICODE_UTYPE2_H")
        fprint("#define FONTFORGE_UNICODE_UTYPE2_H")
        fprint()
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

        fprint("extern struct arabicforms {")
        fprint("    unsigned short initial, medial, final, isolated;")
        fprint("    unsigned int isletter: 1;")
        fprint("    unsigned int joindual: 1;")
        fprint("    unsigned int required_lig_with_alef: 1;")
        fprint(
            "} ArabicForms[256];	/* for chars 0x600-0x6ff, subtract 0x600 to use array */"
        )
        fprint()

        fprint("#endif /* FONTFORGE_UNICODE_UTYPE2_H */")


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
    makearabicforms(unicode, trace)
    makeutypeheader(funcs)


if __name__ == "__main__":
    maketables(1)
