# Based on the algorithm in https://github.com/python/cpython/blob/master/Tools/unicode/makeunicodedata.py

import math
import sys

def getsize(data):
    # return smallest possible integer size for the given array
    maxdata = max(data)
    if maxdata < 256:
        return 1
    elif maxdata < 65536:
        return 2
    else:
        return 4

def splitbinso(t, trace=0):
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
            print("%d+%d bins at shift %d; %d bytes" % (
                len(t1), len(t2), shift, bytes), file=sys.stderr)
        print("Size of original table:", len(t)*getsize(t), "bytes",
              file=sys.stderr)
    n = len(t)-1    # last valid index
    maxshift = 0    # the most we can shift n and still have something left
    if n > 0:
        while n >> 1:
            n >>= 1
            maxshift += 1
    del n
    bytes = sys.maxsize  # smallest total size so far
    t = tuple(t)    # so slices can be dict keys
    for shift in range(maxshift + 1):
        t1 = []
        t2 = []
        size = 2**shift
        bincache = {}
        for i in range(0, len(t), size):
            bin = t[i:i+size]
            index = bincache.get(bin)
            if index is None:
                index = len(t2)
                bincache[bin] = index
                t2.extend(bin)
            t1.append(index >> shift)
        # determine memory size
        b = len(t1)*getsize(t1) + len(t2)*getsize(t2)
        if trace > 1:
            dump(t1, t2, shift, b)
        if b < bytes:
            best = t1, t2, shift
            bytes = b
    t1, t2, shift = best
    if trace:
        print("Best:", end=' ', file=sys.stderr)
        dump(t1, t2, shift, bytes)
    if __debug__:
        # exhaustively verify that the decomposition is correct
        mask = ~((~0) << shift) # i.e., low-bit mask of shift bits
        for i in range(len(t)):
            assert t[i] == t2[(t1[i >> shift] << shift) + (i & mask)]
    return best

def splitbins(arr):
  maxshift = max(math.frexp(len(arr)-1)[1] - 1, 0)

  data = tuple(arr)

  maxsize = sys.maxsize
  best = None
  
  for shift1 in range(maxshift + 1):
    for shift2 in range(0, shift1 + 1):
      t1, t2, t3 = [], [], []
      size1 = 1 << shift1
      size2 = 1 << shift2
      mask1 = ~((~0)<<(shift1 - shift2))
      mask2 = ~((~0)<<shift2)
      t2cache, t3cache = {}, {}

      # print("SHIFTS", shift1, shift2, "SIZES", size1, size2)

      for k in range(0, len(data), size1):
        # steps of A
        indices = []
        for l in range(0, size1, size2):
          # steps of B
          # print("  BIN SIZE", size2, k+l, k+l+size2)
          bin = data[k+l:k+l+size2]
          index = t3cache.get(bin)
          if index is None:
            index = len(t3)
            t3cache[bin] = index
            t3.extend(bin)
          indices.append(index)
  
        bin = tuple(indices)
        # print("  INDICES", bin)
        index = t2cache.get(bin)
        if index is None:
          index = len(t2)
          t2cache[bin] = index
          t2.extend(bin)

        t1.append(index)

      size = len(t1) * getsize(t1) + len(t2) * getsize(t2) + len(t3) * getsize(t3)
      if size < maxsize:
        maxsize = size
        best = (shift1, shift2, t1, t2, t3)
      
      # for i in range(len(data)):
      #   v = arr[i]
      #   vv = t3[t2[t1[i >> shift1] + ((i >> shift2) & mask1)] + (i & mask2)]
      #   if v != vv:
      #     print("WTF", "Shifts", shift1, shift2, "At", i, "values",  v, "vs", vv)
      #     return None

      print(shift1, shift2, len(t1), len(t2), len(t3), size)
  print("MIN", maxsize, best[0], best[1])
  return best
