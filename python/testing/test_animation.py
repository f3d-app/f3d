import tempfile
from pathlib import Path

import f3d


def test_animation():
    testing_dir = Path(__file__).parent.parent.parent / "testing"
    logo = testing_dir / "data/f3d.glb"

    engine = f3d.Engine.create(True)
    engine.window.size = 300, 300

    engine.scene.add(logo)

    # Tests

    # availableAnimations
    assert engine.scene.available_animations() == 1

    # recover animationTimeRange
    engine.scene.load_animation_time(0.5)
    assert engine.scene.animation_time_range() == (0.0, 4.0)

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
    assert  inter_ref == engine.interactor

    # isPlaying after stop
    engine.interactor.stop_animation()
    assert engine.interactor.is_playing_animation() == 0

    # getAnimationName returns name at index
    assert engine.interactor.get_animation_name(0) == "unnamed_0"

    # getAnimationName returns for out of range
    assert engine.interactor.get_animation_name(9999) == "No animation"

    # getAnimationName returns current name
    assert engine.interactor.get_animation_name() == "unnamed_0"

    # getAnimationNames returns names
    assert engine.interactor.get_animation_names() == ["unnamed_0"]
