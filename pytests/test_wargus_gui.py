from __future__ import annotations

import subprocess
import time
from pathlib import Path

import pytest

from helpers import terminate_process, write_wargus_preferences


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


@pytest.mark.gui
@pytest.mark.slow
def test_wargus_driver_launches_join_ip_menu(
    repo_root: Path,
    stratagus_bin: str,
    extracted_wargus_data: Path,
    xvfb_env,
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
        proc = subprocess.Popen(cmd, cwd=repo_root, env=xvfb_env, stdout=out, stderr=err)
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
    xvfb_env,
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
        proc = subprocess.Popen(cmd, cwd=repo_root, env=xvfb_env, stdout=out, stderr=err)
    try:
        time.sleep(5)
        assert proc.poll() is None, "Wargus exited before menu interaction"
        if xvfb_env.get("SDL_VIDEODRIVER") != "x11":
            pytest.skip("interactive click test requires an SDL build with the x11 video driver")

        # RunJoinIpMenu creates a 352x352 panel centered in a 640x480 window.
        # The Connect button is at local coordinates roughly (60, 180).
        click_screen(x=255, y=262, display=xvfb_env["DISPLAY"])
        time.sleep(2)

        assert proc.poll() is None, (
            f"Wargus crashed/exited after empty Connect click with {proc.returncode}\n"
            f"stdout:\n{stdout.read_text(errors='replace')}\n"
            f"stderr:\n{stderr.read_text(errors='replace')}"
        )
    finally:
        terminate_process(proc)
