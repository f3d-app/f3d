import f3d


def test_window_size():
    engine = f3d.Engine.create(True)
    engine.window.size = 300, 400
    assert engine.window.size == (300, 400)
    assert engine.window.width == 300
    assert engine.window.height == 400
    assert engine.window.size == (engine.window.width, engine.window.height)


def test_window_position():
    engine = f3d.Engine.create(True)
    engine.window.position = 100, 200
    engine.window.render()
    # The window position depends on a window manager and is (0, 0) in headless CI, so only check
    # that the property is a 2-tuple rather than asserting a specific value.
    pos = engine.window.position
    assert isinstance(pos, tuple) and len(pos) == 2
    assert engine.window.left == pos[0]
    assert engine.window.top == pos[1]


def test_window_use_hdri_cache():
    engine = f3d.Engine.create(True)
    engine.window.set_use_hdri_cache(True)
    engine.window.render()
    engine.window.set_use_hdri_cache(False)
    engine.window.render()
