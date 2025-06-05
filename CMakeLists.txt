# CMakeList.txt : divisi の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.22)
set(CMAKE_CXX_STANDARD 23)

project(divisi_project VERSION 0.8.7)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

set(DIVISI_TARGET_NAME              ${PROJECT_NAME})

set(DIVISI_RESOURCE rvcnv/ resource/)

add_subdirectory(src)
add_subdirectory(exec)

message(${CMAKE_BUILD_TYPE})
if(CMAKE_BUILD_TYPE MATCHES "Test" OR CMAKE_BUILD_TYPE MATCHES "Test_Debug")
    # Debugモードのフラグを設定
    set(CMAKE_C_FLAGS_TEST_DEBUG "${CMAKE_C_FLAGS_DEBUG}")
    set(CMAKE_CXX_FLAGS_TEST_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_EXE_LINKER_FLAGS_TEST_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
    set(CMAKE_SHARED_LINKER_FLAGS_TEST_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG}")
    
	message("Enable Test")
	add_subdirectory(test)
endif()


# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
