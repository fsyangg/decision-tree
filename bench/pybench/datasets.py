from dataclasses import dataclass
from pathlib import Path

import numpy as np

DATASETS_DIR = Path(__file__).resolve().parents[2] / "datasets"


@dataclass(frozen=True)
class Dataset:
    name: str
    filename: str
    label_col: int

    def load(self) -> tuple[np.ndarray, np.ndarray]:
        raw = np.loadtxt(DATASETS_DIR / self.filename, delimiter=",", dtype=np.int32, ndmin=2)
        y = raw[:, self.label_col].astype(np.int32)
        X = np.delete(raw, self.label_col, axis=1)
        return np.ascontiguousarray(X, dtype=np.float32), y


DATASETS = [
    Dataset("tennis", "tennis.csv", -1),
    Dataset("zoo", "zoo.csv", -1),
    Dataset("car_eval", "car_eval.csv", -1),
    Dataset("mushroom", "mushroom.csv", 0),
]
