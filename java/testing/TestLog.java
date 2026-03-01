import app.f3d.F3D.*;

public class TestLog {

  static class TestCallback implements Log.ForwardCallback {
    @Override
    public void onLogMessage(Log.VerboseLevel level, String message) {
    }
  }

  public static void main(String[] args) {
    Log.VerboseLevel level = Log.getVerboseLevel();

    Log.setVerboseLevel(Log.VerboseLevel.DEBUG);
    Log.getVerboseLevel();

    Log.setVerboseLevel(Log.VerboseLevel.QUIET);
    Log.getVerboseLevel();

    Log.setVerboseLevel(level);

    Log.setUseColoring(true);
    Log.setUseColoring(false);

    Log.VerboseLevel savedLevel = Log.getVerboseLevel();
    Log.setVerboseLevel(Log.VerboseLevel.QUIET);

    Log.print(Log.VerboseLevel.DEBUG, "Test debug message");
    Log.print(Log.VerboseLevel.INFO, "Test info message");
    Log.print(Log.VerboseLevel.WARN, "Test warning message");
    Log.print(Log.VerboseLevel.ERROR, "Test error message");

    Log.debug("Test debug");
    Log.info("Test info");
    Log.warn("Test warning");
    Log.error("Test error");

    Log.setVerboseLevel(Log.VerboseLevel.INFO, true);
    Log.setVerboseLevel(Log.VerboseLevel.INFO, false);

    Log.setVerboseLevel(savedLevel);

    TestCallback callback = new TestCallback();
    Log.forward(callback);

    Log.print(Log.VerboseLevel.WARN, "Test callback message");
    Log.info("First message");
    Log.warn("Second message");

    Log.forward(null);

    Log.VerboseLevel.DEBUG.getValue();
    Log.VerboseLevel.INFO.getValue();
    Log.VerboseLevel.WARN.getValue();
    Log.VerboseLevel.ERROR.getValue();
    Log.VerboseLevel.QUIET.getValue();

    Log.VerboseLevel.fromValue(0);
    Log.VerboseLevel.fromValue(1);
    Log.VerboseLevel.fromValue(2);
    Log.VerboseLevel.fromValue(3);
    Log.VerboseLevel.fromValue(4);
  }
}
