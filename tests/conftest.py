import dataclasses
import logging
import ok_logging_setup
import pytest
import subprocess

ok_logging_setup.install({"OK_LOGGING_OUTPUT": "stdout"})


@dataclasses.dataclass
class OkTestChecker:
    """Accumulates and checks output from an embedded firmware test run."""

    lines: list[str] = dataclasses.field(default_factory=list)
    failures: list[str] = dataclasses.field(default_factory=list)
    started: bool = False
    ended: bool = False

    def add_line(self, line: bytes | str):
        """Incorporates an output line, returns True if tests are complete.
        Raises AssertionError if tests aborted or completed with failures.
        """

        text = (line.decode() if isinstance(line, bytes) else line).strip()
        logging.info("· %s", text)
        assert "#ABORT-TESTS#" not in text
        if "#BEGIN-TESTS#" in text:
            assert not self.started, "Extra #BEGIN-TESTS#: {text}"
            self.started = True
        if self.started and not self.ended:
            self.lines.append(text)
        if "#TEST-FAIL#" in text:
            self.failures.append(text)  # defer reporting until #END-TESTS#
        if "#END-TESTS#" in text:
            assert not self.ended, f"Extra #END-TESTS#: {text}"
            self.ended = True
            self.assert_success()
        return self.ended

    def assert_success(self):
        """Raises AssertionError if tests failed or never completed."""

        assert self.started, "No #BEGIN-TESTS# seen in output"
        assert self.ended, "No #END-TESTS# seen in output"
        assert not self.failures, (
            f"{len(self.failures)} failed\n  {'\n  '.join(self.failures)}"
        )
        return True


@pytest.fixture
def ok_test_checker() -> OkTestChecker:
    return OkTestChecker()
