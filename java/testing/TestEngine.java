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

    testStatefile(args);
  }

  // Set an option, add a file, save the state then restore it into a fresh engine and check both
  // the option and the loaded scene are restored, through both the file and the string API
  static void testStatefile(String[] args) {
    String dataDir = args.length > 0 ? args[0] : "";
    String tempDir = args.length > 1 ? args[1] : "";
    String cow = dataDir + "data/cow.vtp";
    String statefile = tempDir + "TestEngineJavaStatefile.json";

    Engine src = Engine.createNone();
    src.getOptions().setAsBool("ui.scalar_bar", true);
    src.getScene().add(cow);

    // File based round trip
    src.saveStatefile(statefile);
    Engine dst = Engine.createNone();
    dst.loadStatefile(statefile);
    if (!dst.getOptions().getAsBool("ui.scalar_bar")) {
      throw new RuntimeException("options should be restored from the statefile");
    }
    if (dst.getScene().getAddedFiles().size() != 1) {
      throw new RuntimeException("scene should be restored from the statefile");
    }
    dst.close();

    // String based round trip
    String content = src.saveStatefileToString();
    Engine dstStr = Engine.createNone();
    dstStr.loadStatefileFromString(content);
    if (!dstStr.getOptions().getAsBool("ui.scalar_bar")) {
      throw new RuntimeException("options should be restored from the statefile string");
    }
    if (dstStr.getScene().getAddedFiles().size() != 1) {
      throw new RuntimeException("scene should be restored from the statefile string");
    }
    dstStr.close();

    // Loading an invalid statefile should throw
    Engine invalid = Engine.createNone();
    boolean threw = false;
    try {
      invalid.loadStatefile("/does/not/exist/state.json");
    } catch (RuntimeException e) {
      threw = true;
    }
    invalid.close();
    if (!threw) {
      throw new RuntimeException("load_statefile should throw on a missing file");
    }

    src.close();
  }
}
