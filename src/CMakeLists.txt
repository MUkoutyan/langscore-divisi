# CMakeList.txt : divisi の CMake プロジェクト。ソースを含めて、次を定義します:
# プロジェクト専用ロジックはこちらです。
#

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

include_directories(
	"../"
	"../json/single_include"
)

set(lib_files 
	"divisi.cpp" 
	"../include/divisi.h"
	"deserializer.cpp"
	"deserializer.h"
	"writer/csvwriter.cpp"
	"writer/csvwriter.h"
	"writer/writerbase.cpp"
	"writer/writerbase.h"
)

# ソースをこのプロジェクトの実行可能ファイルに追加します。
add_library (divisi_lib STATIC ${lib_files})

message("Include : " ${PROJECT_SOURCE_DIR})
target_include_directories(divisi_lib PUBLIC 
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/json
)

# TODO: テストを追加し、必要な場合は、ターゲットをインストールします。
