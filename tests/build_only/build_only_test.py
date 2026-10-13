"""Tests that a clock64-using sketch builds for a variety of targets."""

import filelock
import ok_subprocess_runner
from pathlib import Path
import yaml


def test_build_only(profile, request, tmp_path):
    # isolate arduino platform data between tests to prevent test conflicts
    # keep platform data in-tree to avoid filling /tmp or requiring re-downloads
    # keep in parent dir otherwise arduino-cli copies it with the sketch
    test_dir = request.path.parent
    arduino_dir = test_dir.parent / f"{test_dir.name}.data.tmp" / profile
    arduino_dir.mkdir(parents=True, exist_ok=True)
    runner = ok_subprocess_runner.SubprocessRunner(
        cwd=test_dir, env={"ARDUINO_DIRECTORIES_DATA": arduino_dir}
    )
    with filelock.FileLock(arduino_dir / "lock"):
        runner(
            "arduino-cli",
            "compile",
            f"--build-path={tmp_path}",
            f"--profile={profile}",
            "--warnings=all",
        )


def pytest_generate_tests(metafunc):
    """Gets the list of target profiles from sketch.yaml."""

    sketch_yaml_path = Path(__file__).parent / "sketch.yaml"
    sketch_yaml = yaml.safe_load(sketch_yaml_path.read_bytes())
    metafunc.parametrize("profile", sketch_yaml["profiles"].keys())
