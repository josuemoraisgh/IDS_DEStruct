call ""C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat""
cd /d ""c:\SourceCode\IDS_DEStruct\build""
qmake ..\IDS_DEStruct.pro ""CONFIG+=release""
nmake
