import static org.junit.jupiter.api.Assertions.*;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import app.f3d.F3D.*;

public class TestWindow {

  // On Windows, try to load opengl32 from Java path
  // It's only useful in order to force Mesa software OpenGL
  static {
    if (System.getProperty("os.name").startsWith("Windows")) {
      try {
        System.loadLibrary("opengl32");
      } catch (UnsatisfiedLinkError e) {
        // Ignore if opengl32 is not available
      }
    }
  }

  private Engine engine;
  private Window window;

  @BeforeEach
  public void setUp() {
    Engine.autoloadPlugins();
    engine = Engine.create(true);
    window = engine.getWindow();
  }

  @AfterEach
  public void tearDown(){
      engine.close();
  }

  @Test
  public void testWindowBasicProperties() {
    assertDoesNotThrow(() -> {
      window.setUseHDRICache(true);
      assertNotNull(window.getType(),"Window type should not be null");
      assertNotNull(window.isOffscreen(),"Calling check for off screen should not be null");
      assertNotNull(window.getCamera(), "Window camera should not be null");
      assertNotNull(window.render(),"Rendering windows should not be null");
      window.isOffscreen();
      window.render();
      window.render();
    }, "Basic window property settings and rendering should execute successfully");
  }

  @Test
  public void testDPI() {
    double dpiScale = window.getDPIScale();
    assertTrue(dpiScale >= 1.0, "DPI scale should be at greater or at least 1.0, but got " + dpiScale);
  }

  @Test // I can not find how the variable in renderToImage is working
  public void testRenderToImage() {
    window.render();

    Image img = window.renderToImage(true);
    assertNotNull(img,"Image should be created with renderToImage");
    assertTrue(img.getWidth() > 0, "Image width should be positive");
    assertTrue(img.getHeight() > 0, "Image height should be positive");
    assertTrue(img.getChannelCount() > 0, "Image channel count should be positive");
    assertNotNull(img.getContent(), "Image content buffer should not be null");
    img.delete();

    Image img2 = window.renderToImage();
    assertNotNull(img2, "Default rendered image should not be null");
    img2.delete();
  }

  @Test
  public void testWindowResize() {
    window.setSize(800, 600);
    int[] size = window.getSize();
    assertNotNull(size, "Window size array should not be null");
    assertEquals(2, size.length, "Window size array should contain exactly 2 elements");

    assertEquals(800, window.getWidth(), "Window width should be 800");
    assertEquals(600, window.getHeight(), "Window height should be 600");

    assertEquals(window.getWidth(), size[0], "Size[0] should match window width");
    assertEquals(window.getHeight(), size[1], "Size[1] should match window height");
  }

  @Test
  public void testWindowPosition() {
    window.setPosition(100, 100);
    window.render();

    // The window position depends on a window manager and is (0, 0) in headless CI, so only check the array shape rather than asserting a specific value.
    int[] position = window.getPosition();
    assertNotNull(position, "Window position should not be null");
    assertEquals(2, position.length, "Window position should contain exactly 2 elements");

    assertEquals(position[0], window.getLeft(), "Window left position should match position[0]");
    assertEquals(position[1], window.getTop(), "Window top position should match position[1]");
  }

  @Test // There are not getters for Icon and WindowsName so this test only check if nothing throws exception
  public void testWindowIconAndName() {
    byte[] icon = new byte[]{1, 2, 3, 4};
    assertDoesNotThrow(() -> {
      window.setIcon(icon);
      window.setWindowName("F3D Test Window");
    }, "Setting window icon and name should execute successfully");
  }

  @Test // There are not getters so checking if they return what they should is not possible, only checking if throw exception is not applied
  public void testCoordinateTransformations() {
    assertDoesNotThrow(() -> {
      double[] worldPoint = new double[]{0.0, 0.0, 0.0};
      window.getDisplayFromWorld(worldPoint);

      double[] displayPoint = new double[]{100.0, 100.0, 0.0};
      window.getWorldFromDisplay(displayPoint);
    }, "Display and world coordinate projections should execute without error");
  }

  @Test // There are no getters to see if these actions are valid, only checking if nothing throws exception
  public void testWindowMethodChaining() {
    assertDoesNotThrow(() -> {
      window.setSize(1024, 768)
           .setPosition(50, 50)
           .setWindowName("Chained");
    }, "Window configuration methods should support fluent method chaining");
  }
}