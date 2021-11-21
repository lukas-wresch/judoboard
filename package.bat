cd bin\Release
mkdir assets
mkdir html
mkdir tournaments
xcopy ..\assets assets /S
xcopy ..\html   html   /S
del *.pdb
del *.lib
del *.exp
del *.zip
zip -r package.zip .
cd ..\..