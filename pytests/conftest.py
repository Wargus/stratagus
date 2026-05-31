from __future__ import annotations

import itertools
import json
import os
import shutil
import shlex
import subprocess
import time
from pathlib import Path

import pytest


def pytest_configure(config):
    config.addinivalue_line("markers", "cross: exercises host/client combinations from STRATAGUS_PARTICIPANTS")
    config.addinivalue_line("markers", "gui: requires a virtual display and game data")
    config.addinivalue_line("markers", "slow: launches external game processes")


def pytest_generate_tests(metafunc):
    participants = _stratagus_participants(Path(metafunc.config.rootpath))
    if "stratagus_pair" in metafunc.fixturenames:
        pairs = list(itertools.product(participants, repeat=2))
        ids = [f"{host['name']}-host__{client['name']}-client" for host, client in pairs]
        metafunc.parametrize("stratagus_pair", pairs, ids=ids)
    if "stratagus_player" in metafunc.fixturenames:
        ids = [f"{p['name']}-player" for p in participants]
        metafunc.parametrize("stratagus_player", participants, ids=ids)


@pytest.fixture(scope="session")
def repo_root() -> Path:
    return Path(__file__).resolve().parents[1]


def _existing_argv(argv: list[str]) -> bool:
    if not argv:
        return False
    executable = shutil.which(argv[0]) if len(argv) > 1 else None
    return Path(argv[-1]).exists() and (len(argv) == 1 or executable is not None)


def _absolute_existing_path(path: str) -> str:
    candidate = Path(path)
    return str(candidate.resolve()) if candidate.exists() else path


_CACHED_AUTO_PARTICIPANTS = None
def _stratagus_participants(repo_root: Path) -> list[dict[str, list[str] | str]]:
    configured = os.environ.get("STRATAGUS_PARTICIPANTS", "auto")
    if configured and configured != "auto":
        data = json.loads(configured)
        participants = []
        for entry in data:
            argv = entry.get("argv")
            if argv is None and entry.get("cmd"):
                argv = shlex.split(entry["cmd"])
            if not entry.get("name") or not argv:
                raise ValueError("STRATAGUS_PARTICIPANTS entries require name and argv/cmd")
            participants.append({"name": entry["name"], "argv": [str(arg) for arg in argv]})
        return participants

    if configured == "auto":
        global _CACHED_AUTO_PARTICIPANTS
        if _CACHED_AUTO_PARTICIPANTS:
            return _CACHED_AUTO_PARTICIPANTS
        candidates = [
            ("native-debug", [str(repo_root / "build" / "stratagus-dbg")]),
            ("native-debug-exe", [str(repo_root / "build" / "stratagus-dbg.exe")]),
            ("native-release", [str(repo_root / "build-release" / "stratagus")]),
            ("native-release-exe", [str(repo_root / "build-release" / "stratagus.exe")]),
            (
                "linux-aarch64",
                [
                    "qemu-aarch64",
                    "-L",
                    "/usr/aarch64-linux-gnu",
                    str(repo_root / "build-aarch64" / "stratagus"),
                ],
            ),
        ]
        if os.environ.get("STRATAGUS_INCLUDE_WINE", "1") == "1":
            wine = _wine_participant(repo_root)
            if wine:
                candidates.append(wine)
        participants = [{"name": name, "argv": argv} for name, argv in candidates if _existing_argv(argv)]
        if participants:
            _CACHED_AUTO_PARTICIPANTS = participants
            return participants

    fallback = os.environ.get("STRATAGUS_BIN") or str(repo_root / "build" / "stratagus-dbg")
    if not Path(fallback).exists():
        fallback = str(repo_root / "build" / "stratagus")
    if not Path(fallback).exists():
        fallback = shutil.which("stratagus") or fallback
    if Path(fallback).exists():
        return [{"name": "default", "argv": [_absolute_existing_path(fallback)]}]
    pytest.skip("Stratagus binary not found; set STRATAGUS_BIN or STRATAGUS_PARTICIPANTS")
    return []


def _wine_participant(repo_root: Path) -> tuple[str, list[str]] | None:
    exe = repo_root / "build-mingw64" / "stratagus.exe"
    if not exe.exists() or not shutil.which("wine"):
        return None

    prefix = repo_root / "build-mingw64" / "wine64-prefix"
    env_prefix = [f"WINEARCH=win64", f"WINEPREFIX={prefix}", "WINEDEBUG=-all"]
    try:
        subprocess.run(
            ["env", *env_prefix, "wineboot", "-u"],
            cwd=repo_root,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            timeout=120,
            check=True,
        )
    except (subprocess.CalledProcessError, subprocess.TimeoutExpired):
        return None

    return ("windows-mingw64", ["env", *env_prefix, "wine", str(exe)])


