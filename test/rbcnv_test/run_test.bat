@echo off

RMDIR /S /Q .\test_analyze
RMDIR /S /Q .\test_packing

set RVCNV_PATH="rvcnv.exe"

rem 解析のテスト
echo "[Analyze Test]"
rvcnv.exe -i "..\data\ソポァゼゾタダＡボマミ\" -o .\test_analyze

IF not EXIST ".\test_analyze" (
    echo File does not exist test_analyze folder.
)
IF not EXIST ".\test_analyze\Scripts" (
    echo File does not exist Scripts Folder.
)

rem パッキングのテスト
echo "[Packing Test]"
%RVCNV_PATH% -p -i "..\data\ソポァゼゾタダＡボマミ\" -o .\test_packing

IF not EXIST ".\test_packing" (
    echo File does not exist.
    GOTO :ERROR
)

IF not EXIST ".\test_packing" (
    echo File does not exist.
    GOTO :ERROR
)

GOTO :EOF

:ERROR
echo An error occurred, exiting script.
pause