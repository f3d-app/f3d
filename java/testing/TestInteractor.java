import static org.junit.jupiter.api.Assertions.*;

import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;

import app.f3d.F3D.*;
import java.util.List;


public class TestInteractor {

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
  private Interactor interactor;
  private String testCommand = null;

  @BeforeEach
  public void engineLoad(){
    Engine.autoloadPlugins();
    engine = Engine.create(true);
    interactor = engine.getInteractor();
  }
  @AfterEach
  public void closeEngine(){
    engine.close();
  }

  @Test
  public void testAddCommand(){
    List<String> testingString = interactor.getCommandActions();
    assertNotNull(testingString,"List of commands should not be null");
    interactor.addCommand("test::hello", command -> {System.out.println("This is testing command");});
    assertNotEquals(interactor.getCommandActions(), testingString, "Command should be added to list");
  }

  @Test
  public void testExecteAddedCommand(){
    interactor.addCommand("test::hello", command -> {testCommand = "Success";});
    boolean triggerCommand = interactor.triggerCommand("test::hello");
    assertNotNull(testCommand,"Test String should not be null");
    assertEquals("Success", testCommand, "Command should have executed");
    assertTrue(triggerCommand, "Trigger of command should be set to True");
    testCommand = null;
  }

  @Test
  public void testRemoveAddedCommand(){
    List<String> listCommands = interactor.getCommandActions();
    interactor.addCommand("test::hello", command -> {testCommand = "Success";});
    interactor.removeCommand("test::hello");
    assertEquals(listCommands, interactor.getCommandActions(), "Before list of commands should be same as after list of commands");
  }

  @Test
  public void testBindKey(){
    Interactor.InteractionBind testingBind = new Interactor.InteractionBind();
    testingBind.inter = "A";
    testingBind.format();
    assertEquals("A", testingBind.inter.toString(),"Binded key should be equal to value binded");
    interactor.removeBinding(testingBind);
  }

  @Test
  public void testDifferentModBindKey(){
    Interactor.InteractionBind testingBind = new Interactor.InteractionBind();
    testingBind.inter = "A";
    testingBind.mod = Interactor.ModifierKeys.CTRL;
    testingBind.format();
    assertEquals("CTRL", testingBind.mod.toString(),"Binded key mod should be equal to modifier (CTRL)");
    testingBind.mod = Interactor.ModifierKeys.CTRL_SHIFT;
    testingBind.format();
    assertEquals("CTRL_SHIFT", testingBind.mod.toString(),"Binded key mod should be equal to modifier (CTRL_SHIFT)");
    testingBind.mod = Interactor.ModifierKeys.SHIFT;
    testingBind.format();
    assertEquals("SHIFT", testingBind.mod.toString(),"Binded key mod should be equal to modifier (SHIFT)");
    testingBind.mod = Interactor.ModifierKeys.ANY;
    testingBind.format();
    assertEquals("ANY", testingBind.mod.toString(),"Binded key mod should be equal to modifier (ANY)");
    testingBind.mod = Interactor.ModifierKeys.NONE;
    testingBind.format();
    assertEquals("NONE", testingBind.mod.toString(),"Binded key mod should be equal to modifier (NONE)");
    interactor.removeBinding(testingBind);
  }

