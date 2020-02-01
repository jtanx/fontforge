# Distributed under the original FontForge BSD 3-clause license

add_subdirectory(html)

add_custom_target(sphinx sphinx-build -b html "${CMAKE_CURRENT_SOURCE_DIR}/sphinx" output)
