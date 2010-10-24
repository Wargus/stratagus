#!/bin/sh

make distclean || exit 1
./autogen.sh || exit 1
./configure --enable-win32 --host=x86_64-w64-mingw32 --prefix=/usr/x86_64-w64-mingw32 --enable-static --without-bzip2 --disable-stdio-redirect || exit 1
make || exit 1
make strip || exit 1
#upx -9 stratagus.exe || exit 1
makensis -DAMD64 stratagus.nsi || exit 1

make distclean || exit 1
./autogen.sh || exit 1
./configure --enable-win32 --host=i686-w64-mingw32 --prefix=/usr/i686-w64-mingw32 --enable-static --without-bzip2 --disable-stdio-redirect || exit 1
make || exit 1
make strip || exit 1
upx -9 stratagus.exe || exit 1
makensis stratagus.nsi || exit 1
