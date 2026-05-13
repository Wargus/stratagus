from __future__ import annotations

import socket
import struct
import subprocess
import time
import zlib
from pathlib import Path
from typing import Any

import pytest

from helpers import terminate_process, write_timeless_tales_preferences


def _read(path: Path) -> str:
    return path.read_text(errors="replace") if path.exists() else ""


def _launch(cmd: list[str], *, cwd: Path, env: dict[str, str], stdout: Path, stderr: Path) -> subprocess.Popen:
    with stdout.open("wb") as out, stderr.open("wb") as err:
        return subprocess.Popen(cmd, cwd=cwd, env=env, stdout=out, stderr=err)


def _free_udp_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


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


def _game_settings_block(output: str) -> str:
    marker = "FINAL NETWORK GAME SETTINGS\n"
    assert marker in output
    block = output.split(marker, 1)[1]
    lines = []
    for line in block.splitlines():
        if (
            line.startswith("Can't open file ")
            or line.startswith("AI:")
            or line.startswith("PYTEST_")
            or line.startswith("Frames ")
            or line.startswith("Thanks for playing")
        ):
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


def _assert_no_runtime_failures(combined: str) -> None:
    for marker in (
        "Network out of sync",
        "sent bad command",
        "Can't load the graphic",
        "Unknown unitType",
        "Segmentation fault",
        "Aborted",
    ):
        assert marker not in combined


def _assert_valid_png(path: Path) -> None:
    data = path.read_bytes()
    assert data.startswith(b"\x89PNG\r\n\x1a\n"), f"{path} has an invalid PNG signature"

    offset = 8
    seen_ihdr = False
    seen_idat = False
    seen_iend = False
    idat = bytearray()
    while offset < len(data):
        assert offset + 12 <= len(data), f"{path} has a truncated PNG chunk header"
        length = struct.unpack(">I", data[offset : offset + 4])[0]
        chunk_type = data[offset + 4 : offset + 8]
        chunk_start = offset + 8
        chunk_end = chunk_start + length
        crc_end = chunk_end + 4
        assert crc_end <= len(data), f"{path} has a truncated {chunk_type!r} chunk"

        chunk = data[chunk_start:chunk_end]
        expected_crc = struct.unpack(">I", data[chunk_end:crc_end])[0]
        actual_crc = zlib.crc32(chunk_type + chunk) & 0xFFFFFFFF
        assert actual_crc == expected_crc, f"{path} has a bad {chunk_type!r} CRC"

        if chunk_type == b"IHDR":
            assert not seen_ihdr, f"{path} has multiple IHDR chunks"
            seen_ihdr = True
            width, height = struct.unpack(">II", chunk[:8])
            assert width > 0 and height > 0, f"{path} has invalid dimensions {width}x{height}"
        elif chunk_type == b"IDAT":
            seen_idat = True
            idat.extend(chunk)
        elif chunk_type == b"IEND":
            seen_iend = True
            offset = crc_end
            break

        offset = crc_end

    assert seen_ihdr, f"{path} has no IHDR chunk"
    assert seen_idat, f"{path} has no IDAT chunk"
    assert seen_iend, f"{path} has no IEND chunk"
    assert offset == len(data), f"{path} has trailing data after IEND"
    zlib.decompress(bytes(idat))


def _run_single_player_map(
    *,
    participant: dict,
    timeless_tales_data: Path,
    xvfb_env,
    tmp_path: Path,
    map_name: str,
    run_seconds: float,
) -> tuple[Path, Path]:
    user_dir = tmp_path / "user"
    write_timeless_tales_preferences(user_dir)
    test_env = dict(xvfb_env)
    test_env["STRATAGUS_UNBUFFERED_STDIO"] = "1"

    stdout = tmp_path / "single-player.stdout"
    stderr = tmp_path / "single-player.stderr"
    cmd = _participant_cmd(
        participant,
        [
            "-d",
            str(timeless_tales_data),
            "-u",
            str(user_dir),
            "-W",
            "640x480",
            "-v",
            "640x480",
            "-g",
            "-c",
            "scripts/stratagus.lua",
            map_name,
        ],
    )

    proc = _launch(cmd, cwd=timeless_tales_data, env=test_env, stdout=stdout, stderr=stderr)
    logs = (stdout, stderr)
    try:
        assert _wait_for_log(stdout, "... ready!", proc, timeout=45), _combined_logs(logs)
        time.sleep(run_seconds)
        assert proc.poll() is None, _combined_logs(logs)
    finally:
        terminate_process(proc)

    _assert_no_runtime_failures(_combined_logs(logs))
    return logs


