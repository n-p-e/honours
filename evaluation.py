#!/usr/bin/env python3

import concurrent.futures as futures
import io
import json
import os
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path
from typing import Optional, TypedDict

import pandas as pd

data_dir = Path(__file__).parent / "dataset"
algo = "v3"
k = 5
prog = "kdef"
timeout_sec = 300
max_workers = 8
exact_prog = Path(__file__).parent.parent / "Maximum-kPlex" / "kPlexS"


class Evaluation(TypedDict):
    dataset: str
    n: int
    m: int
    algo: str
    runtime_ms: float | None
    initial_size: int | None
    solution_size: int | None
    improved_solution: bool
    exact_size: int | None
    exact_runtime_ms: float | None


def main():
    if len(sys.argv) > 1 and sys.argv[1] == "report":
        report()
    else:
        evaluation_loop()


def evaluation_loop():
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
    futs = []

    pool = futures.ThreadPoolExecutor(max_workers=max_workers)
    try:
        for dataset in dataset_files:

            def task(dataset):
                result = evaluate(
                    exe, dataset, prog=prog, algo=algo, k=k, timeout_sec=timeout_sec,
                )
                results.append(result)

            future = pool.submit(task, dataset)
            futs.append(future)
        futures.wait(futs)
    finally:
        with open("output.json", "w") as outfile:
            json.dump(results, outfile)
        pool.shutdown(wait=False)


def compile():
    print("[Compiling]")
    subprocess.run(["make", "rel"], stdout=sys.stdout)
    print("[Compiling] done")
    return "builddir-rel/main"


def evaluate(
    exe: Path, dataset_file: Path, prog="kplex", algo="v2", k: int=5, timeout_sec=30
) -> Optional[Evaluation]:
    dataset_name = str(dataset_file).removesuffix("/edges.txt")
    n, m = dataset_size(dataset_file)
    print(f"[Evaluate] Running on dataset {dataset_name} ({n=}, {m=})")
    try:
        process = subprocess.run(
            map(str, [exe, "-p", prog, "-a", algo, "-k", k, "-g", dataset_file]),
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
        return None
    output = str(process.stdout)
    # print(output)
    initial_size = None
    solution_size = None
    match = re.search(r"Initial solution size = (\d+)", output, re.MULTILINE)
    if match is not None:
        initial_size = int(match.group(1))
    match = re.search(
        (
            r"Found k-plex of size (\d+)"
            if prog == "kplex"
            else r"found k-defective-clique of size (\d+)"
        ),
        output,
        re.MULTILINE,
    )
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

    exact_size = None
    exact_runtime_ms = None
    if prog == "kplex":
        try:
            exact_process = subprocess.run(
                map(str, [exact_prog, "-g", dataset_name, "-k", k]),
                capture_output=True,
                encoding="utf-8",
                timeout=timeout_sec,
            )
        except subprocess.TimeoutExpired:
            pass
        exact_output = str(exact_process.stdout)
        match = re.search(
            r"Maximum kPlex Size: (\d+), Total Time: ([\d\.,]+)",
            exact_output,
            re.MULTILINE,
        )
        if match:
            exact_size = int(match.group(1))
            exact_runtime_ms = float(match.group(2).replace(",", "")) / 1000
    print(
        f"[Evaluate] done, size={solution_size} runtime={runtime_ms:.3f}ms exact_size={exact_size}"
    )

    return Evaluation(
        dataset=dataset_name,
        algo=algo,
        n=n,
        m=m,
        runtime_ms=runtime_ms,
        improved_solution=improved_solution,
        initial_size=initial_size,
        solution_size=solution_size,
        exact_size=exact_size,
        exact_runtime_ms=exact_runtime_ms,
    )


def dataset_size(file: Path):
    with open(file) as f:
        line = f.readline()
        n_vert, n_edges = map(int, line.split())
        return n_vert, n_edges


def report():
    print("Report")
    report_path = sys.argv[2]
    df = pd.read_json(
        report_path, dtype={"initial_size": "int", "solution_size": "int"}
    )
    improved = df[df["improved_solution"]]
    print(
        f"({len(improved)}/{len(df)}) {len(improved) / len(df)  * 100:.2f}% of solutions improved"
    )
    timed_out = df[df["solution_size"].isna()]
    exact_timed_out = df[df["exact_size"].isna()]
    print(f"{len(timed_out)} timed out | {len(exact_timed_out)} exact timed out")
    same_as_exact = df[
        df["solution_size"].eq(df["exact_size"]) & df["solution_size"].notna()
    ]
    df["exact_diff"] = df["exact_size"] - df["solution_size"]
    print(f"{len(same_as_exact)} same as exact")
    print(df.sort_values(by="exact_diff", ascending=False).head(20))
    print("\n")
    print(df.describe())


if __name__ == "__main__":
    main()
