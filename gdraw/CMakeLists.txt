set(MODULE gdraw)

file(GLOB SOURCE "*.c")
file(GLOB HEADERS "*.h")

add_library(${MODULE} ${HEADERS} ${SOURCE})

target_link_libraries(${MODULE} gunicode gutils ${GDK_LIBRARIES})