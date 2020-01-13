@echo off

del Main.exe
set ExtraCompilerFlags=/O2 /MTd /nologo /fp:fast /fp:except- /Gm /GR /EHa /Zo /Oi /WX /W4 
set CompilerFlags=%ExtraCompilerFlags% /D_CRT_SECURE_NO_WARNINGS /Zi /Fe:Main.exe
set L   inkerFlags=
cl %CompilerFlags% main.cpp %LinkerFlags%

if %ERRORLEVEL% EQU 0 (
    Main.exe
    start render.bmp
)