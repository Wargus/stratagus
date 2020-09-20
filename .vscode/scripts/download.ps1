curl https://github.com/Wargus/win32-stratagus-dependencies/releases/download/master-builds/dependencies.zip -OutFile "$args[0]/../dependencies.zip"
Expand-Archive "$args[0]/../dependencies.zip"
move "$args[0]/../dependencies/dependencies/"* "$args[0]/../dependencies/"
curl https://github.com/Wargus/stratagus/releases/download/2015-30-11/ffmpeg.exe -OutFile "$args[0]/../dependencies/bin/ffmpeg.exe"
