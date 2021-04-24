#include <stdio.h>
#include <ustring.h>
// #include "utype2.h"

#include <assert.h>
#include <glib.h>
#include <uninameslist.h>

#if 0
int main() {
    for (int j = 0; j < 100; ++j) {
        for (int i = 0; i < 0x110000; ++i) {
            char *name = copy(uniNamesList_name(i));
            // char *annot = copy(uniNamesList_annot(i));

            // char* name = uniname_name(i);
            // char* annot = uniname_annot(i);

            free(name);
            // free(annot);
        }
    }
}
#endif

#if 0
int main() {

    int LOOPS=1000, MAX=0x11000;
    int c = 0;
    for (int i = 0; i < MAX; ++i) {
        free(uniname_annotation(i));
    }

    int64_t n = g_get_monotonic_time();
    for (int j = 0; j < LOOPS; ++j) {
        for (int i = 0; i < MAX; ++i) {
            char *ret = uniname_annotation(i);
            c += ret != NULL;
            free(ret);
        }
    }

    int64_t d = g_get_monotonic_time() - n;
    printf("%.4fs total, %.2fus/call\n", d*1e-6, ((double)d)/c);
}
#endif

#if 0

int main() {
    for (int i = 0; i < 0x120000; ++i) {
        const struct unicode_range* range = uniname_plane(i);
        if (range != NULL) {
            assert(i >= range->start && i <= range->end);
            printf("U+%04X: %s (U+%04x-U+%04x)\n", i, range->name, range->start, range->end);
        } else {
            assert(!isunicodepointassigned(i));
        }
    }
    return 0;
}
#endif

#if 0
#include <utype.h>

#define BREAK_AFTER 0x1
#define BREAK_BEFORE 0x2
#define BREAK_NONSTART 0x4
#define BREAK_NONEND 0x8
static int BreakClassify(unichar_t ch) {
    int flags = 0;
    switch (g_unichar_break_type(ch)) {
        case G_UNICODE_BREAK_SPACE:
        case G_UNICODE_BREAK_HYPHEN:
        case G_UNICODE_BREAK_AFTER:
        case G_UNICODE_BREAK_ZERO_WIDTH_SPACE:
            flags |= BREAK_AFTER;
            break;
        case G_UNICODE_BREAK_BEFORE:
            flags |= BREAK_BEFORE;
            break;
        case G_UNICODE_BREAK_BEFORE_AND_AFTER:
        case G_UNICODE_BREAK_IDEOGRAPHIC:
            flags |= BREAK_AFTER | BREAK_BEFORE;
            break;
        case G_UNICODE_BREAK_NON_STARTER:
        case G_UNICODE_BREAK_CLOSE_PUNCTUATION:
            flags |= BREAK_NONSTART;
            break;
        case G_UNICODE_BREAK_NON_BREAKING_GLUE:
            flags |= BREAK_NONSTART | BREAK_NONEND;
            break;
        case G_UNICODE_BREAK_OPEN_PUNCTUATION:
        case G_UNICODE_BREAK_COMBINING_MARK:
            flags |= BREAK_NONEND;
            break;
        default:
            break;
    }
    return flags;
}

// This is a copy of the algorithm from the old utype.c/makeutype.c
// See https://github.com/fontforge/fontforge/blob/a5dedb4010cd49a5fcfaeed5d188dd7942294005/Unicode/makeutype.c#L687-L706
static int IsBreakBetweenOk(unichar_t ch1, unichar_t ch2) {
    int b1 = BreakClassify(ch1), b2 = BreakClassify(ch2);
    return (
        ((b1 & BREAK_AFTER) && !(b2 & BREAK_NONSTART)) ||
        ((b2 & BREAK_BEFORE) && !(b1 & BREAK_NONEND)) ||
        (!isdigit(ch2) && ch1 == '/')
    );
}

int main() {
    for (int i = 1; i < 0x10000; ++i) {
        if (isunicodepointassigned(i-1) && isunicodepointassigned(i)) {
            assert(isbreakbetweenok(i-1, i) == IsBreakBetweenOk(i-1, i));
        }
    }
}
#endif

#if 0
#include <utype.h>

