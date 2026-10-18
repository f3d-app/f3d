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

    keyFrames = engine.scene.animation.get_key_frames()
    assert len(keyFrames) == 9
    assert keyFrames[0] == 0
    assert keyFrames[8] == 0.7999999999999999

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

    # triggerEventLoop returns self
    inter_ref = engine.interactor.trigger_event_loop(0.1)
    assert inter_ref == engine.interactor

    # getDeltaTime after triggerEventLoop
    assert engine.interactor.get_delta_time() == 0.03333333333333333

    # animationTime
    engine.scene.animation.load_time(0.0)
    assert engine.scene.animation_time() == 0.0

    engine.scene.animation.load_time(0.5)
    assert engine.scene.animation_time() == 0.5

    # animationFrame
    engine.scene.animation.load_time(0.0)
    assert engine.scene.animation_frame() == 0

    engine.scene.animation.load_time(0.5)
    assert engine.scene.animation_frame() == 15

    # jumpToFrame absolute
    engine.interactor.jump_to_frame(10)
    assert engine.scene.animation_frame() == 10

    # jumpToFrameRelative
    engine.interactor.jump_to_frame_relative(1)
    assert engine.scene.animation_frame() == 11

    # jumpToFrame absolute 0
    engine.interactor.jump_to_frame(0)
    assert engine.scene.animation_frame() == 0

    # jumpToKeyframe absolute
    engine.scene.animation.load_time(0.0)
    engine.interactor.jump_to_keyframe(4)
    assert engine.scene.animation_time() == keyFrames[4]

    # jumpToKeyframeRelative
    engine.interactor.jump_to_keyframe_relative(1)
    assert engine.scene.animation_time() == keyFrames[5]

    # jumpToKeyframe absolute 0
    engine.interactor.jump_to_keyframe(0)
    assert engine.scene.animation_time() == keyFrames[0]


def test_animation_start_stop():
    engine = f3d.Engine.create(True)
    engine.window.render()

    engine.scene.animation.start()  # Play Forward
    assert (
        engine.scene.animation.is_playing()
        and engine.scene.animation.get_direction() == f3d.Animation.Direction.FORWARD
    )
    engine.scene.animation.toggle()  # Pause using toggle
    assert not engine.scene.animation.is_playing()

    engine.scene.animation.toggle(
        f3d.Animation.Direction.BACKWARD
    )  # Play Backward using toggle
    assert (
        engine.scene.animation.is_playing()
        and engine.scene.animation.get_direction() == f3d.Animation.Direction.BACKWARD
    )
    engine.scene.animation.stop()  # Pause using stop
    assert not engine.scene.animation.is_playing()