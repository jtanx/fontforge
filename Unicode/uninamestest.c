#include <stdio.h>
#include <ustring.h>
#include "utype2.h"

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

#if 1

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