@pytest.fixture(scope="session")
def lua51(repo_root: Path) -> str:
    candidates = [
        os.environ.get("LUA51"),
        str(repo_root / "build" / "lua" / "src" / "lua-build" / "lua51"),
        shutil.which("lua5.1"),
        shutil.which("lua"),
    ]
    for candidate in candidates:
        if candidate and Path(candidate).exists():
            return _absolute_existing_path(candidate)
    pytest.skip("Lua 5.1 interpreter not found; set LUA51 or build vendored Lua")
    return ""


@pytest.fixture(scope="session")
def stratagus_bin(repo_root: Path) -> str:
    candidates = [
        os.environ.get("STRATAGUS_BIN"),
        str(repo_root / "build" / "stratagus-dbg"),
        str(repo_root / "build" / "stratagus"),
        shutil.which("stratagus"),
    ]
    for candidate in candidates:
        if candidate and Path(candidate).exists():
            return candidate
    pytest.skip("Stratagus binary not found; set STRATAGUS_BIN or build the stratagus target")
    return ""


def _has_asset_markers(data_dir: Path) -> bool:
    return any((data_dir / marker).exists() for marker in ("graphics", "sounds", "music"))


def _run_or_fail(cmd: list[str], *, cwd: Path, what: str) -> None:
    proc = subprocess.run(cmd, cwd=cwd, text=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    if proc.returncode == 0:
        return
    output = proc.stdout[-4000:]
    pytest.fail(f"{what} failed with exit code {proc.returncode}\nCommand: {' '.join(cmd)}\n{output}")


def _file_signature(path: Path) -> str:
    stat = path.stat()
    return f"{path.resolve()}\n{stat.st_size}\n{stat.st_mtime_ns}\n"


def _unpack_iso(repo_root: Path, iso: Path, output_dir: Path) -> Path | None:
    seven_zip = shutil.which("7z") or shutil.which("7zz")
    if not seven_zip:
        pytest.skip("7z/7zz not found; cannot unpack game ISO")
        return

    marker = output_dir / ".source-iso"
    signature = _file_signature(iso)
    if marker.exists() and marker.read_text() == signature:
        return output_dir

    shutil.rmtree(output_dir, ignore_errors=True)
    output_dir.mkdir(parents=True, exist_ok=True)
    _run_or_fail([seven_zip, "x", "-y", f"-o{output_dir}", str(iso)], cwd=repo_root, what=f"unpacking {iso}")
    marker.write_text(signature)
    return output_dir


def _dependency_cache_args(repo_root: Path) -> list[str]:
    args = []
    deps = {
        "PNG_LIBRARY": repo_root / "build" / "png" / "src" / "png-build" / "libpng16.a",
        "PNG_PNG_INCLUDE_DIR": repo_root / "build" / "png" / "src" / "png-build",
        "ZLIB_LIBRARY": repo_root / "build" / "zlib" / "src" / "zlib-build" / "libz.a",
        "ZLIB_INCLUDE_DIR": repo_root / "build" / "zlib" / "src" / "zlib-build",
    }
    for name, path in deps.items():
        if path.exists():
            args.append(f"-D{name}={path}")
    return args


def _build_wargus_stormlib(repo_root: Path) -> Path:
    library = repo_root / "games" / "wargus" / "build-storm" / "libstorm.a"
    if library.exists():
        return library

    source_dir = repo_root / "games" / "wargus" / "StormLib"
    build_dir = repo_root / "games" / "wargus" / "build-storm"
    bzip_library = repo_root / "build" / "bzip2" / "src" / "bzip2-build" / "libbz2.a"
    bzip_include = source_dir / "src" / "bzip2"

    args = [
        "cmake",
        "-S",
        str(source_dir),
        "-B",
        str(build_dir),
        "-DCMAKE_BUILD_TYPE=Debug",
        "-DSTORM_SKIP_INSTALL=ON",
        *_dependency_cache_args(repo_root),
    ]
    if bzip_library.exists() and bzip_include.exists():
        args.extend([f"-DBZIP2_LIBRARIES={bzip_library}", f"-DBZIP2_INCLUDE_DIR={bzip_include}"])

    _run_or_fail(args, cwd=repo_root, what="configuring StormLib")
    _run_or_fail(["cmake", "--build", str(build_dir), "--config", "Debug"], cwd=repo_root, what="building StormLib")
    return library


def _build_game_tool(repo_root: Path, game: str, tool: str, stratagus_bin: str) -> Path:
    override = os.environ.get(f"{tool.upper()}_BIN")
    if override and Path(override).exists():
        return Path(override)

    game_dir = repo_root / "games" / game
    tool_path = game_dir / "build" / tool
    if tool_path.exists():
        return tool_path

    if not game_dir.exists():
        pytest.skip(f"{game} source tree not found under games/")

    args = [
        "cmake",
        "-S",
        str(game_dir),
        "-B",
        str(game_dir / "build"),
        "-DCMAKE_BUILD_TYPE=Debug",
        f"-DSTRATAGUS={stratagus_bin}",
        f"-DSTRATAGUS_INCLUDE_DIR={repo_root / 'gameheaders'}",
        *_dependency_cache_args(repo_root),
    ]
    if game == "wargus":
        stormlib = _build_wargus_stormlib(repo_root)
        bzip_library = repo_root / "build" / "bzip2" / "src" / "bzip2-build" / "libbz2.a"
        bzip_include = repo_root / "games" / "wargus" / "StormLib" / "src" / "bzip2"
        args.extend([f"-DSTORMLIB_LIBRARY={stormlib}", f"-DSTORMLIB_INCLUDE_DIR={game_dir / 'StormLib' / 'src'}"])
        if bzip_library.exists() and bzip_include.exists():
            args.extend([f"-DBZIP2_LIBRARIES={bzip_library}", f"-DBZIP2_INCLUDE_DIR={bzip_include}"])

    _run_or_fail(args, cwd=repo_root, what=f"configuring {tool}")
    _run_or_fail(
        ["cmake", "--build", str(game_dir / "build"), "--target", tool, "--config", "Debug"],
        cwd=repo_root,
        what=f"building {tool}",
    )
    return tool_path


def _find_iso(repo_root: Path, env_name: str, names: tuple[str, ...]) -> Path | None:
    override = os.environ.get(env_name)
    candidates = [Path(override)] if override else []
    candidates.extend(repo_root / "games" / name for name in names)
    for candidate in candidates:
        if candidate.exists():
            return candidate
    pytest.skip(f"Game ISO not found; set {env_name} or place one of {', '.join(names)} in games/")
    return None


def _extract_game_data(
    repo_root: Path,
    *,
    game: str,
    tool: str,
    iso_env: str,
    iso_names: tuple[str, ...],
    stratagus_bin: str,
) -> Path:
    data_dir = repo_root / "games" / ".pytest-data" / f"{game}-data"
    if _has_asset_markers(data_dir):
        _overlay_game_source(repo_root, game, data_dir)
        return data_dir

    iso = _find_iso(repo_root, iso_env, iso_names)
    assert iso
    iso_dir = _unpack_iso(repo_root, iso, repo_root / "games" / ".pytest-data" / f"{game}-iso")
    tool_path = _build_game_tool(repo_root, game, tool, stratagus_bin)

    shutil.rmtree(data_dir, ignore_errors=True)
    data_dir.mkdir(parents=True, exist_ok=True)
    _run_or_fail([str(tool_path), str(iso_dir), str(data_dir)], cwd=repo_root, what=f"extracting {game} data")
    _overlay_game_source(repo_root, game, data_dir)
    if not _has_asset_markers(data_dir):
        pytest.fail(f"{tool} completed but did not create expected assets in {data_dir}")
    return data_dir


def _overlay_game_source(repo_root: Path, game: str, data_dir: Path) -> None:
    source_dir = repo_root / "games" / game
    if not source_dir.exists():
        return

    if game == "wargus":
        _copy_wargus_runtime_data(source_dir, data_dir)
    elif game == "war1gus":
        _copy_runtime_dirs(source_dir, data_dir, ("campaigns", "contrib", "maps", "scripts", "shaders"))


def _copy_runtime_dirs(source_dir: Path, data_dir: Path, names: tuple[str, ...]) -> None:
    for name in names:
        source = source_dir / name
        if source.exists():
            shutil.copytree(source, data_dir / name, dirs_exist_ok=True)


def _copy_wargus_runtime_data(source_dir: Path, data_dir: Path) -> None:
    # Match wargus.cpp's CONTRIB_DIRECTORIES and CMake install rules so tests use
    # the same composed data tree as the launcher/package, not raw extractor output.
    _copy_runtime_dirs(source_dir, data_dir, ("campaigns", "maps", "scripts", "shaders"))
    contrib = source_dir / "contrib"
    if contrib.exists():
        shutil.copytree(contrib, data_dir / "graphics" / "ui", dirs_exist_ok=True)

    for source_name, target in (
        ("red_cross.png", data_dir / "graphics" / "missiles" / "red_cross.png"),
        ("cross.png", data_dir / "graphics" / "ui" / "cursors" / "cross.png"),
    ):
        source = contrib / source_name
        if source.exists():
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)


