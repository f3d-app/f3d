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

    # availableAnimations
    assert engine.scene.available_animations() == 10

    keyframes = engine.scene.get_animation_keyframes()
    assert len(keyframes) == 9
    assert keyframes[0] == 0
    assert keyframes[8] == 0.7999999999999999

    # recover animationTimeRange
    engine.scene.load_animation_time(0.5)
    assert engine.scene.animation_time_range() == (0.0, 0.7999999999999999)

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

    # getDeltaTime after triggerEventLoop
    assert engine.interactor.get_delta_time() == 0.1

    # isPlaying after stop
    engine.interactor.stop_animation()
    assert engine.interactor.is_playing_animation() == 0

    # getAnimationName returns name at index
    assert engine.scene.get_animation_name(0) == "stand"

    # getAnimationName returns for out of range
    assert engine.scene.get_animation_name(9999) == "No animation"

    # getAnimationName returns current name
    assert engine.scene.get_animation_name() == "stand"

    # getAnimationNames returns names
    assert engine.scene.get_animation_names() == [
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

    # animationTime
    engine.scene.load_animation_time(0.0)
    assert engine.scene.animation_time() == 0.0

    engine.scene.load_animation_time(0.5)
    assert engine.scene.animation_time() == 0.5

    # goToKeyframe absolute
    engine.scene.load_animation_time(0.0)
    engine.interactor.go_to_keyframe(4, False)
    assert engine.scene.animation_time() == keyframes[4]

    # goToKeyframe relative
    engine.interactor.go_to_keyframe(1, True)
    assert engine.scene.animation_time() == keyframes[5]

    # currentFrame
    engine.scene.load_animation_time(0.0)

    engine.scene.load_animation_time(0.5)

    # goToFrame absolute
    engine.interactor.go_to_frame(24, False)

    # goToFrame relative
    engine.interactor.go_to_frame(1, True)

    # goToFrame absolute 0
    engine.interactor.go_to_frame(0, False)
