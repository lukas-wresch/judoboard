cd vsprojects
nuget install ZED\packages.config        -OutputDirectory packages
nuget install GoogleTest\packages.config -OutputDirectory packages
copy packages\libpng.1.6.28.1\build\native\lib\x64\v140\dynamic\Release\libpng16.lib ZED\libpng16.lib
msbuild
msbuild /p:Configuration=Release
cd ..