cmake_minimum_required(VERSION 3.5)
project(lab7 VERSION 1.0.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)

set(SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/src/main.c
        ${PROJECT_SOURCE_DIR}/src/repl.c)

set(ZOX_FS_SOURCE_FILES
        ${PROJECT_SOURCE_DIR}/src/zox-fs/cluster.c
        ${PROJECT_SOURCE_DIR}/src/zox-fs/zox-fs.c)


include_directories(
        ${PROJECT_SOURCE_DIR}/include/
        ${PROJECT_SOURCE_DIR}/include/zox-fs/
)


add_executable(lab7 ${SOURCE_FILES})
add_library(zox-fs SHARED ${ZOX_FS_SOURCE_FILES})

target_link_libraries(lab7 zox-fs)