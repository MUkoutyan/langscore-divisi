# CMakeList.txt : divisi のテスト。
# ルートの CMakeLists.txt から DIVISI_BUILD_TESTS=ON のときだけ add_subdirectory される。
# C++ (googletest) に加え、Ruby / JavaScript / Python のテストも CTest へ登録する。

# ---------------------------------------------------------------------------
# テストデータ (ツクールのゲームプロジェクト) の解決
# サイズと再配布の都合でリポジトリには含めないため、既定ではリポジトリと同階層の
# langscore-divisi-test-data を参照する。
# 環境変数 LANGSCORE_TEST_DATA または -DTEST_DATA_SRC で上書きできる。
# ---------------------------------------------------------------------------
if(DEFINED TEST_DATA_SRC AND NOT TEST_DATA_SRC STREQUAL "")
    # 後方互換: -DTEST_DATA_SRC は data ディレクトリを直接指すため、その親をルートとする。
    get_filename_component(TEST_DATA_ROOT "${TEST_DATA_SRC}" DIRECTORY)
elseif(DEFINED ENV{LANGSCORE_TEST_DATA})
    set(TEST_DATA_ROOT "$ENV{LANGSCORE_TEST_DATA}")
else()
    set(TEST_DATA_ROOT "${PROJECT_SOURCE_DIR}/../langscore-divisi-test-data")
endif()
get_filename_component(TEST_DATA_ROOT "${TEST_DATA_ROOT}" ABSOLUTE)
set(TEST_DATA_SRC "${TEST_DATA_ROOT}/data")

# テストデータが無くても構成は通す。データを必要としないテスト (lscsv の js/py) だけを
# 登録し、CI のようにフィクスチャを置けない環境でも ctest が実行できるようにする。
if(EXISTS "${TEST_DATA_ROOT}")
    set(HAVE_TEST_DATA TRUE)
    message(STATUS "TEST_DATA_ROOT : ${TEST_DATA_ROOT}")
else()
    set(HAVE_TEST_DATA FALSE)
    message(WARNING
        "テストデータが見つかりません: ${TEST_DATA_ROOT}\n"
        "  データを必要とするテスト (cpp / vxace / mv / mz / rvcnv / divisi_ct / lscsv_rb) は登録しません。\n"
        "  リポジトリと同階層に langscore-divisi-test-data を配置するか、"
        "環境変数 LANGSCORE_TEST_DATA / -DTEST_DATA_SRC で場所を指定してください。")
endif()

# ---------------------------------------------------------------------------
# C++ テスト (googletest)
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/releases/download/v1.15.2/googletest-1.15.2.tar.gz
)
# src/CMakeLists.txt の BUILD_SHARED_LIBS=ON がここまで効いてしまうため、このスコープだけ OFF に戻す。
# ON のままだと gtest が DLL になり、divisi_test.exe の隣に無いため起動できない。
set(BUILD_SHARED_LIBS OFF)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

