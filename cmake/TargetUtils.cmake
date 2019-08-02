# Distributed under the original FontForge BSD 3-clause license

#[=======================================================================[.rst:
TargetUtils
-----------

``alias_imported_target` copies an imported target of name ``src` into
a new imported target of name ``dest``. This is used because CMake
currently does not support aliasing imported targets.

``make_object_interface`` creates an interface library from an
object library of name ``objlib``. This interface includes the
target objects from ``objlib``. Additional arguments passed into
this function will be treated as libraries to link to this interface
library. This function is for use on CMake versions less than 3.12,
which do not support calling target_link_libraries directly on
object libraries.

``set_supported_c_compiler_flags`` checks to see if the provided list
of flags are supported by the C compiler. Supported flags are stored
into ``dst`` as a cache variable.

``add_native_scripts`` adds the provided set of native scripts, which
will either be installed to the binary folder on *nix platforms, having
had the appropriate shebang prepended to the script, or installed to
share/fontforge/nativescripts as-is on other platforms.

#]=======================================================================]

function(alias_imported_target dest src)
  if(TARGET ${src} AND NOT TARGET ${dest})
    add_library(${dest} INTERFACE IMPORTED)
    foreach(prop INTERFACE_INCLUDE_DIRECTORIES INTERFACE_LINK_LIBRARIES INTERFACE_LINK_OPTIONS INTERFACE_COMPILE_OPTIONS)
      get_property(_prop_val TARGET ${src} PROPERTY ${prop})
      set_property(TARGET ${dest} PROPERTY ${prop} ${_prop_val})
    endforeach()
  endif()
endfunction()

function(make_object_interface objlib)
  add_library(${objlib}_interface INTERFACE)
  target_sources(${objlib}_interface INTERFACE $<TARGET_OBJECTS:${objlib}>)
  target_link_libraries(${objlib}_interface
    INTERFACE
      ${ARGN}
  )
  target_include_directories(${objlib}
    PUBLIC
      $<TARGET_PROPERTY:${objlib}_interface,INTERFACE_INCLUDE_DIRECTORIES>
  )
endfunction()

function(set_supported_c_compiler_flags dst)
  if(NOT DEFINED "${dst}")
    include(CheckCCompilerFlag)
    include(CMakePushCheckState)
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_QUIET TRUE)

    foreach(_arg ${ARGN})
      message(STATUS "Checking if ${_arg} is supported...")
      check_c_compiler_flag(${_arg} _supported_flag)
      if(_supported_flag)
        message(STATUS "  Flag is supported: ${_arg}")
        list(APPEND _supported_flags ${_arg})
      else()
        message(STATUS "  Flag is unsupported: ${_arg}")
      endif()
      unset(_supported_flag CACHE)
    endforeach()

    set("${dst}" "${_supported_flags}" CACHE STRING "Supported compiler flags")
    cmake_pop_check_state()
  endif()
endfunction()

function(add_native_scripts)
  if(NOT UNIX)
    install(FILES ${ARGN} DESTINATION share/fontforge/nativescripts COMPONENT nativescripts)
  else()
    foreach(_script ${ARGN})
      get_filename_component(_output "${_script}" NAME_WE)
      set(_output "${CMAKE_CURRENT_BINARY_DIR}/${_output}")
      add_custom_command(OUTPUT "${_output}"
        COMMAND "${CMAKE_COMMAND}"
          -D "INPUT:FILEPATH=${CMAKE_CURRENT_SOURCE_DIR}/${_script}"
          -D "OUTPUT:FILEPATH=${_output}"
          -D "HEADER:STRING=#!${CMAKE_INSTALL_PREFIX}/bin/fontforge -lang=ff"
          -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/scripts/AddHeader.cmake"
        DEPENDS "${_script}" "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/scripts/AddHeader.cmake"
        VERBATIM
      )
      install(FILES "${_output}"
        DESTINATION "bin"
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
        COMPONENT nativescripts
      )
      list(APPEND _script_outputs "${_output}")
    endforeach()
    add_custom_target(nativescripts ALL DEPENDS "${_script_outputs}")
  endif()
endfunction()
