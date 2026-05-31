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


def write_war1gus_campaign_start(startup, race, i, extra=""):
    startup.write_text(f"""
    Load("scripts/stratagus.lua")
    SetTitleScreens({{}})
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
      race = "{race}"
      campaign = CreateCampaign(race)
      position = {i}
      currentCampaign = campaign
      currentRace = race
      currentState = {i}
      RunResultsMenu = function()
        if GameResult == GameVictory then
          log("PYTEST_WAR1_WON")
        end
        return
      end
      for i={i + 1},14,1 do
        campaign.steps[i] = function() end
      end
      Briefing = function(title, objs, bgImg, mapbg, mapVideo, text, voices) end
      RunCampaignSubmenu = function(race) end
      AddTrigger(
        function() return GameCycle > 25 end,
        function()
          log("PYTEST_WAR1_LOADED")
          for x=3,63,4 do
            for y=3,63,4 do
              CreateUnit("unit-knight", 0, {{x, y}})
            end
          end
        end)
      {extra}
      RunCampaign(campaign)
    end
    """)


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
@pytest.mark.parametrize("sets", (("orc", 2), ("orc", 3), ("human", 5), ("human", 6), ("orc", 5), ("orc", 6)), ids=["orc2", "orc3", "elwynn", "northshire-abbey", "redridge-mountains", "sunnyglade"])
def test_war1gus_campaign_maps(
    stratagus_player: dict,
    extracted_war1gus_data: Path,
    sets,
    gui_env,
    tmp_path: Path,
):
    write_war1gus_preferences(tmp_path)
    startup = tmp_path / "start.lua"

    extra = ""
    if sets == ("orc", 6):
        # the human tower must survive in this mission
        extra = """
        AddTrigger(
          function() return GameCycle >= 1 end,
          function()
            for i,unit in ipairs(GetUnits(1)) do
              local ident = GetUnitVariable(unit, "Ident")
              if ident == "unit-human-tower" then
                local tower = unit
                AddTrigger(function()
                    SetUnitVariable(tower, "HitPoints", 3000)
                  end,
                  function() return true end)
              end
            end
          end)
        """

    write_war1gus_campaign_start(startup, sets[0], sets[1], extra)
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
    assert "PYTEST_WAR1_LOADED" in _read(host_out), _combined_logs((host_out, host_err))
    assert "PYTEST_WAR1_WON" in _read(host_out), _combined_logs((host_out, host_err))
    for marker in (
        "Network out of sync",
        "sent bad command",
        "Unknown unitType",
        "PYTEST_WAR1_JOINING_MAP_SETTINGS_ERROR",
        "Segmentation fault",
        "Aborted",
    ):
        assert marker not in combined
