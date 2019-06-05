cmake_minimum_required(VERSION 3.0)
project(fontforge)

# Defines and options

option(HAVE_PTHREAD_H "Have pthreads" ON)
option(FONTFORGE_CAN_USE_GDK "Have GDK" ON)
option(X_DISPLAY_MISSING "No X" ON)
option(FONTFORGE_CONFIG_USE_DOUBLE "Use doubles" ON)
option(_NO_LIBUNICODENAMES "No unicodenames" ON)
option(_NO_PYTHON "No python" OFF)

# Set compiler flags
if (CMAKE_COMPILER_IS_GNUCC)
    set(GCC_COMPILE_FLAGS "-std=gnu99 -g")

    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${GCC_COMPILE_FLAGS}")
endif()

if (CMAKE_COMPILER_IS_GNUCXX)
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
    pkg_check_modules(GTK gtk+-3.0)
    include_directories(${GDK_INCLUDE_DIRS})
    link_directories(${GDK_LIBRARY_DIRS})
    include_directories(${GTK_INCLUDE_DIRS})
    link_directories(${GTK_LIBRARY_DIRS})
endif()

find_package(PythonInterp 3.3)
find_package(PythonLibs 3.3)
if (PYTHONLIBS_FOUND)
    include_directories(${PYTHON_INCLUDE_DIRS})
else()
    set(_NO_PYTHON 1)
endif()

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
