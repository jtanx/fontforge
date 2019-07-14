add_library(fontforge_common_headers INTERFACE)

include(FontForgeConfigure)
fontforge_generate_config(fontforge-config.h.in fontforge-config.h)

target_include_directories(fontforge_common_headers
  INTERFACE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

# For backwards compatibility for CMake < 3.12
set(FONTFORGE_COMMON_INCLUDE_DIRS
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  PARENT_SCOPE
)
