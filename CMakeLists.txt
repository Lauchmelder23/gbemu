cmake_minimum_required(VERSION 3.10)

project("GBemu")

if(CMAKE_BUILD_TYPE MATCHES Debug)
	message(STATUS "Debug build detected. Defining macros...")
	add_compile_definitions(DEBUG)
endif()

# Executable 
file(GLOB_RECURSE TARGET_SOURCES
	"src/*.c"
)

file(GLOB_RECURSE TARGET_INCLUDES
	"include/*.h"
)

add_executable(GBemu
	${TARGET_INCLUDES}
	${TARGET_SOURCES}
)

target_include_directories(GBemu PRIVATE
	include
)

add_custom_command(TARGET GBemu POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/roms $<TARGET_FILE_DIR:GBemu>/roms/
)