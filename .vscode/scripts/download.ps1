if (-not (Test-Path "$args[0]/../dependencies")) {
    if (-not (Test-Path "$args[0]/../dependencies.zip")) {
        Invoke-WebRequest https://github.com/Wargus/win32-stratagus-dependencies/releases/download/master-builds/dependencies.zip -OutFile "$args[0]/../dependencies.zip"
    }
    Expand-Archive "$args[0]/../dependencies.zip"
    Move-Item "$args[0]/../dependencies/dependencies/"* "$args[0]/../dependencies/"
}
if (-not (Test-Path "$args[0]/../dependencies/bin/ffmpeg.exe")) {
    Invoke-WebRequest https://github.com/Wargus/stratagus/releases/download/2015-30-11/ffmpeg.exe -OutFile "$args[0]/../dependencies/bin/ffmpeg.exe"
}
if (-not (Test-Path "$args[0]/../wargus")) {
    git clone https://github.com/Wargus/wargus "$args[0]/../wargus"
}
if (-not (Test-Path "$args[0]/../war1gus")) {
    git clone https://github.com/Wargus/wargus "$args[0]/../war1gus"
}
