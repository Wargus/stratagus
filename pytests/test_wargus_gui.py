from __future__ import annotations

import subprocess
import time
from pathlib import Path

import pytest

from helpers import terminate_process, write_wargus_preferences, write_war1gus_preferences


def click_screen(x: int, y: int, display: str) -> None:
    try:
        import pyautogui
    except (ImportError, SystemExit):
        pyautogui = None

    if pyautogui is not None:
        pyautogui.click(x=x, y=y)
        return

    try:
        from Xlib import X, display as xdisplay
        from Xlib.ext import xtest
    except ImportError:
        pytest.skip("pyautogui or python-xlib is required for GUI input")

    disp = xdisplay.Display(display)
    xtest.fake_input(disp, X.MotionNotify, x=x, y=y)
    xtest.fake_input(disp, X.ButtonPress, 1)
    xtest.fake_input(disp, X.ButtonRelease, 1)
    disp.sync()


def _run_driver(
    *,
    repo_root: Path,
    stratagus_bin: str,
    data_dir: Path,
    gui_env,
    tmp_path: Path,
    driver_name: str,
    mode: str,
    marker: str,
    write_preferences,
) -> None:
    stdout = tmp_path / f"{driver_name}-{mode}.stdout"
    stderr = tmp_path / f"{driver_name}-{mode}.stderr"
    user_dir = tmp_path / f"{driver_name}-{mode}-user"
    driver = repo_root / "pytests" / "lua" / f"{driver_name}.lua"
    write_preferences(user_dir)

    cmd = [
        stratagus_bin,
        "-d",
        str(data_dir),
        "-u",
        str(user_dir),
        "-W",
        "640x480",
        "-v",
        "640x480",
        "-c",
        str(driver),
        "-G",
        mode,
    ]
    with stdout.open("wb") as out, stderr.open("wb") as err:
        proc = subprocess.run(cmd, cwd=repo_root, env=gui_env, stdout=out, stderr=err, timeout=45)

    combined = f"stdout:\n{stdout.read_text(errors='replace')}\nstderr:\n{stderr.read_text(errors='replace')}"
    assert proc.returncode == 0, combined
    assert marker in stdout.read_text(errors="replace"), combined


@pytest.mark.gui
@pytest.mark.slow
@pytest.mark.parametrize(
    "mode,marker",
    [
        ("server-lobby-callbacks", "PYTEST_WARGUS_SERVER_LOBBY_CALLBACKS_OK"),
        ("client-lobby-race-slot", "PYTEST_WARGUS_CLIENT_LOBBY_RACE_SLOT_OK"),
    ],
)
def test_wargus_lobby_callbacks_update_setup_slots(
    repo_root: Path,
    stratagus_bin: str,
    extracted_wargus_data: Path,
    gui_env,
    tmp_path: Path,
    mode: str,
    marker: str,
):
    _run_driver(
        repo_root=repo_root,
        stratagus_bin=stratagus_bin,
        data_dir=extracted_wargus_data,
        gui_env=gui_env,
        tmp_path=tmp_path,
        driver_name="wargus_driver",
        mode=mode,
        marker=marker,
        write_preferences=write_wargus_preferences,
    )


@pytest.mark.gui
@pytest.mark.slow
@pytest.mark.parametrize(
    "mode,marker",
    [
        ("server-lobby-callbacks", "PYTEST_WAR1GUS_SERVER_LOBBY_CALLBACKS_OK"),
        ("client-lobby-race-slot", "PYTEST_WAR1GUS_CLIENT_LOBBY_RACE_SLOT_OK"),
    ],
)
def test_war1gus_lobby_callbacks_update_setup_slots(
    repo_root: Path,
    stratagus_bin: str,
    extracted_war1gus_data: Path,
    gui_env,
    tmp_path: Path,
    mode: str,
    marker: str,
):
    _run_driver(
        repo_root=repo_root,
        stratagus_bin=stratagus_bin,
        data_dir=extracted_war1gus_data,
        gui_env=gui_env,
        tmp_path=tmp_path,
        driver_name="war1gus_driver",
        mode=mode,
        marker=marker,
        write_preferences=write_war1gus_preferences,
    )


@pytest.mark.gui
@pytest.mark.slow
def test_wargus_driver_launches_join_ip_menu(
    repo_root: Path,
    stratagus_bin: str,
    extracted_wargus_data: Path,
    gui_env,
    tmp_path: Path,
):
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
        "join-ip-menu",
    ]
    with stdout.open("wb") as out, stderr.open("wb") as err:
        proc = subprocess.Popen(cmd, cwd=repo_root, env=gui_env, stdout=out, stderr=err)
    try:
        time.sleep(5)
        assert proc.poll() is None, (
            f"Wargus exited early with {proc.returncode}\n"
            f"stdout:\n{stdout.read_text(errors='replace')}\n"
            f"stderr:\n{stderr.read_text(errors='replace')}"
        )
    finally:
        terminate_process(proc)


@pytest.mark.gui
@pytest.mark.slow
def test_wargus_join_without_server_selection_stays_alive(
    repo_root: Path,
    stratagus_bin: str,
    extracted_wargus_data: Path,
    gui_env,
    tmp_path: Path,
):
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
        "join-ip-menu",
    ]
    with stdout.open("wb") as out, stderr.open("wb") as err:
        proc = subprocess.Popen(cmd, cwd=repo_root, env=gui_env, stdout=out, stderr=err)
    try:
        time.sleep(5)
        assert proc.poll() is None, "Wargus exited before menu interaction"
        if gui_env.get("SDL_VIDEODRIVER") != "x11":
            pytest.skip("interactive click test requires an SDL build with the x11 video driver")

        # RunJoinIpMenu creates a 352x352 panel centered in a 640x480 window.
        # The Connect button is at local coordinates roughly (60, 180).
        click_screen(x=255, y=262, display=gui_env["DISPLAY"])
        time.sleep(2)

        assert proc.poll() is None, (
            f"Wargus crashed/exited after empty Connect click with {proc.returncode}\n"
            f"stdout:\n{stdout.read_text(errors='replace')}\n"
            f"stderr:\n{stderr.read_text(errors='replace')}"
        )
    finally:
        terminate_process(proc)
