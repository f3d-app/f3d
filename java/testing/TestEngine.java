import app.f3d.F3D.*;

public class TestEngine {

  static {
    if (System.getProperty("os.name").startsWith("Windows"))
    {
      System.loadLibrary("opengl32");
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
