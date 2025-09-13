cmake_minimum_required(VERSION 3.15)
project(markcore)

option(MARKCORE_BUILD_CLI "Build the markcore-cli tool" OFF)

add_library(markcore STATIC
	src/markcore.c
	src/parser.c
	src/renderer.c
	src/buffer.c
	src/stack.c
)

target_include_directories(markcore PUBLIC include)

if(MARKCORE_BUILD_CLI)
    add_executable(markcore-cli tools/markcore-cli.c)
    target_link_libraries(markcore-cli PRIVATE markcore)
endif()