def _run_timeless_tales_script(
    *,
    participant: dict,
    timeless_tales_data: Path,
    xvfb_env,
    tmp_path: Path,
    script: Path,
    ready_marker: str,
    run_seconds: float,
) -> tuple[Path, Path]:
    user_dir = tmp_path / "user"
    write_timeless_tales_preferences(user_dir)
    test_env = dict(xvfb_env)
    test_env["STRATAGUS_UNBUFFERED_STDIO"] = "1"

    stdout = tmp_path / "script.stdout"
    stderr = tmp_path / "script.stderr"
    cmd = _participant_cmd(
        participant,
        [
            "-d",
            str(timeless_tales_data),
            "-u",
            str(user_dir),
            "-W",
            "640x480",
            "-v",
            "640x480",
            "-g",
            "-c",
            str(script),
        ],
    )

    proc = _launch(cmd, cwd=timeless_tales_data, env=test_env, stdout=stdout, stderr=stderr)
    logs = (stdout, stderr)
    try:
        assert _wait_for_log(stdout, ready_marker, proc, timeout=75), _combined_logs(logs)
        time.sleep(run_seconds)
        assert proc.poll() is None, _combined_logs(logs)
    finally:
        terminate_process(proc)

    _assert_no_runtime_failures(_combined_logs(logs))
    return logs


def _run_command_line_multiplayer(
    *,
    stratagus_pair: tuple[dict, dict],
    timeless_tales_data: Path,
    xvfb_env,
    tmp_path: Path,
    map_name: str = "maps/test/(2)pytest-sync-stress.smp",
    run_after_start_seconds: float = 8,
    numplayers: int = 2,
    ai_players: int = 0,
    host_preferences: dict[str, Any] | None = None,
    client_preferences: dict[str, Any] | None = None,
    host_markers: tuple[str, ...] = (),
    client_markers: tuple[str, ...] = (),
) -> tuple[str, str]:
    host_participant, client_participant = stratagus_pair
    port = _free_udp_port()
    host_user = tmp_path / "host-user"
    client_user = tmp_path / "client-user"
    write_timeless_tales_preferences(host_user, host_preferences)
    write_timeless_tales_preferences(client_user, client_preferences)
    test_env = dict(xvfb_env)
    test_env["STRATAGUS_UNBUFFERED_STDIO"] = "1"

    host_out = tmp_path / "host.stdout"
    host_err = tmp_path / "host.stderr"
    client_out = tmp_path / "client.stdout"
    client_err = tmp_path / "client.stderr"
    common = [
        "-d",
        str(timeless_tales_data),
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
        "fow=1",
        "reveal=0",
        "player=pytest-timeless-host",
    ]
    if ai_players:
        server_options.append(f"aiplayers={ai_players}")

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
            f"client,race=orc,ip=127.0.0.1,port={port},player=pytest-timeless-client",
        ],
    )

    host = _launch(host_cmd, cwd=timeless_tales_data, env=test_env, stdout=host_out, stderr=host_err)
    client = None
    logs = (host_out, host_err, client_out, client_err)
    try:
        time.sleep(3)
        assert host.poll() is None, _combined_logs((host_out, host_err))
        client = _launch(client_cmd, cwd=timeless_tales_data, env=test_env, stdout=client_out, stderr=client_err)

        assert _wait_for_log(host_out, "FINAL NETWORK GAME SETUP", host, timeout=90), _combined_logs(logs)
        assert _wait_for_log(client_out, "FINAL NETWORK GAME SETUP", client, timeout=90), _combined_logs(logs)
        for marker in host_markers:
            assert _wait_for_log(host_out, marker, host, timeout=90), _combined_logs(logs)
        for marker in client_markers:
            assert _wait_for_log(client_out, marker, client, timeout=90), _combined_logs(logs)
        time.sleep(run_after_start_seconds)
        assert host.poll() is None, _combined_logs(logs)
        assert client.poll() is None, _combined_logs(logs)
    finally:
        if client is not None:
            terminate_process(client)
        terminate_process(host)

    combined = _combined_logs(logs)
    assert "pytest-timeless-host" in combined
    assert "pytest-timeless-client" in combined
    _assert_no_runtime_failures(combined)
    return _read(host_out), _read(client_out)


