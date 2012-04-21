mkdir Bin
cd Bin
mkdir win-msvc10-debug
mkdir win-msvc10-developer
mkdir win-msvc10-developer-sym
mkdir win-msvc10-golden
cd ..


xcopy Radiance\Extern\Win\Cg\2.2\bin\cg.dll Bin\win-msvc10-debug /D /Y
xcopy Radiance\Extern\Win\Cg\2.2\bin\cgGL.dll Bin\win-msvc10-debug /D /Y
xcopy Radiance\Extern\Win\Cg\2.2\bin\cg.dll Bin\win-msvc10-developer /D /Y
xcopy Radiance\Extern\Win\Cg\2.2\bin\cgGL.dll Bin\win-msvc10-developer /D /Y
xcopy Radiance\Extern\Win\Cg\2.2\bin\cg.dll Bin\win-msvc10-developer-sym /D /Y
xcopy Radiance\Extern\Win\Cg\2.2\bin\cgGL.dll Bin\win-msvc10-developer-sym /D /Y

xcopy Radiance\Extern\Win\Qt\4.8.1\bin\QtCored4.dll Bin\win-msvc10-debug /D /Y
xcopy Radiance\Extern\Win\Qt\4.8.1\bin\QtGuid4.dll Bin\win-msvc10-debug /D /Y
xcopy Radiance\Extern\Win\Qt\4.8.1\bin\QtOpenGLd4.dll Bin\win-msvc10-debug /D /Y
xcopy Radiance\Extern\Win\Qt\4.8.1\bin\QtCore4.dll Bin\win-msvc10-developer /D /Y
xcopy Radiance\Extern\Win\Qt\4.8.1\bin\QtGui4.dll Bin\win-msvc10-developer /D /Y
xcopy Radiance\Extern\Win\Qt\4.8.1\bin\QtOpenGL4.dll Bin\win-msvc10-developer /D /Y
xcopy Radiance\Extern\Win\Qt\4.8.1\bin\QtCore4.dll Bin\win-msvc10-developer-sym /D /Y
xcopy Radiance\Extern\Win\Qt\4.8.1\bin\QtGui4.dll Bin\win-msvc10-developer-sym /D /Y
xcopy Radiance\Extern\Win\Qt\4.8.1\bin\QtOpenGL4.dll Bin\win-msvc10-developer-sym /D /Y

xcopy Radiance\Extern\Win\SDL\1.2.15\SDL.dll Bin\win-msvc10-debug /D /Y
xcopy Radiance\Extern\Win\SDL\1.2.15\SDL.dll Bin\win-msvc10-developer /D /Y
xcopy Radiance\Extern\Win\SDL\1.2.15\SDL.dll Bin\win-msvc10-developer-sym /D /Y
xcopy Radiance\Extern\Win\SDL\1.2.15\SDL.dll Bin\win-msvc10-golden /D /Y

xcopy Radiance\Extern\Win\VLD\1.9h\VS8\bin\dbghelp.dll Bin\win-msvc10-debug /D /Y
xcopy Radiance\Extern\Win\VLD\1.9h\VS8\bin\Microsoft.DTfW.DHL.manifest Bin\win-msvc10-debug /D /Y
xcopy Radiance\Extern\Win\VLD\1.9h\VS8\bin\vld.dll Bin\win-msvc10-debug /D /Y

xcopy Radiance\Extern\Win\OpenAL\bin\softoal\OpenAL32.dll Bin\win-msvc10-debug /D /Y
xcopy Radiance\Extern\Win\OpenAL\bin\softoal\OpenAL32.dll Bin\win-msvc10-developer /D /Y
xcopy Radiance\Extern\Win\OpenAL\bin\softoal\OpenAL32.dll Bin\win-msvc10-developer-sym /D /Y
xcopy Radiance\Extern\Win\OpenAL\bin\softoal\OpenAL32.dll Bin\win-msvc10-golden /D /Y

xcopy Radiance\Extern\Win\PVRTexLib\x32\PVRTexLib.dll Bin\win-msvc10-debug /D /Y
xcopy Radiance\Extern\Win\PVRTexLib\x32\PVRTexLib.dll Bin\win-msvc10-developer /D /Y
xcopy Radiance\Extern\Win\PVRTexLib\x32\PVRTexLib.dll Bin\win-msvc10-developer-sym /D /Y