int64_t flags[0x110000] = {0};
int64_t nflags[0x110000] = {0};
const char* flagnames[] = {
    "isunicodepointassigned",
    "isalpha",
    "isideographic",
    "islefttoright",
    "isrighttoleft",
    "islower",
    "isupper",
    "isdigit",
    "isligvulgfrac",
    "iscombining",
    "iszerowidth",
    "iseuronumeric",
    "iseuronumterm",
    "isarabnumeric",
    "isdecompositionnormative",
    "isarabinitial",
    "isarabmedial",
    "isarabfinal",
    "isarabisolated",
    "isideoalpha",
    "isalnum",
    "isspace",
    "iseuronumsep",
    "iscommonsep",
    "ishexdigit",
    "istitle",
};

void doold() {
    for (int i = 0; i < 0x110000; ++i) {
        if (isunicodepointassigned(i)) flags[i] |= 1LL << 0;
        if (isalpha(i)) flags[i] |= 1LL << 1;
        if (isideographic(i)) flags[i] |= 1LL << 2;
        if (islefttoright(i)) flags[i] |= 1LL << 3;
        if (isrighttoleft(i)) flags[i] |= 1LL << 4;
        if (islower(i)) flags[i] |= 1LL << 5;
        if (isupper(i)) flags[i] |= 1LL << 6;
        if (isdigit(i)) flags[i] |= 1LL << 7;
        if (isligorfrac(i)) flags[i] |= 1LL << 8;
        if (iscombining(i)) flags[i] |= 1LL << 9;
        if (iszerowidth(i)) flags[i] |= 1LL << 10;
        if (iseuronumeric(i)) flags[i] |= 1LL << 11;
        if (iseuronumterm(i))
            flags[i] |= 1LL << 12;
        if (isarabnumeric(i)) flags[i] |= 1LL << 13;
        if (isdecompositionnormative(i)) flags[i] |= 1LL << 14;
        if (isarabinitial(i)) flags[i] |= 1LL << 15;
        if (isarabmedial(i)) flags[i] |= 1LL << 16;
        if (isarabfinal(i)) flags[i] |= 1LL << 17;
        if (isarabisolated(i)) flags[i] |= 1LL << 18;
        if (isideoalpha(i)) flags[i] |= 1LL << 19;
        if (isalnum(i)) flags[i] |= 1LL << 20;
        if (isspace(i)) flags[i] |= 1LL << 21;
        if (iseuronumsep(i)) flags[i] |= 1LL << 22;
        if (iscommonsep(i)) flags[i] |= 1LL << 23;
        if (ishexdigit(i)) flags[i] |= 1LL << 24;
        if (istitle(i)) flags[i] |= 1LL << 25;
    }
}

#include <utype2.h>
void donew() {
    for (int i = 0; i < 0x110000; ++i) {
        if (isunicodepointassigned(i)) nflags[i] |= 1LL << 0;
        if (isalpha(i)) nflags[i] |= 1LL << 1;
        if (isideographic(i)) nflags[i] |= 1LL << 2;
        if (islefttoright(i)) nflags[i] |= 1LL << 3;
        if (isrighttoleft(i)) nflags[i] |= 1LL << 4;
        if (islower(i)) nflags[i] |= 1LL << 5;
        if (isupper(i)) nflags[i] |= 1LL << 6;
        if (isdigit(i)) nflags[i] |= 1LL << 7;
        if (isligvulgfrac(i)) nflags[i] |= 1LL << 8;
        if (iscombining(i)) nflags[i] |= 1LL << 9;
        if (iszerowidth(i)) nflags[i] |= 1LL << 10;
        if (iseuronumeric(i)) nflags[i] |= 1LL << 11;
        if (iseuronumterm(i)) nflags[i] |= 1LL << 12;
        if (isarabnumeric(i)) nflags[i] |= 1LL << 13;
        if (isdecompositionnormative(i)) nflags[i] |= 1LL << 14;
        if (isarabinitial(i)) nflags[i] |= 1LL << 15;
        if (isarabmedial(i)) nflags[i] |= 1LL << 16;
        if (isarabfinal(i)) nflags[i] |= 1LL << 17;
        if (isarabisolated(i)) nflags[i] |= 1LL << 18;
        if (isideoalpha(i)) nflags[i] |= 1LL << 19;
        if (isalnum(i)) nflags[i] |= 1LL << 20;
        if (isspace(i)) nflags[i] |= 1LL << 21;
        if (iseuronumsep(i)) nflags[i] |= 1LL << 22;
        if (iscommonsep(i)) nflags[i] |= 1LL << 23;
        if (ishexdigit(i)) nflags[i] |= 1LL << 24;
        if (istitle(i)) nflags[i] |= 1LL << 25;
    }
}

