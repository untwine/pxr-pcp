import argparse
import contextlib
import difflib
import os
import shlex
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


def env_var(text):
    key, sep, value = text.partition("=")
    if not sep:
        raise argparse.ArgumentTypeError(f"invalid env var '{text}'")
    return key, value


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="USD test wrapper")
    parser.add_argument("--testenv-dir", type=Path)
    parser.add_argument("--diff-stdout", type=Path)
    parser.add_argument("--diff-compare", default=[], type=Path, action="append")
    parser.add_argument("--env-var", dest="env_vars", default=[], type=env_var, action="append")
    parser.add_argument("cmd", metavar="CMD", type=str)
    args = parser.parse_args()
    sys.stdout.reconfigure(line_buffering=True)

    # Test environments live next to this script, with their baselines inside
    testenv = Path(__file__).parent / args.testenv_dir if args.testenv_dir else None
    diffs = ([args.diff_stdout] if args.diff_stdout else []) + args.diff_compare
    if diffs and not testenv:
        sys.exit("Error: --testenv-dir is required to compare with baselines.")

    test_dir = Path(tempfile.mkdtemp())
    print(f"chdir: {test_dir}")
    os.chdir(test_dir)
    if testenv:
        print(f"copying testenv dir: {testenv}")
        shutil.copytree(testenv, test_dir, dirs_exist_ok=True)

    env = dict(os.environ, **dict(args.env_vars))
    # Avoid the just-in-time debugger where possible when running tests.
    env["ARCH_AVOID_JIT"] = "1"

    cmd = shlex.split(args.cmd)
    if cmd[0].endswith(".py"):
        cmd.insert(0, sys.executable)
    print(f"cmd: {cmd}")
    with open(args.diff_stdout, "w") if args.diff_stdout else contextlib.nullcontext() as stdout:
        retcode = subprocess.call(cmd, env=env, stdin=subprocess.DEVNULL, stdout=stdout)
    if retcode != 0:
        sys.exit(f"Error: {cmd[0]} returned {retcode} (kept {test_dir}).")

    for name in diffs:
        baseline = testenv / "baseline" / name
        print(f"diffing {name} with {baseline}")
        diff = list(difflib.unified_diff(
            baseline.read_text(encoding="utf-8", errors="surrogateescape").splitlines(keepends=True),
            Path(name).read_text(encoding="utf-8", errors="surrogateescape").splitlines(keepends=True),
            str(baseline), str(name)))
        if diff:
            sys.stdout.writelines(diff)
            sys.exit(f"Error: {name} differs from its baseline (kept {test_dir}).")

    os.chdir(Path(__file__).parent)
    shutil.rmtree(test_dir, ignore_errors=True)
