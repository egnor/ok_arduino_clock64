from asyncio import create_subprocess_exec, TaskGroup, wait_for
from asyncio.subprocess import PIPE, STDOUT
import shlex


async def test_esp8266(request, tmp_path, ok_test_checker):
    binary_path = tmp_path / "generic_hostcc_test.exe"
    compile_args = [
        *["gcc", "-Wall", "-Werror", "-O", "-I.", "-I../../src"],
        *["../../src/ok_clock64.cpp", "esp8266_hostcc_test.cpp"],
        *["-o", binary_path],
    ]

    print("\n⚒️", " ".join(shlex.quote(str(a)) for a in compile_args))
    test_dir = request.path.parent
    compile = await create_subprocess_exec(*compile_args, cwd=test_dir)
    assert (await compile.wait()) == 0, "compilation failed"

    print("▶️", binary_path)
    run = await create_subprocess_exec(binary_path, stdout=PIPE, stderr=STDOUT)
    try:
        async with TaskGroup() as tasks:
            tasks.create_task(wait_for(run.wait(), timeout=30))
            async for line in run.stdout:
                ok_test_checker.add_line(line)
            ok_test_checker.assert_success()
    finally:
        (run.returncode is None) and run.kill()
        await run.wait()
