import app.f3d.F3D.*;

public class TestCamera {

  static {
    if (System.getProperty("os.name").startsWith("Windows"))
    {
      System.loadLibrary("opengl32");
    }
  }

  public static void main(String[] args) {
    Engine.autoloadPlugins();

    Engine engine = Engine.create(true);
    Window window = engine.getWindow();
    Camera camera = window.getCamera();

    camera.setPosition(new double[]{1.0, 2.0, 3.0});
    camera.getPosition();

    camera.setFocalPoint(new double[]{0.5, 0.6, 0.7});
    camera.getFocalPoint();

    camera.setViewUp(new double[]{0.0, 1.0, 0.0});
    camera.getViewUp();

    camera.setViewAngle(45.0);
    camera.getViewAngle();

    Camera.CameraState state = new Camera.CameraState(
      new double[]{2.0, 3.0, 4.0},
      new double[]{1.0, 1.0, 1.0},
      new double[]{0.0, 1.0, 0.0},
      60.0
    );
    camera.setState(state);
    camera.getState();

    camera.dolly(1.5);
    camera.pan(0.1, 0.2, 0.3);
    camera.pan(0.1, 0.2);
    camera.zoom(1.2);
    camera.roll(10.0);
    camera.azimuth(15.0);
    camera.yaw(20.0);
    camera.elevation(25.0);
    camera.pitch(30.0);

    camera.setCurrentAsDefault();
    camera.resetToDefault();
    camera.resetToBounds(0.8);
    camera.resetToBounds();

    camera.setPosition(new double[]{1.0, 1.0, 1.0})
          .setFocalPoint(new double[]{0.0, 0.0, 0.0})
          .setViewUp(new double[]{0.0, 1.0, 0.0})
          .setViewAngle(30.0)
          .dolly(1.0)
          .roll(5.0);

    engine.close();
  }
}
