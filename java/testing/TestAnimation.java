import app.f3d.F3D.*;

import java.io.*;
import java.lang.String;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class TestAnimation {

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

  public static void main(String[] args) throws FileNotFoundException, IOException {
    Engine.autoloadPlugins();

    String testDataPath = args.length > 0 ? args[0] : ".";
    String data = testDataPath + "data/BoxAnimated.gltf";

    Engine engine = Engine.createNone();
    Scene scene = engine.getScene();

    scene.add(data);

    
    Animation anim = scene.getAnimation();

    // XXX: Only smoke tests for now
    anim.loadTime(0.5);
    anim.timeRange();
    anim.keyFrames();
    anim.count();
    anim.getCurrentName();
    anim.getName(0);
    anim.getNames();

    engine.close();
  }
}
