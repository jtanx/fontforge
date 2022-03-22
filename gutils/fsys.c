/* Copyright (C) 2000-2004 by George Williams */
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

#include <fontforge-config.h>

#include "basics.h"
#include "ffglib.h"
#include "gfile.h"
#include "ustring.h"

#include <fcntl.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/stat.h>		/* for mkdir */
#include <sys/types.h>
#include <unistd.h>

#if !defined(__MINGW32__)
 #include <pwd.h>
#else
 #include <shlobj.h>
 #include <windows.h>
#endif

static char *program_root = NULL;

/**
 * \brief Removes the extension from a file path, if it exists.
 * This method assumes that the path is already normalized.
 * \param path The path to be modified. Is modified in-place.
 * \return A pointer to the input path.
 */
char *GFileRemoveExtension(char *path) {
    char *ext = strrchr(path, '.');
    if (ext) {
        char *fp = strrchr(path, '/');
        if (!fp || ext > fp) {
            *ext = '\0';
        }
    }
    return path;
}

/**
 * \brief Normalizes the file path as necessary.
 * On Windows, this means changing backlashes to slashes.
 *
 * \param path The file path to be modified. Is modified in-place.
 * \return A pointer to the input path
 */
char *GFileNormalizePath(char *path) {
#if defined(__MINGW32__)
    char *ptr;
    for(ptr = path; *ptr; ptr++) {
        if (*ptr == '\\') {
            *ptr = '/';
        }
    }
#endif
    return path;
}

char *GFileGetHomeDir(void) {
    const char* dir = getenv("HOME");
    if (dir == NULL) {
        dir = g_get_home_dir();
    }
    char* ret = copy(dir);
    GFileNormalizePath(ret);
    return ret;
}

static void savestrcpy(char *dest,const char *src) {
    for (;;) {
	*dest = *src;
	if ( *dest=='\0' )
    break;
	++dest; ++src;
    }
}

char *GFileGetAbsoluteName(const char *name) {
    if (!name) {
        return NULL;
    } else if (!strncasecmp(name, "file://", 7)) {
        name += 7;
    }

#if GLIB_CHECK_VERSION(2, 58, 0)
    gchar* abs = g_canonicalize_filename(name, NULL);
    char *ret;
    // If the input ends with '/', preserve that trailing slash
    if (name && (name = strrchr(name, '/')) && name[1] == '\0') {
        ret = smprintf("%s/", abs);
    } else {
        ret = copy(abs);
    }
    g_free(abs);
    return GFileNormalizePath(ret);
#else
    char buffer[1000];

     if ( ! GFileIsAbsolute(name) ) {
	char *pt, *spt, *rpt, *bpt;
	static char dirname_[MAXPATHLEN+1];

	if ( dirname_[0]=='\0' ) {
	    getcwd(dirname_,sizeof(dirname_));
	}
	strcpy(buffer,dirname_);
	if ( buffer[strlen(buffer)-1]!='/' )
	    strcat(buffer,"/");
	strcat(buffer,name);

	/* Normalize out any .. */
	spt = rpt = buffer;
	while ( *spt!='\0' ) {
	    if ( *spt=='/' )  {
		if ( *++spt=='\0' )
	break;
	    }
	    for ( pt = spt; *pt!='\0' && *pt!='/'; ++pt );
	    if ( pt==spt )	/* Found // in a path spec, reduce to / (we've*/
		savestrcpy(spt,spt+1); /*  skipped past the :// of the machine name) */
	    else if ( pt==spt+1 && spt[0]=='.' && *pt=='/' ) {	/* Noop */
		savestrcpy(spt,spt+2);
	    } else if (pt==spt+1 && spt[0]=='.' && *pt=='\0') { /* Remove trailing /. */
		pt = --spt;
		*spt = '\0';
	    } else if ( pt==spt+2 && spt[0]=='.' && spt[1]=='.' ) {
		for ( bpt=spt-2 ; bpt>rpt && *bpt!='/'; --bpt );
		if ( bpt>=rpt && *bpt=='/' ) {
		    savestrcpy(bpt,pt);
		    spt = bpt;
		} else {
		    rpt = pt;
		    spt = pt;
		}
	    } else
		spt = pt;
	}
	name = buffer;
    }
    return copy(name);
#endif
}

