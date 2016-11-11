#!/usr/bin/env bash

# This script downloads and compiles the SFML external libraries for
# Tizen (OGG, FLAC, Vorbis, Freetype and JPEG). Eventually, adjust the
# global variables of this script to modify its behavior such as
# downloading different versions of these extlibs, or for more advanced
# used, tweak the compilation options, or even change the compiler.
#
# They can be compiled for various targets and in various configurations
# which must be specicy in parameters when you invoke this script.
# However it leaves no option for the compiler to use; GNU GCC is used
# rather than Clang.
#
# Command-line interface:
#
#     compile-tizen-extlibs mobile|wearable version device|emulator
#
# It expects the $TIZEN_STUDIO environment variable set and its going to
# compile everything in-place (in the current directory). The result
# will look like the following.
#
# src/     - Where the extlibs tarball are downloaded
# build/   - Where the sources are extracted and compiled
# tmp/     - Temporary install (inter-dependencies between libs)
# install/ - Where result (lib*.a) is placed
#
# The compilation happens in 4 steps. First, it downloads the source
# tarball of all libraries to compile if not done yet. Then erase any
# local build/, tmp/ and install/ directory. For each libraries, the
# source code is extracted in build/, compiled, and installed in tmp/.
# If the process was successful, All .a files relevant to SFML are
# copied over in the install/ directory.

# Request the script to fail early if any error occur
set -e

# Parse command-line arguments and do a bit of checking (checking
# version was omitted)
if ! [ $1 == "mobile" -o $1 == "wearable" ]; then
    echo "args: mobile|wearable version device|emulator"
    exit 1
fi

if ! [ $3 == "device" -o $3 == "emulator" ]; then
    echo "args: mobile|wearable version device|emulator"
    exit 1
fi

# Set script global variables
TIZEN_STUDIO=/opt/tizen-studio

FLAC_VERSION=1.3.1
VORBIS_VERSION=1.3.3
OGG_VERSION=1.3.1
JPEG_VERSION=9

SRCDIR=$PWD/src
BUILDDIR=$PWD/build
TMPDIR=$PWD/tmp
INSTALLDIR=$PWD/install

DEVICE=$1
VERSION=$2
TARGET=$3

# Step 1: download and extract source tarball according to requested versions
rm -rf $BUILDDIR
mkdir -p $BUILDDIR

wget -nc -P src http://www.ijg.org/files/jpegsrc.v$JPEG_VERSION.tar.gz
if [ ! -d "$PWD/tmp/jpeg-$JPEG_VERSION" ]
then
    tar -C build -xf src/jpegsrc.v$JPEG_VERSION.tar.gz
fi

wget -nc -P src http://downloads.xiph.org/releases/flac/flac-$FLAC_VERSION.tar.xz
if [ ! -d "$PWD/tmp/flac-$FLAC_VERSION" ]
then
    tar -C build -xf src/flac-$FLAC_VERSION.tar.xz
fi

wget -nc -P src http://downloads.xiph.org/releases/vorbis/libvorbis-$VORBIS_VERSION.tar.gz
if [ ! -d "$PWD/tmp/libvorbis-$VORBIS_VERSION" ]
then
    tar -C build -xf src/libvorbis-$VORBIS_VERSION.tar.gz
fi

wget -nc -P src http://downloads.xiph.org/releases/ogg/libogg-$OGG_VERSION.tar.gz
if [ ! -d "$PWD/tmp/libogg-$OGG_VERSION" ]
then
    tar -C build -xf src/libogg-$OGG_VERSION.tar.gz
fi

# Step 2: set up compiler and rootstrap according to device, its version
# and target
ROOTSTRAP="${TIZEN_STUDIO}/platforms/tizen-${VERSION}/${DEVICE}/rootstraps/${DEVICE}-${VERSION}-${TARGET}.core"

if [ $TARGET == "device" ]; then

    export CC="${TIZEN_STUDIO}/tools/arm-linux-gnueabi-gcc-4.9/bin/arm-linux-gnueabi-gcc"
    export CXX="${TIZEN_STUDIO}/tools/arm-linux-gnueabi-gcc-4.9/bin/arm-linux-gnueabi-g++"
    export LD="${TIZEN_STUDIO}/tools/arm-linux-gnueabi-gcc-4.9/bin/arm-linux-gnueabi-ld"

elif [ $TARGET == "emulator" ]; then

    export CC="${TIZEN_STUDIO}/tools/i386-linux-gnueabi-gcc-4.9/bin/i386-linux-gnueabi-gcc"
    export CXX="${TIZEN_STUDIO}/tools/i386-linux-gnueabi-gcc-4.9/bin/i386-linux-gnueabi-g++"
    export LD="${TIZEN_STUDIO}/tools/i386-linux-gnueabi-gcc-4.9/bin/i386-linux-gnueabi-ld"

fi

export CFLAGS="-fPIC --sysroot=${ROOTSTRAP} -I${TMPDIR}/usr/include"
export CXXFLAGS="-fPIC --sysroot=${ROOTSTRAP} -I${TMPDIR}/usr/include"
export LDFLAGS="-fPIC --sysroot=${ROOTSTRAP} -L${TMPDIR}/usr/lib"

# Step 3: compile the three libraries one by one
rm -rf $TMPDIR
rm -rf $INSTALLDIR

PREFIX=$TMPDIR/usr

if [ $TARGET == "device" ]; then
    HOSTNAME=arm-linux-gnueabi
elif [ $TARGET == "emulator" ]; then
    HOSTNAME=i386-linux-gnueabi
fi

cd $BUILDDIR/jpeg-*
./configure --host=$HOSTNAME --prefix=$TMPDIR/usr --enable-shared=no
make -j4
make install

cd $BUILDDIR/libogg-*
./configure --host=$HOSTNAME --prefix=$TMPDIR/usr --enable-shared=no
make -j4
make install

cd $BUILDDIR/flac-*
./configure --host=$HOSTNAME --prefix=$TMPDIR/usr --enable-shared=no
make -j4
make install

cd $BUILDDIR/libvorbis-*
./configure --host=$HOSTNAME --prefix=$TMPDIR/usr --enable-shared=no
make -j4
make install

# Step 4: install compiled extlibs in the install directory
mkdir -p $INSTALLDIR

cp $TMPDIR/usr/lib/libjpeg.a $INSTALLDIR
cp $TMPDIR/usr/lib/libogg.a $INSTALLDIR
cp $TMPDIR/usr/lib/libFLAC.a $INSTALLDIR
cp $TMPDIR/usr/lib/libFLAC++.a $INSTALLDIR
cp $TMPDIR/usr/lib/libvorbis.a $INSTALLDIR
cp $TMPDIR/usr/lib/libvorbisenc.a $INSTALLDIR
cp $TMPDIR/usr/lib/libvorbisfile.a $INSTALLDIR
