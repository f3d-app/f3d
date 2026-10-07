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

    # isPlaying after start
    engine.interactor.start_animation()
    assert engine.interactor.is_playing_animation() == 1

    # isPlaying after toggle off
    engine.interactor.toggle_animation()
    assert engine.interactor.is_playing_animation() == 0

    # isPlaying after toggle on
    engine.interactor.toggle_animation()
    assert engine.interactor.is_playing_animation() == 1

    # triggerEventLoop returns self
    inter_ref = engine.interactor.trigger_event_loop(0.1)
    assert inter_ref == engine.interactor

    # isPlaying after stop
    engine.interactor.stop_animation()
    assert engine.interactor.is_playing_animation() == 0

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
