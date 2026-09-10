import app.f3d.F3D.*;

public class TestWindow {

  // On Windows, try to load opengl32 from Java path
  // It's only useful in order to force Mesa software OpenGL
  static {
    if (System.getProperty("os.name").startsWith("Windows"))
    {
      try {
        System.loadLibrary("opengl32");
      } catch (UnsatisfiedLinkError e) {
        // Ignore if opengl32 is not available
      }
    }
  }

  public static void main(String[] args) {
    Engine.autoloadPlugins();

    Engine engine = Engine.create(true);
    Window window = engine.getWindow();
    window.render();

    // List codecs
    for (VideoEncoder.StringPair encoder : VideoEncoder.getAvailableEncoders()) {
      System.out.println("Encoder: " + encoder.first() + ", Description: " + encoder.second());
    }

    // Video testing
    VideoEncoder.Params params = new VideoEncoder.Params();
    params.codec = VideoEncoder.Codec.H264;
    params.width = window.getWidth();
    params.height = window.getHeight();

    try (VideoEncoder encoder = new VideoEncoder(params)) {

      encoder.listen(packet -> {
        if (packet.getPacketData().length <= 0) {
          throw new RuntimeException("Encoded packet size should be positive");
        }
        if (packet.getTimestamp() != 42) {
          throw new RuntimeException("Encoded packet timestamp should be 42");
        }
      });

      try (VideoFrame frame = window.getVideoFrame()) {
        frame.setTimestamp(42);
        encoder.submit(frame);
        encoder.flush();
      }
    }

    engine.close();
  }
}
