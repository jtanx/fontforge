set(MODULE gunicode)

file(GLOB SOURCE "*.c" "charset/*.c")
file(GLOB HEADERS "*.h" "charset/*.h")

add_library(${MODULE} OBJECT ${HEADERS} ${SOURCE})

target_link_libraries(${MODULE} ${ICONV_LIBRARIES})