  @Test
  public void testParseMethod(){
    Interactor.InteractionBind parsed = Interactor.InteractionBind.parse("A");
    assertEquals("A", parsed.inter.toString(),"Parse method key should be equal to value set (A)");
    assertEquals("NONE", parsed.mod.toString(),"Parse modifier key should be equal to value set (NONE)");
    parsed = Interactor.InteractionBind.parse("Ctrl+A");
    assertEquals("A", parsed.inter.toString(),"Parse method key should be equal to value set (A)");
    assertEquals("CTRL", parsed.mod.toString(),"Parse modifier key should be equal to value set (NONE)");
    parsed = Interactor.InteractionBind.parse("Shift+A");
    assertEquals("A", parsed.inter.toString(),"Parse method key should be equal to value set (A)");
    assertEquals("SHIFT", parsed.mod.toString(),"Parse modifier key should be equal to value set (NONE)");
    parsed = Interactor.InteractionBind.parse("Any+A");
    assertEquals("A", parsed.inter.toString(),"Parse method key should be equal to value set (A)");
    assertEquals("ANY", parsed.mod.toString(),"Parse modifier key should be equal to value set (NONE)");
    parsed = Interactor.InteractionBind.parse("Ctrl+Shift+A");
    assertEquals("A", parsed.inter.toString(),"Parse method key should be equal to value set (A)");
    assertEquals("CTRL_SHIFT", parsed.mod.toString(),"Parse modifier key should be equal to value set (NONE)");
    interactor.removeBinding(parsed);
  }

  @Test
  public void testCompareTo(){
    Interactor.InteractionBind testingBind = new Interactor.InteractionBind();
    testingBind.inter = "A";
    Interactor.InteractionBind parsed = Interactor.InteractionBind.parse("A");
    assertEquals(0, parsed.compareTo(testingBind),"Comparing same bind keys should result in 0");
    parsed = Interactor.InteractionBind.parse("B");
    assertEquals(1, parsed.compareTo(testingBind),"Comparing different bind keys should result in 1");
    interactor.removeBinding(parsed);
    interactor.removeBinding(testingBind);
  }

  @Test
  public void testEquals(){
    Interactor.InteractionBind testingBind = new Interactor.InteractionBind();
    testingBind.inter = "A";
    Interactor.InteractionBind parsed = Interactor.InteractionBind.parse("A");
    assertTrue(parsed.equals(testingBind),"Same keys should return True with equals method");
    parsed = Interactor.InteractionBind.parse("B");
    assertFalse(parsed.equals(testingBind),"Different keys should return False with equals method");
    interactor.removeBinding(parsed);
    interactor.removeBinding(testingBind);
  }

  @Test
  public void testHashCode(){
    Interactor.InteractionBind testingBind = new Interactor.InteractionBind();
    testingBind.inter = "A";
    int testHash[] = new int[5];
    for(int i = 0; i < 5; i++)
      testHash[i] = testingBind.hashCode();
    for(int i = 0; i < 5; i++)
      assertEquals(testHash[0], testHash[i],"All hash values should be the same (" + i + ")");
    interactor.removeBinding(testingBind);
  }

  @Test
  public void testAddExistingBinding(){
    Interactor.InteractionBind testingBind = new Interactor.InteractionBind();
    testingBind.inter = "A";
    testingBind.format();

    assertThrows(Interactor.AlreadyExistsException.class, () -> {
      interactor.addBinding(testingBind, "test_command", "test_group", Interactor.BindingType.NUMERICAL, false, true);
    });
    interactor.removeBinding(testingBind);
  }

  @Test
  public void testAddBinding(){
    Interactor.InteractionBind testingBind = new Interactor.InteractionBind();
    testingBind.inter = "l";
    testingBind.format();

    interactor.addBinding(testingBind, "test_command", "test_group", Interactor.BindingType.NUMERICAL, false, true);

    List<Interactor.InteractionBind> groupBindings = interactor.getBindsForGroup("test_group");
    assertNotNull(groupBindings, "Bindings list for the group should not be null");

    boolean isPresent = groupBindings.stream().anyMatch(b -> b.equals(testingBind));
    assertTrue(isPresent, "The group should contain the newly added binding");

    interactor.removeBinding(testingBind);
  }

