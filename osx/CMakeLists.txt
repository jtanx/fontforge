# Distributed under the original FontForge BSD 3-clause license

#foreach(_file FontForge.app/Contents/Info.plist.in FontForge.app/Contents/Resources/English.lproj/InfoPlist.string.in)
#  string(REGEX REPLACE "\\.in$" "" _output "${_file}")
#  configure_file("${_file}" "${_output}" @ONLY)
#endforeach()

configure_file(
  FontForge.app/Contents/Info.plist.in
  FontForge.app/Contents/Info.plist
  @ONLY
)
configure_file(
  FontForge.app/Contents/Resources/English.lproj/InfoPlist.string.in
  FontForge.app/Contents/Resources/English.lproj/InfoPlist.string
  @ONLY
)

add_custom_target(macbundle DEPENDS "${CMAKE_CURRENT_BINARY_DIR}/FontForge.app/Contents/Info.plist" "${CMAKE_CURRENT_BINARY_DIR}/FontForge.app/Contents/Resources/English.lproj/InfoPlist.string")