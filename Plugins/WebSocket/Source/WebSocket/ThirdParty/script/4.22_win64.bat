rem This bat file builds the libraries for UE 4.22, win64 from
rem a libwebsockets repo and a UE4.22 engine install.
rem Perform the neccessary changes, then run this from a VisualStudio2017 cmd prompt
rem see libwebsockets.md for prerequisites, such as cmake
rem @echo off

set UE422=D:\UE4\UE_4.22
set LWS=D:\git\libwebsockets
set INST=D:\inst
set VISUAL_STUDIO=C:\Program Files (x86)\Microsoft Visual Studio
rem CMAKE needs to run only after you update the sources, otherwise you can skip it for faster turnaround.
set RUN_CMAKE=1
set RUN_BUILD=1
set RUN_INSTALL=1
set COPY_HEADERS=1
set COPY_LIBS=1
rem set Debug/Release
set CONFIGURATION=Release

set DEST=%~dp0..

if not defined DevEnvDir (
    call "%VISUAL_STUDIO%\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
)

rem cd to parent folder
rem pushd  %~dp0\..

pushd %LWS%
if not exist build\ mkdir build
cd build

rem build libwebsockets

rem clean and run cmake
if "%RUN_CMAKE%"=="1" (
	del /S /Q *.*

	cmake -G "Visual Studio 15 Win64" -DCMAKE_INSTALL_PREFIX=%INST% ^
	-DLWS_WITH_MORE_LOGS=ON ^
	..
	rem add -DCMAKE_BUILD_TYPE=DEBUG to get full logging
	rem specifying the UE4 libraries does not work.  Just use the installed 1.1.1 version
	rem -DLWS_OPENSSL_INCLUDE_DIRS=%UE422%\Engine\Source\ThirdParty\OpenSSL\1.1.1\Include\Win64\VS2015 ^
	rem -DLWS_OPENSSL_LIBRARIES=%UE422%\Engine\Source\ThirdParty\OpenSSL\1.1.1\lib\Win64\VS2015\Release\libcrypto.lib;%UE422%\Engine\Source\ThirdParty\OpenSSL\1.1.1\lib\Win64\VS2015\Release\libssl.lib ^
)

rem run visual studio
if "%RUN_BUILD%"=="1" (
	msbuild.exe /property:Configuration=%CONFIGURATION% /target:websockets libwebsockets.sln 
)

if "%RUN_INSTALL%"=="1" (
	rem msbuild.exe /property:Configuration=%CONFIGURATION% /target:install libwebsockets.sln 
	msbuild.exe /property:Configuration=%CONFIGURATION% install.vcxproj 
)

if "%COPY_HEADERS%"=="1" (
	pushd "%INST%\include"
	xcopy /S /Y *.* "%DEST%\include\Win64"
	popd
)
if "%COPY_LIBS%"=="1" (
	pushd "%INST%\lib"
	copy websockets_static.lib "%DEST%\lib\Win64\websockets_static422.lib"
	popd
)

popd
