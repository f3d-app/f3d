import os
from pathlib import Path
import tempfile
import pytest

import f3d

@pytest.fixture
def f3d_engine():
    engine = f3d.Engine.create(True)
    engine.window.size = 300, 200
    return engine

def test_loading_stream(f3d_engine: f3d.Engine):
    img = f3d_engine.window.render_to_image(True)
    buffer = img.save_buffer(f3d.Image.SaveFormat.PNG)

    stream_image = f3d.Image(buffer)
    assert stream_image == img
