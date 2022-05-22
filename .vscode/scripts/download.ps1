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
if (-not (Test-Path "$stratagus_folder\\..\\dependencies\\bin\\magick.exe")) {
    Invoke-WebRequest https://github.com/Wargus/stratagus/releases/download/2015-30-11/magick.exe -OutFile "$stratagus_folder\\..\\dependencies\\bin\\magick.exe"
}

if (-not (Test-Path "$stratagus_folder\\.vscode\\settings.json")) {
    Copy-Item "$stratagus_folder\\.vscode\\settings.windows.json" "$stratagus_folder\\.vscode\\settings.json"
}

if (-not (Test-Path "$stratagus_folder\\..\\wargus")) {
    git clone https://github.com/Wargus/wargus "$stratagus_folder\\..\\wargus"
}
if (-not (Test-Path "$stratagus_folder\\..\\wargus\\.vscode")) {
    mkdir "$stratagus_folder\\..\\wargus\\.vscode"
}
if (-not (Test-Path "$stratagus_folder\\..\\wargus\\.vscode\\settings.json")) {
    Copy-Item "$stratagus_folder\\.vscode\\settings.windows.wargus.json" "$stratagus_folder\\..\\wargus\\.vscode\\settings.json"
}
if (-not (Test-Path "$stratagus_folder\\..\\wargus\\.vscode\\launch.json")) {
    Copy-Item "$stratagus_folder\\.vscode\\launch.wargus.json" "$stratagus_folder\\..\\wargus\\.vscode\\launch.json"
}

if (-not (Test-Path "$stratagus_folder\\..\\war1gus")) {
    git clone https://github.com/Wargus/war1gus "$stratagus_folder\\..\\war1gus"
}
if (-not (Test-Path "$stratagus_folder\\..\\war1gus\\.vscode")) {
    mkdir "$stratagus_folder\\..\\war1gus\\.vscode"
}
if (-not (Test-Path "$stratagus_folder\\..\\war1gus\\.vscode\\settings.json")) {
    Copy-Item "$stratagus_folder\\.vscode\\settings.windows.war1gus.json" "$stratagus_folder\\..\\war1gus\\.vscode\\settings.json"
}
if (-not (Test-Path "$stratagus_folder\\..\\war1gus\\.vscode\\launch.json")) {
    Copy-Item "$stratagus_folder\\.vscode\\launch.war1gus.json" "$stratagus_folder\\..\\war1gus\\.vscode\\launch.json"
}

if (-not (Test-Path "$stratagus_folder\\..\\stargus")) {
    git clone https://github.com/Wargus/stargus "$stratagus_folder\\..\\stargus"
}
if (-not (Test-Path "$stratagus_folder\\..\\stargus\\.vscode")) {
    mkdir "$stratagus_folder\\..\\stargus\\.vscode"
}
if (-not (Test-Path "$stratagus_folder\\..\\stargus\\.vscode\\settings.json")) {
    Copy-Item "$stratagus_folder\\.vscode\\settings.windows.stargus.json" "$stratagus_folder\\..\\stargus\\.vscode\\settings.json"
}
if (-not (Test-Path "$stratagus_folder\\..\\stargus\\.vscode\\launch.json")) {
    Copy-Item "$stratagus_folder\\.vscode\\launch.stargus.json" "$stratagus_folder\\..\\stargus\\.vscode\\launch.json"
}
