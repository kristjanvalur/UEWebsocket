# building the libwebsockets for UE4.
This file contains instructions for building the libwebsockets third party library, which is used by UEWebsocket.

Note that this is no longer deemed necessary since at least since version 4.23
UE4 ships with libwebsockets 3.0.0 which is adequate, and with a matching
openssl library.  Only on windows is there an issue which was later fixed
in lws 3.2, in commits 1485db1805ffa175df5d1568f6d568822fc4fd4c and
16e31d4fd6fa7c437bbcd013f99fcacb3bf536d7

Using the provided lws and openssl makes everything simpler.  Please see
WebSocket.Build.cs for the various include file dependencies and link
targets.

libwebsockets depends on openssl.  Our approach is to use the same api level of openssl as UE4
uses.  For recent windows versions, this is 1.1.1.  Android and IOS are still at level 1.0.1.

libwebsockets requires the correct headers for compiling and it creates a static library.  For linking
the plugin, placing a dependency on unreal's OpenSSL module allows us to link.

The full openssl headers are present in the plugin, and also sometimes the openssl
static libs librcypto.a and libssl.a.  These can be used, but it is probably better to link with the OpenSSL third party module in UE4.  For monolithic builds, like on Android, this is necessary since there
cannot be more than one version of the library in the build.

These instructions are how to build the openssl libs and libwebsockets libs to add to the third party libs.

Ready made scripts, perhaps requiring slight modifications, are in the `script` folder.

## Windows, 64 bits.

1. Get *libwebsockets* off github.  Check out the v3.2 stable branch (or whichever branch you intend to use, but this project currently uses that.)  https://github.com/warmcat/libwebsockets/tree/v3.2-stable
2. Read the READMEs/README.build.md
3. Install cmake as instructed for your windows machine

4. Install OpenSSL developer libraries.
   You need the 1.0.x series for Unreal Engine 4.21 and older, and 1.1.1 for 4.22.  This is assuming Win64 targets.  Other targets,
   such as IOS or Android, use different versions.  See \Engine\Source\ThirdParty\OpenSSL\OpenSSL.Build.cs for details.
   The intent is to link to the openssl libraries from UE4, but you need to install the correct development headers for libwebsockets and the *cmake* config for libwebsockets will look only in default places for those.
   Download from here https://slproweb.com/products/Win32OpenSSL.html don't use the 'light' version, and install everything in default place (c:/Program Files/OpenSSL-Win64)

6. construct the projects by running cmake.  For help on cmake options, see the CMakeLists.txt file under the root of the libwebsockets project.
   - Open a Visual studio command shell (Win64)
   - First create a "build" folder under the root and cd into it.
   - execute cmake, here are the options:
   - -G "Visual Studio 15 Win64"  (for win64 and VS2017)
   - -DCMAKE_INSTALL_PREFIX=d:\inst  (or wherever, just not c:\Program Files\)
   - -DLWS_WITH_MORE_LOGS=ON  (enables more log channels in relase build)
   - ..  (the source dir)

   Verify that CMake has found the correct verision of openssl.

6. Open the visual studio project.  Select solution configuration Release or Debug.  Build INSTALL in Release mode
   It will fail all of the configurations that require DLLS, but the libwebsockets_static.lib will get generated and the
   include files.

7. Copy the include files generated in the d:\inst to the Win64/include folder.

8. copy the libwebsockets_static.lib to the Win64/lib folder.  For 4.22 it is called libwebsocket_static422, because
   4.22 contains a different openssl third party library and linking needs to match

## Android

Building the android libraries is best done on linux.  Using for example the Windows Subsystem for Linux, a developer can install Ubuntu on his windows machine and the Android development kit.

1. Install the latest android NDK, e.g. android_ndk_r21
2. get the sources for openess-1.0.1s
3. get the correct branch of libwebsockets, 3.2, that you want to use.
4. Run the script, build_android.sh, which is in the `script` folder.  Modify it according to your local
   setup.
5. Copy the include files and the libraries, minus the .so files (not required) into the include/Android and lib/Android folders in the plugin.

Look at the script file for more details.  Notice that we select the API level 19, it must not be higher than the target API level used to build the Unreal project.  For ARM64, 19 is actually turned to 21 which is the minumum platform for that architecture, 19 is used for armv7 code.

Android apps are linked into a single executable and so the plugin must use the same openssl library
as the engine does.  The openssl libraries are already linked in as part of libcurl,
which is part of the engine, and actually linking with the compiled libraries isn't necessary,
or may cause linker errors due to duplicate symbols.  If necessary for your application, change the
WebSocket.Build.cs as required to link in the libraries or not.
