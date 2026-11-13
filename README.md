    _______________________________________________________________________
         _________ __                 __                               
        /   _____//  |_____________ _/  |______     ____  __ __  ______
        \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
        /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ | 
       /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
               \/                  \/          \//_____/            \/ 
    ______________________                           ______________________
                          T H E   W A R   B E G I N S
           Stratagus - A free fantasy real time strategy game engine

[![Join the chat at https://gitter.im/Wargus](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/Wargus?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge)

[![Discord](https://img.shields.io/discord/780082494447288340?style=flat-square&logo=discord&label=discord)](https://discord.gg/dQGxaw3QfB)

Windows: [![Build Status](https://github.com/Wargus/stratagus/actions/workflows/windows.yml/badge.svg?branch=master)](https://github.com/Wargus/stratagus/actions/workflows/windows.yml)

Linux: [![Build Status](https://github.com/Wargus/stratagus/actions/workflows/ubuntu.yml/badge.svg?branch=master)](https://github.com/Wargus/stratagus/actions/workflows/ubuntu.yml)

macOS: [![Build Status](https://github.com/Wargus/stratagus/actions/workflows/macos.yml/badge.svg?branch=master)](https://github.com/Wargus/stratagus/actions/workflows/macos.yml)

### Downloading

Stratagus is the engine, and players download a game built with it rather than
the engine itself. The games are released together with the engine, under the
same version number, and every one of them publishes a Windows installer, a
macOS `dmg` and a Linux AppImage:

| game | latest stable | latest unstable |
| --- | --- | --- |
| Wargus (Warcraft II) | https://github.com/Wargus/wargus/releases/latest | https://github.com/Wargus/wargus/releases/tag/master-builds |
| War1gus (Warcraft: Orcs & Humans) | https://github.com/Wargus/war1gus/releases/latest | https://github.com/Wargus/war1gus/releases/tag/master-builds |
| Stargus (StarCraft) | https://github.com/Wargus/stargus/releases/latest | https://github.com/Wargus/stargus/releases/tag/master-builds |

The `master-builds` release is updated with every push to `master`, so it is
the newest development build rather than a tested release. See the game
repositories for how each platform is built.

The engine itself publishes only the Windows build of its own binaries
together with the debug symbols that go with them, so that a crash log can be
analysed against the matching engine build:

  - latest stable: https://github.com/Wargus/stratagus/releases/latest
  - latest unstable: https://github.com/Wargus/stratagus/releases/tag/master-builds

Distributions package the engine and the games their own way; the `debian/`
directory of this repository is kept as a starting point for that, but nothing
is published from it any more. See `RELEASING.md` for how a release is made.

### Contributing

If you want to contribute, there is a video that details how to set up a development environment on Windows using VSCode here: https://youtu.be/c1Zm7tt_QtQ 

Read 'doc/index.html' for general information and license information.
Read 'doc/install.html' for Stratagus installation instructions.
Read 'doc/changelog.html' for the Stratagus changelog.

The Windows, macOS and Linux builds are all done on GitHub Actions: the engine
builds are in the workflows of this repository, and the installers, bundles and
AppImages that the games release are built by the workflows of the game
repositories.
