# Releasing Stratagus

The engine and the games are released in lockstep: a release of version `X.Y.Z`
is tagged `vX.Y.Z` in this repository and in each of the game repositories, and
a game built from a tag builds against the engine tag with the matching name,
falling back to the engine `master`.

## Release artifacts

The installers and bundles that players download are built and published by the
game repositories, and the games' release pages are where the releases of a
version are found:

|Artifact         |Game repositories                                                                                                                                                |
|-----------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------|
|Windows installer|[Wargus](https://github.com/Wargus/wargus/releases), [War1gus](https://github.com/Wargus/war1gus/releases), [Stargus](https://github.com/Wargus/stargus/releases)|
|macOS `dmg`      |the same three                                                                                                                                                   |
|Linux AppImage   |the same three                                                                                                                                                   |

## The engine's build logic is here

A game of Stratagus is a launcher, some scripts and some data, so building one
means building the engine with it. That is what
`.github/workflows/game-build.yml` does: given the name of a game repository,
the name of the game to its players and a platform, it checks out both
repositories at the refs that match, builds the engine for that platform, and
then hands over to `tools/ci-build.sh` in the game, which knows the things a
game differs in - the arguments of its own build, which libraries have to end
up inside the bundle, what its installer is made from - and produces the
artifact of that platform.

`.github/workflows/game-engine-check.yml` is a daily check that the games use
to rebuild their assets if the engine changed without a push to the games'
repositories.

We used to publish Debian packages on Launchpad, but that has been retired. The
`debian/` directory is kept for downstream packagers. At times there have been
such downstream packagers building on openSUSE's build farm and preparing Arch
packages.

## Tagging a release

`X.Y.Z` below is the version being released, and `vX.Y.Z` the tag.

1. Increase the version in `CMakeLists.txt`: `STRATAGUS_MAJOR_VERSION`,
   `STRATAGUS_MINOR_VERSION` and `STRATAGUS_PATCH_LEVEL` (and
   `STRATAGUS_PATCH_LEVEL2`, which only has to be reset when the others move).

2. Write the release notes in `doc/changelog.html`, which is the changelog the
   engine shows on the website and in the packaged documentation.

3. Add the changelog entry for the new version with
   `dch -v X.Y.Z` (or `gbp dch` to generate it from the commits since the last
   release), close it with `dch -r`, and update the years in `debian/copyright`
   if the notice at the bottom of `doc/changelog.html` needs to move with them.

4. Commit the result: `git commit -a -m "Release version X.Y.Z"`.

5. Generate the documentation and publish it to the website repository, either
   by hand or with `tools/publish_docs.sh /path/to/wargus.github.io --push`,
   which does the doxygen build, the Lua bindings overview from
   `doc/lua/generate_index.py`, and the commit and push into the website
   repository.

6. Tag and push.

7. Release the games: follow `RELEASING.md` in each of the game repositories
   and tag them with the same `vX.Y.Z`, so that their packages are built
   against this engine release.

8. Update the download links for the stable versions in the website repository.
