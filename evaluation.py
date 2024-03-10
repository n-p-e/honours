#!/usr/bin/env python3

from pathlib import Path
import subprocess
import sys
import os
import io
import re

data_dir = Path(__file__).parent / "dataset"


def main():
    os.chdir(Path(__file__).parent)
    dataset_files = []
    data_dirs = sys.argv[1:]
    for dir in data_dirs:
        data_dir = Path(dir)
        for graph in data_dir.glob("**/edges.txt"):
            print(str(graph))
            dataset_files.append(graph)
    exe = compile()
    for dataset in dataset_files:
        try:
            evaluate(exe, dataset)
        except subprocess.TimeoutExpired:
            print(f"[Evaluate] timeout for dataset {dataset}")
            continue


def compile():
    print("[Compiling]")
    subprocess.run(["make", "rel"], stdout=sys.stdout)
    print("[Compiling] done")
    return "builddir-rel/main"


def evaluate(exe: Path, dataset_file: Path, algo="v2", k=2, timeout_sec=30):
    out = io.StringIO()
    dataset_name = str(dataset_file).removesuffix("/edges.txt")
    n, m = dataset_size(dataset_file)
    print(f"[Evaluate] Running on dataset {dataset_name} ({n=}, {m=})")
    process = subprocess.run(
        map(str, [exe, "-a", algo, "-k", k, "-g", dataset_file]),
        capture_output=True,
        encoding="utf-8",
        timeout=timeout_sec,
    )
    if process.returncode != 0:
        print("[Error]")
        print(process.stdout)
        print(process.stderr)
    else:
        output = str(process.stdout)
        # print(output)
        solution_size = None
        match = re.search(r"Found k-plex of size (\d+)", output, re.MULTILINE)
        if match is not None:
            solution_size = int(match.group(1))

        runtime_ms = None
        match = re.search(r"^\[timer\] ([\d\.]+) microseconds$", output, re.MULTILINE)
        if match is not None:
            runtime_ms = float(match.group(1)) / 1000

        if re.search(r"Found better solution", output, re.MULTILINE):
            print("Found better solution")
        print(f"[Evaluate] done, size = {solution_size} runtime = {runtime_ms:.3f}ms")


def dataset_size(file: Path):
    with open(file) as f:
        line = f.readline()
        n_vert, n_edges = map(int, line.split())
        return n_vert, n_edges


if __name__ == "__main__":
    main()
