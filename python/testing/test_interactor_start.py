from functools import partial
import pytest

import f3d


def stop_fn(inter):
    inter.stop()


def test_start_stop():
    engine = f3d.Engine.create(True)
    inter = engine.interactor
    inter.start(1 / 30, partial(stop_fn, inter))


def test_request_render(capfd):
    engine = f3d.Engine.create(True)
    inter = engine.interactor
    inter.request_render()
    inter.start(1 / 30, partial(stop_fn, inter))
