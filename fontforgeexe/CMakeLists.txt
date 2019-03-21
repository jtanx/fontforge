set(MODULE fontforgeexe)

set(SOURCE "main.c")

if (TRUE)
    set(SOURCE ${SOURCE} startnoui.c)
else()
    message(FATAL ERROR "WTF")
endif()

#file(GLOB SOURCE "*.c")
#file(GLOB HEADERS "*.h")

add_executable(${MODULE} ${HEADERS} ${SOURCE})

target_include_directories(${MODULE} PRIVATE ../fontforge) #yuck

target_link_libraries(${MODULE} fontforge m)
