# Distributed under the original FontForge BSD 3-clause license

#[=======================================================================[.rst:
AddHeader
---------

A super simple script to add a header to a file. Used to add the
shebang to the native scripts that get installed as binaries.

#]=======================================================================]

file(READ "${INPUT}" _input)
file(WRITE "${OUTPUT}" "${HEADER}\n${_input}")
