# Pytest Multiplayer Harness

These tests add optional coverage around the Wargus/War1gus multiplayer menu fixes.

Fast tests:

```sh
python3 -m pytest pytests/test_lua_network_menu.py
```

GUI tests can be opted out because they require extra tools and extracted game data:

```sh
WARGUS_GUI_TESTS=0 python3 -m pytest pytests/test_wargus_gui.py
```

Set `PYTEST_SDL_VIDEODRIVER=dummy` or `PYTEST_SDL_VIDEODRIVER=xvfb` for a
headless run, otherwise it'll open instances on the real screen if there is
one.

The process-level multiplayer smoke test uses the same Xvfb and extracted-data
fixtures, launches one Wargus host and one client, and waits for the lobby to
autostart a real two-player game:

```sh
python3 -m pytest pytests/test_wargus_multiplayer_process.py
```

The free Timeless Tales data tree can also be exercised without proprietary
media. The process suite covers menu startup, single-player startup, a For the
Motherland map, editor startup, localhost multiplayer sync-stress, synchronized
multiplayer settings, one network-human-plus-AI map, and map PNG preview integrity.
By default the fixture uses `games/timeless-tales`; set `TIMELESS_TALES_DATA_DIR`
to point at another checkout or installed data tree:

```sh
WARGUS_GUI_TESTS=1 \
STRATAGUS_BIN=/path/to/stratagus-dbg \
python3 -m pytest pytests/test_timeless_tales_process.py
```

Wargus and War1gus process tests can exercise every host/client pairing from multiple
Stratagus builds. `auto` discovers the native debug build, native release build,
and the aarch64 build under qemu when those paths exist:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_VENDORED_LUA=ON -DBUILD_VENDORED_SDL=ON \
  -DBUILD_VENDORED_MEDIA_LIBS=ON -DBUILD_TESTING=0
cmake --build build-release --target stratagus

cmake -S . -B build-aarch64 -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/linux-aarch64-gnu.cmake \
  -DBUILD_VENDORED_LUA=ON -DBUILD_VENDORED_SDL=ON \
  -DBUILD_VENDORED_MEDIA_LIBS=ON -DBUILD_TESTING=0 \
  -DSTRATAGUS_HOST_TOLUAPP=$PWD/build/lua/src/lua-build/toluapp
cmake --build build-aarch64 --target stratagus

WARGUS_GUI_TESTS=1 PYTEST_SDL_VIDEODRIVER=dummy \
STRATAGUS_PARTICIPANTS=auto \
python3 -m pytest \
  pytests/test_wargus_multiplayer_process.py \
  pytests/test_war1gus_multiplayer_process.py \
  pytests/test_timeless_tales_process.py
```

An optional Windows build can be produced with MinGW:

```sh
cmake -S . -B build-mingw64 -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/windows-mingw64.cmake \
  -DBUILD_VENDORED_LUA=ON -DBUILD_VENDORED_SDL=ON \
  -DBUILD_VENDORED_MEDIA_LIBS=ON -DBUILD_TESTING=0 \
  -DENABLE_STATIC=ON -DENABLE_STDIO_REDIRECT=OFF \
  -DSTRATAGUS_HOST_TOLUAPP=$PWD/build/lua/src/lua-build/toluapp
cmake --build build-mingw64 --target stratagus
```

If Wine is working locally, add `STRATAGUS_INCLUDE_WINE=1` to the
`STRATAGUS_PARTICIPANTS=auto` pytest invocation to include
`build-mingw64/stratagus.exe` in the host/client matrix. The harness initializes
an isolated 64-bit Wine prefix under `build-mingw64/wine64-prefix`.

For custom binaries, set `STRATAGUS_PARTICIPANTS` to JSON, for example
`[{"name":"dbg","argv":["build/stratagus-dbg"]},{"name":"arm","argv":["qemu-aarch64","-L","/usr/aarch64-linux-gnu","build-aarch64/stratagus"]}]`.

If `WARGUS_DATA_DIR` is not set, the fixtures look for `games/WAR2BNECD.ISO`,
unpack it with `7z`, build `wartool`, and extract data into
`games/.pytest-data/wargus-data`. The cached data is then composed with the
same source-side scripts, maps, shaders, campaigns, and contrib assets that the
Wargus launcher/package installs. War1gus helpers do the same with
`games/WAR1.ISO` and `war1tool` for tests that request the
`extracted_war1gus_data` fixture. Timeless Tales is a free data tree and is not
extracted from an ISO; the `timeless_tales_data` fixture only verifies the data
directory contains the expected assets and scripts.

System setup on Debian/Ubuntu:

```sh
sudo apt update
sudo apt install \
  binutils-aarch64-linux-gnu cmake g++-aarch64-linux-gnu \
  g++-mingw-w64-x86-64 gcc-aarch64-linux-gnu gcc-mingw-w64-x86-64 \
  libx11-dev libxcursor-dev libxext-dev libxi-dev libxrandr-dev libxss-dev \
  make mingw-w64 ninja-build p7zip-full pkg-config \
  python3-pip python3-venv qemu-user qemu-user-static \
  wine wine32 wine64 x11-utils xauth xvfb
```

The X11 development packages are needed when rebuilding vendored SDL with X11
support. `p7zip-full` provides `7z` for unpacking the game ISOs, `qemu-user`
runs the aarch64 binary, and the Wine/MinGW packages build and run the Windows
participant.

Python setup:

```sh
python3 -m venv .venv
.venv/bin/python -m pip install -U pip pytest pyautogui python-xlib
```

The GUI harness uses `pytests/lua/wargus_driver.lua` as a startup hook. It loads the normal Wargus scripts, then jumps directly to a target menu so tests do not need to click through the full title/start menu path.
