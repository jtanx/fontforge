set(MODULE fontforgeexe)

set(SOURCE "main.c")

if (FONTFORGE_CAN_USE_GDK)
    file(GLOB SOURCE "*.c")
    file(GLOB HEADERS "*.h")
    list(FILTER SOURCE EXCLUDE REGEX ".*startnoui.c")
else()
    set(SOURCE ${SOURCE} startnoui.c)
endif()



add_executable(${MODULE} ${HEADERS} ${SOURCE})

target_include_directories(${MODULE} PRIVATE ../fontforge) #yuck

target_link_libraries(${MODULE} fontforge m)

if (FONTFORGE_CAN_USE_GDK)
    target_link_libraries(${MODULE} gdraw)
endif()

set_target_properties(${MODULE} PROPERTIES OUTPUT_NAME "fontforge")
