/* This is a GENERATED file - from makeutype.py 1.0 with Unicode 12.1.0 */

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

#ifndef FONTFORGE_UNICODE_UTYPE2_H
#define FONTFORGE_UNICODE_UTYPE2_H

#include <ctype.h>	/* Include here so we can control it. If a system header includes it later bad things happen */
#include "basics.h"	/* Include here so we can use pre-defined int types to correctly size constant data arrays. */

extern int ff_unicode_isunicodepointassigned(unichar_t ch);
extern int ff_unicode_isalpha(unichar_t ch);
extern int ff_unicode_isideographic(unichar_t ch);
extern int ff_unicode_islefttoright(unichar_t ch);
extern int ff_unicode_isrighttoleft(unichar_t ch);
extern int ff_unicode_islower(unichar_t ch);
extern int ff_unicode_isupper(unichar_t ch);
extern int ff_unicode_isdigit(unichar_t ch);
extern int ff_unicode_isnumeric(unichar_t ch);
extern int ff_unicode_isligvulgfrac(unichar_t ch);
extern int ff_unicode_iscombining(unichar_t ch);
extern int ff_unicode_iszerowidth(unichar_t ch);
extern int ff_unicode_iseuronumeric(unichar_t ch);
extern int ff_unicode_iseuronumterm(unichar_t ch);
extern int ff_unicode_isideoalpha(unichar_t ch);
extern int ff_unicode_isalnum(unichar_t ch);
extern int ff_unicode_isspace(unichar_t ch);
extern int ff_unicode_iseuronumsep(unichar_t ch);
extern int ff_unicode_iscommonsep(unichar_t ch);
extern int ff_unicode_ishexdigit(unichar_t ch);
extern int ff_unicode_istitle(unichar_t ch);
extern int ff_unicode_isarabnumeric(unichar_t ch);
extern unichar_t ff_unicode_tolower(unichar_t ch);
extern unichar_t ff_unicode_toupper(unichar_t ch);
extern unichar_t ff_unicode_totitle(unichar_t ch);
extern unichar_t ff_unicode_tomirror(unichar_t ch);

#undef isunicodepointassigned
#undef isalpha
#undef isideographic
#undef islefttoright
#undef isrighttoleft
#undef islower
#undef isupper
#undef isdigit
#undef isnumeric
#undef isligvulgfrac
#undef iscombining
#undef iszerowidth
#undef iseuronumeric
#undef iseuronumterm
#undef isideoalpha
#undef isalnum
#undef isspace
#undef iseuronumsep
#undef iscommonsep
#undef ishexdigit
#undef istitle
#undef isarabnumeric
#undef tolower
#undef toupper
#undef totitle
#undef tomirror

#define isunicodepointassigned(ch) ff_unicode_isunicodepointassigned((ch))
#define isalpha(ch)                ff_unicode_isalpha((ch))
#define isideographic(ch)          ff_unicode_isideographic((ch))
#define islefttoright(ch)          ff_unicode_islefttoright((ch))
#define isrighttoleft(ch)          ff_unicode_isrighttoleft((ch))
#define islower(ch)                ff_unicode_islower((ch))
#define isupper(ch)                ff_unicode_isupper((ch))
#define isdigit(ch)                ff_unicode_isdigit((ch))
#define isnumeric(ch)              ff_unicode_isnumeric((ch))
#define isligvulgfrac(ch)          ff_unicode_isligvulgfrac((ch))
#define iscombining(ch)            ff_unicode_iscombining((ch))
#define iszerowidth(ch)            ff_unicode_iszerowidth((ch))
#define iseuronumeric(ch)          ff_unicode_iseuronumeric((ch))
#define iseuronumterm(ch)          ff_unicode_iseuronumterm((ch))
#define isideoalpha(ch)            ff_unicode_isideoalpha((ch))
#define isalnum(ch)                ff_unicode_isalnum((ch))
#define isspace(ch)                ff_unicode_isspace((ch))
#define iseuronumsep(ch)           ff_unicode_iseuronumsep((ch))
#define iscommonsep(ch)            ff_unicode_iscommonsep((ch))
#define ishexdigit(ch)             ff_unicode_ishexdigit((ch))
#define istitle(ch)                ff_unicode_istitle((ch))
#define isarabnumeric(ch)          ff_unicode_isarabnumeric((ch))
#define tolower(ch)                ff_unicode_tolower((ch))
#define toupper(ch)                ff_unicode_toupper((ch))
#define totitle(ch)                ff_unicode_totitle((ch))
#define tomirror(ch)               ff_unicode_tomirror((ch))

#endif /* FONTFORGE_UNICODE_UTYPE2_H */
