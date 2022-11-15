import app.f3d.F3D.*;

public class TestJavaBindings {

  static {
    if (System.getProperty("os.name").startsWith("Windows"))
    {
      // On Windows, preload the OpenGL library
      // This ensures the OpenGL used is the one in the working directory if any.
      // In practice, it is used in F3D CI to run the test using Mesa, it's not required for production.
      System.loadLibrary("opengl32");
    }
  }

  public static void main(String[] args) {

    Engine.autoloadPlugins();

    // Always use try-with-resources idiom to ensure the native engine is released
    try (Engine engine = new Engine(Window.Type.NATIVE_OFFSCREEN)) {

      Camera camera = engine.getWindow().getCamera();

      camera.setPosition(new double[] { 0, 1, 2 });
      double[] pos = camera.getPosition();

      assert pos[0] == 0.0 : "Position X is not valid";
      assert pos[1] == 1.0 : "Position Y is not valid";
      assert pos[2] == 2.0 : "Position Z is not valid";

      Loader loader = engine.getLoader();
      loader.addFile(args[0] + "data/cow.vtp");
      loader.loadFile(Loader.LoadFileEnum.LOAD_CURRENT);
    }
  }
}
