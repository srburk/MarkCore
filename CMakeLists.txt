cmake_minimum_required(VERSION 3.15)
project(markcore)

option(MARKCORE_BUILD_CLI "Build the markcore-cli tool" OFF)

add_library(markcore STATIC
	src/markcore.c
	src/parser.c
	src/renderer.c
	src/stack.c
	src/renderers/html_renderer.c
)

if (ADDRESS_SANITIZER)
	add_compile_options(-fsanitize=address)
	add_link_options(-fsanitize=address)
endif()

target_include_directories(markcore PUBLIC include)

if(MARKCORE_BUILD_CLI)
    add_executable(markcore-cli tools/markcore-cli.c)
    target_link_libraries(markcore-cli PRIVATE markcore)
endif()