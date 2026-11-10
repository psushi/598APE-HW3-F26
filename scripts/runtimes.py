#!/usr/bin/env python3
import argparse
import hashlib
from pathlib import Path
import statistics
import subprocess
import time

parser = argparse.ArgumentParser(description="Measure whole-process sandpile runtimes.")
parser.add_argument("N", type=int, nargs="?", default=1001)
parser.add_argument("--binary", type=Path, default=Path("sandpile.exe"))
parser.add_argument("--warmups", type=int, default=2)
parser.add_argument("--runs", type=int, default=5)
parser.add_argument("--reference", type=Path, help="PNG to compare against")
args = parser.parse_args()
if args.N < 1 or args.warmups < 0 or args.runs < 1:
    parser.error("N and runs must be positive, and warmups must be nonnegative")
binary = args.binary.resolve()
if not binary.is_file():
    parser.error(f"binary not found: {binary}; build it first")

expected = args.reference.read_bytes() if args.reference else None
times = []
for index in range(args.warmups + args.runs):
    start = time.perf_counter()
    subprocess.run([str(binary), str(args.N)], check=True, stdout=subprocess.PIPE)
    elapsed = time.perf_counter() - start
    output = Path("identity.png").read_bytes()
    if expected is None:
        expected = output
    if output != expected:
        raise SystemExit("PNG differs from the reference or a previous run")
    if index < args.warmups:
        print(f"warmup {index + 1}: {elapsed:.6f} s", flush=True)
    else:
        times.append(elapsed)
        print(f"run {len(times)}: {elapsed:.6f} s", flush=True)

print(f"median: {statistics.median(times):.6f} s")
print(f"PNG SHA256: {hashlib.sha256(expected).hexdigest()}")