def test_timeless_tales_map_png_previews_are_valid(timeless_tales_data: Path):
    pngs = sorted((timeless_tales_data / "maps").rglob("*.png"))
    assert pngs, f"No map PNG previews found in {timeless_tales_data / 'maps'}"
    for png in pngs:
        _assert_valid_png(png)


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_timeless_tales_data_tree_starts_to_main_menu(
    stratagus_pair: tuple[dict, dict],
    timeless_tales_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    participant, unused_client_participant = stratagus_pair
    if participant["name"] != unused_client_participant["name"]:
        pytest.skip("Timeless Tales startup uses one Stratagus process; self-pairs cover each participant")

    user_dir = tmp_path / "user"
    write_timeless_tales_preferences(user_dir)
    test_env = dict(xvfb_env)
    test_env["STRATAGUS_UNBUFFERED_STDIO"] = "1"

    stdout = tmp_path / "timeless.stdout"
    stderr = tmp_path / "timeless.stderr"
    cmd = _participant_cmd(
        participant,
        [
            "-d",
            str(timeless_tales_data),
            "-u",
            str(user_dir),
            "-W",
            "640x480",
            "-v",
            "640x480",
            "-g",
            "-c",
            "scripts/stratagus.lua",
        ],
    )

    proc = _launch(cmd, cwd=timeless_tales_data, env=test_env, stdout=stdout, stderr=stderr)
    logs = (stdout, stderr)
    try:
        assert _wait_for_log(stdout, "... ready!", proc, timeout=45), _combined_logs(logs)
        time.sleep(8)
        assert proc.poll() is None, _combined_logs(logs)
    finally:
        terminate_process(proc)

    combined = _combined_logs(logs)
    _assert_no_runtime_failures(combined)


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_timeless_tales_single_player_map_runs_without_crashing(
    stratagus_pair: tuple[dict, dict],
    timeless_tales_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    participant, unused_client_participant = stratagus_pair
    if participant["name"] != unused_client_participant["name"]:
        pytest.skip("Timeless Tales single-player startup uses one process; self-pairs cover each participant")

    _run_single_player_map(
        participant=participant,
        timeless_tales_data=timeless_tales_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
        map_name="maps/skirmish/(2)timeless-isle.smp.gz",
        run_seconds=20,
    )


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_timeless_tales_for_the_motherland_map_runs_without_crashing(
    stratagus_pair: tuple[dict, dict],
    timeless_tales_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    participant, unused_client_participant = stratagus_pair
    if participant["name"] != unused_client_participant["name"]:
        pytest.skip("Timeless Tales single-player startup uses one process; self-pairs cover each participant")

    _run_single_player_map(
        participant=participant,
        timeless_tales_data=timeless_tales_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
        map_name="maps/ftm/(2)nicks-duel.smp",
        run_seconds=20,
    )


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_timeless_tales_beethoven_day_wise_man_selection_survives(
    stratagus_pair: tuple[dict, dict],
    timeless_tales_data: Path,
    xvfb_env,
    tmp_path: Path,
    repo_root: Path,
):
    participant, unused_client_participant = stratagus_pair
    if participant["name"] != unused_client_participant["name"]:
        pytest.skip("Timeless Tales single-player startup uses one process; self-pairs cover each participant")

    _run_timeless_tales_script(
        participant=participant,
        timeless_tales_data=timeless_tales_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
        script=repo_root / "pytests" / "lua" / "timeless_tales_select_wiseman.lua",
        ready_marker="PYTEST_TIMELESS_SELECTED_WISEMAN",
        run_seconds=8,
    )


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_timeless_tales_editor_map_runs_without_crashing(
    stratagus_pair: tuple[dict, dict],
    timeless_tales_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    participant, unused_client_participant = stratagus_pair
    if participant["name"] != unused_client_participant["name"]:
        pytest.skip("Timeless Tales editor startup uses one process; self-pairs cover each participant")

    user_dir = tmp_path / "user"
    write_timeless_tales_preferences(user_dir)
    test_env = dict(xvfb_env)
    test_env["STRATAGUS_UNBUFFERED_STDIO"] = "1"

    stdout = tmp_path / "editor.stdout"
    stderr = tmp_path / "editor.stderr"
    cmd = _participant_cmd(
        participant,
        [
            "-d",
            str(timeless_tales_data),
            "-u",
            str(user_dir),
            "-W",
            "640x480",
            "-v",
            "640x480",
            "-g",
            "-e",
            "maps/skirmish/(2)timeless-isle.smp.gz",
            "-c",
            "scripts/stratagus.lua",
        ],
    )

    proc = _launch(cmd, cwd=timeless_tales_data, env=test_env, stdout=stdout, stderr=stderr)
    logs = (stdout, stderr)
    try:
        assert _wait_for_log(stdout, "... ready!", proc, timeout=45), _combined_logs(logs)
        time.sleep(20)
        assert proc.poll() is None, _combined_logs(logs)
    finally:
        terminate_process(proc)

    _assert_no_runtime_failures(_combined_logs(logs))


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_timeless_tales_command_line_multiplayer_runs_without_desync_or_crash(
    stratagus_pair: tuple[dict, dict],
    timeless_tales_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    host_output, client_output = _run_command_line_multiplayer(
        stratagus_pair=stratagus_pair,
        timeless_tales_data=timeless_tales_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
        run_after_start_seconds=0,
        host_markers=(
            "PYTEST_TIMELESS_SYNC_STRESS_LOADED",
            "PYTEST_TIMELESS_SYNC_STRESS_OPENING_ORDERS",
            "PYTEST_TIMELESS_SYNC_STRESS_BLOCKER_BUILD_ORDERS",
            "PYTEST_TIMELESS_SYNC_STRESS_REINFORCEMENTS",
        ),
        client_markers=(
            "PYTEST_TIMELESS_SYNC_STRESS_LOADED",
            "PYTEST_TIMELESS_SYNC_STRESS_OPENING_ORDERS",
            "PYTEST_TIMELESS_SYNC_STRESS_BLOCKER_BUILD_ORDERS",
            "PYTEST_TIMELESS_SYNC_STRESS_REINFORCEMENTS",
        ),
    )

    for output in (host_output, client_output):
        assert "PYTEST_TIMELESS_SYNC_STRESS_LOADED" in output
        assert "PYTEST_TIMELESS_SYNC_STRESS_OPENING_ORDERS" in output
        assert "PYTEST_TIMELESS_SYNC_STRESS_BLOCKER_BUILD_ORDERS" in output
        assert "PYTEST_TIMELESS_SYNC_STRESS_REINFORCEMENTS" in output


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_timeless_tales_multiplayer_uses_synchronized_settings_for_simulation_preferences(
    stratagus_pair: tuple[dict, dict],
    timeless_tales_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    host_output, client_output = _run_command_line_multiplayer(
        stratagus_pair=stratagus_pair,
        timeless_tales_data=timeless_tales_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
        host_preferences={"SimplifiedAutoTargeting": True},
        client_preferences={"SimplifiedAutoTargeting": False},
    )

    host_settings = _game_settings_block(host_output)
    client_settings = _game_settings_block(client_output)
    assert host_settings == client_settings
    assert _setting_value(host_settings, "RevealMap") == "0"
    assert int(_setting_value(host_settings, "Flags")) & 1 == 0


@pytest.mark.gui
@pytest.mark.cross
@pytest.mark.slow
def test_timeless_tales_multiplayer_real_map_runs_with_network_humans_and_ai(
    stratagus_pair: tuple[dict, dict],
    timeless_tales_data: Path,
    xvfb_env,
    tmp_path: Path,
):
    host_output, _client_output = _run_command_line_multiplayer(
        stratagus_pair=stratagus_pair,
        timeless_tales_data=timeless_tales_data,
        xvfb_env=xvfb_env,
        tmp_path=tmp_path,
        map_name="maps/skirmish/New folder/(3)three-ways-to-cross.smp.gz",
        ai_players=1,
        run_after_start_seconds=15,
    )

    assert "FINAL NETWORK GAME SETUP" in host_output
