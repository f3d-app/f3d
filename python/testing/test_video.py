import f3d


def test_video_api():
    engine = f3d.Engine.create(True)
    engine.window.size = 256, 256

    # List available encoders
    available_encoders = f3d.VideoEncoder.get_available_encoders()
    for encoder in available_encoders:
        print(f"Encoder: {encoder[0]}, Description: {encoder[1]}")

    params = f3d.VideoEncoderParams()
    params.width = engine.window.size[0]
    params.height = engine.window.size[1]
    params.frame_rate = 30
    encoder = f3d.VideoEncoder.create(params)
    assert isinstance(encoder, f3d.VideoEncoder)
    assert encoder.width == engine.window.size[0]
    assert encoder.height == engine.window.size[1]

    engine.window.render()
    frame = engine.window.get_video_frame()
    assert isinstance(frame, f3d.VideoFrame)

    received_timestamps = []
    encoder.listen(lambda packet: received_timestamps.append(packet.get_timestamp()))

    frame.set_timestamp(42)
    encoder.submit(frame)

    encoder.flush()

    assert len(received_timestamps) == 1
    assert received_timestamps[0] == 42
