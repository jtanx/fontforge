/* Copyright (C) 2000-2012 by George Williams */
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
#ifndef _GUTILS_H
#define _GUTILS_H

#include <fontforge-config.h>
#include <time.h>
#include <sys/stat.h>

#pragma push_macro("PRINTF_FORMAT_ATTRIBUTE")
#ifdef __GNUC__
#  if defined(__USE_MINGW_ANSI_STDIO) && __USE_MINGW_ANSI_STDIO != 0
#    define PRINTF_FORMAT_ATTRIBUTE(x, y) __attribute__((format(gnu_printf, x, y)))
#  else
#    define PRINTF_FORMAT_ATTRIBUTE(x, y) __attribute__((format(printf, x, y)))
#  endif
#else
#  define PRINTF_FORMAT_ATTRIBUTE(x, y)
#endif


extern const char *GetAuthor(void);
extern time_t GetTime(void);
extern time_t GetST_MTime(struct stat s);

extern char *GUFormat(const char* fmt, ...) PRINTF_FORMAT_ATTRIBUTE(1, 2);

#pragma pop_macro("PRINTF_FORMAT_ATTRIBUTE")

#endif
