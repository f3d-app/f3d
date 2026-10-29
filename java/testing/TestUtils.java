import static org.junit.jupiter.api.Assertions.*;

import java.util.List;
import org.junit.jupiter.api.Test;

import app.f3d.F3D.*;

public class TestUtils {

  @Test // I did not find how exactly is distance calculating so i hardcoded values based on previous runs
  public void testTextDistance() {
    int distance1 = Utils.textDistance("kitten", "sitting");
    assertNotNull(distance1,"Distance1 should not be null");
    assertEquals(3, distance1, "Distance between 'kitten' and 'sitting' should be 3");

    int distance2 = Utils.textDistance("same", "same");
    assertNotNull(distance2,"Distance2 should not be null");
    assertEquals(0, distance2, "Distance between identical strings should be 0");
  }

  @Test // There is nowhere description on how and why there is optional boolean value os i can not make a test for it
  public void testTokenize() {
    List<String> tokens1 = Utils.tokenize("one two \"three four\" # comment");
    assertNotNull(tokens1, "Tokens list 1 should not be null");
    assertFalse(tokens1.isEmpty(), "Tokens list 1 should not be empty");
    assertEquals(3, tokens1.size(),"Size of tokens list 1 should be 3");

    List<String> tokens2 = Utils.tokenize("one two \"three four\" # comment", true);
    assertNotNull(tokens2, "Tokens list 2 should not be null");

    List<String> tokens3 = Utils.tokenize("one two \"three four\" # comment", false);
    assertNotNull(tokens3, "Tokens list 3 should not be null");
  }

  @Test
  public void testCollapsePath() {
    String collapsed1 = Utils.collapsePath(".");
    assertNotNull(collapsed1, "Collapsed path 1 should not be null");

    String collapsed2 = Utils.collapsePath(".", "");
    assertNotNull(collapsed2, "Collapsed path 2 should not be null");

    String collapsed3 = Utils.collapsePath("test.txt", "/tmp");
    assertNotNull(collapsed3, "Collapsed path 3 should not be null");
  }

  @Test
  public void testGlobToRegex() {
    String regex1 = Utils.globToRegex("*.txt");
    assertNotNull(regex1, "Regex 1 should not be null");

    String regex2 = Utils.globToRegex("*.txt", '/');
    assertNotNull(regex2, "Regex 2 should not be null");

    String regex3 = Utils.globToRegex("test/**/*.txt", '/');
    assertNotNull(regex3, "Regex 3 should not be null");
  }

  @Test
  public void testGetEnv() {
    String envNonExistent = Utils.getEnv("F3D_NONEXISTENT_VAR_12345");
    assertTrue(envNonExistent == null || envNonExistent.isEmpty(), "Nonexistent environment variable should be null or empty");
    String envPath = Utils.getEnv("PATH");
    assertNotNull(envPath,"PATH should be existing");
  }

  @Test
  public void testKnownFolderEnum() {
    assertNotNull(Utils.KnownFolder.ROAMINGAPPDATA.getValue());
    assertNotNull(Utils.KnownFolder.LOCALAPPDATA.getValue());
    assertNotNull(Utils.KnownFolder.PICTURES.getValue());

    assertEquals(Utils.KnownFolder.ROAMINGAPPDATA, Utils.KnownFolder.fromValue(Utils.KnownFolder.ROAMINGAPPDATA.getValue()));
    assertEquals(Utils.KnownFolder.LOCALAPPDATA, Utils.KnownFolder.fromValue(Utils.KnownFolder.LOCALAPPDATA.getValue()));
    assertEquals(Utils.KnownFolder.PICTURES, Utils.KnownFolder.fromValue(Utils.KnownFolder.PICTURES.getValue()));
  }

  @Test
  public void testFromValueError(){
    assertThrows(IllegalArgumentException.class, () -> {
      Utils.KnownFolder.fromValue(-1);
    },"Unknown folder should throw IllegalArgumentException");
  }

  @Test
  public void testGetKnownFolder() {
    assertDoesNotThrow(() -> {
      Utils.getKnownFolder(Utils.KnownFolder.ROAMINGAPPDATA);
      Utils.getKnownFolder(Utils.KnownFolder.LOCALAPPDATA);
      Utils.getKnownFolder(Utils.KnownFolder.PICTURES);
    }, "Getting known folders should execute without throwing exceptions");
  }
}