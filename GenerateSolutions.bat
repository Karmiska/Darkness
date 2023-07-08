@echo off
echo Compile Sharpmake
MSBuild tools\Sharpmake\Sharpmake.sln /property:Configuration=Release /p:Platform="Any CPU"
echo Generate Solution
%~dp0tools\Sharpmake\Sharpmake.Application\bin\Release\net6.0\Sharpmake.Application.exe "/sources(@'%~dp0main.sharpmake.cs')
echo Compile ShaderCompiler
MSBuild ide\vs2022\darkness.vs2022.sln /target:DarknessShaderCompiler /property:Configuration=Release
echo Compile Shaders (and create c++ interfaces)
CompileShaders.bat
echo Generate Final Solution
%~dp0tools\Sharpmake\Sharpmake.Application\bin\Release\net6.0\Sharpmake.Application.exe "/sources(@'%~dp0main.sharpmake.cs')
echo Compile protoc
MSBuild ide\vs2017\darkness.vs2017.sln /target:Externals\Protobuf\protoc /property:Configuration=Release
