import app.f3d.F3D.*;

import java.io.*;
import java.lang.String;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class TestScene {

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
    String world = testDataPath + "data/world.obj";
    String logo = testDataPath +  "data/f3d.glb";
    String sphere = testDataPath + "data/mb/recursive/mb_1_0.vtp";

    Engine engine = Engine.createNone();
    Scene scene = engine.getScene();

    scene.supports("test.obj");

    scene.add(sphere);
    scene.add(new ArrayList<>(Arrays.asList(world, logo)));
    scene.clear();

    float[] points = new float[] { 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.5f, 1.0f, 0.0f };
    int[] faceSides = new int[] { 3 };
    int[] faceIndices = new int[] { 0, 1, 2 };
    Types.Mesh mesh = new Types.Mesh(points, new float[0], new float[0], faceSides, faceIndices);

    scene.add(mesh);

    scene.loadAnimationTime(0.5);
    scene.animationTimeRange();
    scene.availableAnimations();
    scene.getAnimationName();
    scene.getAnimationName(0);
    scene.getAnimationNames();

    Types.LightState lightState = new Types.LightState();
    lightState.type = Types.LightType.HEADLIGHT;
    lightState.intensity = 1.0;
    lightState.switchState = true;

    int lightIdx = scene.addLight(lightState);

    scene.getLightCount();

    if (lightIdx >= 0)
    {
      scene.getLight(lightIdx);

      Types.LightState updateLight = new Types.LightState();
      updateLight.type = Types.LightType.HEADLIGHT;
      updateLight.intensity = 2.0;
      updateLight.switchState = true;

      scene.updateLight(lightIdx, updateLight);

      scene.removeLight(lightIdx);
    }

    Types.LightState light1 = new Types.LightState();
    light1.type = Types.LightType.HEADLIGHT;
    light1.intensity = 1.0;

    Types.LightState light2 = new Types.LightState();
    light2.type = Types.LightType.CAMERA_LIGHT;
    light2.intensity = 1.0;

    light1.equals(light2);

    scene.removeAllLights();

    engine.close();
  }
}
