cmake_minimum_required(VERSION 3.7)
set(PROJECT_NAME ax25base)

project(${PROJECT_NAME} 
    VERSION 0.1.0
    DESCRIPTION "AX25 base for Baukasten"
)

set(SOURCES
    ax25_i.cpp
    ax25_s.cpp
    ax25_u.cpp
    ax25exceptions.cpp
    ax25frame.cpp
    ax25header.cpp
    ax25invalidframe.cpp
    ax25payload.cpp
    ax25primitive.cpp
    ax25types.cpp
    l2callsign.cpp
)

set(HEADERS
    ax25_i.hpp
    ax25_s.hpp
    ax25_u.hpp
    ax25exceptions.hpp
    ax25frame.hpp
    ax25header.hpp
    ax25invalidframe.hpp
    ax25payload.hpp
    ax25primitive.hpp
    ax25types.hpp
    l2callsign.hpp
)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

target_compile_features(${PROJECT_NAME}
    PUBLIC cxx_std_20
)

target_include_directories(${PROJECT_NAME} PUBLIC BEFORE
            "${CMAKE_CURRENT_SOURCE_DIR}/.."
            "${CMAKE_INSTALL_PREFIX}/${CMAKE_BUILD_TYPE}/include/"
            "${PROJECT_BINARY_DIR}")

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
        Threads::Threads
)
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME  DESTINATION "${CMAKE_BUILD_TYPE}/bin"
    LIBRARY  DESTINATION "${CMAKE_BUILD_TYPE}/lib")
