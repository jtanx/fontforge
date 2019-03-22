cmake_minimum_required(VERSION 2.8)
project(fontforge)

# Defines and options

option(LIBCLIPBOARD_FORCE_WIN32 "Force building the Win32 backend (default:off)" OFF)
option(LIBCLIPBOARD_FORCE_X11 "Force building the X11 backend (default:off)" OFF)
option(LIBCLIPBOARD_FORCE_COCOA "Force building the Cocoa backend(default:off)" OFF)

option(HAVE_PTHREAD_H "Have pthreads" ON)
option(FONTFORGE_CAN_USE_GDK "Have GDK" ON)
option(X_DISPLAY_MISSING "No X" ON)
option(FONTFORGE_CONFIG_USE_DOUBLE "Use doubles" ON)
option(_NO_LIBUNICODENAMES "No unicodenames" ON)
option(_NO_PYTHON "No python" ON)

# Set compiler flags
if (CMAKE_COMPILER_IS_GNUCC OR LIBCLIPBOARD_BUILD_COCOA)
    set(GCC_COMPILE_FLAGS "-std=gnu99 -g")

    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion
                    OUTPUT_VARIABLE GCC_VERSION)
    if (GCC_VERSION VERSION_GREATER 4.9 OR GCC_VERSION VERSION_EQUAL 4.9)
        set(GCC_COMPILE_FLAGS "${GCC_COMPILE_FLAGS} -fdiagnostics-color=always")
    endif()
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")
endif()

if (CMAKE_COMPILER_IS_GNUCXX OR LIBCLIPBOARD_BUILD_COCOA)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic -g")
endif()

# Dependencies
include(FindPkgConfig REQUIRED)
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

pkg_check_modules(GDK gdk-3.0)
if (GDK_FOUND)
    include_directories(${GDK_INCLUDE_DIRS})
    link_directories(${GDK_LIBRARY_DIRS})
endif()

include_directories("uthash/src")

# Include directories
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/inc")
include_directories("${CMAKE_CURRENT_BINARY_DIR}/inc")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}") #nein
include_directories("${CMAKE_CURRENT_BINARY_DIR}") #nein

# Configure header
configure_file("inc/fontforge-config.h.in" "inc/fontforge-config.h")

# Set the output folders
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

add_subdirectory(Unicode)
add_subdirectory(gutils)
add_subdirectory(gdraw)
add_subdirectory(fontforge)
add_subdirectory(fontforgeexe)

# Install options
#install(TARGETS clipboard
#        RUNTIME DESTINATION bin
#        LIBRARY DESTINATION lib
#        ARCHIVE DESTINATION lib)
#install(FILES ${HEADERS} DESTINATION include)

# Uninstall target
#configure_file(
#    "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
#    "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
#    IMMEDIATE @ONLY)

#add_custom_target(uninstall
#COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
