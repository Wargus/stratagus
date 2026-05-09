from __future__ import annotations

import re
import subprocess
from pathlib import Path


GAME_MENUS = [
    p for p in (
        Path("games/wargus/scripts/menus/network.lua"),
        Path("games/war1gus/scripts/menus/network.lua"),
    ) if p.exists()
]


def test_network_menu_lua_syntax(repo_root: Path, lua51: str):
    for relpath in GAME_MENUS:
        script = repo_root / relpath
        result = subprocess.run(
            [lua51, "-e", f'assert(loadfile("{script}"))'],
            cwd=repo_root,
            text=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            check=False,
        )
        assert result.returncode == 0, result.stderr


def test_client_race_callback_uses_dropdown_argument(repo_root: Path):
    for relpath in GAME_MENUS:
        text = (repo_root / relpath).read_text()
        assert "race:getSelected()" not in text
        assert re.search(r"LocalSetupState\.Race\[Hosts\[NetLocalHostsSlot\]\.PlyNr\]\s*=\s*dd:getSelected\(\)\s*-\s*1", text)


def test_local_server_ip_parsing_is_guarded(repo_root: Path):
    for relpath in GAME_MENUS:
        text = (repo_root / relpath).read_text()
        assert "if ip == nil then" in text
        assert "NetworkSetupServerAddress(ip)" in text


def test_network_menu_command_line_autostart_paths_are_valid(repo_root: Path):
    for relpath in GAME_MENUS:
        text = (repo_root / relpath).read_text()
        assert "startFunc()" not in text
        assert "menu.button_start_game.callback()" in text
        assert "menu.option_fow.callback(menu.option_fow)" in text


def test_network_join_menu_forwards_command_line_ready_options(repo_root: Path):
    for relpath in GAME_MENUS:
        text = (repo_root / relpath).read_text()
        assert "function RunJoiningGameMenu(optRace, optReady)" in text
        assert "RunJoiningMapMenu(optRace, optReady)" in text
