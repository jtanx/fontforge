function(resolve_inputs gen_dir src_dir)
  foreach(_input ${ARGN})
    set(_resolved_input "${gen_dir}/${_input}")
    if(EXISTS "${_resolved_input}")
      list(APPEND _resolved_inputs "${_resolved_input}")
    else()
      set(_resolved_input "${src_dir}/${_input}")
      if(EXISTS "${_resolved_input}")
        list(APPEND _resolved_inputs "${_resolved_input}")
      else()
        message(WARNING "Required input '${_input}' not found in either '${gen_dir}' or '${src_dir}'")
        set(RESOLVE_FAILED 1 PARENT_SCOPE)
        return()
        #message(FATAL_ERROR "Required input '${_input}' not found in either '${gen_dir}' or '${src_dir}'")
      endif()
    endif()
  endforeach()
  set(RESOLVED_INPUTS ${_resolved_inputs} PARENT_SCOPE)
endfunction()

function(invoke prefix)
  string(REPLACE ";" " " _description "${ARGN}")
  get_filename_component(_workdir "${TEST_DIR}" ABSOLUTE)
  message(STATUS "[${prefix}] description: ${TEST_DESCRIPTION}")
  message(STATUS "[${prefix}] working directory is ${_workdir}")
  message(STATUS "[${prefix}] calling ${_description}")

  unset(ENV{LD_LIBRARY_PATH})
  execute_process(
    COMMAND
      ${ARGN}
    WORKING_DIRECTORY
      "${TEST_DIR}"
    RESULT_VARIABLE
      _test_result
    OUTPUT_VARIABLE
      _test_output
    ERROR_VARIABLE
      _test_output
  )

  # For some reason OUTPUT_FILE/ERROR_FILE doesn't work on 3.5
  if(_test_output)
    file(WRITE "${TEST_DIR}/test_output.log" "${_test_output}")
  else()
    set(_test_output "<no output>")
  endif()

  if(NOT ${_test_result} EQUAL 0)
    message(FATAL_ERROR "[${prefix}] ${BASE_NAME} failed with exit code ${_test_result}:\n${_test_output}")
  else()
    message(STATUS "[${prefix}] success")
  endif()
endfunction()
