cmake_minimum_required(VERSION 2.8)
project(app)

set(APP_VERSION_MAJOR 1)
set(APP_VERSION_MINOR 1)

option(COMPILER_LIB_MODE
    "Use to provide COMPILER implementation" ON)
option(LVAL_LIB_MODE
    "Use to provide LVAL implementation" ON)

configure_file(
    "${PROJECT_SOURCE_DIR}/config.h.in"
    "${PROJECT_SOURCE_DIR}/config.h"
)
if (COMPILER_LIB_MODE)
    message("use compiler")
    include_directories("${PROJECT_SOURCE_DIR}/compiler")
    add_subdirectory("compiler")
    set(EXTRA_LIBS ${EXTRA_LIBS} COMPILER)
else()
    message(FATAL_ERROR "cmakedefine must add 'COMPILER_LIB_MODE=ON'")
endif()

if (LVAL_LIB_MODE)
    message("generate library: LVAL")
    add_library(LVAL lenv.cpp lval.cpp)
else()
    message("run test for lval")
    aux_source_directory(. DIR_SRCS)
    add_executable(app ${DIR_SRCS})
endif()

target_link_libraries(app ${EXTRA_LIBS})
