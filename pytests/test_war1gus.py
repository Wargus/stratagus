from __future__ import annotations

import socket
import subprocess
import time
from pathlib import Path

import pytest

from helpers import terminate_process, write_war1gus_preferences


def _read(path: Path) -> str:
    return path.read_text(errors="replace") if path.exists() else ""


def _free_udp_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def _launch(cmd: list[str], *, cwd: Path, env: dict[str, str], stdout: Path, stderr: Path) -> subprocess.Popen:
    with stdout.open("wb") as out, stderr.open("wb") as err:
        try:
            return subprocess.Popen(cmd, cwd=cwd, env=env, stdout=out, stderr=err)
        finally:
            if pos := env.get("SDL_VIDEO_WINDOW_POS"):
                env["SDL_VIDEO_WINDOW_POS"] = ",".join(str(int(c) + 640) for c in pos.split(","))


def _wait_for_log(path: Path, needle: str, proc: subprocess.Popen, timeout: float) -> bool:
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if needle in _read(path):
            return True
        if proc.poll() is not None:
            return False
        time.sleep(0.5)
    return False


def _combined_logs(paths: tuple[Path, ...]) -> str:
    return "\n".join(f"--- {path.name} ---\n{_read(path)}" for path in paths)


def _participant_cmd(participant: dict, args: list[str]) -> list[str]:
    return [*participant["argv"], *args]


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_war1gus_elwynn_forest(
    stratagus_player: dict,
    extracted_war1gus_data: Path,
    gui_env,
    tmp_path: Path,
):
    write_war1gus_preferences(tmp_path)
    startup = tmp_path / "start.lua"
    startup.write_text("""
    Load("scripts/stratagus.lua")
    SetTitleScreens({})
    local function log(message)
      if not (os and os.getenv and os.getenv("STRATAGUS_UNBUFFERED_STDIO")) then
        return
      end
      print(message)
      if io and io.stdout then
        io.stdout:flush()
      end
    end
    CustomStartup = function()
      Load("scripts/campaigns.lua")
      race = "human"
      campaign = CreateCampaign(race)
      position = 5
      currentCampaign = campaign
      currentRace = race
      currentState = 5
      RunResultsMenu = function()
        if GameResult == GameVictory then
          log("PYTEST_WAR1_ELWYNN_WON")
        end
        return
      end
      for i=6,14,1 do
        campaign.steps[i] = function() end
      end
      Briefing = function(title, objs, bgImg, mapbg, mapVideo, text, voices) end
      RunCampaignSubmenu = function(race) end
      AddTrigger(
        function() return GameCycle > 25 end,
        function()
          log("PYTEST_WAR1_ELWYNN_LOADED")
          CreateUnit("unit-knight", 0, {12, 15})
          CreateUnit("unit-knight", 0, {20, 15})
          CreateUnit("unit-knight", 0, {12, 11})
          CreateUnit("unit-knight", 0, {16, 15})
          CreateUnit("unit-knight", 0, {16, 12})
          CreateUnit("unit-knight", 0, {23, 12})
          CreateUnit("unit-knight", 0, {20, 12})
          CreateUnit("unit-knight", 0, {11, 19})
          CreateUnit("unit-knight", 0, {14, 19})
          CreateUnit("unit-knight", 0, {17, 19})
          CreateUnit("unit-knight", 0, {20, 19})
          CreateUnit("unit-knight", 0, {34, 29})
          CreateUnit("unit-knight", 0, {22, 18})
          CreateUnit("unit-knight", 0, {31, 26})
          CreateUnit("unit-knight", 0, {33, 11})
          CreateUnit("unit-knight", 0, {16, 18})
          CreateUnit("unit-knight", 0, {34, 25})
          CreateUnit("unit-knight", 0, {35, 26})
          CreateUnit("unit-knight", 0, {32, 25})
          CreateUnit("unit-knight", 0, {34, 27})
          CreateUnit("unit-knight", 0, {35, 28})
          CreateUnit("unit-knight", 0, {9, 6})
          CreateUnit("unit-knight", 0, {19, 18})
          CreateUnit("unit-knight", 0, {19, 14})
          CreateUnit("unit-knight", 0, {23, 14})
          CreateUnit("unit-knight", 0, {33, 26})
          CreateUnit("unit-knight", 0, {17, 27})
          CreateUnit("unit-knight", 0, {18, 22})
          CreateUnit("unit-knight", 0, {9, 21})
          CreateUnit("unit-knight", 0, {7, 7})
          CreateUnit("unit-knight", 0, {22, 11})
          CreateUnit("unit-knight", 0, {28, 21})
          CreateUnit("unit-knight", 0, {31, 19})
          CreateUnit("unit-knight", 0, {9, 14})
          CreateUnit("unit-knight", 0, {11, 14})
          CreateUnit("unit-knight", 0, {15, 14})
        end)
      RunCampaign(campaign)
    end
    """)
    test_env = dict(gui_env)
    test_env["STRATAGUS_UNBUFFERED_STDIO"] = "1"

    host_out = tmp_path / ".stdout"
    host_err = tmp_path / ".stderr"
    common = [
        "-d",
        str(extracted_war1gus_data),
        "-W",
        "640x480",
        "-v",
        "640x480",
        "-g",
    ]
    host_cmd = _participant_cmd(
        stratagus_player,
        [
            *common,
            "-b",
            "-u",
            str(tmp_path),
            "-c",
            str(startup),
        ],
    )

    # War1gus still has legacy map-loading paths during network start that may
    # retry the raw relative map name. Run from the composed data tree so those
    # fallbacks resolve to the same files as the -d path.
    host = _launch(host_cmd, cwd=extracted_war1gus_data, env=test_env, stdout=host_out, stderr=host_err)
    logs = (host_out, host_err)
    try:
        host.wait(120)
    finally:
        terminate_process(host)

    combined = _combined_logs(logs)
    assert "PYTEST_WAR1_ELWYNN_LOADED" in _read(host_out), _combined_logs((host_out, host_err))
    assert "PYTEST_WAR1_ELWYNN_WON" in _read(host_out), _combined_logs((host_out, host_err))
    for marker in (
        "Network out of sync",
        "sent bad command",
        "Unknown unitType",
        "PYTEST_WAR1_JOINING_MAP_SETTINGS_ERROR",
        "Segmentation fault",
        "Aborted",
    ):
        assert marker not in combined
