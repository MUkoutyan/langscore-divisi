# CMakeList.txt : divisi の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
	"../"
	"../json/single_include"
)

set(lib_files 
	"divisi.cpp" 
	"../include/divisi.h"
	"../include/config.h"
	"config.cpp"
	"project_deserializer.cpp"
	"project_deserializer.h"
	"serialize_base.h"
	"writer/csvwriter.cpp"
	"writer/csvwriter.h"
	"writer/writerbase.cpp"
	"writer/writerbase.h"
	"reader/readerbase.h"
	"reader/readerbase.cpp"
	"reader/csvreader.h"
	"reader/csvreader.cpp"
	"writer/rbscriptwriter.h"
	"writer/rbscriptwriter.cpp"
	"platform/divisi_vxace.h" 
    "platform/divisi_vxace.cpp" 
	"platform/platform_base.h" 
	"platform/platform_base.cpp")

# ソースをこのプロジェクトの実行可能ファイルに追加します。
add_library (divisi_lib STATIC ${lib_files})
set(CMAKE_CXX23_STANDARD_COMPILE_OPTION "-std:c++latest")
set(CMAKE_CXX23_EXTENSION_COMPILE_OPTION "-std:c++latest")

target_include_directories(divisi_lib PUBLIC 
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/json
)

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
