set(MODULE gutils)

file(GLOB SOURCE "*.c")
file(GLOB HEADERS "*.h")

add_library(${MODULE} OBJECT ${HEADERS} ${SOURCE})

target_link_libraries(${MODULE} ${GLIB_LIBRARIES} ${GIO_LIBRARIES} pthread)