@echo off
SetLocal EnableDelayedExpansion

set Opts=-g -Wno-pointer-sign -mavx2 -Wno-deprecated-declarations
set Incs= -I../download/ -IC:/VulkanSDK/1.4.341.1/Include
set Defs=-DDEBUG
@REM set Libs=-lkernel32 -luser32 -lgdi32 -lwinmm -lopengl32
set Libs=-lkernel32 -luser32 -lgdi32 -lwinmm -lvulkan-1
set Name=main.exe

if not exist build mkdir build
echo Building.
pushd build
  @REM clang %Opts% ../win32_opengl.c %Incs% %Defs% %Libs% -o main.exe
  @REM clang %Opts% ../app.c %Incs% %Defs% %Libs% -shared -o app.dll

  C:/VulkanSDK/1.4.341.1/Bin/glslc.exe ../shaders/shader.vert -o vert.spv
  C:/VulkanSDK/1.4.341.1/Bin/glslc.exe ../shaders/shader.frag -o frag.spv

  clang %Opts% ../win32_vulkan.c %Incs% %Defs% %Libs% -o main.exe
  if %ERRORLEVEL% equ 0 echo Built succesfully.
  if %ERRORLEVEL% neq 0 echo Build failed.
popd
