# CMakeList.txt : divisi の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.22)
project (divisi_test VERSION 1)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
# FetchContentを利用してGoogle Testを取得
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)


if(MSVC)
    string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_DEBUG            ${CMAKE_C_FLAGS_DEBUG})
    string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_MINSIZEREL       ${CMAKE_C_FLAGS_MINSIZEREL})
    string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE          ${CMAKE_C_FLAGS_RELEASE})
    string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELWITHDEBINFO   ${CMAKE_C_FLAGS_RELWITHDEBINFO})
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG          ${CMAKE_CXX_FLAGS_DEBUG})
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_MINSIZEREL     ${CMAKE_CXX_FLAGS_MINSIZEREL})
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE        ${CMAKE_CXX_FLAGS_RELEASE})
    string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
endif(MSVC)

# CMAKE_BUILD_TYPEをプリプロセッサ定数として追加
add_definitions(-DCMAKE_BUILD_TYPE_STRING="${CMAKE_BUILD_TYPE}")

set(Test_Includes
	"../"
	"../include"
	"../src"
	"../3rdparty/json/single_include"
	"../3rdparty/hash-library"
	"../3rdparty/tree-sitter/include"
)

file(GLOB_RECURSE DIVISI_SOURCES
	../src/*.cpp
    ../3rdparty/hash-library/md5.cpp
    ../3rdparty/hash-library/crc32.cpp
)

set(test_sources 
	./test_main.cpp
    ${DIVISI_SOURCES}
)

add_executable(divisi_test ${test_sources})

target_include_directories(divisi_test PUBLIC ${Test_Includes})

target_link_libraries(divisi_test
    gtest
    gtest_main
    gmock
    gmock_main
    ${PROJECT_SOURCE_DIR}/../3rdparty/tree-sitter/tree-sitterD.lib
)

#debug ${PROJECT_SOURCE_DIR}/../3rdparty/tree-sitter/tree-sitterD.lib
#optimized ${PROJECT_SOURCE_DIR}/../3rdparty/tree-sitter/tree-sitter.lib

file(GLOB GTEST_DLL_FILES "${CMAKE_BINARY_DIR}/bin/*.dll")

foreach(DLL_FILE ${GTEST_DLL_FILES})
    add_custom_command(TARGET divisi_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${DLL_FILE}"
        "${CMAKE_BINARY_DIR}"
    )
endforeach()


add_test(NAME test COMMAND divisi_test)

configure_file(${CMAKE_SOURCE_DIR}/test/test_config.h.in ${CMAKE_SOURCE_DIR}/test/test_config.h )
