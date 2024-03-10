#!/usr/bin/env python3

import io
import json
import os
import re
import subprocess
import sys
from pathlib import Path
from typing import TypedDict

data_dir = Path(__file__).parent / "dataset"
algo = "v3"
timeout_sec = 120


class Evaluation(TypedDict):
    dataset: str
    n: int
    m: int
    algo: str
    runtime_ms: float | None
    initial_size: int | None
    solution_size: int | None
    improved_solution: bool


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
    results: list[Evaluation] = []

    try:
        for dataset in dataset_files:
            result = evaluate(exe, dataset, algo=algo, timeout_sec=timeout_sec)
            results.append(result)
    finally:
        with open("output.json", "w") as outfile:
            json.dump(results, outfile)


def compile():
    print("[Compiling]")
    subprocess.run(["make", "rel"], stdout=sys.stdout)
    print("[Compiling] done")
    return "builddir-rel/main"


def evaluate(
    exe: Path, dataset_file: Path, algo="v2", k=2, timeout_sec=30
) -> Evaluation:
    dataset_name = str(dataset_file).removesuffix("/edges.txt")
    n, m = dataset_size(dataset_file)
    print(f"[Evaluate] Running on dataset {dataset_name} ({n=}, {m=})")
    try:
        process = subprocess.run(
            map(str, [exe, "-a", algo, "-k", k, "-g", dataset_file]),
            capture_output=True,
            encoding="utf-8",
            timeout=timeout_sec,
        )
    except subprocess.TimeoutExpired:
        print(f"[Evaluate] timeout for dataset {dataset_name}")
        return Evaluation(
            dataset=dataset_name,
            algo=algo,
            n=n,
            m=m,
            runtime_ms=None,
            improved_solution=False,
        )
    if process.returncode != 0:
        print("[Error]")
        print(process.stdout)
        print(process.stderr)
    else:
        output = str(process.stdout)
        # print(output)
        initial_size = None
        solution_size = None
        match = re.search(r"Initial solution size = (\d+)", output, re.MULTILINE)
        if match is not None:
            initial_size = int(match.group(1))
        match = re.search(r"Found k-plex of size (\d+)", output, re.MULTILINE)
        if match is not None:
            solution_size = int(match.group(1))

        runtime_ms = None
        match = re.search(r"^\[timer\] ([\d\.]+) microseconds$", output, re.MULTILINE)
        if match is not None:
            runtime_ms = float(match.group(1)) / 1000

        improved_solution = False
        if initial_size != solution_size:
            print("Found better solution")
            improved_solution = True
        print(f"[Evaluate] done, size = {solution_size} runtime = {runtime_ms:.3f}ms")
        return Evaluation(
            dataset=dataset_name,
            algo=algo,
            n=n,
            m=m,
            runtime_ms=runtime_ms,
            improved_solution=improved_solution,
            initial_size=initial_size,
            solution_size=solution_size,
        )


def dataset_size(file: Path):
    with open(file) as f:
        line = f.readline()
        n_vert, n_edges = map(int, line.split())
        return n_vert, n_edges


if __name__ == "__main__":
    main()
