import app.f3d.F3D.*;

import java.util.List;

public class TestUtils {

  static {
    if (System.getProperty("os.name").startsWith("Windows"))
    {
      System.loadLibrary("opengl32");
    }
  }

  public static void main(String[] args) {
    int distance1 = Utils.textDistance("kitten", "sitting");
    int distance2 = Utils.textDistance("same", "same");

    List<String> tokens1 = Utils.tokenize("one two \"three four\" # comment");
    List<String> tokens2 = Utils.tokenize("one two \"three four\" # comment", true);
    List<String> tokens3 = Utils.tokenize("one two \"three four\" # comment", false);

    String collapsed1 = Utils.collapsePath(".");
    String collapsed2 = Utils.collapsePath(".", "");
    String collapsed3 = Utils.collapsePath("test.txt", "/tmp");

    String regex1 = Utils.globToRegex("*.txt");
    String regex2 = Utils.globToRegex("*.txt", '/');
    String regex3 = Utils.globToRegex("test/**/*.txt", '/');

    String envPath = Utils.getEnv("PATH");
    String envNonExistent = Utils.getEnv("F3D_NONEXISTENT_VAR_12345");

    Utils.KnownFolder.ROAMINGAPPDATA.getValue();
    Utils.KnownFolder.LOCALAPPDATA.getValue();
    Utils.KnownFolder.PICTURES.getValue();
    Utils.KnownFolder.fromValue(0);
    Utils.KnownFolder.fromValue(1);
    Utils.KnownFolder.fromValue(2);

    String knownFolder1 = Utils.getKnownFolder(Utils.KnownFolder.ROAMINGAPPDATA);
    String knownFolder2 = Utils.getKnownFolder(Utils.KnownFolder.LOCALAPPDATA);
    String knownFolder3 = Utils.getKnownFolder(Utils.KnownFolder.PICTURES);
  }
}
