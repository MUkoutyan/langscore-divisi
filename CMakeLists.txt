# CMakeList.txt : divisi の CMake プロジェクト。
#
cmake_minimum_required (VERSION 3.28)
set(CMAKE_CXX_STANDARD 23)

project(divisi_project VERSION 0.8.12)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}")

set(DIVISI_TARGET_NAME              ${PROJECT_NAME})

set(DIVISI_RESOURCE rvcnv/ resource/)

# テスト一式 (C++/Ruby/JavaScript/Python) を CTest に登録する。
# 独自ビルドタイプ (Test / Test_Debug) は廃止し、通常の Debug / Release に
# このオプションを組み合わせる。CMakePresets.json の *-test プリセット参照。
option(DIVISI_BUILD_TESTS "テストをビルドし CTest に登録する" OFF)

add_subdirectory(src)
add_subdirectory(exec)

if(DIVISI_BUILD_TESTS)
    # enable_testing() はトップレベルで呼ぶ必要がある (ビルドルートで ctest を実行するため)
    enable_testing()
    add_subdirectory(test)
endif()
