import pytest
from sklearn.tree import DecisionTreeClassifier

from datasets import DATASETS


def pytest_addoption(parser):
    parser.addoption("--criterion", default="gini")
    parser.addoption("--max-depth", type=int, default=None)
    parser.addoption("--min-samples-split", type=int, default=2)
    parser.addoption("--min-samples-leaf", type=int, default=1)


@pytest.fixture(params=DATASETS, ids=lambda d: d.name)
def dataset(request):
    return request.param


@pytest.fixture
def make_classifier(request):
    opt = request.config.getoption

    def factory():
        return DecisionTreeClassifier(
            criterion=opt("criterion"),
            splitter="best",
            max_depth=opt("max_depth"),
            min_samples_split=opt("min_samples_split"),
            min_samples_leaf=opt("min_samples_leaf"),
            random_state=0,
        )

    return factory
