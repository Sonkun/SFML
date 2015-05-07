#!/usr/bin/env bash

FLAC_VERSION=1.3.1
VORBIS_VERSION=1.3.3
OGG_VERSION=1.3.1

FLAC=flac-$FLAC_VERSION
VORBIS=libvorbis-$VORBIS_VERSION
OGG=libogg-$OGG_VERSION

mkdir build

wget -nc -P src http://downloads.xiph.org/releases/flac/$FLAC.tar.xz
if [ ! -d "$PWD/tmp/$FLAC" ]
then
	tar -C build -xf src/$FLAC.tar.xz
fi

wget -nc -P src http://downloads.xiph.org/releases/vorbis/$VORBIS.tar.gz
if [ ! -d "$PWD/tmp/$VORBIS" ]
then
	tar -C build -xf src/$VORBIS.tar.gz
fi

wget -nc -P src http://downloads.xiph.org/releases/ogg/$OGG.tar.gz
if [ ! -d "$PWD/tmp/$OGG" ]
then
	tar -C build -xf src/$OGG.tar.gz
fi
