cmake_minimum_required(VERSION 3.7)
set(PROJECT_NAME bk_ax25)

project(${PROJECT_NAME} 
    VERSION 0.1.0
    DESCRIPTION "AX.25 plugin for Baukasten"
)

set(SOURCES
    dlc.cpp
    plugin.cpp
    server.cpp
    session.cpp
)

set(HEADERS
    dlc.hpp
    elements.hpp
    plugin.hpp
    server.hpp
    session.hpp
    threadsavequeue.hpp
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

install(TARGETS ${PROJECT_NAME}
    RUNTIME  DESTINATION "${CMAKE_BUILD_TYPE}/bin"
    LIBRARY  DESTINATION "${CMAKE_BUILD_TYPE}/lib")
