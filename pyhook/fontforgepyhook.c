#include "fontforge-config.h"
#include <Python.h>

#if PY_MAJOR_VERSION >= 3
typedef PyObject* (*FFPY_RET)(const char*);
PyMODINIT_FUNC PyInit_fontforge(void);
#define FFPY_PYTHON_ENTRY_FUNCTION fontforge_python3_init
#else
typedef void (*FFPY_RET)(const char*);
PyMODINIT_FUNC initfontforge(void);
#define FFPY_PYTHON_ENTRY_FUNCTION fontforge_python2_init
#endif

#ifdef WIN32
#include <Windows.h>
//Workaround for mingw-w64 inlining strsafe functions
#  ifdef __MINGW64_VERSION_MAJOR
#    pragma push_macro("__CRT__NO_INLINE")
#    undef __CRT__NO_INLINE
#  endif
#include <Strsafe.h>
#  ifdef __MINGW64_VERSION_MAJOR
#    pragma pop_macro("__CRT__NO_INLINE")
#  endif

#define STRINGIFY(x) #x
#define STRINGVAL(x) STRINGIFY(x)
#define FF_VERSION_STR STRINGVAL(FONTFORGE_LIBFF_VERSION_MAJOR)
#define FFPY_ENTRY_NAME STRINGVAL(FFPY_PYTHON_ENTRY_FUNCTION)

/**
 * \brief Dynamically loads the FontForge library.
 * It first attempts to load libfontforge using the normal DLL search path.
 * If this fails, an attempt is made to read the registry and to load it
 * from where it has been installed.
 * The install path is read from HKLM\Software\FontForge\InstallPath
 *
 * \param hLib A pointer to the handle to libfontforge. If loading fails, this
 *             value will be NULL.
 * \param hook A pointer to hold the Python hook function from libfontforge.
 *             If loading fails, this value is undefined.
 */
static void load_hook(HINSTANCE *hLib, FFPY_RET *hook) {
     const TCHAR *ffLibraryName = TEXT("libfontforge-" FF_VERSION_STR ".dll");
     
     *hLib = LoadLibrary(ffLibraryName);
     if (*hLib == NULL) {
        HKEY hKey;
        LONG ret;
        TCHAR installPath[MAX_PATH], dllPath[MAX_PATH];
        DWORD valueType, valueSize = sizeof(installPath);
        
        //Open the registry key
        ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\FontForge"), 0,
                           KEY_READ, &hKey);
        if (ret != ERROR_SUCCESS) {
            return;
        }
        
        //Get the install path. RegGetValue does not exist on 32-bit XP.
        ret = RegQueryValueEx(hKey, TEXT("InstallPath"), NULL, &valueType,
                              (LPBYTE) installPath, &valueSize); 
        RegCloseKey(hKey);
        if (ret != ERROR_SUCCESS || valueType != REG_SZ) {
            return;
        }
        
        //Null-terminate it.
        valueSize /= sizeof(TCHAR);
        installPath[valueSize] = 0;

        //Set the DLL directory
        StringCchPrintf(dllPath, MAX_PATH, TEXT("%s\\bin"), installPath);
        SetDllDirectory(dllPath);

        //Set the path to the dll and try to load it
        StringCchPrintf(dllPath, MAX_PATH, TEXT("%s\\bin\\%s"), 
                        installPath, ffLibraryName);
        *hLib = LoadLibrary(dllPath);

        //Reset the DLL directory
        SetDllDirectory(NULL);
        if (*hLib == NULL) {
            return;
        }
     }
     
     *hook = (FFPY_RET) GetProcAddress(*hLib, FFPY_ENTRY_NAME);
     if (*hook == NULL) {
        FreeLibrary(*hLib);
        *hLib = NULL;
     }
}

#  if PY_MAJOR_VERSION >= 3
/* Python 3 module initialization */
PyMODINIT_FUNC PyInit_fontforge(void) {
    HINSTANCE hLib;
    FFPY_RET hook;
    PyObject *ret = NULL;
    
    load_hook(&hLib, &hook);
    if (hLib != NULL) {
        ret = hook("fontforge");
    }
    return ret;
}
#  else
/* Python 2 module initialization */
PyMODINIT_FUNC initfontforge(void) {
    HINSTANCE hLib;
    FFPY_RET hook;
    
    load_hook(&hLib, &hook);
    if (hLib != NULL) {
        hook("fontforge");
    }
}
#  endif

#else /* !WIN32 */
extern PyMODINIT_FUNC FFPY_PYTHON_ENTRY_FUNCTION(const char* modulename);

#  if PY_MAJOR_VERSION >= 3
/* Python 3 module initialization */
PyMODINIT_FUNC PyInit_fontforge(void) {
    return FFPY_PYTHON_ENTRY_FUNCTION("fontforge");
}
#  else
/* Python 2 module initialization */
PyMODINIT_FUNC initfontforge(void) {
    return FFPY_PYTHON_ENTRY_FUNCTION("fontforge");
}
#  endif

#endif /* WIN32 */