@pytest.fixture(scope="session")
def wargus_data_dir(repo_root: Path, stratagus_bin: str) -> Path:
    data_dir = Path(os.environ.get("WARGUS_DATA_DIR", repo_root / "games" / "wargus"))
    if not data_dir.exists():
        pytest.skip(f"Wargus data directory not found: {data_dir}")
    if os.environ.get("WARGUS_ALLOW_SOURCE_DATA") == "1":
        return data_dir
    if not _has_asset_markers(data_dir) and not os.environ.get("WARGUS_DATA_DIR"):
        return _extract_game_data(
            repo_root,
            game="wargus",
            tool="wartool",
            iso_env="WARGUS_ISO",
            iso_names=("WAR2BNECD.ISO", "WAR2.ISO", "WARCRAFT2.ISO"),
            stratagus_bin=stratagus_bin,
        )
    return data_dir


@pytest.fixture(scope="session")
def extracted_wargus_data(wargus_data_dir: Path) -> Path:
    if os.environ.get("WARGUS_ALLOW_SOURCE_DATA") == "1":
        return wargus_data_dir
    if not _has_asset_markers(wargus_data_dir):
        pytest.skip(
            "Extracted Wargus data not found; set WARGUS_DATA_DIR or "
            "WARGUS_ALLOW_SOURCE_DATA=1 for source-tree smoke tests"
        )
    return wargus_data_dir


