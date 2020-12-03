#!/bin/bash
stratagus_folder="$1"
if [ ! -d "${stratagus_folder}/../wargus" ]; then
    git clone https://github.com/Wargus/wargus "${stratagus_folder}/../wargus"
fi
if [ ! -d "$stratagus_folder/../war1gus" ]; then
    git clone https://github.com/Wargus/wargus "${stratagus_folder}/../war1gus"
fi