char *GFileNameTail(const char *oldname) {
    char *pt = strrchr(oldname, '/');
    return pt ? pt + 1 : (char*)oldname;
}

int GFileIsAbsolute(const char *file) {
#if defined(__MINGW32__)
    if( (file[1]==':') && (('a'<=file[0] && file[0]<='z') || ('A'<=file[0] && file[0]<='Z')) )
return ( true );
#else
    if ( *file=='/' )
return( true );
#endif
    if ( strstr(file,"://")!=NULL )
return( true );

return( false );
}

int GFileIsDir(const char *file) {
  struct stat info;
  if ( stat(file, &info)==-1 )
return 0;
  else
return( S_ISDIR(info.st_mode) );
}

int GFileExists(const char *file) {
return( access(file,0)==0 );
}

int GFileReadable(const char *file) {
return( access(file,04)==0 );
}

/**
 *  Creates a temporary file, similar to tmpfile.
 *  Used because the default tmpfile implementation on Windows is broken
 */
FILE *GFileTmpfile() {
#ifndef _WIN32
    return tmpfile();
#else
    wchar_t temp_path[MAX_PATH + 1];
    DWORD ret = GetTempPathW(MAX_PATH + 1, temp_path);
    if (!ret) {
        return NULL;
    }

    while(true) {
        wchar_t *temp_name = _wtempnam(temp_path, L"FF_");
        if (!temp_name) {
            return NULL;
        }

        HANDLE handle = CreateFileW(
            temp_name,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            CREATE_NEW,
            FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE,
            NULL
        );
        free(temp_name);

        if (handle == INVALID_HANDLE_VALUE) {
            if (GetLastError() != ERROR_FILE_EXISTS) {
                return NULL;
            }
        } else {
            int fd = _open_osfhandle((intptr_t)handle, _O_RDWR|_O_CREAT|_O_TEMPORARY|_O_BINARY);
            if (fd == -1) {
                CloseHandle(handle);
                return NULL;
            }

            FILE *fp = _fdopen(fd, "w+");
            if (!fp) {
                _close(fd);
                return NULL;
            }

            return fp;
        }
    }
    return NULL;
#endif
}

/**
 * Removes a file or folder.
 *
 * @param [in] path The path to be removed.
 * @param [in] recursive Specify true to remove a folder and all of its
 *                       sub-contents.
 * @return true if the deletion was successful or the path does not exist. It
 *         will fail if trying to remove a directory that is not empty and
 *         where `recursive` is false.
 */
int GFileRemove(const char *path, int recursive) {
    GDir *dir;
    const gchar *entry;

    if (g_remove(path) != 0) {
        if (recursive && (dir = g_dir_open(path, 0, NULL))) {
            while ((entry = g_dir_read_name(dir))) {
                gchar *fpath = g_build_filename(path, entry, NULL);
                if (g_remove(fpath) != 0 && GFileIsDir(fpath)) {
                    GFileRemove(fpath, recursive);
                }
                g_free(fpath);
            }
            g_dir_close(dir);
        }
        return (g_remove(path) == 0 || !GFileExists(path));
    }

    return true;
}

int GFileMkDir(const char *name, int mode) {
#ifndef _WIN32
	return mkdir(name, mode);
#else
	return mkdir(name);
#endif
}

void FindProgRoot(const char *prog) {
    char *tmp = NULL;
    gchar *rprog = NULL;
    if (program_root != NULL) {
        return;
    }

#ifdef _WIN32
    char path[MAX_PATH+4];
    unsigned int len = GetModuleFileNameA(NULL, path, MAX_PATH);
    path[len] = '\0';
    prog = GFileNormalizePath(path);
#endif

    if (prog != NULL) {
        if (strchr(prog, '/') == NULL) {
            prog = rprog = g_find_program_in_path(prog);
        }
        if (prog) {
            tmp = smprintf("%s/../..", prog);
        }
        program_root = GFileGetAbsoluteName(tmp);
        free(tmp);
    }

    if (program_root == NULL) {
        program_root = GFileGetAbsoluteName(FONTFORGE_INSTALL_PREFIX);
    }

    // Sigh glib doesn't provide symlink resolution
#ifdef HAVE_REALPATH
    tmp = smprintf("%s/share/fontforge", program_root);
    if (!GFileExists(tmp)) {
        free(tmp);
        tmp = realpath(prog, NULL);
        if (tmp) {
            char *real_root = smprintf("%s/../..", tmp);
            free(tmp);
            free(program_root);

            program_root = GFileGetAbsoluteName(real_root);
            free(real_root);
        }
    } else {
        free(tmp);
    }
#endif

    g_free(rprog);
    TRACE("Program root: %s\n", program_root);
}

