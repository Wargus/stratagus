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


def _participant_cmd(participant: dict, args: list[str]) -> list[str]:
    return [*participant["argv"], *args]


def _is_emulated(participant: dict) -> bool:
    return any(str(arg).startswith("qemu-") for arg in participant["argv"])


def _uses_emulator(stratagus_pair: tuple[dict, dict]) -> bool:
    return any(_is_emulated(participant) for participant in stratagus_pair)


def _host_uses_emulator(stratagus_pair: tuple[dict, dict]) -> bool:
    return _is_emulated(stratagus_pair[0])


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


def _network_setup_lines(output: str) -> list[str]:
    marker = "FINAL NETWORK GAME SETUP\n"
    assert marker in output
    lines = []
    for line in output.split(marker, 1)[1].splitlines():
        if line.startswith("FINAL NETWORK GAME SETTINGS"):
            break
        if line:
            lines.append(line)
    return lines


def _setting_value(settings: str, name: str) -> str:
    prefix = f"{name} = "
    for line in settings.splitlines():
        if line.startswith(prefix):
            return line[len(prefix) :]
    raise AssertionError(f"Missing setting {name} in:\n{settings}")


def _run_command_line_multiplayer(
    *,
    repo_root: Path,
    extracted_wargus_data: Path,
    xvfb_env,
    tmp_path: Path,
    stratagus_bin: str | None = None,
    stratagus_pair: tuple[dict, dict] | None = None,
    map_name: str = "maps/skirmish/multiplayer/(2)timeless-isle.smp.gz",
    run_after_start_seconds: float = 8,
    fog_of_war: int = 1,
    reveal_map: int = 0,
    numplayers: int = 2,
    ai_players: int = 0,
    dedicated: bool = False,
    client_races: tuple[str, ...] = ("orc",),
    setup_timeout: float = 70,
    host_preferences: dict[str, Any] | None = None,
    client_preferences: dict[str, Any] | None = None,
    require_running_after_start: bool = True,
) -> tuple[str, list[str]]:
    if stratagus_pair is None:
        assert stratagus_bin is not None
        host_participant = client_participant = {"argv": [stratagus_bin]}
    else:
        host_participant, client_participant = stratagus_pair

    port = _free_udp_port()
    host_user = tmp_path / "host-user"
    write_wargus_preferences(host_user, host_preferences)
    client_users = []
    for i, _race in enumerate(client_races):
        client_user = tmp_path / f"client-{i}-user"
        write_wargus_preferences(client_user, client_preferences)
        client_users.append(client_user)

    host_out = tmp_path / "host.stdout"
    host_err = tmp_path / "host.stderr"
    client_outs = [tmp_path / f"client-{i}.stdout" for i, _race in enumerate(client_races)]
    client_errs = [tmp_path / f"client-{i}.stderr" for i, _race in enumerate(client_races)]
    common = [
        "-d",
        str(extracted_wargus_data),
        "-W",
        "640x480",
        "-v",
        "640x480",
        "-g",
    ]
    server_options = [
        "server",
        "race=human",
        f"map={map_name}",
        f"numplayers={numplayers}",
        "resources=High",
        "units=1",
        f"fow={fog_of_war}",
        f"reveal={reveal_map}",
        "player=pytest-host",
    ]
    if ai_players:
        server_options.append(f"aiplayers={ai_players}")
    if dedicated:
        server_options.append("dedicated")
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
            ",".join(server_options),
        ],
    )
    client_cmds = []
    for i, race in enumerate(client_races):
        client_cmds.append(
            _participant_cmd(
                client_participant,
                [
                    *common,
                    "-u",
                    str(client_users[i]),
                    "-P",
                    "0",
                    "-c",
                    "scripts/multiplayer.lua",
                    "-G",
                    f"client,race={race},ip=127.0.0.1,port={port},player=pytest-client-{i}",
                ],
            )
        )

    host = _launch(host_cmd, cwd=repo_root, env=xvfb_env, stdout=host_out, stderr=host_err)
    clients = []
    logs = (host_out, host_err, *client_outs, *client_errs)
    try:
        time.sleep(3)
        assert host.poll() is None, _combined_logs((host_out, host_err))
        for i, client_cmd in enumerate(client_cmds):
            clients.append(
                _launch(client_cmd, cwd=repo_root, env=xvfb_env, stdout=client_outs[i], stderr=client_errs[i])
            )
            time.sleep(1)

        assert _wait_for_log(host_out, "FINAL NETWORK GAME SETUP", host, timeout=setup_timeout), _combined_logs(
            logs
        )
        time.sleep(run_after_start_seconds)
        if require_running_after_start:
            assert host.poll() is None, _combined_logs(logs)
            for client in clients:
                assert client.poll() is None, _combined_logs(logs)
        else:
            assert host.poll() in (None, 0), _combined_logs(logs)
            for client in clients:
                assert client.poll() in (None, 0), _combined_logs(logs)

        combined = _combined_logs(logs)
        for marker in ("Network out of sync", "sent bad command", "Unknown unitType", "Segmentation fault", "Aborted"):
            assert marker not in combined
    finally:
        for client in clients:
            terminate_process(client)
        terminate_process(host)

    combined = _combined_logs(logs)
    for marker in ("Network out of sync", "sent bad command", "Unknown unitType", "Segmentation fault", "Aborted"):
        assert marker not in combined
    return _read(host_out), [_read(path) for path in client_outs]


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_wargus_command_line_multiplayer_host_and_client_start_game(
    repo_root: Path,
    stratagus_pair: tuple[dict, dict],
    extracted_wargus_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    host_output, _client_outputs = _run_command_line_multiplayer(
        repo_root=repo_root,
        stratagus_pair=stratagus_pair,
        extracted_wargus_data=extracted_wargus_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
    )

    assert "pytest-client-0" in host_output
    assert "pytest-host" in host_output


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_wargus_multiplayer_uses_synchronized_settings_for_simulation_preferences(
    repo_root: Path,
    stratagus_pair: tuple[dict, dict],
    extracted_wargus_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    host_output, client_outputs = _run_command_line_multiplayer(
        repo_root=repo_root,
        stratagus_pair=stratagus_pair,
        extracted_wargus_data=extracted_wargus_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
        host_preferences={"SimplifiedAutoTargeting": True},
        client_preferences={"SimplifiedAutoTargeting": False},
    )

    host_settings = _game_settings_block(host_output)
    client_settings = _game_settings_block(client_outputs[0])
    assert host_settings == client_settings

    # Keep fog/visibility active in multiplayer process tests; visibility
    # differences are part of the desync surface this suite is meant to cover.
    assert _setting_value(host_settings, "RevealMap") == "0"
    assert int(_setting_value(host_settings, "Flags")) & 1 == 0


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_wargus_multiplayer_sync_stress_map_runs_without_desync(
    repo_root: Path,
    stratagus_pair: tuple[dict, dict],
    extracted_wargus_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    # Covers the resource/fog/network-sync surface from Wargus/stratagus#569.
    host_output, _client_outputs = _run_command_line_multiplayer(
        repo_root=repo_root,
        stratagus_pair=stratagus_pair,
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


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
@pytest.mark.parametrize(
    "map_name,ai_players",
    [
        pytest.param(
            "maps/skirmish/(3)three-ways-to-cross.smp.gz",
            1,
        ),
        pytest.param(
            "maps/skirmish/multiplayer/(4)central-park.smp.gz",
            2,
        ),
        pytest.param(
            "maps/skirmish/(8)bridge-to-bridge-combat.smp.gz",
            2,
        ),
        pytest.param(
            "maps/skirmish/(8)garden-of-war.smp.gz",
            2,
        ),
    ],
)
def test_wargus_reported_multiplayer_maps_run_with_network_humans_and_ai(
    repo_root: Path,
    stratagus_pair: tuple[dict, dict],
    extracted_wargus_data: Path,
    xvfb_env,
    tmp_path: Path,
    map_name: str,
    ai_players: int,
):
    host_output, _client_outputs = _run_command_line_multiplayer(
        repo_root=repo_root,
        stratagus_pair=stratagus_pair,
        extracted_wargus_data=extracted_wargus_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
        map_name=map_name,
        ai_players=ai_players,
        run_after_start_seconds=15,
    )

    assert "FINAL NETWORK GAME SETUP" in host_output


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
@pytest.mark.parametrize(
    "map_name,client_races,ai_players",
    [
        pytest.param(
            "maps/skirmish/(3)three-ways-to-cross.smp.gz",
            ("orc",),
            1,
            id="three-ways-to-cross",
        ),
        pytest.param(
            "maps/skirmish/multiplayer/(4)central-park.smp.gz",
            ("human", "orc"),
            1,
            id="central-park",
        ),
        pytest.param(
            "maps/skirmish/(8)bridge-to-bridge-combat.smp.gz",
            ("human", "orc"),
            2,
            id="bridge-to-bridge-combat",
        ),
        pytest.param(
            "maps/skirmish/(8)garden-of-war.smp.gz",
            ("human", "orc"),
            2,
            id="garden-of-war",
        ),
    ],
)
def test_wargus_dedicated_ai_server_starts_reported_ai_maps(
    repo_root: Path,
    stratagus_pair: tuple[dict, dict],
    extracted_wargus_data: Path,
    xvfb_env,
    tmp_path: Path,
    map_name: str,
    client_races: tuple[str, ...],
    ai_players: int,
):
    if _host_uses_emulator(stratagus_pair):
        pytest.skip("qemu-aarch64 host does not reach Wargus command-line setup reliably")

    host_output, _client_outputs = _run_command_line_multiplayer(
        repo_root=repo_root,
        stratagus_pair=stratagus_pair,
        extracted_wargus_data=extracted_wargus_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
        map_name=map_name,
        numplayers=len(client_races),
        ai_players=ai_players,
        dedicated=True,
        client_races=client_races,
        setup_timeout=300 if _uses_emulator(stratagus_pair) else 35,
        run_after_start_seconds=0,
        require_running_after_start=False,
    )

    assert "FINAL NETWORK GAME SETUP" in host_output

    player_prefixes = tuple(f"{i}: CO: " for i in range(1, 16))
    setup = _network_setup_lines(host_output)
    assert setup[0].startswith("0: CO: 2")
    assert "Host:" not in setup[0]
    assert sum(
        line.startswith(tuple(f"{prefix}0" for prefix in player_prefixes)) and "Host:" in line
        for line in setup
    ) == len(client_races)
    assert sum(
        line.startswith(tuple(f"{prefix}1" for prefix in player_prefixes)) and "Host:" not in line
        for line in setup
    ) >= ai_players
