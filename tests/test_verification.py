"""Discriminate missing or silently skipped runtime evidence."""

from __future__ import annotations

import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "tools"))
from verification import REQUIRED_TESTS, check_results, compiler_pair


class VerificationTests(unittest.TestCase):
    def test_native_compilers(self) -> None:
        self.assertEqual(compiler_pair("Windows"), ("clang-cl", "clang-cl"))
        self.assertEqual(compiler_pair("Darwin"), ("/usr/bin/clang", "/usr/bin/clang++"))
        with self.assertRaisesRegex(ValueError, "No Alchemy"):
            compiler_pair("Android")

    def test_results_require_executed_product_tests(self) -> None:
        scratch = Path(__file__).resolve().parents[1] / "scratch"
        scratch.mkdir(exist_ok=True)
        with tempfile.TemporaryDirectory(dir=scratch) as directory:
            report = Path(directory) / "results.xml"
            passing = "".join(f'<testcase name="{name}"/>' for name in sorted(REQUIRED_TESTS))
            report.write_text(
                f'<testsuite>{passing}<testcase name="igb_image_real">'
                "<skipped/></testcase></testsuite>",
                encoding="utf-8",
            )
            self.assertEqual(check_results(report), len(REQUIRED_TESTS) + 1)
            report.write_text("<testsuite/>", encoding="utf-8")
            with self.assertRaisesRegex(ValueError, "omitted required tests"):
                check_results(report)
            for result, error in (("skipped", "was skipped"), ("failure", "failure")):
                broken = passing.replace(
                    '<testcase name="input"/>', f'<testcase name="input"><{result}/></testcase>'
                )
                report.write_text(f"<testsuite>{broken}</testsuite>", encoding="utf-8")
                with self.assertRaisesRegex(ValueError, error):
                    check_results(report)


if __name__ == "__main__":
    unittest.main()
