from __future__ import annotations

import socket
import subprocess
import time
from pathlib import Path
from typing import Any

import pytest

from helpers import terminate_process, write_wargus_preferences


def _read(path: Path) -> str:
    return path.read_text(errors="replace") if path.exists() else ""


def _free_udp_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def _launch(cmd: list[str], *, cwd: Path, env: dict[str, str], stdout: Path, stderr: Path) -> subprocess.Popen:
    with stdout.open("wb") as out, stderr.open("wb") as err:
        return subprocess.Popen(cmd, cwd=cwd, env=env, stdout=out, stderr=err)


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


def _game_settings_block(output: str) -> str:
    marker = "FINAL NETWORK GAME SETTINGS\n"
    assert marker in output
    block = output.split(marker, 1)[1]
    lines = []
    for line in block.splitlines():
        if line.startswith("Can't open file ") or line.startswith("AI:"):
            break
        if line:
            lines.append(line)
    return "\n".join(lines)


def _setting_value(settings: str, name: str) -> str:
    prefix = f"{name} = "
    for line in settings.splitlines():
        if line.startswith(prefix):
            return line[len(prefix) :]
    raise AssertionError(f"Missing setting {name} in:\n{settings}")


def _run_command_line_multiplayer(
    *,
    repo_root: Path,
    stratagus_bin: str,
    extracted_wargus_data: Path,
    xvfb_env,
    tmp_path: Path,
    map_name: str = "maps/skirmish/multiplayer/(2)timeless-isle.smp.gz",
    run_after_start_seconds: float = 8,
    fog_of_war: int = 1,
    reveal_map: int = 0,
    host_preferences: dict[str, Any] | None = None,
    client_preferences: dict[str, Any] | None = None,
) -> tuple[str, str]:
    port = _free_udp_port()
    host_user = tmp_path / "host-user"
    client_user = tmp_path / "client-user"
    write_wargus_preferences(host_user, host_preferences)
    write_wargus_preferences(client_user, client_preferences)

    host_out = tmp_path / "host.stdout"
    host_err = tmp_path / "host.stderr"
    client_out = tmp_path / "client.stdout"
    client_err = tmp_path / "client.stderr"
    common = [
        stratagus_bin,
        "-d",
        str(extracted_wargus_data),
        "-W",
        "640x480",
        "-v",
        "640x480",
        "-g",
    ]
    host_cmd = [
        *common,
        "-u",
        str(host_user),
        "-P",
        str(port),
        "-c",
        "scripts/multiplayer.lua",
        "-G",
        f"server,race=human,map={map_name},numplayers=2,resources=High,units=1,"
        f"fow={fog_of_war},reveal={reveal_map},player=pytest-host",
    ]
    client_cmd = [
        *common,
        "-u",
        str(client_user),
        "-P",
        "0",
        "-c",
        "scripts/multiplayer.lua",
        "-G",
        f"client,race=orc,ip=127.0.0.1,port={port},player=pytest-client",
    ]

    host = _launch(host_cmd, cwd=repo_root, env=xvfb_env, stdout=host_out, stderr=host_err)
    client = None
    logs = (host_out, host_err, client_out, client_err)
    try:
        time.sleep(3)
        assert host.poll() is None, _combined_logs((host_out, host_err))
        client = _launch(client_cmd, cwd=repo_root, env=xvfb_env, stdout=client_out, stderr=client_err)

        assert _wait_for_log(host_out, "FINAL NETWORK GAME SETUP", host, timeout=70), _combined_logs(logs)
        time.sleep(run_after_start_seconds)
        assert host.poll() is None, _combined_logs(logs)
        assert client.poll() is None, _combined_logs(logs)

        combined = _combined_logs(logs)
        for marker in ("Network out of sync", "sent bad command", "Unknown unitType", "Segmentation fault", "Aborted"):
            assert marker not in combined
    finally:
        if client is not None:
            terminate_process(client)
        terminate_process(host)

    combined = _combined_logs(logs)
    for marker in ("Network out of sync", "sent bad command", "Unknown unitType", "Segmentation fault", "Aborted"):
        assert marker not in combined
    return _read(host_out), _read(client_out)


@pytest.mark.gui
@pytest.mark.slow
def test_wargus_command_line_multiplayer_host_and_client_start_game(
    repo_root: Path,
    stratagus_bin: str,
    extracted_wargus_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    host_output, _client_output = _run_command_line_multiplayer(
        repo_root=repo_root,
        stratagus_bin=stratagus_bin,
        extracted_wargus_data=extracted_wargus_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
    )

    assert "pytest-client" in host_output
    assert "pytest-host" in host_output


@pytest.mark.gui
@pytest.mark.slow
def test_wargus_multiplayer_uses_synchronized_settings_for_simulation_preferences(
    repo_root: Path,
    stratagus_bin: str,
    extracted_wargus_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    host_output, client_output = _run_command_line_multiplayer(
        repo_root=repo_root,
        stratagus_bin=stratagus_bin,
        extracted_wargus_data=extracted_wargus_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
        host_preferences={"SimplifiedAutoTargeting": True},
        client_preferences={"SimplifiedAutoTargeting": False},
    )

    host_settings = _game_settings_block(host_output)
    client_settings = _game_settings_block(client_output)
    assert host_settings == client_settings

    # Keep fog/visibility active in multiplayer process tests; visibility
    # differences are part of the desync surface this suite is meant to cover.
    assert _setting_value(host_settings, "RevealMap") == "0"
    assert int(_setting_value(host_settings, "Flags")) & 1 == 0


@pytest.mark.gui
@pytest.mark.slow
def test_wargus_multiplayer_sync_stress_map_runs_without_desync(
    repo_root: Path,
    stratagus_bin: str,
    extracted_wargus_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    # Covers the resource/fog/network-sync surface from Wargus/stratagus#569.
    host_output, _client_output = _run_command_line_multiplayer(
        repo_root=repo_root,
        stratagus_bin=stratagus_bin,
        extracted_wargus_data=extracted_wargus_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
        map_name="maps/test/(2)pytest-sync-stress.smp",
        run_after_start_seconds=45,
    )

    assert "PYTEST_SYNC_STRESS_LOADED" in host_output
    assert "PYTEST_SYNC_STRESS_OPENING_ORDERS" in host_output
    assert "PYTEST_SYNC_STRESS_BLOCKER_BUILD_ORDERS" in host_output
    assert "PYTEST_SYNC_STRESS_REINFORCEMENTS" in host_output
    assert "PYTEST_SYNC_STRESS_GOLD_EXHAUSTED" in host_output
    assert "PYTEST_SYNC_STRESS_SECOND_ATTACK" in host_output
