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
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE MATCHES "Test" OR CMAKE_BUILD_TYPE MATCHES "Test_Debug")
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
	"writer/jsscriptwriter.cpp" "config_writer.cpp" 
    "writer/marshal_replacer.h" "writer/marshal_replacer.cpp")

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
	${PROJECT_SOURCE_DIR}/3rdparty/freetype/include
	${PROJECT_SOURCE_DIR}/3rdparty/icu/include
)

if(MSVC)

target_link_libraries(divisi_lib
	debug ${PROJECT_SOURCE_DIR}/3rdparty/tree-sitter/tree-sitterD.lib
    optimized ${PROJECT_SOURCE_DIR}/3rdparty/tree-sitter/tree-sitter.lib
	debug ${PROJECT_SOURCE_DIR}/3rdparty/freetype/freetypeD.lib
    optimized ${PROJECT_SOURCE_DIR}/3rdparty/freetype/freetype.lib
    ${PROJECT_SOURCE_DIR}/3rdparty/icu/lib64/icuin.lib
    ${PROJECT_SOURCE_DIR}/3rdparty/icu/lib64/icuio.lib
    ${PROJECT_SOURCE_DIR}/3rdparty/icu/lib64/icutest.lib
    ${PROJECT_SOURCE_DIR}/3rdparty/icu/lib64/icutu.lib
    ${PROJECT_SOURCE_DIR}/3rdparty/icu/lib64/icuuc.lib
    ${PROJECT_SOURCE_DIR}/3rdparty/icu/lib64/icudt.lib
)

else()

# ICU =============
set(ICU_ROOT /opt/icu76)

find_package(ICU REQUIRED COMPONENTS i18n uc data)

if(NOT ICU_FOUND)
    set(ICU_INCLUDE_DIR ${ICU_ROOT}/include)
    set(ICU_LIB_DIR ${ICU_ROOT}/lib)
    set(ICU_LIBRARIES ${ICU_LIB_DIR}/libicuuc.so ${ICU_LIB_DIR}/libicui18n.so ${ICU_LIB_DIR}/libicudata.so)
endif()

target_include_directories(divisi_lib PUBLIC ${ICU_INCLUDE_DIR})
target_link_libraries(divisi_lib ${ICU_LIBRARIES})
#==================

find_package(Freetype REQUIRED)
target_include_directories(divisi_lib PUBLIC ${FREETYPE_INCLUDE_DIRS})
target_link_libraries(divisi_lib ${FREETYPE_LIBRARIES})

target_link_libraries(divisi_lib
	debug ${PROJECT_SOURCE_DIR}/3rdparty/tree-sitter/libtree-sitterD.a
    optimized ${PROJECT_SOURCE_DIR}/3rdparty/tree-sitter/libtree-sitter.a
)

endif(MSVC)
		


# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
