# building the libwebsocket for UE4.  Target is windows, 64 bits.

1. Get libwebsockets off github.  Check out the v3.1 stable branch (or whichever branch you intend to use)
2. Read the READMEs/README.build.md
3. Install cmake as instructed for your windows machine

4. *Optional:* Install OpenSSL developer libraries.
   You need the 1.0.x series for 4.21 and older, and 1.1.1 for 4.22.
   Download from here https://slproweb.com/products/Win32OpenSSL.html don't use the 'light' version, and install everything in default place (c:/Program Files/OpenSSL-Win64)
   Note that UE4.22 has upgraded openssl to 1.1.0 so you need to install that.

5. Better: Just install ue4.22, and use the openssl from its thirdparty folder. see below

6. construct the projects by running cmake.  For help on cmake options, see the CMakeLists.txt file under the root.
   - Open a Visual studio command shell (Win64)
   - First create a "build" folder under the root and cd into it.
   - execute cmake, here are the options:
   - -G "Visual Studio 15 Win64"  (for win64 and VS2017)
   - -DCMAKE_INSTALL_PREFIX=d:\inst  (or wherever, just not c:\Program Files\)
   - ..  (the source dir)

   If ue422 is in `d:\UE4\` you need these additional flags to cmake:
     - -DLWS_OPENSSL_INCLUDE_DIRS=D:\UE4\UE_4.22\Engine\Source\ThirdParty\OpenSSL\1.1.1\Include\Win64\VS2015
     - -DLWS_OPENSSL_LIBRARIES=D:\UE4\UE_4.22\Engine\Source\ThirdParty\OpenSSL\1.1.1\lib\Win64\VS2015\Release\libcrypto.lib;D:\UE4\UE_4.22\Engine\Source\ThirdParty\OpenSSL\1.1.1\lib\Win64\VS2015\Release\libssl.lib
     - *UPDATE* the above doesn't work, cmake won't be able to make the correct library for some reason.

   Verify that CMake has found the correct verision of openssl.

6. Open the visual studio project.  Select solution configuration Release or Debug.  Build INSTALL in Release mode
   It will fail all of the configurations that require DLLS, but the libwebsockets_static.lib will get generated and the
   include files.

7. Copy the include files generated in the d:\inst to the include folder.

8. copy the libwebsockets_static.lib to the lib folder.  For 4.22 it is called libwebsocket_static422, because
   4.22 contains a different openssl third party library and linking needs to match

