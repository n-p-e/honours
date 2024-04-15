#!/usr/bin/env python3

import concurrent.futures as futures
import time
from datetime import datetime
import json
import os
import re
import subprocess
import sys
import traceback
from pathlib import Path
from typing import Optional, TypedDict

import pandas as pd

data_dir = Path(__file__).parent / "dataset"
logs_dir = Path(__file__).parent / "evaluation" / "logs"
results_dir = Path(__file__).parent / "evaluation" / "results"
algo = "naive"
k = 5
alpha = 0.9
prog = "quasi"
timeout_sec = 300
max_workers = 8
exact_prog = Path(__file__).parent.parent / "Maximum-kPlex" / "kPlexS"

run_date = datetime.utcnow().strftime("%Y%m%d_%H%M%S")
out_path = results_dir / f"output_{run_date}.json"


class Evaluation(TypedDict):
    dataset: str
    n: int
    m: int
    algo: str
    k: int
    alpha: float
    runtime_ms: float | None
    initial_size: int | None
    solution_size: int | None
    improved_solution: bool
    exact_size: int | None
    exact_runtime_ms: float | None


def main():
    os.makedirs(logs_dir, exist_ok=True)
    os.makedirs(results_dir, exist_ok=True)
    action = sys.argv[1] if len(sys.argv) > 1 else "evaluation_loop"
    if action == "report":
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
                try:
                    result = evaluate(
                        exe,
                        dataset,
                        prog=prog,
                        algo=algo,
                        k=k,
                        timeout_sec=timeout_sec,
                    )
                except Exception:
                    traceback.print_exc()
                results.append(result)

            future = pool.submit(task, dataset)
            futs.append(future)
        futures.wait(futs)
    finally:
        with open(out_path, "w") as outfile:
            json.dump(results, outfile)
            outfile.write("\n")
        pool.shutdown(wait=False)


def compile():
    print("[Compiling]")
    subprocess.run(["make", "rel"], stdout=sys.stdout)
    print("[Compiling] done")
    return "builddir-rel/main"


def evaluate(
    exe: Path,
    dataset_file: Path,
    prog="kplex",
    algo="v2",
    k: int = 5,
    alpha: float = 0.9,
    timeout_sec=30,
) -> Optional[Evaluation]:
    dataset_name = str(dataset_file).removesuffix("/edges.txt")
    n, m = dataset_size(dataset_file)
    print(f"[Evaluate] Running on dataset {dataset_name} ({n=}, {m=})")
    stdout_path = (
        logs_dir / f"out-{dataset_name.rsplit('/', 1)[-1]}-{int(time.time())}.txt"
    )
    try:
        with open(stdout_path, "ab+") as stdout:
            process = subprocess.run(
                map(
                    str,
                    [
                        exe,
                        "-p",
                        prog,
                        "-a",
                        algo,
                        "-k",
                        k,
                        "--alpha",
                        alpha,
                        "-g",
                        dataset_file,
                    ],
                ),
                stdout=stdout,
                stderr=subprocess.STDOUT,
                timeout=timeout_sec,
            )
    except subprocess.TimeoutExpired:
        print(f"[Evaluate] timeout for dataset {dataset_name}")
        return Evaluation(
            dataset=dataset_name,
            algo=algo,
            n=n,
            m=m,
            k=k,
            alpha=alpha,
            runtime_ms=None,
            improved_solution=False,
        )
    if process.returncode != 0:
        print(f"[Error] see ${str(stdout_path)}")
        return None
    with open(stdout_path, "r", encoding="utf-8") as stdout:
        output = stdout.read()
    initial_size = None
    solution_size = None
    match = re.search(r"Initial solution size = (\d+)", output, re.MULTILINE)
    if match is not None:
        initial_size = int(match.group(1))
    match = re.search(r"Result size = (\d+)", output, re.MULTILINE)
    if match is not None:
        solution_size = int(match.group(1))
    print("solution size", solution_size)
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
            exact_output = str(exact_process.stdout)
        except subprocess.TimeoutExpired:
            exact_output = ""
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
        k=k,
        alpha=alpha,
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
