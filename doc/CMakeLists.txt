# Distributed under the original FontForge BSD 3-clause license

add_custom_target(sphinx sphinx-build -n -W -b html -d doctrees "${CMAKE_CURRENT_SOURCE_DIR}/sphinx" output)
