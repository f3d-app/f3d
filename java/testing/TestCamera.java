import app.f3d.F3D.*;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Disabled;

public class TestCamera {

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

  // Declaration of classes for testing
  private Engine engine;
  private Window window;
  private Camera camera;

  // Making and unmaking of test classes
  @BeforeEach
  public void setUp() {
    Engine.autoloadPlugins();
    engine = Engine.create(true);
    window = engine.getWindow();
    camera = window.getCamera();
  }

  @AfterEach
  public void clearTestEngine() {
    engine.close();
  }

  // Check if camera is created correctly
  @Test
  public void checkOfCreatingCamera() {
    assertNotNull(engine,"Engine should be created");
    assertNotNull(window,"Window of engine should be created");
    assertNotNull(camera,"Camera of window should be created");
  }

  // Helper method for checking coordinates for camera testing
  private void assertionTests(Class<?> testedClass, double[] setPos, double[] getPos, String testingFunction) {
    String nameOfClass = testedClass.getSimpleName();
    assertNotNull(getPos,"Get of " + nameOfClass + " position should not be null");
    assertEquals(setPos[0], getPos[0], 0.00001 , "First position[0] of " + nameOfClass + " should be same to set and get method (Function: " + testingFunction + ")");
    assertEquals(setPos[1], getPos[1], 0.00001 , "Second position[1] of " + nameOfClass + " should be same to set and get method (Function: " + testingFunction + ")");
    assertEquals(setPos[2], getPos[2], 0.00001 , "Third position[2] of " + nameOfClass + " should be same to set and get method (Function: " + testingFunction + ")");
  }

  @Test
  public void testCameraPosition() {
    double[] setPos = new double[]{1.0,2.0,3.0};
    camera.setPosition(setPos);
    double[] getPos = camera.getPosition();

    assertionTests(camera.getClass(), setPos, getPos, "Camera Position");
  }

  @Test
  public void testCameraFocalPoint() {
    double[] setPos = new double[]{0.5,0.6,0.7};
    camera.setFocalPoint(setPos);
    double[] getPos = camera.getFocalPoint();

    assertionTests(camera.getClass(), setPos, getPos, "Focal Point");
  }

  @Test
  public void testViewUp() {
    double[] setPos = new double[]{0.0,1.0,0.0};
    camera.setViewUp(setPos);
    double[] getPos = camera.getViewUp();

    assertionTests(camera.getClass(), setPos, getPos, "View Up");
  }

  @Test
  public void testViewAngle() {
    double setPos = 45.0;
    camera.setViewAngle(setPos);
    double getPos = camera.getViewAngle();

    assertNotNull(getPos,"Get of " + camera.getClass().getSimpleName() + " position should not be null");
    assertEquals(setPos, getPos, 0.00001 , "Angle value of " + camera.getClass().getSimpleName() + " should be same to set and get method (Function: View Angle)");
  }

  @Test
  @Disabled("view up state is returning wrong value")
  public void testCameraState() {
    double[] setPosition = new double[]{2.0, 3.0, 4.0};
    double[] setFocalPoint = new double[]{1.0, 1.0, 1.0};
    double[] setViewUp = new double[]{0.0, 1.0, 0.0};
    double setViewAngle = 60.0;
    Camera.CameraState state = new Camera.CameraState(
      setPosition,
      setFocalPoint,
      setViewUp,
      setViewAngle
    );
    assertNotNull(state,"State of Camera should not be null");
    camera.setState(state);
    assertionTests(camera.getClass(), setPosition, camera.getPosition(), "Camera Position - State");
    assertionTests(camera.getClass(), setFocalPoint, camera.getFocalPoint(), "Focal Point - State");
    assertionTests(camera.getClass(), setViewUp, camera.getViewUp(), "View Up - State");
    assertEquals(setViewAngle, camera.getViewAngle(), 0.00001 , "Angle value of " + camera.getClass().getSimpleName() + " should be same to set and get method (Function: View Angle - State)");
  }

  @Test // These two tests have exception rule but more or less they are still smoke tests because i do not have means to validate if before and after operation are equal
  public void testCameraManipulation() {
    assertDoesNotThrow(() -> {
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
    },"Camera manipulation should execute without throwing an exception");

  }

  @Test
  public void testCustomCamera() {
    assertDoesNotThrow(() -> {
        camera.setPosition(new double[]{1.0, 1.0, 1.0})
          .setFocalPoint(new double[]{0.0, 0.0, 0.0})
          .setViewUp(new double[]{0.0, 1.0, 0.0})
          .setViewAngle(30.0)
          .dolly(1.0)
          .roll(5.0);
    },"Making of custom camera should not throw exception");
  }
}