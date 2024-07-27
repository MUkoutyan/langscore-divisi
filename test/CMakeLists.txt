# CMakeList.txt : divisi の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

enable_testing()
project (divisi_test VERSION 1)
add_definitions(-DENABLE_TEST)
include(CTest)

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。


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
    "../3rdparty/iutest/include"
    "../3rdparty/googletest/googletest/include"
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

set(TestProjectName ${PROJECT_NAME}-test)

set(DIVISI_TEST_RESOURCE plugin/data/)
foreach(dir ${DIVISI_TEST_RESOURCE})
file(COPY ./${dir} DESTINATION ${PROJECT_BINARY_DIR}/../${dir})
endforeach()


# Now simply link against gtest or gtest_main as needed. Eg
add_executable(${TestProjectName} ${test_sources})

target_include_directories(${TestProjectName} PUBLIC ${Test_Includes})

target_link_libraries(${TestProjectName}
	debug ${PROJECT_SOURCE_DIR}/../3rdparty/tree-sitter/tree-sitterD.lib
    optimized ${PROJECT_SOURCE_DIR}/../3rdparty/tree-sitter/tree-sitter.lib
)

add_test(NAME test COMMAND ${TestProjectName})

configure_file( ${CMAKE_SOURCE_DIR}/test/test_config.h.in ${CMAKE_SOURCE_DIR}/test/test_config.h )

# テストデータをコピーするディレクトリを定義
#set(TEST_DATA_OUTPUT_DIR "./data")

# テストデータのコピー先ディレクトリを作成
#file(MAKE_DIRECTORY ${TEST_DATA_OUTPUT_DIR})

# テストデータをコピーするカスタムターゲットを作成
#add_custom_target(copy_test_data ALL)


# テストデータファイルを一つずつコピー
#file(GLOB TEST_DATA_FILES "./data/*")
#foreach(TEST_DATA_FILE ${TEST_DATA_FILES})
#    get_filename_component(FILE_NAME ${TEST_DATA_FILE} NAME)
#    set(OUTPUT_FILE "${TEST_DATA_OUTPUT_DIR}/${FILE_NAME}")
#    
#    # 依存関係を削除して、コピー元ファイルを常に新しいものとみなす
#    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${TEST_DATA_FILE})
#
#    add_custom_command(
#        TARGET copy_test_data
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different
#                ${TEST_DATA_FILE}
#                ${OUTPUT_FILE}
#        DEPENDS ${TEST_DATA_FILE}
#    )
#endforeach()