"""Tests generic rollover detection code by compiling with host gcc."""

from ok_subprocess_runner import run, stdout_lines


def test_generic(request, tmp_path, ok_test_checker):
    binary_path = tmp_path / "generic_hostcc_test.exe"
    run(
        *["gcc", "-Wall", "-Werror", "-O", "-I.", "-I../../src"],
        *["../../src/ok_clock64.cpp", "generic_hostcc_test.cpp"],
        *["-o", binary_path],
        cwd=request.path.parent,
    )

    for line in stdout_lines(binary_path):
        ok_test_checker.add_line(line)
    ok_test_checker.assert_success()
