# Distributed under the original FontForge BSD 3-clause license

#[=======================================================================[.rst:
FindGTK3
---------

Find the GTK3 library. This currently depends on pkg-config to work.

Imported Targets
^^^^^^^^^^^^^^^^

This module defines the following :prop_tgt:`IMPORTED` target:

``GTK3::GTK3``
  The GTK3 ``GTK3`` library, if found

Result Variables
^^^^^^^^^^^^^^^^

This module will set the following variables in your project:

``GTK3_FOUND``
  true if the GTK3 headers and libraries were found
``GTK3_INCLUDE_DIRS``
  directories containing the GTK3 headers.
``GTK3_LIBRARIES``
  the library to link against
``GTK3_VERSION``
  the version of GTK3 found

#]=======================================================================]

find_package(PkgConfig)
pkg_check_modules(GTK3 QUIET IMPORTED_TARGET gtk+-3.0)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  GTK3
  REQUIRED_VARS
    GTK3_LIBRARIES
    GTK3_INCLUDE_DIRS
  VERSION_VAR
    GTK3_VERSION
)

include(${CMAKE_CURRENT_LIST_DIR}/../TargetUtils.cmake)
alias_imported_target(GTK3::GTK3 PkgConfig::GTK3)
