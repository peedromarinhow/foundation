@echo off
SetLocal EnableDelayedExpansion

set Opts=-g -Wno-deprecated-declarations -Wno-pointer-sign -Wno-writable-strings -Wno-unknown-warning-option -Wno-microsoft-anon-tag -fdiagnostics-absolute-paths
set Defs=-DENABLE_ASSERT=1 -DDEBUG=1
set Srcs=../main.c -o main.exe
set Incs=-I.
set Libs=-luser32 -lwinmm -lgdi32

call %vcvarsall% x64

if not exist build mkdir build

pushd build
if "%~1"=="" echo Building.
clang %Opts% %Defs% %Srcs% %Incs% %Libs%
if "%~1"=="run" if %ERRORLEVEL% equ 0 main.exe
if "%~1"==""    if %ERRORLEVEL% equ 0 echo Built succesfully.
if "%~1"==""    if %ERRORLEVEL% neq 0 echo Build failed.

popd
