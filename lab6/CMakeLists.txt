cmake_minimum_required(VERSION 3.5)
project(lab6 VERSION 1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

find_package(PkgConfig)
pkg_check_modules(GLIB2 glib-2.0)

include_directories(include/zox-mem ${GLIB2_INCLUDE_DIRS})

set(LIBRARY_SRC ${PROJECT_SOURCE_DIR}/src/zox-mem/zox-mem.c
                ${PROJECT_SOURCE_DIR}/src/zox-mem/heap_block.c)
set(MAIN_SRC ${PROJECT_SOURCE_DIR}/src/main.c)

add_library(zox-mem ${LIBRARY_SRC})
add_executable(lab6 ${MAIN_SRC})
target_link_libraries(lab6 zox-mem ${GLIB2_LIBRARIES})