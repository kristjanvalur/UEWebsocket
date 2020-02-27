#!/bin/bash
set -eux
SCRIPT=`realpath $0`
SCRIPTPATH=`dirname $SCRIPT`

# script settings
OPENSSL_CONFIGURE=1
OPENSSL_BUILD=1
LWS_CMAKE=1
LWS_MAKE=1

# Setup standalone NDK toolchain
ANDROID_NDK_HOME=~/Android/android-ndk-r21
# select the abi and api. Lowest possible API for max compatibility with UE4
ANDROID_NDK_ABI=arm64-v8a
ANDROID_NDK_PREFIXC=aarch64-linux-android
ANDROID_NDK_PREFIXA=aarch64-linux-android
ANDROID_NDK_API=21
#or
#ANDROID_NDK_ABI=armeabi-v7a #official name
#ANDROID_NDK_PREFIXC=armv7a-linux-androideabi
#ANDROID_NDK_PREFIXA=arm-linux-androideabi
#ANDROID_NDK_API=19

# maximum compatibility with UE4.  For arm64, this will be upped to 21
TOOLCHAIN="${ANDROID_NDK_HOME}/toolchains/llvm/prebuilt/linux-x86_64"
CLANG="${TOOLCHAIN}/bin/${ANDROID_NDK_PREFIXC}${ANDROID_NDK_API}-clang"
AR="${TOOLCHAIN}/bin/${ANDROID_NDK_PREFIXA}-ar"

# Currently, Android still uses openssl 1.0.1s.  Executables are linked
# statically, so all code must use the same version of a library, even
# a plugin, like UEWebSocket.  Unlike Windows, it doesn't live in its own
# dll but is linked in with all other code.
OPENSSL_DIR=$SCRIPTPATH/openssl-1.0.1s
#OPENSSL_DIR=$SCRIPTPATH/openssl-1.1.1d
LWS_SRC=$SCRIPTPATH/libwebsockets
LWS_BUILD=$SCRIPTPATH/lws_build
OUTPUT_ROOT_DIR="$SCRIPTPATH/Library_Build"
OUTPUT_INCLUDE="$OUTPUT_ROOT_DIR/include"
OUTPUT_LIB="$OUTPUT_ROOT_DIR/lib/${ANDROID_NDK_ABI}"

# Configure the OpenSSL environment
# We use a generic linux 64 bit config, and simply use the toolchain
pushd ${OPENSSL_DIR}
if [[ $OPENSSL_CONFIGURE -gt 0 ]]
then
echo time to configure OPENSSL
./Configure linux-generic64
make clean
fi

if [[ $OPENSSL_BUILD -gt 0 ]]
then
echo time to make OPENSSL
# Build
# for openssl 1.0.1, the make file d esn't use ARFLAGS, have to supply it manually to AR
# and CFLAGS cannot be set here (for -fPIC) then everything goes wrong
make "CC=${CLANG} -fPIC" "AR=${AR} rv" "ARFLAGS=rv"
# openssl 1.1.1 correctly uses CFLAGS and ARFLAGS
#make "CC=${CLANG}" "CLFAGS=-fPIC" "AR=${AR}" "ARFLAGS=rv"

# Copy the outputs
mkdir -p $OUTPUT_INCLUDE
mkdir -p $OUTPUT_LIB
cp -RL include/openssl "$OUTPUT_INCLUDE"
cp libcrypto.a "$OUTPUT_LIB"
cp libssl.a "$OUTPUT_LIB"
echo ssl built files copied to \"$OUTPUT_ROOT_DIR\"
fi
popd

if [[ $LWS_CMAKE -gt 0 ]]
then
rm -rf ${LWS_BUILD}
mkdir -p ${LWS_BUILD}
pushd ${LWS_BUILD}
cmake $LWS_SRC \
-DLWS_WITH_MORE_LOGS=ON \
-DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK_HOME/build/cmake/android.toolchain.cmake  \
-DANDROID_ABI=$ANDROID_NDK_ABI \
-DANDROID_PLATFORM=android-$ANDROID_NDK_API \
-DOPENSSL_INCLUDE_DIR=$OUTPUT_INCLUDE \
-DOPENSSL_SSL_LIBRARY=$OUTPUT_LIB/libssl.a \
-DOPENSSL_CRYPTO_LIBRARY=$OUTPUT_LIB/libcrypto.a
#-DOPENSSL_ROOT_DIR=$OPENSSL_ROOT_DIR \
popd
fi

if [[ $LWS_MAKE -gt 0 ]]
then
pushd ${LWS_BUILD}
make -j 
cp -RL include/* $OUTPUT_INCLUDE
cp lib/* $OUTPUT_LIB
echo lws built files copied to \"$OUTPUT_ROOT_DIR\"
popd
fi
