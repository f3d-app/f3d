import app.f3d.F3D.*;

public class TestEngine {

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

    Engine eng1 = Engine.create(true);
    eng1.close();

    Engine eng2 = Engine.create();
    eng2.close();

    Engine eng3 = Engine.createNone();
    eng3.close();

    Engine.loadPlugin("native");

    Engine.getPluginsList(".");

    Engine.getAllReaderOptionNames();

    Engine.getLibInfo();

    Engine.getReadersInfo();

    Engine.getRenderingBackendList();

    Engine engine = Engine.create(true);

    engine.setCachePath("/tmp/f3d_test");

    engine.getOptions();

    engine.getScene();

    engine.getWindow();

    engine.getInteractor();

    engine.close();
  }
}
