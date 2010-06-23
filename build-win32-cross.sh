#!/bin/sh

make distclean || exit 1
./autogen.sh || exit 1
./configure --enable-win32 --host=i586-mingw32msvc --prefix=/usr/i586-mingw32msvc --enable-static --without-bzip2 --with-theora --disable-stdio-redirect || exit 1
make || exit 1
makensis stratagus.nsi || exit 1
mv stratagus-install.exe stratagus-install-i586.exe || exit 1

make distclean || exit 1
./autogen.sh || exit 1
./configure --enable-win32 --host=amd64-mingw32msvc --prefix=/usr/amd64-mingw32msvc --enable-static --without-bzip2 --with-theora --disable-stdio-redirect || exit 1
make || exit 1
makensis -DAMD64 stratagus.nsi || exit 1
mv stratagus-install.exe stratagus-install-amd64.exe || exit 1
