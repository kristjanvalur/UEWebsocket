# building the libwebsockets for UE4.  Target is windows, 64 bits.

This file contains instructions for building the libwebsockets third party library, which is used by UEWebsocket.  For Windows, a set of headers and libraries must be generated and added to this project.  For other platforms, well, it is unknown.

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
   - ..  (the source dir)

   Verify that CMake has found the correct verision of openssl.

6. Open the visual studio project.  Select solution configuration Release or Debug.  Build INSTALL in Release mode
   It will fail all of the configurations that require DLLS, but the libwebsockets_static.lib will get generated and the
   include files.

7. Copy the include files generated in the d:\inst to the include folder.

8. copy the libwebsockets_static.lib to the lib folder.  For 4.22 it is called libwebsocket_static422, because
   4.22 contains a different openssl third party library and linking needs to match

