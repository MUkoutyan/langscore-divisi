set PATH=%PATH%;.D:\Programming\Github\langscore-divisi\rvcnv\ruby\bin
REM ocra rvcnv.rb --windows --icon "./icon.ico" --dll ruby_builtin_dlls\libssp-0.dll --dll ruby_builtin_dlls\libgmp-10.dll --dll ruby_builtin_dlls\zlib1.dll && ruby generate_hash.rb
.\ruby\bin\ocra.bat rvcnv.rb --windows --icon "./icon.ico" --dll ruby_builtin_dlls\libssp-0.dll --dll ruby_builtin_dlls\libgmp-10.dll --dll ruby_builtin_dlls\zlib1.dll && ruby generate_hash.rb

ruby generate_hash.rb