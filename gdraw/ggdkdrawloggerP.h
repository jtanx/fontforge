/* Copyright (C) 2016-2019 by Jeremy Tan */
/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.

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

/**
 *  \file  ggdkdrawloggerP.h
 *  \brief Private header file for the GDK logger.
 */

#ifndef _GGDKDRAWLOGGER_H
#define _GGDKDRAWLOGGER_H

#include <fontforge-config.h>
#ifdef FONTFORGE_CAN_USE_GTK_COMMON

#include <ffgtk.h>

//To get around a 'pedantic' C99 rule that you must have at least 1 variadic arg, combine fmt into that.
#define Log(level, ...) LogEx(level, __func__, __FILE__, __LINE__, __VA_ARGS__)

/** An enum to make the severity of log messages human readable in code **/
enum { LOGNONE = 0,
       LOGERR = 1,
       LOGWARN = 2,
       LOGINFO = 3,
       LOGDEBUG = 4,
       LOGVERBOSE = 5 };

extern void LogInit(void);
extern void LogEx(int level, const char *funct, const char *file, int line, const char *fmt, ...) G_GNUC_PRINTF(5, 6); // General function for printing log messages to stderr
extern const char *GdkEventName(int code);

#endif // FONTFORGE_CAN_USE_GTK_COMMON

#endif // _GGDKDRAWLOGGER_H
