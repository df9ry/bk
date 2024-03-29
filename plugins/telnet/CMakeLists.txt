cmake_minimum_required(VERSION 3.7)
set(PROJECT_NAME bk_telnet)

project(${PROJECT_NAME} 
    VERSION 0.1.0
    DESCRIPTION "Telnet plugin for Baukasten"
)

set(SOURCES
    plugin.cpp
    server.cpp
    session.cpp
)

set(HEADERS
    plugin.hpp
    server.hpp
    session.hpp
)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

target_compile_features(${PROJECT_NAME}
    PUBLIC cxx_std_20
)

target_include_directories(${PROJECT_NAME} PUBLIC BEFORE
            "${CMAKE_CURRENT_SOURCE_DIR}/../.."
            "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/include/"
            "${PROJECT_BINARY_DIR}")

target_link_directories(${PROJECT_NAME} PUBLIC
    BEFORE "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/lib/")

if (WIN32)
    target_link_libraries(${PROJECT_NAME}
        bk_base
        JsonX
)
else()
    target_link_libraries(${PROJECT_NAME}
        bk_base
        JsonX
        stdc++
        telnet
        Threads::Threads
)
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME  DESTINATION "${CMAKE_BUILD_TYPE}/bin"
    LIBRARY  DESTINATION "${CMAKE_BUILD_TYPE}/lib")
