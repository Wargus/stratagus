# Releasing Stratagus

The engine and the games are released in lockstep: a release of version
`X.Y.Z` is tagged `vX.Y.Z` in this repository and in each of the game
repositories, and a game built from a tag builds against the engine tag with the
matching name, falling back to the engine `master` branch when there is no such
tag. `.github/actions/engine-ref` is what works that out.

## Release artifacts

What a release of this repository consists of:

| artifact | built by | published to |
| --- | --- | --- |
| `compiled-binaries.zip`: `stratagus.exe`, `stratagus-midiplayer.exe` and the matching `.pdb` debug symbols | the `windows` workflow | the release for this tag, and the [master-builds release](https://github.com/Wargus/stratagus/releases/tag/master-builds) for every push to `master` |

That is all the engine publishes. The installers and bundles that players
download are built and published by the game repositories, and the games'
release pages are where the releases of a version are found:

| artifact | game repositories |
| --- | --- |
| Windows installer | [Wargus](https://github.com/Wargus/wargus/releases), [War1gus](https://github.com/Wargus/war1gus/releases), [Stargus](https://github.com/Wargus/stargus/releases) |
| macOS `dmg` | the same three |
| Linux AppImage | the same three |

## The games are built from here

A game of Stratagus is a launcher, some scripts and some data, so building one
means building the engine with it. That is what
`.github/workflows/game-build.yml` does: given the name of a game repository, the
name of the game to its players and a platform, it checks out both repositories at
the refs that match, builds the engine for that platform, and then hands over to
`tools/ci-build.sh` in the game, which knows the things a game differs in - the
arguments of its own build, which libraries have to end up inside the bundle, what
its installer is made from - and produces the artifact of that platform. The
workflows in a game repository are therefore its triggers and nothing else.

What an AppImage can do is decided by `game-build.yml`, because the vendored
libraries it is built from take whatever headers they find: the list of packages
that workflow installs on Linux is the list of video and audio backends an AppImage
supports. Nothing more is installed before the engine is built, and a game that
needs other packages installs them itself afterwards, in its `tools/ci-build.sh`.

`.github/workflows/game-engine-check.yml` is the other shared half: the daily look
for a moved engine that `engine.yml` in each game repository runs. Together with
`.github/actions/publish-release`, which uploads a built artifact to the release
its revision belongs to, that is everything the three games used to each have
their own copy of.

Nor does it have to announce its releases to the games: each game repository has
an `engine.yml` workflow that runs daily, compares the engine
`master` with the artifacts of its `master-builds` release and starts that
repository's build workflow when the engine has moved. A change that only
touches the engine is therefore picked up within a day without any repository
needing a token that is valid in another one.

Debian packages used to be built on Launchpad for us, and Ubuntu and Debian
users could install them from a PPA. That has been retired: the artifacts
above are the release artifacts, and packaging for a distribution is something
that distribution does, not us. The `debian/` directory is kept as a starting
point for anyone doing that, and because its `changelog` is the release record
of this repository.

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
   by hand or with `tools/publish_docs.sh /path/to/Wargus.github.io --push`,
   which does the doxygen build, the Lua bindings overview from
   `doc/lua/generate_index.py`, and the commit and push into the website
   repository.

6. Tag and push. The `windows` workflow then builds the engine for the tag,
   creates the release for it, and uploads the binaries and symbols.

   ```sh
   git tag vX.Y.Z
   git push origin master --tags
   ```

   Once the assets are there, write the notes for the release, e.g. with
   `gh release edit vX.Y.Z --verify-tag --notes-file notes.md`.

7. Release the games: follow `RELEASING.md` in each of the game repositories
   and tag them with the same `vX.Y.Z`, so that their Windows installers,
   macOS `dmg`s and Linux AppImages are built against this engine release.

8. Update the download links for the stable versions in the website
   repository.
