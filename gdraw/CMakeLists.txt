set(MODULE gdraw)

file(GLOB SOURCE "*.c")
file(GLOB HEADERS "*.h")

add_library(${MODULE} OBJECT ${HEADERS} ${SOURCE})

target_link_libraries(${MODULE} gunicode gutils ${GDK_LIBRARIES} ${GTK_LIBRARIES})

set_source_files_properties(ggtkdraw.c PROPERTIES COMPILE_FLAGS "-Wall -Wno-format-zero-length")