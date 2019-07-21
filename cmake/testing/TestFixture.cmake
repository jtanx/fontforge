cmake_policy(VERSION 3.5)

include("${CMAKE_CURRENT_LIST_DIR}/TestFixtureUtils.cmake")
set(ALLOWED_MODES ff py pyhook)

if(NOT TEST_SCRIPT)
  message(FATAL_ERROR "No test script specified")
elseif(NOT "${TEST_LANG}" IN_LIST ALLOWED_MODES)
  message(FATAL_ERROR "Invalid test language; must be either ff or py, got ${TEST_LANG}")
elseif(NOT EXISTS "${TEST_EXECUTABLE}")
  message(FATAL_ERROR "Invalid path to the test executable")
endif()

get_filename_component(BASE_NAME "${TEST_SCRIPT}" NAME)
string(REGEX REPLACE "[^a-zA-Z0-9]" "_" TEST_DIR "${BASE_NAME}")

if("${TEST_LANG}" STREQUAL "pyhook")
  set(TEST_DIR "${TEST_DIR}_pyhook")
endif()

file(REMOVE_RECURSE "${TEST_DIR}")
file(MAKE_DIRECTORY "${TEST_DIR}")

resolve_inputs("${GEN_FONTS_DIR}" "${SRC_FONTS_DIR}" ${TEST_INPUTS})
if(RESOLVE_FAILED)
  return()
endif()

if("${TEST_LANG}" STREQUAL "pyhook")
  set(ENV{PYTHONPATH} "${HOOK_DIR}")
  if(WIN32)
    # So the DLL resolves
    set(ENV{PATH} "${BIN_DIR};$ENV{PATH}")
  endif()
  invoke(PyHook "${TEST_EXECUTABLE}" "-Ss" "${TEST_SCRIPT}" ${RESOLVED_INPUTS})
else()
  invoke(FF "${TEST_EXECUTABLE}" "-lang=${TEST_LANG}" "-script" "${TEST_SCRIPT}" ${RESOLVED_INPUTS})
endif()
