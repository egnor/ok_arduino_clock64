"""Tests that a clock64-using sketch builds for a variety of targets."""

from ok_subprocess_runner import run
from pathlib import Path
import yaml


def test_build_only(profile, request, tmp_path):
    run(
        "arduino-cli",
        "compile",
        f"--build-path={tmp_path}",
        f"--profile={profile}",
        "--warnings=all",
        cwd=request.path.parent,
    )


def pytest_generate_tests(metafunc):
    """Gets the list of target profiles from sketch.yaml."""

    sketch_yaml_path = Path(__file__).parent / "sketch.yaml"
    sketch_yaml = yaml.safe_load(sketch_yaml_path.read_bytes())
    metafunc.parametrize("profile", sketch_yaml["profiles"].keys())
