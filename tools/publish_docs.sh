#!/bin/bash
#
# Build the source code documentation and the Lua bindings overview, and put
# them where the website repository wants them.
#
# The engine does not ship documentation with its release artifacts: the
# generated HTML lives in the website repository (Wargus/Wargus.github.io,
# served as stratagus.com), which expects the doxygen output in doxygen/ and
# the Lua bindings overview as lua_bindings.html.
#
# Usage:  tools/publish_docs.sh [/path/to/website/checkout] [--push]
#
# The website checkout defaults to ../Wargus.github.io. Without --push the
# files are only written and committed changes are left to you, so the script
# can be used to look at the result first.
#
# Doxygen needs to be able to run the doc target of a real build, so the build
# directory this configures (build-docs/, override with DOCS_BUILD_DIR) pulls
# in the vendored dependencies and takes a few minutes on a cold cache.

set -ex

SRC=$(cd "$(dirname "$0")/.." && pwd)
SITE=${1:-$SRC/../Wargus.github.io}
PUSH=
for arg in "$@"; do
    if [ "$arg" = "--push" ]; then
        PUSH=yes
    fi
done
BUILD=${DOCS_BUILD_DIR:-$SRC/build-docs}

if [ ! -d "$SITE/.git" ]; then
    echo "$SITE is not a git checkout of the website repository." >&2
    echo "git clone https://github.com/Wargus/Wargus.github.io $SITE" >&2
    exit 1
fi

DOXYGEN=$(command -v doxygen || true)
if [ -z "$DOXYGEN" ]; then
    echo "doxygen is not installed, and it is needed to generate the API docs." >&2
    exit 1
fi

cmake -S "$SRC" -B "$BUILD" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_VENDORED_LUA=ON \
    -DBUILD_VENDORED_SDL=ON \
    -DBUILD_VENDORED_MEDIA_LIBS=ON \
    -DENABLE_DOC=ON \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build "$BUILD" --target doc

# generate_index.py writes index.html next to the current directory and finds
# the sources it documents relative to its own location.
rm -rf "$BUILD/lua-bindings"
mkdir -p "$BUILD/lua-bindings"
(
    cd "$BUILD/lua-bindings"
    python3 "$SRC/doc/lua/generate_index.py"
)

rm -rf "$SITE/doxygen"
cp -a "$BUILD/doxygen" "$SITE/doxygen"
cp "$BUILD/lua-bindings/index.html" "$SITE/lua_bindings.html"

git -C "$SITE" add --all doxygen lua_bindings.html
git -C "$SITE" status --short

if [ -n "$PUSH" ]; then
    git -C "$SITE" commit -m "Update the engine documentation"
    git -C "$SITE" push
else
    echo
    echo "Documentation copied into $SITE and staged. Check it over, then"
    echo "commit and push it, or run this script again with --push."
fi
