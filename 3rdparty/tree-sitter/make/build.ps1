Param(
    [String]$Arg1
)

if( "force" -eq $Arg1 ) {
    Remove-Item -Recurse -Force .\tree-sitter, .\tree-sitter-ruby, .\tree-sitter-javascript, .\lib
}

if( !(Test-Path -Path .\tree-sitter) ){
    & git clone https://github.com/tree-sitter/tree-sitter
}
if( !(Test-Path -Path .\tree-sitter-ruby) ){
    & git clone https://github.com/tree-sitter/tree-sitter-ruby
}
if( !(Test-Path -Path .\tree-sitter-javascript) ){
    & git clone https://github.com/tree-sitter/tree-sitter-javascript
}

if( !(Test-Path -Path .\lib) ){
    Copy-Item -Path .\tree-sitter\lib -Destination .\ -Recurse

    Get-Content -Path .\lib\include\tree_sitter\api.h -Encoding UTF8 | Set-Content -Path .\lib\include\tree_sitter\api.h -Encoding utf8BOM
    Copy-Item -Path .\tree-sitter-javascript\src\tree_sitter\parser.h -Destination lib\include\tree_sitter\parser.h
    Copy-Item -Path .\tree-sitter-javascript\src\tree_sitter\alloc.h -Destination lib\include\tree_sitter\alloc.h
    Copy-Item -Path .\tree-sitter-javascript\src\tree_sitter\array.h -Destination lib\include\tree_sitter\array.h

    Copy-Item -Path .\tree-sitter-ruby\src\parser.c -Destination lib\src\parser_rb.c
    Copy-Item -Path .\tree-sitter-ruby\src\scanner.c -Destination lib\src\scanner_rb.c
    Copy-Item -Path .\tree-sitter-javascript\src\parser.c -Destination lib\src\parser_js.c
    Copy-Item -Path .\tree-sitter-javascript\src\scanner.c -Destination lib\src\scanner_js.c
}

Copy-Item -Path ..\Makefile -Destination .\
Copy-Item -Path ..\tree-sitter.pc.in -Destination .\

Remove-Item -Recurse -Force .\CMakeFiles, .\build-debug, .\build-release, .\build-linux-debug, .\build-linux-release -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Path "build-debug", "build-release", "build-linux-debug", "build-linux-release"

Set-Location build-debug
cmake.exe -G "Ninja" .. -DCMAKE_BUILD_TYPE:STRING="Debug" -DCMAKE_C_COMPILER:STRING="cl.exe" -DCMAKE_CXX_COMPILER:STRING="cl.exe" -DCMAKE_CXX_CL_SHOWINCLUDES_PREFIX="" -DCMAKE_MAKE_PROGRAM="C:\PROGRAM FILES\MICROSOFT VISUAL STUDIO\2022\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\Ninja\ninja.exe" "D:\Programming\Github\tree-sitter\build-debug"
& ninja
Copy-Item -Path .\tree-sitter.lib -Destination ..\..\tree-sitterD.lib
Set-Location ../

Set-Location build-release
cmake.exe -G "Ninja" .. -DCMAKE_BUILD_TYPE:STRING="Release" -DCMAKE_C_COMPILER:STRING="cl.exe" -DCMAKE_CXX_COMPILER:STRING="cl.exe" -DCMAKE_CXX_CL_SHOWINCLUDES_PREFIX="" -DCMAKE_MAKE_PROGRAM="C:\PROGRAM FILES\MICROSOFT VISUAL STUDIO\2022\COMMUNITY\COMMON7\IDE\COMMONEXTENSIONS\MICROSOFT\CMAKE\Ninja\ninja.exe" "D:\Programming\Github\tree-sitter\build-release"
& ninja
Copy-Item -Path .\tree-sitter.lib -Destination ..\..\tree-sitter.lib
Set-Location ../

wsl ./build_linux.sh

Copy-Item -Path .\build-linux-debug\libtree-sitter.a -Destination ..\libtree-sitterD.a
Copy-Item -Path .\build-linux-release\libtree-sitter.a -Destination ..\libtree-sitter.a