from __future__ import annotations

import subprocess
from pathlib import Path

import pytest

from helpers import write_wargus_preferences


def _read(path: Path) -> str:
    return path.read_text(errors="replace") if path.exists() else ""


def _assert_no_crash_markers(combined: str) -> None:
    for marker in (
        "Stratagus failed to load game data",
        "double free",
        "corruption",
        "Segmentation fault",
        "Aborted",
        "invalid pointer",
    ):
        assert marker not in combined


@pytest.mark.gui
@pytest.mark.slow
def test_wargus_repeated_victory_cleanup_and_map_reload_does_not_crash(
    repo_root: Path,
    stratagus_bin: str,
    extracted_wargus_data: Path,
    gui_env,
    tmp_path: Path,
):
    # Covers Wargus/Wargus#502's linked victory-button crash reports, especially
    # Wargus/stratagus#686's released-unit cleanup backtrace.
    stdout = tmp_path / "wargus.stdout"
    stderr = tmp_path / "wargus.stderr"
    user_dir = tmp_path / "user"
    driver = repo_root / "pytests" / "lua" / "wargus_driver.lua"
    write_wargus_preferences(user_dir)

    cmd = [
        stratagus_bin,
        "-d",
        str(extracted_wargus_data),
        "-u",
        str(user_dir),
        "-W",
        "640x480",
        "-v",
        "640x480",
        "-c",
        str(driver),
        "-G",
        "lifecycle-cleanup-reload-stress",
    ]

    with stdout.open("wb") as out, stderr.open("wb") as err:
        proc = subprocess.run(cmd, cwd=repo_root, env=gui_env, stdout=out, stderr=err, timeout=90)

    combined = f"--- stdout ---\n{_read(stdout)}\n--- stderr ---\n{_read(stderr)}"
    assert proc.returncode == 0, combined
    output = _read(stdout)
    assert "PYTEST_LIFECYCLE_MAP_LOADED" in output
    assert "PYTEST_LIFECYCLE_RELEASE_UNITS" in output
    assert "PYTEST_LIFECYCLE_VICTORY" in output
    assert "PYTEST_LIFECYCLE_RELOAD_DONE" in output
    _assert_no_crash_markers(combined)
