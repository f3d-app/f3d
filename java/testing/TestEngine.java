import app.f3d.F3D.*;

import org.junit.jupiter.api.Test;
import static org.junit.jupiter.api.Assertions.*;

import java.io.IOException;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;

import java.nio.file.Files;
import java.nio.file.Path;

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

  private Engine engine;

  @BeforeEach
  public void setUp() {
    Engine.autoloadPlugins();
  }

  @AfterEach
  public void clearTestEngine() {
    if(engine != null){
      engine.close();
    }
  }

  @Test
  public void testNativeCreateEngineTrue() {
    // Use try-with-resources to guarantee proper cleanup
    try (Engine eng = Engine.create(true)){
      assertNotNull(eng, "Engine created with true should not be null");
    }
  }

  @Test
  public void testNativeCreateEngineFalse() {
    try (Engine eng = Engine.create()){
      assertNotNull(eng, "Engine created with false should not be null");
    }
  }

  @Test
  public void testNativeCreateEngineNone() {
    try (Engine eng = Engine.createNone()) {
      assertNotNull(eng, "Engine created with createNone should not be null");
      assertThrows(Engine.NoInteractorException.class, () -> {
        eng.getInteractor();
      }, "Calling getInteractor on engine created with createNone should throw exception");
    }
  }

  @Test
  public void testLoadValidPlugin() {
    assertDoesNotThrow(() -> {
      Engine.loadPlugin("native");
    }, "Loading the native plugin should succeed without errors");
  }

  @Test
  public void testLoadNonExistentPluginThrowsException() {
    assertThrows(Engine.PluginException.class, () -> {
      Engine.loadPlugin("__nonexistent_plugin_f3d_test__");
    }, "Loading a nonexistent plugin must throw Engine.PluginException");
  }

  @Test
  public void testEngineStaticInfoAndPlugins() {
    assertNotNull(Engine.getPluginsList("."), "Plugins list should not be null");
    assertNotNull(Engine.getAllReaderOptionNames(), "All reader option names should not be null");
    assertNotNull(Engine.getLibInfo(), "Library info should not be null");
    assertNotNull(Engine.getReadersInfo(), "Readers info should not be null");
    assertNotNull(Engine.getRenderingBackendList(), "Rendering backend list should not be null");
  }

  @Test
  public void testEngineCachePathAndComponents() {
    engine = Engine.create(true);
    assertNotNull(engine, "Engine should not be null");

    String testPath = "/tmp/f3d_test";
    engine.setCachePath(testPath);
    assertEquals(testPath, engine.getCachePath(), "getCachePath should return the path set with setCachePath");

    assertNotNull(engine.getOptions(), "Engine options should not be null");
    assertNotNull(engine.getScene(), "Engine scene should not be null");
    assertNotNull(engine.getWindow(), "Engine window should not be null");
    assertNotNull(engine.getInteractor(), "Engine interactor should not be null");
  }

  @Test
  public void testInvalidStateFileThrows() {
    assertThrows(RuntimeException.class, () -> {
      Engine.State.fromFile("/does/not/exist/state.json");
    }, "State.fromFile should throw on a missing file");
  }

  @Test
  public void testLoadFile() throws IOException {
    Path filePath = Path.of("dev", "f3d", "src", "testing", "data", "cow.vtp");
    assertTrue(Files.exists(filePath), "File should exist");
    String cow = filePath.toAbsolutePath().toString().replace('\\', '/');

    engine = Engine.createNone();
    engine.getOptions().setAsBool("ui.scalar_bar", true);
    engine.getScene().add(cow);

    assertNotNull(engine.getScene().getSceneInfo(), "Scene info should not be null");
    assertEquals(1, engine.getScene().getAddedFiles().size(), "Scene should have 1 added file");

    Path testingPath = Path.of(System.getProperty("java.io.tmpdir"), "TestEngineJavaStatefile.json");
    String loadedTestingPath = testingPath.toAbsolutePath().toString().replace('\\', '/');

    try(Engine.State state = engine.dump()) {
      assertNotNull(state, "New state should not be null");
      state.toFile(loadedTestingPath);
    }
    assertTrue(Files.exists(testingPath), "File should be created");

    try(Engine dst = Engine.createNone();
    Engine.State state = Engine.State.fromFile(loadedTestingPath)) {
      assertNotNull(dst, "Destination Engine should be created");
      dst.load(state);
      assertTrue(dst.getOptions().getAsBool("ui.scalar_bar"), "Option should be loaded from file");
      assertEquals(1, dst.getScene().getAddedFiles().size(), "Should load one item from scene from file");
    }
  }

  @Test
  public void testLoadString() throws IOException {
    Path filePath = Path.of("dev", "f3d", "src", "testing", "data", "cow.vtp");
    assertTrue(Files.exists(filePath), "File should exist");
    String cow = filePath.toAbsolutePath().toString().replace('\\', '/');

    engine = Engine.createNone();
    engine.getOptions().setAsBool("ui.scalar_bar", true);
    engine.getScene().add(cow);

    assertNotNull(engine.getScene().getSceneInfo(), "Scene info should not be null");
    assertEquals(1, engine.getScene().getAddedFiles().size(), "Scene should have 1 added file");

    String fileFromString;
    try (Engine.State state = engine.dump()) {
      fileFromString = state.toString();
    }
    assertNotNull(fileFromString, "File from string should not be null");

    try (Engine dst = Engine.createNone();
         Engine.State state = Engine.State.fromString(fileFromString)) {
      assertNotNull(dst, "Destination Engine should be created");
      dst.load(state);
      assertTrue(dst.getOptions().getAsBool("ui.scalar_bar"), "Option should be loaded from string");
      assertEquals(1, dst.getScene().getAddedFiles().size(), "Should load one item from scene from string");
    }
  }
}