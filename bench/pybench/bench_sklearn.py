def bench_fit(benchmark, dataset, make_classifier):
    X, y = dataset.load()
    model = benchmark(lambda: make_classifier().fit(X, y))
    benchmark.extra_info.update(
        rows=int(X.shape[0]),
        features=int(X.shape[1]),
        nodes=int(model.tree_.node_count),
        depth=int(model.get_depth()),
        train_accuracy=float((model.predict(X) == y).mean()),
    )


def bench_predict(benchmark, dataset, make_classifier):
    X, y = dataset.load()
    model = make_classifier().fit(X, y)
    benchmark(model.predict, X)
    benchmark.extra_info.update(
        rows=int(X.shape[0]),
        ns_per_row=benchmark.stats.stats.min * 1e9 / X.shape[0],
    )
