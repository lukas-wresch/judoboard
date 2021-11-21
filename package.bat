cd bin\Release
mkdir assets
mkdir html
mkdir tournaments
xcopy ..\assets assets /S /Y
xcopy ..\html   html   /S /Y
xcopy ..\*.bat  .      /S /Y
del GoogleTest.exe
del *.pdb
del *.lib
del *.exp
del *.txt
del *.csv
del *.zip
zip -r package.zip .
cd ..\..