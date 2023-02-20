cmake_minimum_required(VERSION 3.7)
set(PROJECT_NAME bk)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(${PROJECT_NAME} 
    VERSION 0.1.0
    DESCRIPTION "Main program for the Baukasten project"
)

configure_file(version.hpp.in version.h)
add_subdirectory("plugins/telnet")

set(PUBLIC_HEADERS
    bk/error.h
    bk/module.h
    bk/send.h
    bk/service.h
)

set(HEADERS
    getopt.h
    service.hpp
    so.hpp
    version.hpp.in
)

set(SOURCES
    main.cpp
    getopt.c
    service.cpp
    so.cpp
)

set(EXTRA
    bk.conf
)

add_executable(${PROJECT_NAME} ${SOURCES} ${PUBLIC_HEADERS} ${HEADERS} ${EXTRA})

add_custom_command(TARGET ${PROJECT_NAME} PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/bk.conf
        ${CMAKE_CURRENT_BINARY_DIR}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    PUBLIC_HEADER "${PUBLIC_HEADERS}"
)

target_compile_features(${PROJECT_NAME}
    PUBLIC cxx_std_20)

target_include_directories(${PROJECT_NAME} PUBLIC
    BEFORE "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/include/"
    BEFORE "${PROJECT_BINARY_DIR}")

target_link_directories(${PROJECT_NAME} PUBLIC
    BEFORE "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib/")

if (WIN32)
    target_link_libraries(${PROJECT_NAME}
        JsonX
)
else()
    target_link_libraries(${PROJECT_NAME}
        JsonX
        stdc++
        uuid
        dl
)
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME       DESTINATION "${CMAKE_BUILD_TYPE}/bin"
    PUBLIC_HEADER DESTINATION "${CMAKE_BUILD_TYPE}/include/bk"
)
