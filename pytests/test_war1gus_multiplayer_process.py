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


def _participant_cmd(participant: dict, args: list[str]) -> list[str]:
    return [*participant["argv"], *args]


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_war1gus_command_line_multiplayer_host_and_client_run_sync_stress_map(
    stratagus_pair: tuple[dict, dict],
    extracted_war1gus_data: Path,
    gui_env,
    tmp_path: Path,
):
    host_participant, client_participant = stratagus_pair
    port = _free_udp_port()
    host_user = tmp_path / "host-user"
    client_user = tmp_path / "client-user"
    write_war1gus_preferences(host_user)
    write_war1gus_preferences(client_user)
    test_env = dict(gui_env)
    test_env["STRATAGUS_UNBUFFERED_STDIO"] = "1"

    host_out = tmp_path / "host.stdout"
    host_err = tmp_path / "host.stderr"
    client_out = tmp_path / "client.stdout"
    client_err = tmp_path / "client.stderr"
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
        host_participant,
        [
            *common,
            "-u",
            str(host_user),
            "-P",
            str(port),
            "-c",
            "scripts/multiplayer.lua",
            "-G",
            "server,race=human,map=maps/test/(2)pytest-sync-stress.smp,numplayers=2,"
            "resources=High,fow=1,reveal=0,player=pytest-war1-host",
        ],
    )
    client_cmd = _participant_cmd(
        client_participant,
        [
            *common,
            "-u",
            str(client_user),
            "-P",
            "0",
            "-c",
            "scripts/multiplayer.lua",
            "-G",
            f"client,race=orc,ip=127.0.0.1,port={port},player=pytest-war1-client",
        ],
    )

    # War1gus still has legacy map-loading paths during network start that may
    # retry the raw relative map name. Run from the composed data tree so those
    # fallbacks resolve to the same files as the -d path.
    host = _launch(host_cmd, cwd=extracted_war1gus_data, env=test_env, stdout=host_out, stderr=host_err)
    client = None
    logs = (host_out, host_err, client_out, client_err)
    try:
        time.sleep(3)
        assert host.poll() is None, _combined_logs((host_out, host_err))
        client = _launch(client_cmd, cwd=extracted_war1gus_data, env=test_env, stdout=client_out, stderr=client_err)

        assert _wait_for_log(host_out, "FINAL NETWORK GAME SETUP", host, timeout=120), _combined_logs(logs)
        assert host.poll() is None, _combined_logs(logs)
        assert _wait_for_log(client_out, "PYTEST_WAR1_JOINING_MAP_RUN", client, timeout=90), _combined_logs(logs)
        assert _wait_for_log(host_out, "PYTEST_WAR1_SYNC_STRESS_OPENING_ORDERS", host, timeout=120), _combined_logs(logs)
        assert _wait_for_log(client_out, "PYTEST_WAR1_SYNC_STRESS_OPENING_ORDERS", client, timeout=120), _combined_logs(logs)
        assert _wait_for_log(host_out, "PYTEST_WAR1_SYNC_STRESS_BLOCKER_BUILD_ORDERS", host, timeout=120), _combined_logs(logs)
        assert _wait_for_log(client_out, "PYTEST_WAR1_SYNC_STRESS_BLOCKER_BUILD_ORDERS", client, timeout=120), _combined_logs(logs)
        assert host.poll() is None, _combined_logs(logs)
        assert client.poll() is None, _combined_logs(logs)
    finally:
        if client is not None:
            terminate_process(client)
        terminate_process(host)

    combined = _combined_logs(logs)
    assert "PYTEST_WAR1_SYNC_STRESS_LOADED" in _read(host_out)
    assert "PYTEST_WAR1_SYNC_STRESS_LOADED" in _read(client_out)
    assert "pytest-war1-cli" in _read(host_out)
    assert "pytest-war1-hos" in _read(host_out)
    for marker in (
        "Network out of sync",
        "sent bad command",
        "Unknown unitType",
        "PYTEST_WAR1_JOINING_MAP_SETTINGS_ERROR",
        "Segmentation fault",
        "Aborted",
    ):
        assert marker not in combined
