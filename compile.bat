cd vsprojects
nuget install ZED\packages.config        -OutputDirectory packages
nuget install GoogleTest\packages.config -OutputDirectory packages
msbuild /p:Platform="x86"
msbuild /p:Configuration=Release /p:Platform="x86"
copy packages\libpng.redist.1.6.28.1\build\native\bin\Win32\v140\dynamic\Debug\libpng16.dll   ..\bin\Debug
copy packages\libpng.redist.1.6.28.1\build\native\bin\Win32\v140\dynamic\Release\libpng16.dll ..\bin\Release
copy packages\zlib.v140.windesktop.msvcstl.dyn.rt-dyn.1.2.8.8\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\Win32\Release\zlib.dll ..\bin\Release\zlib.dll
cd ..
mkdir bin\tournaments