from __future__ import annotations

import gzip
import os
import shutil
import subprocess
import time
from dataclasses import dataclass
from pathlib import Path

import pytest

from helpers import write_wargus_preferences, write_war1gus_preferences


@dataclass(frozen=True)
class SavegameCase:
    game: str
    filename: str
    issue: str
    symptom: str
    run_seconds: float = 8.0
    lua_epilogue: str = ""
    results_menu: bool = False
    expected_failure: str = ""


SAVEGAME_CASES = [
    SavegameCase(
        "war1gus",
        "stratagus-601-wc1-crash.sav.gz",
        "Wargus/stratagus#601",
        "destroyed-unit reference assertion while loading/running a WC1 save",
    ),
    SavegameCase(
        "wargus",
        "stratagus-719-10l.sav.gz",
        "Wargus/stratagus#719",
        "intermittent victory-button crash after Siege of Vanguard",
        lua_epilogue="""
local pytest_previous_game_starting = GameStarting
GameStarting = function()
  if pytest_previous_game_starting ~= nil then
    pytest_previous_game_starting()
  end
  StopGame(GameVictory)
end
""",
        results_menu=True,
    ),
    SavegameCase(
        "wargus",
        "wargus-452-game001.sav.gz",
        "Wargus/wargus#452",
        "campaign crash after winning or loading a campaign save",
    ),
    SavegameCase(
        "wargus",
        "wargus-195-crash-to-desktop.sav.gz",
        "Wargus/wargus#195",
        "Assault on Blackrock Spire crash shortly after loading",
    ),
    SavegameCase(
        "wargus",
        "wargus-195-ctd-blackrock-spire-again.sav.gz",
        "Wargus/wargus#195",
        "second Assault on Blackrock Spire crash save",
    ),
    SavegameCase(
        "wargus",
        "wargus-195-h14hard-lothar-alive.sav.gz",
        "Wargus/wargus#195",
        "earlier Assault on Blackrock Spire save from the crashing run",
    ),
    SavegameCase(
        "wargus",
        "wargus-172-buggy-hall-upgrade.sav.gz",
        "Wargus/wargus#172",
        "hall-to-keep upgrade cancellation after save/load",
    ),
    SavegameCase(
        "wargus",
        "wargus-176-buggy-holy-vision.sav.gz",
        "Wargus/wargus#176",
        "Holy Vision targeting behavior after save/load",
    ),
    SavegameCase(
        "wargus",
        "wargus-176-buggy-holy-vision-2.sav.gz",
        "Wargus/wargus#176",
        "second Holy Vision targeting save",
    ),
    SavegameCase(
        "wargus",
        "wargus-178-buggy-transport.sav.gz",
        "Wargus/wargus#178",
        "transport movement/passenger state after save/load",
    ),
    SavegameCase(
        "wargus",
        "wargus-315-game.sav.gz",
        "Wargus/wargus#315",
        "dead-body passability after save/load",
    ),
    SavegameCase(
        "wargus",
        "stratagus-720-11a.sav.gz",
        "Wargus/stratagus#720",
        "Khadgar spell availability after save/load",
        lua_epilogue="""
local pytest_previous_game_starting = GameStarting
GameStarting = function()
  if pytest_previous_game_starting ~= nil then
    pytest_previous_game_starting()
  end
  if GetPlayerData(0, "Allow", "upgrade-blizzard") ~= "R" then
    error("Khadgar save did not restore researched Blizzard")
  end
end
""",
        expected_failure="Khadgar save did not restore researched Blizzard",
    ),
]


FAILURE_MARKERS = (
    "Assertion failed",
    "Segmentation fault",
    "Aborted",
    "bad argument",
    "double free",
    "corruption",
    "invalid pointer",
    "Unsupported old savegame",
    "Unsupported tag",
    "Network out of sync",
    "sent bad command",
)


def _read(path: Path) -> str:
    return path.read_text(errors="replace") if path.exists() else ""


def _assert_no_failure_markers(combined: str) -> None:
    for marker in FAILURE_MARKERS:
        assert marker not in combined


def _copy_fixture_to_user_save(case: SavegameCase, repo_root: Path, user_dir: Path) -> str:
    fixture = repo_root / "pytests" / "fixtures" / "savegames" / case.game / case.filename
    assert fixture.exists(), f"missing savegame fixture {fixture}"
    save_root = user_dir / ("wc1" if case.game == "war1gus" else "wc2") / "save"
    save_root.mkdir(parents=True, exist_ok=True)
    target = save_root / case.filename
    if case.lua_epilogue:
        with gzip.open(fixture, "rb") as src, gzip.open(target, "wb") as dst:
            shutil.copyfileobj(src, dst)
            dst.write(b"\n")
            dst.write(case.lua_epilogue.encode())
    else:
        shutil.copy2(fixture, target)
    return f"~save/{case.filename}"


@pytest.mark.slow
@pytest.mark.parametrize("case", SAVEGAME_CASES, ids=lambda case: f"{case.issue}:{case.filename}")
def test_attached_savegame_loads_and_runs_without_crashing(
    case: SavegameCase,
    request: pytest.FixtureRequest,
    repo_root: Path,
    stratagus_bin: str,
    tmp_path: Path,
):
    data_dir = request.getfixturevalue(
        "extracted_war1gus_data" if case.game == "war1gus" else "extracted_wargus_data"
    )
    user_dir = tmp_path / "user"
    if case.game == "war1gus":
        write_war1gus_preferences(user_dir)
    else:
        write_wargus_preferences(user_dir)
    save_arg = _copy_fixture_to_user_save(case, repo_root, user_dir)

    stdout = tmp_path / "savegame.stdout"
    stderr = tmp_path / "savegame.stderr"
    driver_name = "savegame_results_driver.lua" if case.results_menu else "savegame_driver.lua"
    driver = repo_root / "pytests" / "lua" / driver_name
    command = [
        stratagus_bin,
        "-d",
        str(data_dir),
        "-u",
        str(user_dir),
        "-W",
        "640x480",
        "-v",
        "640x480",
        "-b",
        "-c",
        str(driver),
        "-G",
        save_arg,
    ]

    env = {**os.environ, "SDL_VIDEODRIVER": "dummy", "SDL_AUDIODRIVER": "dummy"}
    with stdout.open("wb") as out, stderr.open("wb") as err:
        proc = subprocess.Popen(command, cwd=repo_root, env=env, stdout=out, stderr=err)
        deadline = time.monotonic() + case.run_seconds
        while time.monotonic() < deadline:
            if proc.poll() is not None:
                break
            time.sleep(0.25)
        if proc.poll() is None:
            proc.kill()
            proc.wait(timeout=5)

    combined = f"{case.issue}: {case.symptom}\n--- stdout ---\n{_read(stdout)}\n--- stderr ---\n{_read(stderr)}"
    _assert_no_failure_markers(combined)
    if case.expected_failure and case.expected_failure in combined:
        pytest.xfail(f"{case.issue}: {case.expected_failure}")
    if case.results_menu:
        assert "PYTEST_SAVEGAME_RESULTS_START" in combined, combined
    assert proc.returncode in (0, -9), combined
