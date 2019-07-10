cmake_minimum_required(VERSION 3.5)
cmake_policy(SET CMP0048 NEW) # Actually set the version as passed in to project()
cmake_policy(SET CMP0057 NEW) # Allow IN_LIST in if statements

# Update the version for each new release
project(fontforge VERSION 20190709 LANGUAGES C CXX)

# Add folder for custom cmake modules
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake)

# Include any required modules
include(BuildOption REQUIRED)

# Options
build_option(ENABLE_GUI BOOL ON "Build FontForge with GUI support")
build_option(ENABLE_GDK BOOL ON "Build the GUI using the GDK backend instead of the X11 backend" "ENABLE_GUI")

build_option(ENABLE_NATIVE_SCRIPTING BOOL ON "Enables FontForge's native scripting support")
build_option(ENABLE_PYTHON_SCRIPTING BOOL ON "Enables FontForge's Python scripting support")
build_option(ENABLE_PYTHON_EXTENSION BOOL ON "Builds the Python models for use with system python" "ENABLE_PYTHON_SCRIPTING")
build_option(ENABLE_CODE_COVERAGE    BOOL OFF "Build with code coverage support")
build_option(ENABLE_DEBUG            BOOL OFF "Turn on debug build and FONTFORGE_DEBUG conditional code")
build_option(ENABLE_DEBUG_RAW_POINTS BOOL OFF "Add a raw mode to the points window of the debugger")
build_option(ENABLE_FONTFORGE_EXTRAS BOOL OFF "Builds programs from the contrib directory")
build_option(ENABLE_MAINTAINER_TOOLS BOOL OFF "Build programs normally only used by FontForge maintainers and developers")
build_option(ENABLE_TILE_PATH        BOOL OFF "Enable a 'tile path' command (a variant of 'expand stroke')")
build_option(ENABLE_WRITE_PFM        BOOL OFF "Add the ability to save a PFM file without creating the associated font file")

build_option(ENABLE_LIBSPIRO         BOOL ON "Enables libspiro support")
build_option(ENABLE_LIBUNINAMESLIST  BOOL ON "Enables libuninameslist support")
build_option(ENABLE_LIBGIF           AUTO ON "Enables GIF support")
build_option(ENABLE_LIBJPEG          AUTO ON "Enables JPEG support")
build_option(ENABLE_LIBPNG           AUTO ON "Enables PNG support")
build_option(ENABLE_LIBREADLINE      AUTO ON "Enables Readline support")
build_option(ENABLE_LIBTIFF          AUTO ON "Enables TIFF support")
build_option(ENABLE_WOFF2            AUTO ON "Enables WOFF2 support")

build_option(ENABLE_FREETYPE_DEBUGGER PATH "" "Use FreeType's internal debugger within FontForge.\
 If enabling this option, this must be set to the path of the FreeType source.\
 The version of that source must match the version that FontForge will link to.")

build_option(REAL_TYPE ENUM "double" "Sets the floating point type used." "double" "float")
build_option(THEME     ENUM "tango"  "Sets the GUI theme." "tango" "2012")

# Dependencies
include(FindPkgConfig REQUIRED)

# Required packages
pkg_check_modules(GLIB glib-2.0 REQUIRED)
include_directories(${GLIB_INCLUDE_DIRS})
link_directories(${GLIB_LIBRARY_DIRS})

pkg_check_modules(GIO gio-2.0 REQUIRED)
include_directories(${GIO_INCLUDE_DIRS})
link_directories(${GIO_LIBRARY_DIRS})

pkg_check_modules(FREETYPE freetype2 REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
link_directories(${FREETYPE_LIBRARY_DIRS})

pkg_check_modules(LIBXML2 libxml-2.0 REQUIRED)
include_directories(${LIBXML2_INCLUDE_DIRS})
link_directories(${LIBXML2_LIBRARY_DIRS})

pkg_check_modules (ZLIB zlib REQUIRED)
include_directories(${ZLIB_INCLUDE_DIRS})
link_directories(${ZLIB_LIBRARY_DIRS})

pkg_check_modules(ICONV iconv)
include_directories(${ICONV_INCLUDE_DIRS})
link_directories(${ICONV_LIBRARY_DIRS})

# Requirements for options

pkg_check_modules(GDK gdk-3.0)
if (GDK_FOUND)
    pkg_check_modules(GTK gtk+-3.0)
    include_directories(${GDK_INCLUDE_DIRS})
    link_directories(${GDK_LIBRARY_DIRS})
    include_directories(${GTK_INCLUDE_DIRS})
    link_directories(${GTK_LIBRARY_DIRS})
endif()

print_build_options()
