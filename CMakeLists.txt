cmake_minimum_required(VERSION 2.8)
project(app)

set(APP_VERSION_MAJOR 1)
set(APP_VERSION_MINOR 0)

option(USE_MY_COMPILER
    "Use Provided compiler implementation" OFF)

configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_SOURCE_DIR}/config.h"
)
if (USE_MY_COMPILER)
    message("use my compiler")
    include_directories("${PROJECT_SOURCE_DIR}/compiler")
    add_subdirectory("compiler")
    set(EXTRA_LIBS ${EXTRA_LIBS} COMPILER)
else()
    message("not use my compiler")
endif()

aux_source_directory(. DIR_SRCS)
add_executable(app ${DIR_SRCS})

target_link_libraries(app ${EXTRA_LIBS})
