# Based on newer CMake sources.
# Distributed under the OSI-approved BSD 3-Clause License.
# See https://cmake.org/licensing for details.

#[=======================================================================[.rst:
PackageUtils
------------

``pkg_config_result_to_target`` converts the result of a call to
``pkg_check_modules`` into an :prop_tgt:`IMPORTED` target.

This is used over the ``IMPORTED_TARGET`` feature that ``pkg_check_modules``
supports in later versions of CMake both for backwards compatibility and
consistency in how imported targets are created.

``find_package_with_target`` calls ``find_package`` with the provided
arguments, additionally creating an imported target if it is found, and
if the find module does not provide one itself. The imported target will
be named ``package::package`` where ``package` is the name of the
package being found.

``find_package_auto`` calls ``find_package`` with the provided
arguments, iff ``auto_option`` evaluates to a truthy value or ``AUTO``.
If ``auto_option`` is truthy (but not ``AUTO``), ``REQUIRED`` is
passed into ``find_package``.

#]=======================================================================]

function(pkg_config_result_to_target prefix target_name)
  # only create the target if it is linkable, i.e. no executables
  if(NOT TARGET ${target_name}
     AND ( ${prefix}_INCLUDE_DIRS OR ${prefix}_LINK_LIBRARIES OR ${prefix}_LDFLAGS_OTHER OR ${prefix}_CFLAGS_OTHER ))

    add_library(${target_name} INTERFACE IMPORTED ${_global_opt})

    if(${prefix}_INCLUDE_DIRS)
      set_property(TARGET ${target_name} PROPERTY
                   INTERFACE_INCLUDE_DIRECTORIES "${${prefix}_INCLUDE_DIRS}")
    endif()
    if(${prefix}_LINK_LIBRARIES)
      set_property(TARGET ${target_name} PROPERTY
                   INTERFACE_LINK_LIBRARIES "${${prefix}_LINK_LIBRARIES}")
    endif()
    if(${prefix}_LDFLAGS_OTHER)
      set_property(TARGET ${target_name} PROPERTY
                   INTERFACE_LINK_OPTIONS "${${prefix}_LDFLAGS_OTHER}")
    endif()
    if(${prefix}_CFLAGS_OTHER)
      set_property(TARGET ${target_name} PROPERTY
                   INTERFACE_COMPILE_OPTIONS "${${prefix}_CFLAGS_OTHER}")
    endif()
  endif()
endfunction()

macro(find_package_with_target)
  find_package(${ARGV})
  if(${ARGV0}_FOUND AND NOT TARGET ${ARGV0}::${ARGV0})
    add_library(${ARGV0}::${ARGV0} INTERFACE IMPORTED)
    set_property(TARGET ${ARGV0}::${ARGV0} PROPERTY
                INTERFACE_INCLUDE_DIRECTORIES "${${ARGV0}_INCLUDE_DIRS}")
    set_property(TARGET ${ARGV0}::${ARGV0} PROPERTY
                INTERFACE_LINK_LIBRARIES "${${ARGV0}_LIBRARIES}")
  endif()
endmacro()

macro(find_package_auto auto_option)
  if(${auto_option})
    unset(_find_package_auto_required)
    if(NOT ${${auto_option}} STREQUAL "AUTO")
      set(_find_package_auto_required REQUIRED)
    endif()

    find_package(${ARGN} ${_find_package_auto_required})
    set(${auto_option}_RESULT ${${ARGV1}_FOUND})
  endif()
endmacro()

function(target_link_libraries_auto auto_option)
  if(${auto_option} AND (${auto_option}_RESULT OR (NOT "${${auto_option}}" STREQUAL "AUTO")))
    message(STATUS "LINKING ${ARGV1} TO ${ARGV3}")
    target_link_libraries(${ARGN})
  endif()
endfunction()