@pytest.fixture(scope="session")
def extracted_war1gus_data(repo_root: Path, stratagus_bin: str) -> Path:
    data_dir = os.environ.get("WAR1GUS_DATA_DIR")
    if data_dir:
        path = Path(data_dir)
        if _has_asset_markers(path):
            return path
        pytest.skip(f"Extracted War1gus data not found in {path}")
    return _extract_game_data(
        repo_root,
        game="war1gus",
        tool="war1tool",
        iso_env="WAR1GUS_ISO",
        iso_names=("WAR1.ISO", "WARCRAFT.ISO"),
        stratagus_bin=stratagus_bin,
    )


@pytest.fixture(scope="session")
def timeless_tales_data(repo_root: Path) -> Path:
    data_dir = Path(os.environ.get("TIMELESS_TALES_DATA_DIR", repo_root / "games" / "timeless-tales"))
    if not data_dir.exists():
        pytest.skip(f"Timeless Tales data directory not found: {data_dir}")
    data_dir = data_dir.resolve()
    if not _has_asset_markers(data_dir) or not (data_dir / "scripts" / "stratagus.lua").exists():
        pytest.skip(f"Timeless Tales data directory is incomplete: {data_dir}")
    return data_dir


@pytest.fixture
def gui_env(stratagus_pair: tuple[dict, dict], tmp_path: Path):
    if os.environ.get("WARGUS_GUI_TESTS", "1") != "1":
        # Explicit GUI tests skip
        pytest.skip("Set WARGUS_GUI_TESTS=1 to run GUI tests")
        return

    env = os.environ.copy()
    video_driver = os.environ.get("PYTEST_SDL_VIDEODRIVER")
    proc = None

    if video_driver == "xvfb":
        env["SDL_VIDEODRIVER"] = "x11"
        xvfb = shutil.which("Xvfb")
        if not xvfb:
            pytest.skip("Xvfb not found")
            return
        display = os.environ.get("PYTEST_XVFB_DISPLAY", f":{90 + (os.getpid() % 9)}")
        env["DISPLAY"] = display
        log = tmp_path / "xvfb.log"
        with log.open("wb") as out:
            proc = subprocess.Popen(
                [xvfb, display, "-screen", "0", "1024x768x24"],
                stdout=out,
                stderr=subprocess.STDOUT,
            )
        time.sleep(0.5)
        if proc.poll() is not None:
            pytest.fail(f"Xvfb exited early; see {log}")
    elif not (os.environ.get("WAYLAND_DISPLAY") or os.environ.get("DISPLAY")):
        env["SDL_VIDEODRIVER"] = "dummy"
    elif any(any(a.startswith("qemu") for a in p["argv"]) for p in stratagus_pair):
        env["SDL_VIDEODRIVER"] = "dummy"

    env["SDL_AUDIODRIVER"] = "dummy"
    env["SDL_VIDEO_WINDOW_POS"] = "0,0"
    try:
        yield env
    finally:
        if proc:
            proc.terminate()
            try:
                proc.wait(timeout=5)
            except subprocess.TimeoutExpired:
                proc.kill()
                proc.wait(timeout=5)
