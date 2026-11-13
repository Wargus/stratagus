---
name: update-vendored-dependencies
description: Bump the upstream libraries vendored in ../dependencies and roll the change out to the stratagus engine and the wargus, war1gus and stargus games. Use when asked to update/inline third-party deps, fix a vendored build failure (CMake 4, new compilers), refresh update.sh refs, migrate code to a new upstream API, or open the coordinated draft PRs. Always draft PRs, never request reviewers.
---

# Updating vendored dependencies

## Repositories

Make sure that you have the relevant repositories as siblings of this repo:

|dir                                    |remote               |role                                                                                            |
|---------------------------------------|---------------------|------------------------------------------------------------------------------------------------|
|`.` (`stratagus`)                      |`Wargus/stratagus`   |engine; `third-party` submodule → dependencies, games get their engine headers from here        |
|`../dependencies`                      |`Wargus/dependencies`|upstream sources **inlined** (no submodules inside it) + `CMakeLists.txt` of `vendored_*` macros|
|`../wargus`, `../war1gus`, `../stargus`|`Wargus/<name>`      |games; each also has a `third-party` submodule → dependencies                                   |

`stratagus/CMakeLists.txt` does `add_subdirectory(third-party ...)` and calls `vendored_lua()`,
`vendored_sdl()`, `vendored_sdl_image()`, `vendored_bzip2()`, `vendored_oggvorbis()`,
`vendored_theora()`, `vendored_guisan()`, `vendored_stormlib()`, … gated by `BUILD_VENDORED_LUA` / `BUILD_VENDORED_SDL` /
`BUILD_VENDORED_MEDIA_LIBS`. The library *source* and its *build glue* live in different
repos but change together.

Two kinds of deps (see `../dependencies/README.md`): **updated** ones are tracked in
`update.sh`; **frozen** ones (e.g. `lua-5.1.5`) sit in a version-named dir with no upstream —
do not try to bump those.

## PR rules — non negotiable

- **Every PR is a draft, always.** `gh pr create --draft`. Never `gh pr ready`, never
  auto-merge, never merge, never close someone else's PR.
- **Never request reviewers, assignees or labels.** The human reviews and merges by hand.
- Title: one short imperative line (`Update dependencies`, `Pick up updated dependencies`).
- Body: 2–6 short lines. The human writes the final description — your job is to hand them
  the facts, not prose. Either "Version bumps only, no API or build changes", or the handful
  of points a reviewer actually needs: dependency whose **API changed** (and which repo
  adapts to it), a dependency **dropped/added** and why, a **build/link decision**
  (e.g. one shared Vorbis instead of two static copies), anything you **know** is untested.
- Reviewer time is the scarcest resource here. One screen of reading per PR, maximum. No
  tables, no changelog dumps, no "Summary"/"Test plan" headings, no restating commit
  messages. If you cannot say it in a sentence, it does not belong in the body.
- Escalate instead of guessing: if you are unsure whether a behaviour change is safe,
  put the one-line doubt in the body and let the human decide during their first review.

## 1. Find what moved

```bash
cd ../dependencies
sed -n '/^REPOS=/,/^)/p' update.sh           # repo, ref, files to strip
git ls-remote --tags --refs <upstream-url> | tail      # or: gh api repos/<o>/<r>/tags
gh run list --repo Wargus/stratagus --limit 10         # is CI red? which job (ubuntu/windows/macos/AppImage)?
gh run view <id> --log-failed | grep -i "CMake Error\|error:\|Compatibility with CMake"
```

Decide the target ref per dep. Prefer tags; a raw SHA works via update.sh's clone+checkout
fallback. Do not jump a major version (SDL2 → SDL3) unless asked.

## 2. Branch once, everywhere

Same branch name in all five repos so the bump is one review per repo (`modernize-deps` was
used for the last run):

```bash
BR=modernize-deps
for r in ../dependencies . ../wargus ../war1gus ../stargus; do
  git -C $r fetch -q origin && git -C $r switch -c $BR origin/master
done
```

## 3. Re-inline one dependency at a time

`update.sh` re-clones **every** listed repo, deletes the dirs and commits each one — never run
it blind. Run one entry per attempt, and commit:

```bash
cp update.sh /tmp/update-one.sh
vi /tmp/update-one.sh            # keep one REPOS= 3-tuple, keep the loop below it
bash /tmp/update-one.sh
git -C ../dependencies log --oneline -1   # "Inline <url> at <ref>" is the convention
```

If the deps repo already carries a patch for that lib (`bzip2.patch`, `guichan.patch`),
update.sh applies it and prints rejects. Fix the code, re-diff the patch against the *new*
upstream so the next run still applies, and commit the patch refresh separately
(`Modify guichan.patch: …`).

Then keep the tree reproducible:

- Strip new junk via the entry's third field (`del_pattern`) — whatever you deleted by hand
  must be listed there, or the next run produces a different tree.
- If upstream grew/shrank, adjust that lib's `vendored_*` macro in
  `../dependencies/CMakeLists.txt` (file lists, `SOURCE_DIR`, cache flags). Adding a lib = new
  macro there **plus** a call in `stratagus/CMakeLists.txt` (and in a game's CMakeLists if the
  game links it directly, like StormLib).

