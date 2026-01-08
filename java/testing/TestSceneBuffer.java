import app.f3d.F3D.*;

import java.io.*;
import java.lang.String;

public class TestSceneBuffer {

  public static void main(String[] args) throws FileNotFoundException, IOException {
    Engine.autoloadPlugins();

    String testDataPath = args.length > 0 ? args[0] : ".";

    Engine engine = Engine.createNone();
    Scene scene = engine.getScene();

    char[] array = new char[256];
    FileReader input = new FileReader(testDataPath + "data/points.ply");
    int size = input.read(array);
    input.close();
    Options options = engine.getOptions();
    options.setAsString("scene.force_reader", "PLYReader");
    scene.addBuffer(new String(array).getBytes(), size);

    engine.close();
  }
}