  @Test
  public void testGroupBindings(){
    Interactor.InteractionBind repeatBind = new Interactor.InteractionBind();
    repeatBind.mod = Interactor.ModifierKeys.NONE;
    repeatBind.inter = "R";
    interactor.removeBinding(repeatBind);

    interactor.addBinding(repeatBind, "test_command", "test_group", Interactor.BindingType.NUMERICAL, false, true);
    List<Interactor.InteractionBind> testBindings = interactor.getBindsForGroup("test_group");
    assertNotNull(testBindings, "Bindings list for group should not be null");
    boolean allMatch = testBindings.stream().allMatch((binding) -> binding.equals(repeatBind));
    assertTrue(allMatch, "All bindings in group should match repeatBind");

    interactor.removeBinding(repeatBind);
  }

  @Test
  public void testAnimationControls(){
    assertDoesNotThrow(() -> {
      interactor.startAnimation();
      assertTrue(interactor.isPlayingAnimation(), "Animation should be playing after start");

      interactor.stopAnimation();
      assertFalse(interactor.isPlayingAnimation(), "Animation should not be playing after stop");

      interactor.startAnimation(Interactor.AnimationDirection.FORWARD);
      assertEquals(Interactor.AnimationDirection.FORWARD, interactor.getAnimationDirection(), "Direction should be equal to tested direction (Forward)");

      interactor.startAnimation(Interactor.AnimationDirection.BACKWARD);
      assertEquals(Interactor.AnimationDirection.BACKWARD, interactor.getAnimationDirection(), "Direction should be equal to tested direction (Backward)");

      interactor.stopAnimation();
    }, "Animation controls should execute without errors");
  }

  @Test
  public void testAnimationDirectionEnum(){
    assertNotNull(Interactor.AnimationDirection.FORWARD.getValue(), "Value of animation should not be null (Forward)");
    assertNotNull(Interactor.AnimationDirection.BACKWARD.getValue(), "Value of animation should not be null (Forward)");

    assertEquals(Interactor.AnimationDirection.FORWARD, Interactor.AnimationDirection.fromValue(Interactor.AnimationDirection.FORWARD.getValue()), "Value of getValue from fromValue should be equal (Forward)");
    assertEquals(Interactor.AnimationDirection.BACKWARD, Interactor.AnimationDirection.fromValue(Interactor.AnimationDirection.BACKWARD.getValue()), "Value of getValue from fromValue should be equal (Forward)");
  }

  // This needs more testing because there is no documentation on what is in bindStr and output of these values are mapped wrong
  @Test
  @Disabled("This needs more testing on how setNotificationCallback binds parameters")
  public void testNotificationCallbackValues(){
    final String[] capturedDesc = new String[1];
    final String[] capturedValue = new String[1];
    final String[] capturedBindStr = new String[1];
    final double[] capturedDuration = new double[1];
    final boolean[] callbackInvoked = {false};

    interactor.setNotificationCallback((desc, value, bindStr, duration) -> {
      callbackInvoked[0] = true;
      capturedDesc[0] = desc;
      capturedValue[0] = value;
      capturedBindStr[0] = bindStr;
      capturedDuration[0] = duration;
      return true;
    });

    interactor.triggerNotification("foo", "bar", 3.0);

    assertTrue(callbackInvoked[0], "Notification callback should have been invoked");
    assertEquals("foo", capturedDesc[0], "Notification description should match");
    assertEquals("bar", capturedValue[0], "Notification value should match");
    assertEquals("", capturedBindStr[0], "Notification BindStr should match");
    assertEquals(3.0, capturedDuration[0], 0.0001, "Notification duration should match");
  }

  @Test
  public void testCameraMovementAndRender(){
    assertDoesNotThrow(() -> {
      interactor.disableCameraMovement();
      interactor.enableCameraMovement();
      interactor.requestRender();
    }, "Camera movement and render requests should execute successfully");
  }

  @Test
  public void testInteractorMethodChaining(){
    assertDoesNotThrow(() -> {
      interactor.toggleAnimation()
             .enableCameraMovement();
      interactor.stopAnimation();
    }, "Interactor methods should support fluent chaining");
  }
}
