cmake_minimum_required(VERSION 3.7)
set(PROJECT_NAME bk)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

project(${PROJECT_NAME} 
    VERSION 0.1.0
    DESCRIPTION "Main program for the Baukasten project"
)

configure_file(version.hpp.in version.h)
add_subdirectory("plugins/telnet")
add_subdirectory("plugins/agw")
add_subdirectory("plugins/ax25")
add_subdirectory("plugins/ipserver")
add_subdirectory("plugins/udpclient")

set(PUBLIC_HEADERS
    bk/error.h
    bk/module.h
    bk/service.h
)

set(HEADERS
    cli.hpp
    getopt.h
    semaphore.hpp
    service.hpp
    so.hpp
    version.hpp.in
)

set(SOURCES
    cli.cpp
    main.cpp
    getopt.c
    service.cpp
    so.cpp
)

set(EXTRA
    conf.json
)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_executable(${PROJECT_NAME} ${SOURCES} ${PUBLIC_HEADERS} ${HEADERS} ${EXTRA})

add_custom_target(CopyConf ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/conf.json"
        "${CMAKE_CURRENT_BINARY_DIR}")

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
        Threads::Threads
)
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME       DESTINATION "${CMAKE_BUILD_TYPE}/bin"
    PUBLIC_HEADER DESTINATION "${CMAKE_BUILD_TYPE}/include/bk"
)
