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


set(Test_Includes
	"../"
	"../include"
	"../src"
    "../3rdparty/iutest/include"
	"../3rdparty/json/single_include"
	"../3rdparty/hash-library"
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

set(DIVISI_TEST_RESOURCE data/)
foreach(dir ${DIVISI_TEST_RESOURCE})
file(COPY ./${dir} DESTINATION ${PROJECT_BINARY_DIR}/../${dir})
endforeach()

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(${TestProjectName} ${test_sources})

target_include_directories(${TestProjectName} PUBLIC ${Test_Includes})

add_test(NAME test COMMAND ${TestProjectName})