# Distributed under the original FontForge BSD 3-clause license

#[=======================================================================[.rst:
BuildOption
-----------

Standardises the way in which to specify a build option. Supports any
type that the set() function supports. Options are stored in the cache.

For BOOL options, passing in a 5th parameter (after the description)
will lead to that parameter being evaluated. If it evaluates to false,
the build option is forced to OFF.

The AUTO type is for tri-state Boolean options. The 'value' parameter is
ignored; it will always be initialised to AUTO.

The ENUM type is for options that may be one of a number of defined
values. All arguments afer the description are treated as allowed values
of the enumeration.

All other types are passed directly to set().
#]=======================================================================]

function(build_option variable type value description)
    if (${type} STREQUAL BOOL)
        if (${ARGC} EQUAL 4)
            option("${variable}" "${description}" "${value}")
        elseif (${ARGC} EQUAL 5)
            if (${ARGV4})
                option("${variable}" "${description}" "${value}")
            else ()
                set("${variable}" OFF CACHE BOOL "${description}" FORCE)
            endif ()
        else ()
            message(FATAL_ERROR "Invalid number of arguments for dependent option")
        endif ()
    elseif (${type} STREQUAL AUTO)
        set("${variable}" AUTO CACHE STRING "${description}")
        set_property(CACHE "${variable}" PROPERTY STRINGS AUTO ON OFF)
    elseif (${type} STREQUAL ENUM)
        if (${ARGC} LESS 5)
            message(FATAL_ERROR "Must pass at least one enum type")
        endif ()
        set("${variable}" "${value}" CACHE STRING "${description} Valid values: ${ARGN}")
        set_property(CACHE "${variable}" PROPERTY STRINGS ${ARGN})
        if (NOT "${${variable}}" IN_LIST ARGN)
            message(FATAL_ERROR "'${${variable}}' is not a valid value for ${variable}, expect one of ${ARGN}")
        endif ()
    else ()
        set("${variable}" "${value}" CACHE "${type}" "${description}")
    endif ()

    if (NOT "${variable}" IN_LIST BUILD_OPTIONS)
        set(BUILD_OPTIONS "${BUILD_OPTIONS};${variable}" CACHE INTERNAL "List of build options")
    endif ()
endfunction()

function(print_build_options)
    message(STATUS "Build options: ")
    foreach (opt ${BUILD_OPTIONS})
        message(STATUS "${opt} = ${${opt}}")
    endforeach ()
endfunction()