set(Test_Includes
	${PROJECT_SOURCE_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/src
	${PROJECT_SOURCE_DIR}/3rdparty/json/single_include
	${PROJECT_SOURCE_DIR}/3rdparty/hash-library
	${PROJECT_SOURCE_DIR}/3rdparty/tree-sitter/include
	${PROJECT_SOURCE_DIR}/3rdparty/freetype/include
	${PROJECT_SOURCE_DIR}/3rdparty/icu/include
)

# divisi_lib とは別にソースをビルドする (テスト専用の定義を入れられるようにするため)。
file(GLOB_RECURSE DIVISI_SOURCES
	${PROJECT_SOURCE_DIR}/src/*.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/hash-library/md5.cpp
	${PROJECT_SOURCE_DIR}/3rdparty/hash-library/crc32.cpp
)

add_executable(divisi_test ./test_main.cpp ${DIVISI_SOURCES})

if(MSVC)
	set(CMAKE_CXX23_STANDARD_COMPILE_OPTION "-std:c++latest")
	set(CMAKE_CXX23_EXTENSION_COMPILE_OPTION "-std:c++latest")
else()
	add_compile_options(-stdlib=libc++)
endif()

target_include_directories(divisi_test PUBLIC ${Test_Includes} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(divisi_test
    gtest
    gtest_main
    gmock
    gmock_main
)

if(MSVC)
    # 構成毎のライブラリ選択は src/CMakeLists.txt と同じく debug/optimized キーワードで行う。
    # MSVC ランタイム (/MD, /MDd) は CMake の既定に任せる (gtest_force_shared_crt と一致する)。
    target_link_libraries(divisi_test
        debug     ${PROJECT_SOURCE_DIR}/3rdparty/tree-sitter/tree-sitterD.lib
        optimized ${PROJECT_SOURCE_DIR}/3rdparty/tree-sitter/tree-sitter.lib
        debug     ${PROJECT_SOURCE_DIR}/3rdparty/freetype/freetypeD.lib
        optimized ${PROJECT_SOURCE_DIR}/3rdparty/freetype/freetype.lib
        ${PROJECT_SOURCE_DIR}/3rdparty/icu/lib64/icuin.lib
        ${PROJECT_SOURCE_DIR}/3rdparty/icu/lib64/icuio.lib
        ${PROJECT_SOURCE_DIR}/3rdparty/icu/lib64/icutest.lib
        ${PROJECT_SOURCE_DIR}/3rdparty/icu/lib64/icutu.lib
        ${PROJECT_SOURCE_DIR}/3rdparty/icu/lib64/icuuc.lib
        ${PROJECT_SOURCE_DIR}/3rdparty/icu/lib64/icudt.lib
    )

    # divisi_test.exe の実行に必要な ICU のランタイム DLL を出力先へコピーする。
    file(GLOB ICU_DLL_FILES "${PROJECT_SOURCE_DIR}/3rdparty/icu/bin/*.dll")
    foreach(DLL_FILE ${ICU_DLL_FILES})
        add_custom_command(TARGET divisi_test POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL_FILE}" "$<TARGET_FILE_DIR:divisi_test>")
    endforeach()
endif()

# VXAce のテストは invoker 経由で rvcnv.exe を実行する。
# invoker::exec は appPath が空の場合 "./rvcnv.exe" を探すため、実行ファイルと同じ場所に必要。
add_custom_command(TARGET divisi_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${PROJECT_SOURCE_DIR}/rvcnv/rvcnv.exe" "$<TARGET_FILE_DIR:divisi_test>")

# langscore.rb / Langscore.js 等のテンプレートも "./resource" から参照される。
add_custom_command(TARGET divisi_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
    "${PROJECT_SOURCE_DIR}/resource" "$<TARGET_FILE_DIR:divisi_test>/resource")

# test_config.h はビルド構成毎に絶対パスが変わるため、ソースツリーではなくビルドツリーへ生成する。
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/test_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/test_config.h)

# ---------------------------------------------------------------------------
# テストランナーの検出
# 見つからない処理系のテストは登録しない (その処理系が無い環境でも ctest が通る)。
# ---------------------------------------------------------------------------
find_package(Python3 COMPONENTS Interpreter)
find_program(RUBY_EXECUTABLE NAMES ruby)
find_program(NPM_EXECUTABLE NAMES npm.cmd npm)

set(TEST_DIR    ${CMAKE_CURRENT_SOURCE_DIR})
set(LSCSV_DIR   ${TEST_DIR}/lscsv)
set(PLUGIN_DIR  ${TEST_DIR}/plugin)

set(DATA_TESTS "")   # LANGSCORE_TEST_DATA を渡す必要があるテスト

# --- C++ --------------------------------------------------------------------
# gtest の JSON は、レポート生成 (run_test.py) が失敗したテストケース名を拾うために出力する。
if(HAVE_TEST_DATA)
    add_test(NAME cpp
        COMMAND divisi_test --gtest_output=json:${PROJECT_BINARY_DIR}/cpp_test_results.json
        WORKING_DIRECTORY $<TARGET_FILE_DIR:divisi_test>)
    # 初回実行時にテストデータをビルドディレクトリへコピーするため長めに取る。
    set_tests_properties(cpp PROPERTIES TIMEOUT 1800)
    list(APPEND DATA_TESTS cpp)
endif()

# --- lscsv (resource/lscsv.{js,rb,py} の CSV パーサ) ------------------------
if(NPM_EXECUTABLE)
    if(NOT EXISTS ${LSCSV_DIR}/node_modules)
        message(WARNING "test/lscsv/node_modules がありません。'npm ci' を実行してください。")
    endif()
    add_test(NAME lscsv_js COMMAND ${NPM_EXECUTABLE} test WORKING_DIRECTORY ${LSCSV_DIR})
endif()
if(Python3_Interpreter_FOUND)
    add_test(NAME lscsv_py COMMAND ${Python3_EXECUTABLE} lscsv_test.py WORKING_DIRECTORY ${LSCSV_DIR})
endif()
if(RUBY_EXECUTABLE AND HAVE_TEST_DATA)
    # lscsv_test.rb は Skills.csv / CommonEvents.csv をカレントディレクトリから開くため、
    # テストデータの lscsv フォルダを作業ディレクトリにする。
    add_test(NAME lscsv_rb COMMAND ${RUBY_EXECUTABLE} ${LSCSV_DIR}/lscsv_test.rb
        WORKING_DIRECTORY ${TEST_DATA_ROOT}/lscsv)
    list(APPEND DATA_TESTS lscsv_rb)
endif()

# --- rvcnv -----------------------------------------------------------------
if(Python3_Interpreter_FOUND AND HAVE_TEST_DATA)
    add_test(NAME rvcnv
        COMMAND ${Python3_EXECUTABLE} run_rvcnv_test.py ${PROJECT_SOURCE_DIR}/rvcnv/rvcnv.exe
        WORKING_DIRECTORY ${TEST_DIR}/rvcnv_test)
    set_tests_properties(rvcnv PROPERTIES TIMEOUT 900)
    list(APPEND DATA_TESTS rvcnv)
endif()

# --- VXAce (Game.exe / RGSS3 を起動して実機上で実行する) --------------------
if(Python3_Interpreter_FOUND AND HAVE_TEST_DATA)
    foreach(VARIANT default deflang patch)
        add_test(NAME vxace_${VARIANT}
            COMMAND ${Python3_EXECUTABLE} run_vxace_test.py ${VARIANT}
            WORKING_DIRECTORY ${PLUGIN_DIR})
        # 3 つの variant は同じゲームプロジェクト (Game.ini を書き換える) を共有するため直列化する。
        set_tests_properties(vxace_${VARIANT} PROPERTIES
            RESOURCE_LOCK vxace_project
            RUN_SERIAL TRUE
            TIMEOUT 900)
        list(APPEND DATA_TESTS vxace_${VARIANT})
    endforeach()
endif()

# --- MV / MZ プラグイン (mocha) ---------------------------------------------
if(NPM_EXECUTABLE AND HAVE_TEST_DATA)
    if(NOT EXISTS ${PLUGIN_DIR}/node_modules)
        message(WARNING "test/plugin/node_modules がありません。'npm ci' を実行してください。")
    endif()
    # sync_plugin.js が resource/ のテンプレートをテストプロジェクトへ書き出す。
    # 全プラグインテストの前処理なので CTest のフィクスチャにする。
    add_test(NAME plugin_sync COMMAND ${NPM_EXECUTABLE} run pretest WORKING_DIRECTORY ${PLUGIN_DIR})
    set_tests_properties(plugin_sync PROPERTIES FIXTURES_SETUP plugin_sync TIMEOUT 300)
    list(APPEND DATA_TESTS plugin_sync)

    foreach(PROJ mv mz)
        foreach(RUNTIME nwjs browser)
            set(T plugin_${PROJ}_${RUNTIME})
            add_test(NAME ${T} COMMAND ${NPM_EXECUTABLE} run test-${PROJ}-${RUNTIME}
                WORKING_DIRECTORY ${PLUGIN_DIR})
            set_tests_properties(${T} PROPERTIES
                FIXTURES_REQUIRED plugin_sync
                RESOURCE_LOCK ${PROJ}_test_project
                TIMEOUT 600)
            list(APPEND DATA_TESTS ${T})
        endforeach()
    endforeach()
endif()

# --- divisi 結合テスト (bin/divisi.exe を叩く) ------------------------------
if(Python3_Interpreter_FOUND AND HAVE_TEST_DATA)
    # run_divisi_test.py は <repo>/bin/divisi.exe を実行する。exec/CMakeLists.txt の
    # POST_BUILD でコピーされるため、ctest の前に divisi をビルドしておく必要がある。
    add_test(NAME divisi_ct
        COMMAND ${Python3_EXECUTABLE} run_divisi_test.py
        WORKING_DIRECTORY ${TEST_DIR}/divisi_ct)
    set_tests_properties(divisi_ct PROPERTIES TIMEOUT 3000)
    list(APPEND DATA_TESTS divisi_ct)
endif()

# ---------------------------------------------------------------------------
# データを使うテストには CMake が解決したテストデータの場所を環境変数で渡し、
# 各ランナー (test_core.py / test_data_root.js) と参照先を一致させる。
# ---------------------------------------------------------------------------
if(DATA_TESTS)
    set_tests_properties(${DATA_TESTS} PROPERTIES
        ENVIRONMENT "LANGSCORE_TEST_DATA=${TEST_DATA_ROOT};PYTHONIOENCODING=utf-8")
endif()
