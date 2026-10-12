import ok_subprocess_runner


def test_host_generic(request, tmp_path):
    binary_path = tmp_path / "host_generic_test.exe"
    ok_subprocess_runner.run(
        *["gcc", "-Wall", "-O", "-I.", "-I../../src"],
        *["-o", binary_path, "host_generic_test.cpp"],
        cwd=str(request.path.parent),
    )
