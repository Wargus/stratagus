#!/bin/sh

echo "Exporting svn"
svn export . boswars
rm -rf boswars/installers
rm -rf boswars/engine

cp engine/Release/boswars.exe boswars
cp installers/nsi/SDL.dll boswars
cp installers/nsi/msvcp80.dll boswars
cp installers/nsi/msvcr80.dll boswars
cp installers/nsi/Microsoft.VC80.CRT.manifest boswars

echo "Creating installer"
"c:/Program Files/NSIS/makensis.exe" /NOCD installers/nsi/bos.nsi

echo "Cleaning up"
rm -rf boswars

echo "Done"

