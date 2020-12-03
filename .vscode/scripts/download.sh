#!/bin/bash

# directory names should be uppercase for launch config to work

stratagus_folder="$1"
if [ ! -d "${stratagus_folder}/../Wargus" ]; then
    if [ -d "${stratagus_folder}/../wargus" ]; then
        pushd "${stratagus_folder}/../wargus"
        target="$(pwd)"
        popd
        ln -s "${target}" "${stratagus_folder}/../Wargus"
    else
        git clone https://github.com/Wargus/wargus "${stratagus_folder}/../Wargus"
    fi
fi
if [ ! -d "$stratagus_folder/../War1gus" ]; then
    if [ -d "${stratagus_folder}/../war1gus" ]; then
        pushd "${stratagus_folder}/../war1gus"
        target="$(pwd)"
        popd
        ln -s "${target}" "${stratagus_folder}/../War1gus"
    else
        git clone https://github.com/Wargus/wargus "${stratagus_folder}/../War1gus"
    fi
fi
