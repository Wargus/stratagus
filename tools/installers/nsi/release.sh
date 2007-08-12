#!/bin/sh

echo "Exporting svn"
svn export . boswars
rm -rf boswars/tools
rm -rf boswars/engine
rm -f boswars/bos.sln
rm -f boswars/SConstruct

cp engine/Release/boswars.exe boswars
cp tools/installers/nsi/SDL.dll boswars
cp tools/installers/nsi/msvcp80.dll boswars
cp tools/installers/nsi/msvcr80.dll boswars
cp tools/installers/nsi/Microsoft.VC80.CRT.manifest boswars

echo "Creating installer"
"c:/Program Files/NSIS/makensis.exe" /NOCD tools/installers/nsi/bos.nsi

echo "Cleaning up"
rm -rf boswars

echo "Done"

