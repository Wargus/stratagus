#!/bin/bash
stratagus_folder="$1"

if [ ! -e "${stratagus_folder}/.vscode/settings.json" ]; then
    cp "${stratagus_folder}/.vscode/settings.linux.json" "${stratagus_folder}/.vscode/settings.json"
fi

if [ ! -d "${stratagus_folder}/../wargus" ]; then
    git clone https://github.com/Wargus/wargus "${stratagus_folder}/../wargus"
fi
if [ ! -d "${stratagus_folder}/../wargus/.vscode" ]; then
    mkdir "${stratagus_folder}/../wargus/.vscode"
fi
if [ ! -e "${stratagus_folder}/../wargus/.vscode/settings.json" ]; then
    cp "${stratagus_folder}/.vscode/settings.linux.wargus.json" "${stratagus_folder}/../wargus/.vscode/settings.json"
fi
if [ ! -e "${stratagus_folder}/../wargus/.vscode/launch.json" ]; then
    cp "${stratagus_folder}/.vscode/launch.wargus.json" "${stratagus_folder}/../wargus/.vscode/launch.json"
fi

if [ ! -d "${stratagus_folder}/../war1gus" ]; then
    git clone https://github.com/Wargus/war1gus "${stratagus_folder}/../war1gus"
fi
if [ ! -d "${stratagus_folder}/../war1gus/.vscode" ]; then
    mkdir "${stratagus_folder}/../war1gus/.vscode"
fi
if [ ! -e "${stratagus_folder}/../war1gus/.vscode/settings.json" ]; then
    cp "${stratagus_folder}/.vscode/settings.linux.war1gus.json" "${stratagus_folder}/../war1gus/.vscode/settings.json"
fi
if [ ! -e "${stratagus_folder}/../war1gus/.vscode/launch.json" ]; then
    cp "${stratagus_folder}/.vscode/launch.war1gus.json" "${stratagus_folder}/../war1gus/.vscode/launch.json"
fi

if [ ! -d "${stratagus_folder}/../stargus" ]; then
    git clone https://github.com/Wargus/stargus "${stratagus_folder}/../stargus"
fi
if [ ! -d "${stratagus_folder}/../stargus/.vscode" ]; then
    mkdir "${stratagus_folder}/../stargus/.vscode"
fi
if [ ! -e "${stratagus_folder}/../stargus/.vscode/settings.json" ]; then
    cp "${stratagus_folder}/.vscode/settings.linux.stargus.json" "${stratagus_folder}/../stargus/.vscode/settings.json"
fi
if [ ! -e "${stratagus_folder}/../stargus/.vscode/launch.json" ]; then
    cp "${stratagus_folder}/.vscode/launch.stargus.json" "${stratagus_folder}/../stargus/.vscode/launch.json"
fi
