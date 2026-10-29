import static org.junit.jupiter.api.Assertions.*;

import java.util.concurrent.atomic.AtomicBoolean;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;

import app.f3d.F3D.*;

public class TestVideo {

  // On Windows, try to load opengl32 from Java path
  // It's only useful in order to force Mesa software OpenGL
  static {
    if (System.getProperty("os.name").startsWith("Windows")) {
      try {
        System.loadLibrary("opengl32");
      } catch (UnsatisfiedLinkError e) {
        // Ignore if opengl32 is not available
      }
    }
  }

  private Engine engine;
  private Window window;

  @BeforeEach
  public void setUp() {
    Engine.autoloadPlugins();
    engine = Engine.create(true);
    window = engine.getWindow();
  }

  @AfterEach
  public void tearDown() {
      engine.close();
  }

  @Test
  public void testAvailableEncoders() {
    Iterable<VideoEncoder.StringPair> encoders = VideoEncoder.getAvailableEncoders();
    assertNotNull(encoders, "Available video encoders list should not be null");
  }

  @Test
  public void testParams() {
    window.render();
    VideoEncoder.Params params = new VideoEncoder.Params();
    assertNotNull(params,"Params should not be null");
    params.codec = VideoEncoder.Codec.H264;
    assertEquals(VideoEncoder.Codec.H264, params.codec, "Params codec should be equal to enum of value");
    params.width = window.getWidth();
    assertNotNull(params.width, "Width of params should not be null");
    params.height = window.getHeight();
    assertNotNull(params.height, "Height of params should not be null");
  }

  @Test
  public void testAtomicBoolean() {
    AtomicBoolean packetReceived = new AtomicBoolean(false);
    assertNotNull(packetReceived,"Atomic boolean should not be null");
    assertFalse(packetReceived.get(), "Atomic value should be false");
  }

  @Test
  @Disabled("As of now, this test can not be completed because it needs f3d to be build with FFMPEG support and i can not achieve that at the moment, so it needs to be tested on another machine")
  public void testVideoEncode() {
    window.render();

    VideoEncoder.Params params = new VideoEncoder.Params();
    params.codec = VideoEncoder.Codec.H264;
    params.width = window.getWidth();
    params.height = window.getHeight();

    AtomicBoolean packetReceived = new AtomicBoolean(false);

    try (VideoEncoder encoder = new VideoEncoder(params)) {
      encoder.listen(packet -> {
        assertNotNull(packet, "Encoded packet should not be null");
        assertNotNull(packet.getPacketData(), "Packet data byte array should not be null");
        assertTrue(packet.getPacketData().length > 0, "Encoded packet size should be positive");
        assertEquals(67, packet.getTimestamp(), "Encoded packet timestamp should be 67");
        packetReceived.set(true);
      });

      try (VideoFrame frame = window.getVideoFrame()) {
        assertNotNull(frame, "Video frame captured from window should not be null");
        frame.setTimestamp(67);
        encoder.submit(frame);
        encoder.flush();
      }
    }

    assertTrue(packetReceived.get(), "Video encoder packet callback should have been triggered");
  }
}