// #define iseuronumtermo(ch)	(ffUnicodeUtype((ch))&FF_UNICODE_ENT)

int main() {
    doold();
    donew();

    for (int i = 0; i < 0x10000; ++i) {
        int al = !!ff_unicode_isalnum(i);
        int gl = !!g_unichar_isalnum(i);
        if (al != gl) {
            printf("U+%04X GALPHA %d %d\n",
            i, al, gl);
        }
        if (flags[i] != nflags[i]) {
            for (int j = 0; j < sizeof(flagnames)/sizeof(flagnames[0]); ++j) {
                int64_t res = (flags[i] & (1LL << j)) == (nflags[i] & (1LL << j));
                if (!res && (flags[i] & 1)) {
                    // printf("%d\n", iseuronumtermo(i));
                    printf("U+%04X: %s: old(%d) new(%d) oldassigned(%d) newassigned(%d)\n",
                        i, flagnames[j], !!(flags[i] & (1LL << j)), !!(nflags[i] & (1LL << j)),
                        !!(flags[i] & (1LL << 0)), !!(nflags[i] & (1LL << 0)));
                }
            }
        }

        int l1 = ffUnicodeToLower(i);
        int l2 = ff_unicode_tolower(i);
        if (l1 != l2 && (flags[i]&1)) {
            printf("LOWER U+%04X: U+%04x vs U+%04X\n", i, l1, l2);
        }
        l1 = ffUnicodeToUpper(i);
        l2 = ff_unicode_toupper(i);
        if (l1 != l2 && (flags[i]&1)) {
            printf("UPPER U+%04X: U+%04x vs U+%04X\n", i, l1, l2);
        }
        l1 = ffUnicodeToTitle(i);
        l2 = ff_unicode_totitle(i);
        if (l1 != l2 && (flags[i]&1)) {
            printf("TITLE U+%04X: U+%04x vs U+%04X\n", i, l1, l2);
        }
        l1 = ffUnicodeToMirror(i);
        l2 = ff_unicode_tomirror(i);
        if (l1 != l2 && (flags[i]&1)) {
            printf("MIRROR U+%04X: U+%04x vs U+%04X\n", i, l1, l2);
        }
    }
}


#endif

#if 0
int main() {
    int maxlen = 0, maxidx = 0;
    for (int i = 0; i < 0x110000; ++i) {
        char* name = uniname_name(i);
        char* annot = uniname_annotation(i);

        // char *name = copy(uniNamesList_name(i));
        // char *annot = copy(uniNamesList_annot(i));

        if (!name && !annot) {
            continue;
        }
        if (name) {
            int l = strlen(name);
            if (l > maxlen) {
                maxlen = l;
                maxidx = i;
            }
        }
        printf("%04X", i);
        if (name) {
            printf("\t%s\n", name);
        } else {
            printf("\n");
        }
        if (annot) {
            printf("%s\n", annot);
        }
        free(name);
        free(annot);
    }
    printf("MAX U+%04X: %d\n", maxidx,maxlen);
    return 0;
}
#endif

#if 1

#include <chardata.h>
#include <utype.h>
#include <utype2.h>

#undef isunicodepointassigned

static const unichar_t* get_old_decomp(unichar_t ch) {
    if (unicode_alternates[ch>>8]) {
        return unicode_alternates[ch>>8][ch&0xff];
    }
    return NULL;
}

int main() {
    for (int i = 0; i < 0x10000; ++i) {
        const unichar_t* old = get_old_decomp(i);
        const unichar_t* new = decomposition(i);

        if ((old == NULL) != (new == NULL)) {
            printf("U+%04x: %p vs %p (%d)\n",
                i, old, new, isunicodepointassigned(i));
            if (old) {
                do  {
                    printf("0x%02x ", *old);
                } while (*old++);
                printf("\n");
            }
        } else if (old && new) {
            if (u_strcmp(old, new)) {
                printf("U+%04X: OLD(%s), NEW(%s), CIRCLED(%d)\n",
                    i, u2utf8_copy(old), u2utf8_copy(new),
                    isdecompcircle(i));
            }
        }
    }
}
#endif