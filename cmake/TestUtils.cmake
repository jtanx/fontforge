function(add_download_target font url)
  add_custom_command(
    OUTPUT
      "${CMAKE_CURRENT_BINARY_DIR}/fonts/${font}"
    COMMAND "${CMAKE_COMMAND}"
      -D DEST:FILEPATH="${CMAKE_CURRENT_BINARY_DIR}/fonts/${font}"
      -D URL:STRING="${url}"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/DownloadIfMissing.cmake"
  )
endfunction()

function(add_ff_test test_name test_script)
  list(LENGTH ARGN _arglen)
  if (${_arglen} LESS 1)
    message(FATAL_ERROR "Must pass a description as the last argument")
  endif()
  list(GET ARGN -1 _description)
  list(REMOVE_AT ARGN -1)

  if(${CMAKE_VERSION} VERSION_LESS "3.9")
    set(_skip_arg --skip-as-pass)
  endif()

  add_test(NAME ${test_name}_ff
    COMMAND systestdriver
      --mode ff
      --binary "$<TARGET_FILE:fontforgeexe>"
      --script "${CMAKE_CURRENT_SOURCE_DIR}/${test_script}"
      --exedir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
      --libdir "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
      --argdir "${CMAKE_CURRENT_BINARY_DIR}/fonts"
      --argdir "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
      --desc "${_description}"
      ${_skip_arg}
      ${ARGN}
    WORKING_DIRECTORY
      "${CMAKE_CURRENT_BINARY_DIR}"
    )
  set_tests_properties(${test_name}_ff PROPERTIES SKIP_RETURN_CODE 77)
endfunction()

function(add_py_test test_name test_script)
  list(FIND ARGN PYHOOK_DISABLED _disable_pyhook_index)
  if(NOT _disable_pyhook_index LESS 0)
    list(REMOVE_AT ARGN ${_disable_pyhook_index})
    set(_disable_pyhook 1)
  endif()

  list(LENGTH ARGN _arglen)
  if (${_arglen} LESS 1)
    message(FATAL_ERROR "Must pass a description as the last argument")
  endif()
  list(GET ARGN -1 _description)
  list(REMOVE_AT ARGN -1)

  if(${CMAKE_VERSION} VERSION_LESS "3.9")
    set(_skip_arg --skip-as-pass)
  endif()

  add_test(NAME ${test_name}_py
    COMMAND systestdriver
      --mode py
      --binary "$<TARGET_FILE:fontforgeexe>"
      --script "${CMAKE_CURRENT_SOURCE_DIR}/${test_script}"
      --exedir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
      --libdir "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
      --argdir "${CMAKE_CURRENT_BINARY_DIR}/fonts"
      --argdir "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
      --desc "${_description}"
      ${_skip_arg}
      ${ARGN}
    WORKING_DIRECTORY
      "${CMAKE_CURRENT_BINARY_DIR}"
    )
  set_tests_properties(${test_name}_py PROPERTIES SKIP_RETURN_CODE 77)

  if(ENABLE_PYTHON_EXTENSION AND NOT _disable_pyhook)
    add_test(NAME ${test_name}_pyhook
      COMMAND systestdriver
        --mode pyhook
        --binary "${Python3_EXECUTABLE}"
        --script "${CMAKE_CURRENT_SOURCE_DIR}/${test_script}"
        --exedir "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
        --libdir "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}"
        --argdir "${CMAKE_CURRENT_BINARY_DIR}/fonts"
        --argdir "${CMAKE_CURRENT_SOURCE_DIR}/fonts"
        --desc "${_description}"
        ${_skip_arg}
        ${ARGN}
      WORKING_DIRECTORY
        "${CMAKE_CURRENT_BINARY_DIR}"
      )
    set_tests_properties(${test_name}_pyhook PROPERTIES SKIP_RETURN_CODE 77)
  endif()
endfunction()
