import static org.junit.jupiter.api.Assertions.*;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;

import app.f3d.F3D.*;

public class TestOptions {

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
  private Options options;

  @BeforeEach
  public void setUp() {
    Engine.autoloadPlugins();
    engine = Engine.createNone();
    options = engine.getOptions();
  }

  @AfterEach
  public void tearDown() {
      engine.close();
  }

  @Test
  public void testBooleanOptions() {
    options.setAsBool("ui.scalar_bar", true);
    assertTrue(options.getAsBool("ui.scalar_bar"), "Option should be true");

    options.toggle("ui.scalar_bar");
    assertFalse(options.getAsBool("ui.scalar_bar"), "Option should be false after toggle");
  }

  @Test
  public void testIntOptions() {
    options.setAsInt("model.scivis.component", 5);
    assertEquals(5, options.getAsInt("model.scivis.component"), "Integer option value should match");
  }

  @Test
  public void testDoubleOptions() {
    options.setAsDouble("render.line_width", 2.0);
    assertEquals(2.0, options.getAsDouble("render.line_width"), "Double option value should match");
  }

  @Test
  public void testStringOptions() {
    options.setAsString("model.color.texture", "test.png");
    assertEquals("test.png", options.getAsString("model.color.texture"), "String option value should match");
  }

  @Test
  public void testDoubleVectorOptions() {
    double[] vec = new double[]{1.0, 2.0, 3.0};
    options.setAsDoubleVector("render.background.color", vec);
    assertArrayEquals(vec, options.getAsDoubleVector("render.background.color"),"Double vector options should match");
  }

  @Test
  public void testIntVectorOptions() {
    int[] vec = new int[]{1, 2};
    options.setAsIntVector("scene.animation.indices", vec);
    assertArrayEquals(vec, options.getAsIntVector("scene.animation.indices"), "Int vector options should match");
  }

  @Test
  @Disabled ("Function getAsStringRepresentation is return 2 different values for 5.0 -> 5 and for 5.3 -> 5.3, the return value should be uniformed")
  public void testStringRepresentationOptions() {
    options.setAsStringRepresentation("render.point_size", "5.0");
    assertEquals("5.0", options.getAsStringRepresentation("render.point_size"), "String representation should match");
  }

  @Test // getAllNames in not dependant on anything, it can return null or list so i don not know how to make a test for it
  // isOptional is not stated how it is determining the value so i can not make a test for it
  public void testIntrospectionAndQueries() {
    options.setAsBool("ui.scalar_bar", true);
    assertTrue(options.hasValue("ui.scalar_bar"), "Function hasValue should return true for ui.scalar_bar");
    assertNotNull(options.getNames(), "Base names list should not be null");
    assertNotNull(options.getClosestOption("ui.scalar"), "Closest option lookup should return a result");
    assertNotNull(options.getType("ui.scalar_bar"), "ui.scalar_bar type should not be null");
    assertEquals(Options.OptionType.BOOL, options.getType("ui.scalar_bar"),"ui.scalar_bar should be type of BOOL");
  }

  @Test
  public void testOptionsCopy() {
    options.setAsBool("ui.scalar_bar", true);
    Engine engineToCopy = Engine.create(true);
    Options optionsToCopy = engineToCopy.getOptions();
    assertNotNull(optionsToCopy,"Options for copying should be created");
    optionsToCopy.copy(options, "ui.scalar_bar");
    assertTrue(optionsToCopy.isSame(options, "ui.scalar_bar"),"Options and copy of options should be same");
    engineToCopy.close();
  }

  @Test
  public void testReset() {
    options.setAsInt("model.scivis.component", 5);
    assertEquals(5,options.getAsInt("model.scivis.component"),"Option should be value of setter");
    options.reset("model.scivis.component");
    assertEquals(-1,options.getAsInt("model.scivis.component"),"Option should be value of -1 after reset");
  }

  @Test
  public void testRemove() {
    options.setAsString("model.color.texture", "test.png");
    assertEquals("test.png", options.getAsString("model.color.texture"),"Value of string option should be equal to set value");
    options.removeValue("model.color.texture");
    assertFalse(options.hasValue("model.color.texture"), "Value should be removed");
  }

  @Test // options for increase, decrease and cycle have no value returning so it can not be checked explicitly
  @Disabled("Index is null for domain with fresh options")
  public void testDomains() {
    assertTrue(options.hasDomain("scene.camera.index"),"Option hasDomain should not be null");
    assertNotNull(options.getDomainStyle("scene.camera.index"),"Option DomainStyle should not be null");
    assertNotNull(options.getRangeDomainAsDouble("render.line_width"),"Option RangeDomainAsDouble should not be null");
    assertNotNull(options.getRangeDomainAsInt("render.raytracing.samples"),"Option RangeDomainAsInt should not be null");
    assertNotNull(options.getEnumDomainAsString("render.effect.blending.mode"),"Option EnumDomainAsString should not be null");
    assertNotNull(options.getIndexDomain("scene.camera.index"),"Option IndexDomain should not be null");
    assertDoesNotThrow(() -> {
      options.increase("render.raytracing.samples");
      options.decrease("render.raytracing.samples");
      options.cycle("render.effect.blending.mode");
    }, "Domain increase, decrease and cycle should not throw anything");
  }

  @Test
  public void testInexistentOptionThrowsException() {
    assertThrows(Options.InexistentException.class, () -> {
      options.setAsBool("this.option.does.not.exist.at.all", true);
    }, "Setting a nonexistent option should throw Options.InexistentException");
  }

  @Test
  public void testIncompatibleTypeThrowsException() {
    options.setAsDoubleVector("render.background.color", new double[]{1.0, 2.0, 3.0});

    assertThrows(Options.IncompatibleException.class, () -> {
      options.getAsBool("render.background.color");
    }, "Getting an option with the wrong type must throw Options.IncompatibleException");
  }
}