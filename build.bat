@echo off
@set OUT_DIR=debug
@set OUT_EXE=win32_main
@set INCLUDES=/Iufbx/**.h
@set SOURCES=win32_main.cpp
@set LIBS=User32.lib gdi32.lib d3d11.lib d3dcompiler.lib Xaudio2.lib
@set FLAGS=-wd4091 -DDEBUG=1
IF NOT EXIST %OUT_DIR%\ MKDIR %OUT_DIR%

cl /nologo /Zi /EHsc %FLAGS% %INCLUDES% %SOURCES% /Fe%OUT_DIR%/%OUT_EXE%.exe /Fo%OUT_DIR%/ /link %LIBS% 
