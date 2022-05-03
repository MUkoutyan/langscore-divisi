# CMakeList.txt : divisi の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)

project (divisi VERSION 0.5)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

set(DIVISI_TARGET_NAME              ${PROJECT_NAME})

add_subdirectory(src)

if(DIVISI_BUILD_EXAMPLE)
add_subdirectory(example)
endif()



# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
