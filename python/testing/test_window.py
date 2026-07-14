import f3d


def test_window_size():
    engine = f3d.Engine.create(True)
    engine.window.size = 300, 400
    assert engine.window.size == (300, 400)
    assert engine.window.width == 300
    assert engine.window.height == 400


def test_window_position():
    engine = f3d.Engine.create(True)
    engine.window.position = 100, 200
    assert engine.window.position == (100, 200)
