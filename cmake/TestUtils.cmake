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

function(add_generation_target output input)
  add_custom_command(
    OUTPUT
      "${CMAKE_CURRENT_BINARY_DIR}/fonts/${output}"
    COMMAND "${CMAKE_COMMAND}"
      -E
      make_directory
      "${CMAKE_CURRENT_BINARY_DIR}/fonts"
    COMMAND fontforgeexe
      -lang=ff
      -c
      "Open($1); Generate($2)"
      "${CMAKE_CURRENT_SOURCE_DIR}/fonts/${input}"
      "${CMAKE_CURRENT_BINARY_DIR}/fonts/${output}"
    DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/fonts/${input}"
    VERBATIM
  )
endfunction()

function(add_ff_test test_name test_script)
  list(LENGTH ARGN _arglen)
  if (${_arglen} LESS 1)
    message(FATAL_ERROR "Must pass a description as the last argument")
  endif()
  list(GET ARGN -1 _description)
  list(REMOVE_AT ARGN -1)

  add_test(NAME ${test_name}
    COMMAND "${CMAKE_COMMAND}"
      -D "TEST_SCRIPT:STRING=${CMAKE_CURRENT_SOURCE_DIR}/${test_script}"
      -D "TEST_DESCRIPTION:STRING=${_description}"
      -D "TEST_EXECUTABLE:FILEPATH=$<TARGET_FILE:fontforgeexe>"
      -D "GEN_FONTS_DIR:PATH=${CMAKE_CURRENT_BINARY_DIR}/fonts"
      -D "SRC_FONTS_DIR:PATH=${CMAKE_CURRENT_SOURCE_DIR}/fonts"
      -D "TEST_INPUTS:LIST=${ARGN}"
      -D "TEST_LANG:STRING=ff"
      -P "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/testing/TestFixture.cmake"
    WORKING_DIRECTORY
      "${CMAKE_CURRENT_BINARY_DIR}"
  )
endfunction()
