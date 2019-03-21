set(MODULE fontforge)

file(GLOB SOURCE "*.c")
file(GLOB HEADERS "*.h")

add_library(${MODULE} ${HEADERS} ${SOURCE})

target_link_libraries(${MODULE} gunicode gutils ${FREETYPE_LIBRARIES} ${LIBXML2_LIBRARIES} ${ZLIB_LIBRARIES})