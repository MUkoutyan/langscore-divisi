# CMakeList.txt : divisi の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.22)
project (divisi_test VERSION 1)
if(MSVC)
	set(CMAKE_CXX_STANDARD 23)
elseif()
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_CXX_COMPILER clang++)
	set(CMAKE_C_COMPILER clang)
endif()
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


# if(MSVC)
#     string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_DEBUG            ${CMAKE_C_FLAGS_DEBUG})
#     string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_MINSIZEREL       ${CMAKE_C_FLAGS_MINSIZEREL})
#     string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELEASE          ${CMAKE_C_FLAGS_RELEASE})
#     string(REPLACE "/MD" "/MT" CMAKE_C_FLAGS_RELWITHDEBINFO   ${CMAKE_C_FLAGS_RELWITHDEBINFO})
#     string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_DEBUG          ${CMAKE_CXX_FLAGS_DEBUG})
#     string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_MINSIZEREL     ${CMAKE_CXX_FLAGS_MINSIZEREL})
#     string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELEASE        ${CMAKE_CXX_FLAGS_RELEASE})
#     string(REPLACE "/MD" "/MT" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
# endif(MSVC)

# CMAKE_BUILD_TYPEをプリプロセッサ定数として追加
add_definitions(-DCMAKE_BUILD_TYPE_STRING="${CMAKE_BUILD_TYPE}")

set(Test_Includes
	"../"
	"../include"
	"../src"
	"../3rdparty/json/single_include"
	"../3rdparty/hash-library"
	"../3rdparty/tree-sitter/include"
	${PROJECT_SOURCE_DIR}/../3rdparty/json
	${PROJECT_SOURCE_DIR}/../3rdparty/hash-library
	${PROJECT_SOURCE_DIR}/../3rdparty/tree-sitter/include
	${PROJECT_SOURCE_DIR}/../3rdparty/freetype/include
	${PROJECT_SOURCE_DIR}/../3rdparty/icu/include
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

if(MSVC)
	set(CMAKE_CXX23_STANDARD_COMPILE_OPTION "-std:c++latest")
	set(CMAKE_CXX23_EXTENSION_COMPILE_OPTION "-std:c++latest")
else()
    add_compile_options(-stdlib=libc++)
	target_link_libraries(divisi_test)
endif()

target_include_directories(divisi_test PUBLIC ${Test_Includes})

target_link_libraries(divisi_test
    gtest
    gtest_main
    gmock
    gmock_main
)

add_compile_options(/P /Fi"${CMAKE_BINARY_DIR}/preprocessed_output.i")

if(CMAKE_BUILD_TYPE STREQUAL "Debug" OR CMAKE_BUILD_TYPE STREQUAL "Test_Debug")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL") # /MDd
    message("Link Debug Library")
    string(REPLACE "/MD" "/MDd" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REPLACE "/MD" "/MDd" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
    message(${CMAKE_CXX_FLAGS})
    #add_compile_definitions(_ITERATOR_DEBUG_LEVEL=2)
    target_link_libraries(divisi_test
        ${PROJECT_SOURCE_DIR}/../3rdparty/tree-sitter/tree-sitterD.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/freetype/freetypeD.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icuin.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icuio.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icutest.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icutu.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icuuc.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icudt.lib
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Test")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL") # /MD
    message("Link Release Library")
    string(REPLACE "/MDd" "/MD" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
    string(REPLACE "/MDd" "/MD" CMAKE_C_FLAGS ${CMAKE_C_FLAGS})
    #add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)
    target_link_libraries(divisi_test
        ${PROJECT_SOURCE_DIR}/../3rdparty/tree-sitter/tree-sitter.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/freetype/freetype.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icuin.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icuio.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icutest.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icutu.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icuuc.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icudt.lib
    )
else()
    message(WARNING "Unknown CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}. Defaulting to Release libraries.")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL") # /MD
    #add_compile_definitions(_ITERATOR_DEBUG_LEVEL=0)
    target_link_libraries(divisi_test
        ${PROJECT_SOURCE_DIR}/../3rdparty/tree-sitter/tree-sitter.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/freetype/freetype.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icuin.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icuio.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icutest.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icutu.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icuuc.lib
        ${PROJECT_SOURCE_DIR}/../3rdparty/icu/lib64/icudt.lib
    )
endif()

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

# gmock.dll をコピーする処理を追加
add_custom_command(TARGET divisi_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_BINARY_DIR}/bin/gmock.dll"
    "${CMAKE_BINARY_DIR}"
)

add_test(NAME test COMMAND ${CMAKE_BINARY_DIR}/divisi_test)
set_tests_properties(test PROPERTIES COMMAND_EXPAND_LISTS TRUE)
set_tests_properties(test PROPERTIES COMMAND "${CMAKE_BINARY_DIR}/divisi_test;--gtest_output=json:${CMAKE_BINARY_DIR}/cpp_test_results.json")

message(${PROJECT_SOURCE_DIR})
configure_file(${PROJECT_SOURCE_DIR}/test_config.h.in ${PROJECT_SOURCE_DIR}/test_config.h )
