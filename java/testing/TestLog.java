import static org.junit.jupiter.api.Assertions.*;

import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;
import app.f3d.F3D.*;

public class TestLog {

  static class TestCallback implements Log.ForwardCallback {
    private boolean messageReceived = false;

    @Override
    public void onLogMessage(Log.VerboseLevel level, String message){
      messageReceived = true;
    }

    public boolean isMessageReceived(){
      return messageReceived;
    }
  }

  @Test
  public void testObjectMaking(){
    Log.VerboseLevel testLevel = Log.getVerboseLevel();
    assertNotNull(testLevel, "Verbose level should not be null");
    TestCallback callback = new TestCallback();
    assertNotNull(callback,"Callback should not be null");
  }

  @Test
  public void testVerboseLevelGetAndSet(){
    Log.VerboseLevel originalLevel = Log.getVerboseLevel();
    try{
      Log.setVerboseLevel(Log.VerboseLevel.DEBUG);
      assertEquals(Log.VerboseLevel.DEBUG, Log.getVerboseLevel(), "Verbose level should be set to DEBUG");
      Log.setVerboseLevel(Log.VerboseLevel.QUIET);
      assertEquals(Log.VerboseLevel.QUIET, Log.getVerboseLevel(), "Verbose level should be set to QUIET");
    }finally{
      Log.setVerboseLevel(originalLevel);
    }
  }

  @Test
  @Disabled("There is no documentation on how boolean is affecting setter")
  public void testVerboseLevelBooleanSetting(){
    Log.VerboseLevel originalLevel = Log.getVerboseLevel();
    try{
      // TODO
    }finally{
      Log.setVerboseLevel(originalLevel);
    }
  }

  @Test // There is missing getter for use color setting so this only tests if operation of setting color does not throw exception
  public void testColoringSetting(){
    assertDoesNotThrow(() -> {
      Log.setUseColoring(true);
      Log.setUseColoring(false);
    }, "Configuring terminal coloring should execute without exception");
  }

  @Test // For better testing i suggest print function to not be void but boolean so it is easier to check individual callings
  public void testLogPrintingAndShorthands(){
    Log.VerboseLevel originalLevel = Log.getVerboseLevel();
    try{
      Log.setVerboseLevel(Log.VerboseLevel.DEBUG);

      assertDoesNotThrow(() -> {
        Log.print(Log.VerboseLevel.DEBUG, "Test debug message");
        Log.print(Log.VerboseLevel.INFO, "Test info message");
        Log.print(Log.VerboseLevel.WARN, "Test warning message");
        Log.print(Log.VerboseLevel.ERROR, "Test error message");

        Log.debug("Test debug");
        Log.info("Test info");
        Log.warn("Test warning");
        Log.error("Test error");
      }, "Log printing methods should execute without exceptions");
    }finally{
      Log.setVerboseLevel(originalLevel);
    }
  }

  @Test // There should be some kind of return value or getter for verbose level with print function to see if it was correctly received
  public void testForwardCallback(){
    TestCallback callback = new TestCallback();
    Log.forward(callback);

    assertDoesNotThrow(() -> {
      Log.print(Log.VerboseLevel.WARN,"Test callback message");
      Log.info("First message");
      Log.warn("Second message");
    },"Messages should be called without error");

    assertTrue(callback.isMessageReceived(), "Forward callback should receive dispatched log messages");
    Log.forward(null);
  }

  @Test
  public void testVerboseLevelEnumValues(){
    assertNotNull(Log.VerboseLevel.DEBUG.getValue());
    assertNotNull(Log.VerboseLevel.INFO.getValue());
    assertNotNull(Log.VerboseLevel.WARN.getValue());
    assertNotNull(Log.VerboseLevel.ERROR.getValue());
    assertNotNull(Log.VerboseLevel.QUIET.getValue());

    assertEquals(Log.VerboseLevel.DEBUG, Log.VerboseLevel.fromValue(Log.VerboseLevel.DEBUG.getValue()));
    assertEquals(Log.VerboseLevel.INFO, Log.VerboseLevel.fromValue(Log.VerboseLevel.INFO.getValue()));
    assertEquals(Log.VerboseLevel.WARN, Log.VerboseLevel.fromValue(Log.VerboseLevel.WARN.getValue()));
    assertEquals(Log.VerboseLevel.ERROR, Log.VerboseLevel.fromValue(Log.VerboseLevel.ERROR.getValue()));
    assertEquals(Log.VerboseLevel.QUIET, Log.VerboseLevel.fromValue(Log.VerboseLevel.QUIET.getValue()));
  }

  @Test
  public void testIllegalArgumentFromValue(){
    assertThrows(IllegalArgumentException.class, () -> {
      Log.VerboseLevel.fromValue(-1);
    },"Making invalid verbose level from invalid value with fromValue should thor IllegalArgumentException");
  }
}