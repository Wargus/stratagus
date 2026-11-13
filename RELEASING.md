# Releasing Stratagus

The engine and the games are released in lockstep: a release of version
`X.Y.Z` is tagged `vX.Y.Z` in this repository and in each of the game
repositories, and the game workflows build against the engine tag with the
matching name, falling back to the engine `master` branch when there is no
such tag. See the *Resolve which Stratagus to build against* step of a game
workflow for the details of that lookup.

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

The engine does not build any of those itself, and it does not have to announce
them either: each game repository has an `engine.yml` workflow that runs daily,
compares the engine `master` with the artifacts of its `master-builds` release
and starts the three build workflows of that repository when the engine has
moved. A change that only touches the engine is therefore picked up within a day
without any repository needing a token that is valid in another one.

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