const char *getShareDir(void) {
    static char *sharedir=NULL;
    if (!sharedir) {
        sharedir = smprintf("%s/share/fontforge", program_root);
    }
    return sharedir;
}

const char *getLocaleDir(void) {
    static char *localedir=NULL;
    if (!localedir) {
        localedir = smprintf("%s/share/locale", program_root);
    }
    return localedir;
}

const char *getPixmapDir(void) {
    static char *pixmapdir=NULL;
    if (!pixmapdir) {
        pixmapdir = smprintf("%s/pixmaps", getShareDir());
    }
    return pixmapdir;
}

const char *getHelpDir(void) {
    static char *helpdir=NULL;
    if (!helpdir) {
        helpdir = smprintf("%s/share/doc/fontforge/", program_root);
    }
    return helpdir;
}

char *getFontForgeUserDir() {
    const char *path = NULL;

#ifdef _WIN32
    /* Allow for preferences to be saved locally in a 'portable' configuration. */
    if (getenv("FF_PORTABLE") != NULL) {
        path = smprintf("%s/preferences/", getShareDir());
    }
#endif

    if (!path) {
        path = smprintf("%s/fontforge", g_get_user_config_dir());
    }
    if (g_mkdir_with_parents(path, 0755) != 0) {
        free(path);
        return NULL;
    }
    return path;
}

off_t GFileGetSize(char *name) {
/* Get the binary file size for file 'name'. Return -1 if error. */
    struct stat buf;
    long rc;

    if ( (rc=stat(name,&buf)) )
	return( -1 );
    return( buf.st_size );
}

char *GFileReadAll(char *name) {
/* Read file 'name' all into one large string. Return 0 if error. */
    char *ret;
    long sz;

    if ( (sz=GFileGetSize(name))>=0 && \
	 (ret=calloc(1,sz+1))!=NULL ) {
	FILE *fp;
	if ( (fp=fopen(name,"rb"))!=NULL ) {
	    size_t bread=fread(ret,1,sz,fp);
	    fclose(fp);

	    if( bread==(size_t)sz )
		return( ret );
	}
	free(ret);
    }
    return( 0 );
}

/*
 * Write char string 'data' into file 'name'. Return -1 if error.
 **/
int GFileWriteAll(char *filepath, char *data) {

    if( !data )
	return -1;

    size_t bwrite = strlen(data);
    FILE* fp;

    if ( (fp = fopen( filepath, "wb" )) != NULL ) {
	if ( (fwrite( data, 1, bwrite, fp ) == bwrite) && \
	     (fflush(fp) == 0) )
	    return( (fclose(fp) == 0 ? 0: -1) );
	fclose(fp);
    }
    return -1;
}

char *GFileGetHomeDocumentsDir(void)
{
    static char* ret = 0;
    if( ret )
	return ret;

#if defined(__MINGW32__)

    CHAR my_documents[MAX_PATH+2];
    HRESULT result = SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, my_documents );
    if (result != S_OK)
    {
    	fprintf(stderr,"Error: Can't get My Documents path!'\n");
        return ret;
    }
    int pos = strlen(my_documents);
    my_documents[ pos++ ] = '\\';
    my_documents[ pos++ ] = '\0';
    ret = copy( my_documents );
	GFileNormalizePath(ret);
    return ret;
#endif

    // On GNU/Linux and OSX it was decided that this should be just the
    // home directory itself.
    ret = GFileGetHomeDir();
    return ret;
}

