from __future__ import annotations

import subprocess
from pathlib import Path
from typing import Any


def terminate_process(proc: subprocess.Popen, timeout: float = 5) -> None:
    if proc.poll() is not None:
        return
    proc.terminate()
    try:
        proc.wait(timeout=timeout)
    except subprocess.TimeoutExpired:
        proc.kill()
        proc.wait(timeout=timeout)


def _lua_value(value: Any) -> str:
    if isinstance(value, bool):
        return "true" if value else "false"
    if isinstance(value, (int, float)):
        return str(value)
    return f"{value!r}"


def write_wargus_preferences(user_dir: Path, preferences: dict[str, Any] | None = None) -> None:
    preferences_path = user_dir / "wc2" / "preferences.lua"
    preferences_path.parent.mkdir(parents=True, exist_ok=True)
    custom = preferences or {}
    extra = "".join(f"wc2.preferences.{key} = {_lua_value(value)}\n" for key, value in custom.items())
    preferences_path.write_text(
        "wc2 = wc2 or {}\n"
        "wc2.preferences = wc2.preferences or {}\n"
        "wc2.preferences.EnableMouseScrolling = false\n"
        "wc2.preferences.PauseOnLeave = false\n"
        "wc2.preferences.ShowTips = false\n"
        f"{extra}"
    )


def write_timeless_tales_preferences(user_dir: Path, preferences: dict[str, Any] | None = None) -> None:
    preferences_path = user_dir / "wc2" / "preferences.lua"
    preferences_path.parent.mkdir(parents=True, exist_ok=True)
    custom = preferences or {}
    extra = "".join(f"wc2.preferences.{key} = {_lua_value(value)}\n" for key, value in custom.items())
    preferences_path.write_text(
        "wc2 = wc2 or {}\n"
        "wc2.preferences = wc2.preferences or {}\n"
        "wc2.preferences.EnableMouseScrolling = false\n"
        "wc2.preferences.PauseOnLeave = false\n"
        "wc2.preferences.ShowTips = false\n"
        "wc2.preferences.MusicEnabled = false\n"
        "wc2.preferences.VideoFullScreen = false\n"
        "wc2.preferences.VideoHeight = 480\n"
        "wc2.preferences.VideoWidth = 640\n"
        f"{extra}"
    )


def write_war1gus_preferences(user_dir: Path, preferences: dict[str, Any] | None = None) -> None:
    preferences_path = user_dir / "wc1" / "preferences.lua"
    preferences_path.parent.mkdir(parents=True, exist_ok=True)
    custom = preferences or {}
    extra = "".join(f"wc1.preferences.{key} = {_lua_value(value)}\n" for key, value in custom.items())
    preferences_path.write_text(
        "wc1 = wc1 or {}\n"
        "wc1.preferences = wc1.preferences or {}\n"
        "wc1.preferences.EnableMouseScrolling = false\n"
        "wc1.preferences.MusicEnabled = false\n"
        "wc1.preferences.PauseOnLeave = false\n"
        "wc1.preferences.ShowTips = false\n"
        "wc1.preferences.VideoFullScreen = false\n"
        "wc1.preferences.VideoHeight = 480\n"
        "wc1.preferences.VideoWidth = 640\n"
        f"{extra}"
    )
