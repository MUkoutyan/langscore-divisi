# CMakeList.txt : divisi の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
	set(CMAKE_CXX_STANDARD 23)
elseif()
	set(CMAKE_CXX_STANDARD 23)
	set(CMAKE_CXX_COMPILER clang++)
	set(CMAKE_C_COMPILER clang)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Test" OR CMAKE_BUILD_TYPE MATCHES "Test_Debug")
add_definitions(-DENABLE_TEST)
include_directories(
	"../3rdparty/iutest/include"
)
endif()

include_directories(
	"./"
	"../include"
	"../3rdparty/json/single_include"
)

set(lib_files 
	"divisi.cpp" 
	"../include/divisi.h"
	"../include/config.h"
	"config.cpp"
	"invoker.cpp"
	"invoker.h"
	"serialize_base.h"
	"writer/csvwriter.cpp"
	"writer/csvwriter.h"
	"writer/writerbase.cpp"
	"writer/writerbase.h"
	"scripttextparser.hpp"
	"reader/vxacejsonreader.hpp"
	"reader/readerbase.h"
	"reader/readerbase.cpp"
	"reader/csvreader.h"
	"reader/csvreader.cpp"
	"writer/rbscriptwriter.h"
	"writer/rbscriptwriter.cpp"
	"platform/divisi_vxace.h" 
    "platform/divisi_vxace.cpp" 
	"platform/divisi_mvmz.h" 
    "platform/divisi_mvmz.cpp" 
	"platform/platform_base.h" 
	"platform/platform_base.cpp"
	${PROJECT_SOURCE_DIR}/rvcnv/rvcnv_hash.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/hash-library/md5.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/hash-library/crc32.cpp
    "writer/jsscriptwriter.h"
	"writer/jsscriptwriter.cpp" "config_writer.cpp")

# ソースをこのプロジェクトの実行可能ファイルに追加します。
add_library (divisi_lib STATIC ${lib_files})
if(MSVC)
	set(CMAKE_CXX23_STANDARD_COMPILE_OPTION "-std:c++latest")
	set(CMAKE_CXX23_EXTENSION_COMPILE_OPTION "-std:c++latest")
else()
    add_compile_options(-stdlib=libc++)
	target_link_libraries(divisi_lib)
endif()

target_include_directories(divisi_lib PUBLIC 
	./
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/3rdparty/json
	${PROJECT_SOURCE_DIR}/3rdparty/hash-library
	${PROJECT_SOURCE_DIR}/3rdparty/tree-sitter/include
)

if(MSVC)

target_link_libraries(divisi_lib
	debug ${PROJECT_SOURCE_DIR}/3rdparty/tree-sitter/tree-sitterD.lib
    optimized ${PROJECT_SOURCE_DIR}/3rdparty/tree-sitter/tree-sitter.lib
)

else()

target_link_libraries(divisi_lib
	debug ${PROJECT_SOURCE_DIR}/3rdparty/tree-sitter/libtree-sitterD.a
    optimized ${PROJECT_SOURCE_DIR}/3rdparty/tree-sitter/libtree-sitter.a
)

endif(MSVC)
		


# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
