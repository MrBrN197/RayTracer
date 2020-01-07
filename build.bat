@echo off

set CompilerFlags=/Od /Zi /EHa /std:c++17 /Fe:Main.exe
set LinkerFlags=
cl %CompilerFlags% Main.cpp %LinkerFlags%

if %ERRORLEVEL% EQU 0 (
    Main.exe
    start render.bmp
)