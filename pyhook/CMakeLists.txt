add_library(fontforge_pyhook MODULE fontforgepyhook.c)
add_library(psMat_pyhook MODULE psMatpyhook.c)

set_target_properties(fontforge_pyhook PROPERTIES OUTPUT_NAME "fontforge" PREFIX "")
set_target_properties(psMat_pyhook PROPERTIES OUTPUT_NAME "psMat" PREFIX "")

if(WIN32)
  set_target_properties(fontforge_pyhook PROPERTIES SUFFIX ".pyd")
  set_target_properties(psMat_pyhook PROPERTIES SUFFIX ".pyd")
endif()

target_link_libraries(fontforge_pyhook PRIVATE fontforge)
target_link_libraries(psMat_pyhook PRIVATE fontforge)
