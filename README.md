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

Windows: <a href="https://ci.appveyor.com/project/timfel/stratagus"><img width="100" src="https://ci.appveyor.com/api/projects/status/github/Wargus/stratagus?branch=master&svg=true"></a>

Linux: [![Build Status](https://github.com/Wargus/stratagus/actions/workflows/appimage.yml/badge.svg?branch=master)](https://github.com/Wargus/stratagus/actions/workflows/appimage.yml)

<!---
For Mac OS X, Stratagus comes bundled in the app bundles for Wargus, Stargus, and War1gus:
  - Wargus: https://github.com/Wargus/stratagus/wiki/osx/Wargus.app.tar.gz
  - War1gus: https://github.com/Wargus/stratagus/wiki/osx/War1gus.app.tar.gz
  - Stargus: https://github.com/Wargus/stratagus/wiki/osx/Stargus.app.tar.gz
--->

The latest Linux dev builds come as [AppImage](https://appimage.org/):
  - Go to [our workflow](https://github.com/Wargus/stratagus/actions/workflows/appimage.yml?query=branch%3Amaster+event%3Apush+is%3Asuccess), click the latest run, and scroll down to download the game you want.

For releases on Linux, you might want packages:
  - Ubuntu/Debian, you might want the deb packages:
     - https://launchpad.net/~stratagus/+archive/ubuntu/ppa
     - Note that the game packages you probably want are `wargus`, `war1gus`, and `stargus`
  - At least the OpenSUSE, Fedora, and Arch Linux communities maintain packages as well

For Windows, you likely want the game installers:
  - Wargus: https://github.com/Wargus/wargus/releases
  - War1gus: https://github.com/Wargus/war1gus/releases
  - Stargus: https://github.com/Wargus/stargus/releases

### Contributing

If you want to contribute, there is a video that details how to set up a development environment on Windows using VSCode here: https://youtu.be/c1Zm7tt_QtQ 

Read 'doc/index.html' for general information and license information.
Read 'doc/install.html' for Stratagus installation instructions.
Read 'doc/changelog.html' for the Stratagus changelog.

The Windows builds are done on Appveyor, Linux on Github Actions. The Ubuntu packages
are built on Launchpad:
  - https://code.launchpad.net/~stratagus/+recipe/stratagus-github
  - https://code.launchpad.net/~stratagus/+recipe/war1gus-github
  - https://code.launchpad.net/~stratagus/+recipe/stargus-github
  - https://code.launchpad.net/~stratagus/+recipe/wargus-github
