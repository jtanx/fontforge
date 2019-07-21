if(NOT EXISTS "${DEST}")
  message(STATUS "Fetching ${URL} to ${DEST}...")
  file(DOWNLOAD "${URL}" "${DEST}")
endif()