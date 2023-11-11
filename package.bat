del Judoboard.zip
cd bin
rmdir /s /q Package
mkdir Package
copy *.dll Package\
copy Judoboard.exe Package\
copy demo.bat Package\
copy ..\external\vcruntime140_1.dll Package\
cd Package
mkdir assets
mkdir html
mkdir tournaments
xcopy ..\assets assets\  /S /Y
xcopy ..\html   html\    /S /Y
xcopy ..\*.bat   .       /S /Y
del GoogleTest.exe
del zlibd.dll
del ZEDd.dll
del *.pdb
del *.lib
del *.exp
del *.txt
del *.csv
del *.zip
del test.bat
zip -r ../../Judoboard.zip .
cd ..
rmdir /s /q Package
cd ..