cmake_minimum_required(VERSION 3.15)
project(markcore LANGUAGES C)

option(MARKCORE_BUILD_CLI "Build the markcore-cli tool" OFF)
option(MARKCORE_BUILD_TESTS "Build markcore regression tests" OFF)

add_library(markcore STATIC
	src/markcore.c
	src/parser.c
	src/renderer.c
	src/stack.c
	src/renderers/html_renderer.c
)

target_include_directories(markcore PUBLIC include)

if (ADDRESS_SANITIZER)
	target_compile_options(markcore PUBLIC -fsanitize=address -fno-omit-frame-pointer)
	target_link_options(markcore PUBLIC -fsanitize=address)
endif()

if(MARKCORE_BUILD_CLI)
	add_executable(markcore-cli tools/markcore-cli.c)
	target_link_libraries(markcore-cli PRIVATE markcore)
endif()

if(MARKCORE_BUILD_TESTS)
	enable_testing()
	add_executable(markcore-tests tests/test_markcore.c)
	target_link_libraries(markcore-tests PRIVATE markcore)
	add_test(NAME markcore-tests COMMAND markcore-tests)

	add_executable(markcore-bench tests/bench_markcore.c)
	target_link_libraries(markcore-bench PRIVATE markcore)
	if(NOT APPLE)
		target_compile_definitions(markcore-bench PRIVATE MARKCORE_BENCH_WRAP_ALLOC=1)
		target_link_options(markcore-bench PRIVATE
			-Wl,--wrap=malloc
			-Wl,--wrap=calloc
			-Wl,--wrap=realloc
			-Wl,--wrap=free)
	endif()
	add_test(NAME markcore-bench COMMAND markcore-bench --ci)
endif()
