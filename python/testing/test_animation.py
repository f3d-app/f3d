import tempfile
from pathlib import Path

import f3d


def test_animation():
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    logo = testing_dir / "data/soldier_animations.mdl"

    engine = f3d.Engine.create(True)
    engine.window.size = 300, 300

    engine.scene.add(logo)

    # Tests

    # animation count
    assert engine.scene.animation.count() == 10

    keyframes = engine.scene.animation.get_key_frames()
    assert len(keyframes) == 9
    assert keyframes[0] == 0
    assert keyframes[8] == 0.7999999999999999

    # recover animationTimeRange
    engine.scene.animation.load_time(0.5)
    assert engine.scene.animation.get_time_range() == (0.0, 0.7999999999999999)

    # getAnimationName current
    assert engine.scene.animation.get_name() == "stand"

    # getAnimationName returns name at index
    assert engine.scene.animation.get_name(1) == "dead"

    # getAnimationName returns for out of range
    assert engine.scene.animation.get_name(9999) == "No animation"

    # getAnimationName returns current name
    assert engine.scene.animation.get_name() == "stand"

    # getAnimationNames returns names
    assert engine.scene.animation.get_names() == [
        "stand",
        "dead",
        "dead_right",
        "reload",
        "hit",
        "down",
        "stumble",
        "run",
        "shoot",
        "walk",
    ]


def test_animation_start_stop(capfd: pytest.CaptureFixture[str]):
    engine = f3d.Engine.create(True)
    engine.window.render()

    engine.scene.animation.start()  # Play Forward
    assert (
        engine.scene.animation.is_playing()
        and engine.scene.animation.get_direction() == f3d.animation.Direction.FORWARD
    )
    engine.scene.animation.toggle()  # Pause using toggle
    assert not engine.scene.animation.is_playing()

    engine.scene.animation.toggle(
        f3d.animation.Direction.BACKWARD
    )  # Play Backward using toggle
    assert (
        engine.scene.animation.is_playing()
        and engine.scene.animation.get_direction() == f3d.animation.Direction.BACKWARD
    )
    engine.scene.animation.stop()  # Pause using stop
    assert not engine.scene.animation.is_playing()
