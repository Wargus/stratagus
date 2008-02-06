#!/bin/sh

echo "Exporting svn"
svn export . boswars
rm -rf boswars/tools
rm -rf boswars/engine
rm -f boswars/bos.sln
rm -f boswars/SConstruct

wget -N http://www.boswars.org/dependencies/win32/windows-runtime-libs.zip
unzip windows-runtime-libs.zip
mv windows-runtime-libs/*.dll boswars/
rmdir windows-runtime-libs

cp engine/Release/boswars.exe boswars
cp tools/installers/nsi/Microsoft.VC80.CRT.manifest boswars

echo "Creating installer"
"c:/Program Files/NSIS/makensis.exe" /NOCD tools/installers/nsi/bos.nsi

echo "Cleaning up"
rm -rf boswars

echo "Done"

