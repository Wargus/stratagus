$stratagus_folder = $args[0]
if (-not (Test-Path "$stratagus_folder\\..\\dependencies")) {
    if (-not (Test-Path "$stratagus_folder\\..\\dependencies.zip")) {
        Invoke-WebRequest https://github.com/Wargus/win32-stratagus-dependencies/releases/download/master-builds/dependencies.zip -OutFile "$stratagus_folder\\..\\dependencies.zip"
    }
    Expand-Archive "$stratagus_folder\\..\\dependencies.zip" -DestinationPath "$stratagus_folder\\..\\dependencies"
    Move-Item "$stratagus_folder\\..\\dependencies\\dependencies\\*" "$stratagus_folder\\..\\dependencies\\"
}
if (-not (Test-Path "$stratagus_folder\\..\\dependencies\\bin\\ffmpeg.exe")) {
    Invoke-WebRequest https://github.com/Wargus/stratagus/releases/download/2015-30-11/ffmpeg.exe -OutFile "$stratagus_folder\\..\\dependencies\\bin\\ffmpeg.exe"
}
if (-not (Test-Path "$stratagus_folder\\..\\wargus")) {
    git clone https://github.com/Wargus/wargus "$stratagus_folder\\..\\wargus"
}
if (-not (Test-Path "$stratagus_folder\\..\\war1gus")) {
    git clone https://github.com/Wargus/war1gus "$stratagus_folder\\..\\war1gus"
}
