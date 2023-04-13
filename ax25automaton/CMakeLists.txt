cmake_minimum_required(VERSION 3.7)
set(PROJECT_NAME ax25automaton)

project(${PROJECT_NAME} 
    VERSION 0.1.0
    DESCRIPTION "AX25 automaton for Baukasten"
)

set(SOURCES
    ax25timer.cpp
    datalinkdisconnected.cpp
    exec.cpp
    ifc.cpp
    primitive.cpp
    procedures.cpp
    state.cpp
)

set(HEADERS
    ax25timer.hpp
    ifc.hpp
    primitive.hpp
    state.hpp
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

set(APP_EXE ${PROJECT_NAME}_Test)
add_executable(${APP_EXE} test.cpp)
target_compile_features(${APP_EXE} PUBLIC cxx_std_20)
if (WIN32)
    target_link_libraries(${APP_EXE} ${PROJECT_NAME} bk_base ax25base Threads::Threads stdc++)
else()
    target_link_libraries(${APP_EXE} ${PROJECT_NAME} bk_base ax25base Threads::Threads stdc++ m)
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME  DESTINATION "${CMAKE_BUILD_TYPE}/bin"
    LIBRARY  DESTINATION "${CMAKE_BUILD_TYPE}/lib")
