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

    window.getType();
    window.isOffscreen();
    window.getCamera();

    window.render();

    double dpiScale = window.getDPIScale();
    if (dpiScale < 1.0) {
      throw new RuntimeException("DPI scale value unexpected: " + dpiScale);
    }

    Image img = window.renderToImage(true);
    img.getWidth();
    img.getHeight();
    img.getChannelCount();
    img.getContent();
    img.delete();

    Image img2 = window.renderToImage();
    img2.delete();

    window.setSize(800, 600);
    window.getWidth();
    window.getHeight();

    int[] size = window.getSize();
    if (size.length != 2 || size[0] != window.getWidth() || size[1] != window.getHeight()) {
      throw new RuntimeException("getSize should return {width, height}");
    }

    window.setPosition(100, 100);
    window.render();
    // The window position depends on a window manager and is (0, 0) in headless CI, so only check
    // the array shape rather than asserting a specific value.
    int[] position = window.getPosition();
    if (position.length != 2) {
      throw new RuntimeException("getPosition should return {x, y}");
    }

    if (window.getLeft() != position[0] || window.getTop() != position[1]) {
      throw new RuntimeException("getLeft/getTop should match getPosition");
    }

    byte[] icon = new byte[]{1, 2, 3, 4};
    window.setIcon(icon);

    window.setWindowName("F3D Test Window");

    double[] worldPoint = new double[]{0.0, 0.0, 0.0};
    window.getDisplayFromWorld(worldPoint);

    double[] displayPoint = new double[]{100.0, 100.0, 0.0};
    window.getWorldFromDisplay(displayPoint);

    window.setSize(1024, 768)
          .setPosition(50, 50)
          .setWindowName("Chained");

    engine.close();
  }
}
