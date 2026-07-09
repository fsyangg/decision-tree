import argparse
import json
import platform
import statistics
import sys
import time
from pathlib import Path
from typing import Callable

import numpy as np
import sklearn
from sklearn.tree import DecisionTreeClassifier

from datasets import BY_NAME, DATASETS


def autorange(fn: Callable[[], object], min_batch: float, min_total: float, min_reps: int) -> list[float]:
    fn()
    batch = 1
    while True:
        elapsed = time_batch(fn, batch)
        if elapsed >= min_batch or batch >= 1 << 22:
            break
        batch *= 2

    samples = []
    deadline = time.perf_counter() + min_total
    while len(samples) < min_reps or time.perf_counter() < deadline:
        samples.append(time_batch(fn, batch) / batch)
    return samples


def time_batch(fn: Callable[[], object], batch: int) -> float:
    start = time.perf_counter()
    for _ in range(batch):
        fn()
    return time.perf_counter() - start


def make_classifier(args: argparse.Namespace) -> DecisionTreeClassifier:
    return DecisionTreeClassifier(
        criterion=args.criterion,
        max_depth=args.max_depth,
        min_samples_split=args.min_samples_split,
        min_samples_leaf=args.min_samples_leaf,
        random_state=0,
    )


def run(dataset, args: argparse.Namespace) -> dict:
    X, y = dataset.load()

    fit_samples = autorange(lambda: make_classifier(args).fit(X, y), args.min_batch, args.min_total, args.min_reps)

    model = make_classifier(args).fit(X, y)
    predict_samples = autorange(lambda: model.predict(X), args.min_batch, args.min_total, args.min_reps)

    accuracy = float((model.predict(X) == y).mean())
    return {
        "dataset": dataset.name,
        "rows": int(X.shape[0]),
        "features": int(X.shape[1]),
        "classes": int(len(np.unique(y))),
        "fit_ms_min": min(fit_samples) * 1e3,
        "fit_ms_median": statistics.median(fit_samples) * 1e3,
        "predict_ms_min": min(predict_samples) * 1e3,
        "predict_ms_median": statistics.median(predict_samples) * 1e3,
        "predict_ns_per_row": min(predict_samples) * 1e9 / X.shape[0],
        "train_accuracy": accuracy,
        "node_count": int(model.tree_.node_count),
        "depth": int(model.get_depth()),
    }


COLUMNS = [
    ("dataset", "dataset", 10, ""),
    ("rows", "rows", 6, ""),
    ("features", "feats", 6, ""),
    ("classes", "cls", 4, ""),
    ("fit_ms_min", "fit ms", 10, ".4f"),
    ("predict_ms_min", "pred ms", 10, ".4f"),
    ("predict_ns_per_row", "pred ns/row", 12, ".1f"),
    ("train_accuracy", "train acc", 10, ".4f"),
    ("node_count", "nodes", 6, ""),
    ("depth", "depth", 6, ""),
]


def print_table(results: list[dict]) -> None:
    header = " ".join(f"{label:>{width}}" for _, label, width, _ in COLUMNS)
    print(header)
    print("-" * len(header))
    for row in results:
        print(" ".join(f"{row[key]:>{width}{spec}}" for key, _, width, spec in COLUMNS))


def main() -> int:
    parser = argparse.ArgumentParser(description="scikit-learn decision tree baseline")
    parser.add_argument("datasets", nargs="*", default=[d.name for d in DATASETS])
    parser.add_argument("--criterion", default="gini", choices=["gini", "entropy", "log_loss"])
    parser.add_argument("--max-depth", type=int, default=None)
    parser.add_argument("--min-samples-split", type=int, default=2)
    parser.add_argument("--min-samples-leaf", type=int, default=1)
    parser.add_argument("--min-batch", type=float, default=0.02)
    parser.add_argument("--min-total", type=float, default=0.5)
    parser.add_argument("--min-reps", type=int, default=7)
    parser.add_argument("--json", type=str, default=None)
    args = parser.parse_args()

    unknown = [name for name in args.datasets if name not in BY_NAME]
    if unknown:
        parser.error(f"unknown dataset(s): {', '.join(unknown)}")

    results = [run(BY_NAME[name], args) for name in args.datasets]
    print_table(results)

    if args.json:
        payload = {
            "impl": "sklearn",
            "sklearn_version": sklearn.__version__,
            "python": platform.python_version(),
            "machine": platform.machine(),
            "params": {
                "criterion": args.criterion,
                "max_depth": args.max_depth,
                "min_samples_split": args.min_samples_split,
                "min_samples_leaf": args.min_samples_leaf,
            },
            "results": results,
        }
        path = Path(args.json)
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(json.dumps(payload, indent=2))
        print(f"\nwrote {path}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
