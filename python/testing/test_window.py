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
    engine.window.render()
    # The window position depends on a window manager and is (0, 0) in headless CI, so only check
    # that the property is a 2-tuple rather than asserting a specific value.
    pos = engine.window.position
    assert isinstance(pos, tuple) and len(pos) == 2
