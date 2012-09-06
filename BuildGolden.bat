call "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86
devenv Source/VSProjects/VS10/Abducted.sln /clean "Golden - Release|Win32"
devenv Source/VSProjects/VS10/Abducted.sln /build "Golden - Release|Win32" /project Abducted
pause