char *GFileDirNameEx(const char *path, int treat_as_file)
{
    char *ret = NULL;
    if (path != NULL) {
        //Must allocate enough space to append a trailing slash.
        size_t len = strlen(path);
        ret = malloc(len + 2);

        if (ret != NULL) {
            char *pt;

            strcpy(ret, path);
            GFileNormalizePath(ret);
            if (treat_as_file || !GFileIsDir(ret)) {
                pt = strrchr(ret, '/');
                if (pt != NULL) {
                    *pt = '\0';
                }
            }

            //Keep only one trailing slash
            len = strlen(ret);
            for (pt = ret + len - 1; pt >= ret && *pt == '/'; pt--) {
                *pt = '\0';
            }
            *++pt = '/';
            *++pt = '\0';
        }
    }
    return ret;
}

char *GFileDirName(const char *path) {
    return GFileDirNameEx(path, 0);
}

static int mime_comp(const void *k, const void *v) {
    return strmatch((const char*)k, ((const char**)v)[0]);
}

char* GFileMimeType(const char *path) {
    char* ret, *pt;
    gboolean uncertain = false;
    gchar* res = g_content_type_guess(path, NULL, 0, &uncertain);
    gchar* mres = g_content_type_get_mime_type(res);
    g_free(res);

    if (!mres || uncertain || strstr(mres, "application/x-ext") || !strcmp(mres, "application/octet-stream")) {
        path = GFileNameTail(path);
        pt = strrchr(path, '.');

        if (pt == NULL) {
            if (!strmatch(path, "makefile") || !strmatch(path, "makefile~"))
                ret = copy("application/x-makefile");
            else if (!strmatch(path, "core"))
                ret = copy("application/x-core");
            else
                ret = copy("application/octet-stream");
        } else {
            pt = copy(pt + 1);
            int len = strlen(pt);
            if (pt[len - 1] == '~') {
                pt[len - 1] = '\0';
            }

            // array MUST be sorted by extension
            static const char* ext_mimes[][2] = {
                {"bdf",   "application/x-font-bdf"},
                {"bin",   "application/x-macbinary"},
                {"bz2",   "application/x-compressed"},
                {"c",     "text/c"},
                {"cff",   "application/x-font-type1"},
                {"cid",   "application/x-font-cid"},
                {"css",   "text/css"},
                {"dfont", "application/x-mac-dfont"},
                {"eps",   "text/ps"},
                {"gai",   "font/otf"},
                {"gif",   "image/gif"},
                {"gz",    "application/x-compressed"},
                {"h",     "text/h"},
                {"hqx",   "application/x-mac-binhex40"},
                {"html",  "text/html"},
                {"jpeg",  "image/jpeg"},
                {"jpg",   "image/jpeg"},
                {"mov",   "video/quicktime"},
                {"o",     "application/x-object"},
                {"obj",   "application/x-object"},
                {"otb",   "font/otf"},
                {"otf",   "font/otf"},
                {"pcf",   "application/x-font-pcf"},
                {"pdf",   "application/pdf"},
                {"pfa",   "application/x-font-type1"},
                {"pfb",   "application/x-font-type1"},
                {"png",   "image/png"},
                {"ps",    "text/ps"},
                {"pt3",   "application/x-font-type1"},
                {"ras",   "image/x-cmu-raster"},
                {"rgb",   "image/x-rgb"},
                {"rpm",   "application/x-compressed"},
                {"sfd",   "application/vnd.font-fontforge-sfd"},
                {"sgi",   "image/x-sgi"},
                {"snf",   "application/x-font-snf"},
                {"svg",   "image/svg+xml"},
                {"tar",   "application/x-tar"},
                {"tbz",   "application/x-compressed"},
                {"text",  "text/plain"},
                {"tgz",   "application/x-compressed"},
                {"ttf",   "font/ttf"},
                {"txt",   "text/plain"},
                {"wav",   "audio/wave"},
                {"woff",  "font/woff"},
                {"woff2", "font/woff2"},
                {"xbm",   "image/x-xbitmap"},
                {"xml",   "text/xml"},
                {"xpm",   "image/x-xpixmap"},
                {"z",     "application/x-compressed"},
                {"zip",   "application/x-compressed"},
            };

            const char** elem = bsearch(pt, ext_mimes,
                sizeof(ext_mimes)/sizeof(ext_mimes[0]), sizeof(ext_mimes[0]),
                mime_comp);
            ret = copy(elem ? elem[1] : "application/octet-stream");
            free(pt);
        }
    } else {
        ret = copy(mres);
    }
    g_free(mres);
    return ret;
}