## 4. Verify locally before pushing anything

Point the engine's submodule worktree at the local deps commits (this leaves the submodule
dirty; the *pinned gitlink* stays untouched):

```bash
git -C third-party fetch -q ../dependencies && git -C third-party checkout -q FETCH_HEAD -- .
cmake -S . -B /tmp/sgb -DCMAKE_BUILD_TYPE=Release -DBUILD_VENDORED_LUA=ON \
  -DBUILD_VENDORED_SDL=ON -DBUILD_VENDORED_MEDIA_LIBS=ON -DBUILD_TESTING=1 \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5            # same flags as .github/workflows/ubuntu.yml
cmake --build /tmp/sgb --config Release
ctest --test-dir /tmp/sgb --output-on-failure
```

Use a **fresh** build dir per checked dep: the deps are `ExternalProject`s with in-tree
`SOURCE_DIR`, so stamp files in an old build dir hide changed sources. `ctest` covers the
doctest suite; `pytests` needs `python3-pytest` + `xvfb` as in `ubuntu.yml`.
Commit in `../dependencies` only once the engine configures, builds and tests clean — amend
rather than layering "fix build" commits.

## 5. Migrate the code, in dependency order

dependencies → stratagus → wargus / war1gus / stargus. One commit per concern, subject
≤ 72 chars, body after a blank line saying what a reviewer must know.

Typical work when a dep actually changes shape:

- **API changes**: adapt the call sites (mdns discovery lives in
  `src/network/mdns_wrapper.cpp`).
- **Dropped libs**: e.g. libmng/lcms were dropped because upstream SDL_image does animated
  images natively — engine side that meant `src/video/animation.cpp` replacing
  `src/video/mng.cpp`, `src/include/video.h`, `cmake/modules/FindMNG.cmake` deleted, and the
  tolua `.pkg` files (`src/tolua/video.pkg`) since the games compile against the generated
  Lua bindings. **Any binding signature or class rename can break wargus/war1gus/stargus Lua**
  (`../wargus/scripts`, `../war1gus/scripts`, `../stargus/scripts`) — grep the games for the
  renamed symbols before declaring it done.
- **Extraction tools**: the same format change reaches the data tools
  (`stargus/src/startool.cpp`, `Smacker.cpp`, `PortraitsConverter.cpp`) — pick a format the
  *vendored* codecs actually still support (the vendored SDL_image has libwebp/libjxl/
  libtiff/libavif stripped by `del_pattern`, so animated GIF, not WebP).
- **Games**: commit the new `third-party` gitlink (`Use the shared dependencies revision …`),
  drop now-redundant in-repo copies/submodules, touch their CMake/workflows if they used the
  old lib.

## 6. Push in order, check CI, open drafts

Push dependencies **first**: CI does `submodules: recursive`, so every gitlink you reference
from another repo must be reachable on a pushed branch of `Wargus/dependencies`.

```bash
SHA=$(git -C ../dependencies rev-parse HEAD)
git -C ../dependencies push -q origin HEAD:$BR
for r in . ../wargus ../war1gus ../stargus; do
  git -C $r/third-party fetch -q origin && git -C $r/third-party checkout -q $SHA
  git -C $r add third-party
  git -C $r commit -m "Use the shared dependencies revision" -m "<one line a reviewer needs>"
  git -C $r push -q origin HEAD:$BR
done
```

Then one draft PR per repo and watch the checks (never add reviewers):

```bash
gh pr create --draft --base master --title "Pick up updated dependencies" \
  --body "Bumps SDL/SDL_image/SDL_mixer/mdns/StormLib. mdns changed its API and MNG is gone in favour of SDL_image animations — engine and Stargus adapt to both."
gh pr checks --watch
gh run list --branch $BR --limit 10
gh run view <id> --log-failed | tail -50
```

Keep the dependencies PR a draft too, but say in its body which downstream drafts depend on
it — merging order is dependencies → stratagus → games, and that is the human's call.

## Gotchas

- `update.sh --branch <ref>` only works for tags/branches; SHAs fall back to full clone.
- Game CIs build the **engine from master** (`stargus/.github/workflows/cmake.yml` downloads
  `Wargus/stratagus/archive/master.zip`; `macos.yml`s check out `Wargus/stratagus`), so a green
  game PR proves nothing about an unmerged engine branch — say so in the body if a game change
  depends on engine work.
- The game workflows listen for `repository_dispatch: [engine-update]` and run nightly, but the
  stratagus workflow that would send that dispatch does not exist yet.
- A clean Ninja build of stratagus can fail on the vendored Lua IMPORTED file (it works with
  make, like CI, or once the artifact exists) — don't chase it as a regression.
- Local CMake ≥ 4 needs `CMAKE_POLICY_VERSION_MINIMUM=3.5` for the older inlined libs; the
  point of a bump is usually to make that unnecessary again.
- Leave other people's work in the submodule worktree alone: check `git -C third-party status`
  before `checkout -q FETCH_HEAD -- .`, which overwrites it.
