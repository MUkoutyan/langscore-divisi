# CMakeList.txt : divisi の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)

set(EXAMPLE_NAME divisi_example)

message(${CMAKE_CURRENT_BINARY_DIR})
add_executable(${EXAMPLE_NAME} "main.cpp")

target_link_libraries(${EXAMPLE_NAME} divisi_lib)

add_custom_command(TARGET ${EXAMPLE_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${resources} $<TARGET_FILE_DIR:${EXAMPLE_NAME}>)

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
