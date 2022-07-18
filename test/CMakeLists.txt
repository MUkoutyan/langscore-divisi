# CMakeList.txt : divisi の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
cmake_minimum_required (VERSION 3.22)
set(CMAKE_CXX_STANDARD 20)

project (divisi_test VERSION 1)
add_definitions(-DENABLE_TEST)

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
    "../iutest/include"
	"../json/single_include"
)

file(GLOB_RECURSE DIVISI_SOURCES
	../src/*.cpp
)

set(test_sources 
	./test_main.cpp
    ${DIVISI_SOURCES}
)

set(TestProjectName ${PROJECT_NAME}-test)

# Now simply link against gtest or gtest_main as needed. Eg
add_executable(${TestProjectName} ${test_sources})
target_include_directories(${TestProjectName} PUBLIC ${Test_Includes})
add_test(NAME test COMMAND ${TestProjectName})
enable_testing()