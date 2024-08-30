cmake_minimum_required(VERSION 3.16)

project(tree-sitter VERSION 0.20.9)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Set default flags
if (MSVC)
    set(CMAKE_C_FLAGS "/O2 /W3 /wd4100 /wd4201 /wd4214 /D_CRT_SECURE_NO_WARNINGS" CACHE STRING "MSVC compile flags")
else()
    set(CMAKE_C_FLAGS "-O3 -Wall -Wextra -Wshadow -pedantic" CACHE STRING "GCC/Clang compile flags")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c11 -fPIC -fvisibility=hidden")
endif()


# Add include directories
include_directories(./lib/src ./lib/include)

# Collect sources
file(GLOB_RECURSE SRC "lib/src/*.cpp" "lib/src/*.c")
list(REMOVE_ITEM SRC 
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/src/lib.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/src/wasm/stdlib.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/lib/src/wasm/wasm-stdlib.h"
)
list(REMOVE_ITEM SRC "${CMAKE_CURRENT_SOURCE_DIR}/lib/src/lib.c")

message("Source : ${SRC}")

# Create library targets
add_library(tree-sitter STATIC ${SRC})
set_target_properties(tree-sitter PROPERTIES OUTPUT_NAME tree-sitter)

# Install library and headers
install(TARGETS tree-sitter
        RUNTIME DESTINATION bin
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib)