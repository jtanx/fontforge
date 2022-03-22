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

#ifndef FONTFORGE_GFILE_H
#define FONTFORGE_GFILE_H

#include "basics.h"

extern char *GFileNormalizePath(char *path);

extern char* GFileGetHomeDir(void);

extern char *GFileRemoveExtension(char *path);
extern char *GFileGetAbsoluteName(const char *name);
/**
 * Return the file name for the full path 'path'. This is like the
 * shell "basename" command, for example:
 * GFileBaseName("/a/b/c/foo.sfd") returns "foo.sfd".
 *
 * You might be looking for GFileBaseName(), this function does basename from the shell.
 *
 * The return value is a pointer either being the same as path or a
 * pointer into the string that path points to. So no memory is
 * allocated by this function and the return value is dependent on the
 * 'path' string you passed in.
 */ 
extern char *GFileNameTail(const char *oldname);
extern int GFileIsAbsolute(const char *file);
extern int GFileIsDir(const char *file);
/**
 * Returns true if the file exists
 */
extern int GFileExists(const char *file);
extern int GFileReadable(const char *file);
extern FILE* GFileTmpfile();
extern int GFileRemove(const char *path, int recursive);
extern int GFileMkDir(const char *name, int mode);
extern char* GFileMimeType(const char *path);
extern off_t GFileGetSize(char *name);
extern char *GFileReadAll(char *name);
extern int   GFileWriteAll(char *filepath, char *data);
extern void  FindProgRoot(const char *prog);
extern const char *getShareDir();
extern const char *getLocaleDir(void);
extern const char *getPixmapDir(void);
extern const char *getHelpDir(void);
extern char *getFontForgeUserDir();

/**
 * This is the full path of ~ on OSX and Linux
 * and something like c:\Users\foo\Documents on windows
 */
extern char *GFileGetHomeDocumentsDir(void);

/**
 * Return the directory name for the full path 'path'.
 * This is like the shell "dirname" command, for example:
 * GFileDirName("/a/b/c/foo.sfd") returns "/a/b/c/".
 * This will also handle mingw paths as expected.
 * A trailing slash is always appended.
 *
 * The return value must be freed.
 */
extern char *GFileDirName(const char *path);

/**
 * Exactly like GFileDirName, but optionally treats the path as if
 * it were a file. This is needed for cases of treating UFO and sfdir folders as
 * 'files'.
 */
extern char *GFileDirNameEx(const char *path, int treat_as_file);

#endif /* FONTFORGE_GFILE_H */
