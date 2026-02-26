@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul 2>&1
cd /d "c:\SourceCode\IDS_DEStruct\build"
nmake clean >nul 2>&1
qmake ..\IDS_DEStruct.pro "CONFIG+=release"
nmake 2>&1
