cd vsprojects
nuget install ZED\packages.config        -OutputDirectory packages
nuget install GoogleTest\packages.config -OutputDirectory packages
msbuild
msbuild /p:Configuration=Release
copy packages\libpng.redist.1.6.28.1\build\native\bin\x64\v140\dynamic\Release\libpng16.dll ..\bin\libpng16.dll
copy packages\zlib.v140.windesktop.msvcstl.dyn.rt-dyn.1.2.8.8\lib\native\v140\windesktop\msvcstl\dyn\rt-dyn\x64\Release\zlib.dll ..\bin\zlib.dll
cd ..
mkdir bin\tournaments