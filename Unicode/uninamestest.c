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