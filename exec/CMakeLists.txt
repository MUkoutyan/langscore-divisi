
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
	"../"
)


# ソースをこのプロジェクトの実行可能ファイルに追加します。
add_executable (divisi "main.cpp")
set(CMAKE_CXX23_STANDARD_COMPILE_OPTION "-std:c++latest")
set(CMAKE_CXX23_EXTENSION_COMPILE_OPTION "-std:c++latest")

target_include_directories(divisi PUBLIC 
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/json
)

target_link_libraries(divisi divisi_lib)

install(FILES ${PROJECT_BINARY_DIR}/divisi.exe
        DESTINATION ${PROJECT_SOURCE_DIR}/bin/)
install(FILES ${PROJECT_SOURCE_DIR}/rvcnv/rvcnv.exe DESTINATION ${PROJECT_SOURCE_DIR}/bin/)

file(COPY ${PROJECT_SOURCE_DIR}/rvcnv/rvcnv.exe DESTINATION ${PROJECT_BINARY_DIR}/)
file(COPY ${PROJECT_SOURCE_DIR}/resource/ DESTINATION ${PROJECT_BINARY_DIR}/resource/)