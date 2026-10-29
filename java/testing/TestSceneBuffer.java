import static org.junit.jupiter.api.Assertions.*;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Test;

import app.f3d.F3D.*;

public class TestSceneBuffer {

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
  private Scene scene;
  private Path pointsPlyPath;

  @BeforeEach
  public void setUp(){
    Engine.autoloadPlugins();
    engine = Engine.createNone();
    scene = engine.getScene();

    pointsPlyPath = Path.of("dev", "f3d", "src", "testing", "data", "points.ply");
  }

  @AfterEach
  public void tearDown(){
    if (engine != null) {
      engine.close();
    }
  }

  @Test
  public void testScenePath(){
    assertTrue(Files.exists(pointsPlyPath), "points.ply file should exist on disk");
  }

  @Test
  public void testFile() throws IOException {
    byte[] fileBytes = Files.readAllBytes(pointsPlyPath);
    assertNotNull(fileBytes, "File byte array should not be null");
    assertTrue(fileBytes.length > 0, "File byte array should contain data");
  }

  @Test
  public void testOptions(){
    Options options = engine.getOptions();
    options.setAsString("scene.force_reader", "PLYReader");
    assertEquals("PLYReader", options.getAsString("scene.force_reader"),"Force reader option should match PLYReader");
  }

  @Test
  public void testAddSceneFromBuffer() throws IOException {
    byte[] fileBytes = Files.readAllBytes(pointsPlyPath);
    assertDoesNotThrow(() -> {
      scene.add(fileBytes);
    }, "Adding scene data from a byte buffer should succeed without throwing exceptions");

    assertNotNull(scene.getSceneInfo(), "Scene info should not be null after buffer loading");
  }
}