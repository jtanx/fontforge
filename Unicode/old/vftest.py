import fontforge
import sys
import unicodedata

from data2 import alts

def getname(x):
    try:
        return chr(x)
    except:
        return "<DOESNTEXIST>" #lol

def getuname(x):
    try:
        return unicodedata.name(chr(x))
    except Exception as e:
        return ''

for i in range(0x10ffff):
    loc = fontforge.ucVulChartGetLoc(i)
    isvulg = 'VULGAR' in getuname(i)
    if loc < 0:
        if isvulg:
            print('WTF', i)
        continue
    elif not isvulg:
        print(f'NOTF: U+{i:04x} ({getname(i)}) ({getuname(i)})')
    cnt = fontforge.ucVulChartGetAltCnt(loc)
    if cnt <= 0:
        continue

    av = []
    for j in range(cnt):
        alt = fontforge.ucVulChartGetAltVal(loc, j)
        av.append(alt)
        print(f'U+{i:04x} ({getname(i)}) {j}/{cnt}: U+{alt:04x} ({getname(alt)})')
    
    if av and av != alts.get(i):
        print(f'U+{i:04x} DIFF: GOT {av}, ALTS HAS {alts.get(i)}')

for i in range(0x10ffff):
    loc = fontforge.ucOFracChartGetLoc(i)
    isfrac = 'FRACTION' in getuname(i) and 'VULGAR' not in getuname(i)
    if loc < 0:
        if isfrac:
            print('WTF', i)
        continue
    elif not isfrac:
        print(f'NOTF: U+{i:04x} ({getname(i)}) ({getuname(i)})')
    cnt = fontforge.ucOFracChartGetAltCnt(loc)
    if cnt <= 0:
        continue
    av = []
    for j in range(cnt):
        alt = fontforge.ucOFracChartGetAltVal(loc, j)
        av.append(alt)
        print(f'U+{i:04x} ({getname(i)}) {j}/{cnt}: U+{alt:04x} ({getname(alt)})')
    
    if av and av != alts.get(i):
        print(f'U+{i:04x} DIFF: GOT {av}, ALTS HAS {alts.get(i)}')



for i in range(0x10ffff):
    loc = fontforge.ucLigChartGetLoc(i)
    islig = 'LIGATURE' in getuname(i)
    if loc < 0:
        if islig:
            print('WTF', i)
        continue
    elif not islig:
        print(f'NOTF: U+{i:04x} ({getname(i)}) ({getuname(i)})')
    cnt = fontforge.ucLigChartGetAltCnt(loc)
    if cnt <= 0:
        continue
    print(f'U+{i:04x} ({getname(i)}) {cnt} alts:', end=' ' )
    av = []
    for j in range(cnt):
        alt = fontforge.ucLigChartGetAltVal(loc, j)
        av.append(alt)
        print(f'U+{alt:04x} ({getname(alt)})', end=' ')
    if av and av != alts.get(i):
        print(f'U+{i:04x} DIFF: GOT {av}, ALTS HAS {alts.get(i)}')
    print()