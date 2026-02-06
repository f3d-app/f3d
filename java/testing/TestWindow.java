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

    window.setPosition(100, 100);

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
