
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
	"../"
    "../src"
)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/versioninfo.h.in
  ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.h
  @ONLY)

configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/version.rc.in
  ${CMAKE_CURRENT_BINARY_DIR}/version.rc
  @ONLY)


# ソースをこのプロジェクトの実行可能ファイルに追加します。
if(MSVC)
  add_executable (divisi "main.cpp" ${CMAKE_CURRENT_BINARY_DIR}/version.rc)
	set(CMAKE_CXX23_STANDARD_COMPILE_OPTION "-std:c++latest")
	set(CMAKE_CXX23_EXTENSION_COMPILE_OPTION "-std:c++latest")
    
    add_compile_options(/MP)

    # リリースビルドでもデバッグ情報を生成してバックトレースでシンボル名を表示
	target_compile_options(divisi PRIVATE 
		$<$<CONFIG:Release>:/Zi>  # リリースビルドでもデバッグ情報を生成
		$<$<CONFIG:RelWithDebInfo>:/Zi>
	)
	
	target_link_options(divisi PRIVATE 
		$<$<CONFIG:Release>:/DEBUG>  # リリースビルドでもPDBファイルを生成
		$<$<CONFIG:RelWithDebInfo>:/DEBUG>
	)
else()
  add_executable (divisi "main.cpp")
  add_compile_options(-stdlib=libc++)
	target_link_libraries(divisi_lib)
endif()

target_include_directories(divisi PUBLIC 
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/json
	${CMAKE_CURRENT_BINARY_DIR}
)

target_link_libraries(divisi divisi_lib)

add_custom_command(
    TARGET divisi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:divisi>/bin"
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/rvcnv/rvcnv.exe $<TARGET_FILE_DIR:divisi>/bin
    COMMAND ${CMAKE_COMMAND} -E copy ${PROJECT_SOURCE_DIR}/rvcnv/rvcnv.exe ${PROJECT_SOURCE_DIR}/bin
    COMMAND ${CMAKE_COMMAND} -E copy  $<TARGET_FILE:divisi> ${PROJECT_SOURCE_DIR}/bin
)

add_custom_command(TARGET divisi POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${PROJECT_SOURCE_DIR}/resource"
    "$<TARGET_FILE_DIR:divisi>/resource"    
    COMMENT "Copying resource directory to output directory"
)



if(MSVC)

file(GLOB ICU_DLL_FILES "${PROJECT_SOURCE_DIR}/3rdparty/icu/bin/*.dll")

foreach(DLL_FILE ${ICU_DLL_FILES})
    add_custom_command(TARGET divisi POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${DLL_FILE}"
        $<TARGET_FILE_DIR:divisi>
    )
endforeach()


install(FILES ${ICU_DLL_FILES} DESTINATION ${PROJECT_SOURCE_DIR}/bin/)

endif()