import static org.junit.jupiter.api.Assertions.*;

import java.nio.file.Files;
import java.nio.file.Path;
import java.util.List;

import org.junit.jupiter.api.Test;
import org.junit.jupiter.api.AfterEach;
import org.junit.jupiter.api.BeforeEach;
import org.junit.jupiter.api.Disabled;

import app.f3d.F3D.*;

public class TestImage {

  private Image img;

  @BeforeEach
  public void setUp() {
    Engine.autoloadPlugins();
  }

  @AfterEach
  public void clearTestImage() {
    if(img != null) {
      img.delete();
    }
  }

  @Test
  public void testImageSettings() {
    img = new Image(300,200,3);
    assertEquals(300, img.getWidth(), "Width of image should equal to maker settings");
    assertEquals(200, img.getHeight(), "Height of image should equal to maker settings");
    assertEquals(3, img.getChannelCount(), "Channel count of image should equal to maker settings");
    assertEquals(Image.ChannelType.BYTE, img.getChannelType(), "Channel type should be type of BYTE");
    assertEquals(1, img.getChannelTypeSize(), "Channel type should be size of 1");
  }

  @Test
  public void testManipulationOfImage() {
    byte[] buffer = new byte[300*200*3];
    img = new Image(300,200,3);

    img.setContent(buffer);
    assertArrayEquals(buffer, img.getContent(), "Content of image should be modified");
  }

  // i can not find how this operates so i can not make test
  @Test
  public void testNormalizedPixels() {
    //img = new Image(300,200,3);
    //double test[] = img.getNormalizedPixel(10, 10);
  }

  @Test
  public void testSavePNG() {
    Path filePath = Path.of("dev", "f3d", "src", "testing", "tmp", "test.png");
    String fileDst = filePath.toAbsolutePath().toString().replace('\\', '/');
    img = new Image(300,200,3);
    img.save(fileDst);
    filePath = Path.of("dev", "f3d", "src", "testing", "tmp", "test.png");
    assertTrue(Files.exists(filePath), "File .png should be created exist");
  }

  @Test
  public void testSaveJPG() {
    Path filePath = Path.of("dev", "f3d", "src", "testing", "tmp", "test.jpg");
    String fileDst = filePath.toAbsolutePath().toString().replace('\\', '/');
    img = new Image(300,200,3);
    img.save(fileDst,Image.SaveFormat.JPG);
    filePath = Path.of("dev", "f3d", "src", "testing", "tmp", "test.jpg");
    assertTrue(Files.exists(filePath), "File .jpg should be created exist");
  }

  @Test
  public void testSaveContent() {
    byte buffer[] = new byte[100 * 100 * 2];
    img = new Image(300,200,3);
    img.setContent(buffer);
    byte testBuffer[] = new byte[buffer.length];
    System.arraycopy(img.getContent(), 0, testBuffer, 0, buffer.length);
    assertArrayEquals(testBuffer, buffer, "Content should be changed");
  }

  // This is not defined in scope of functions ↓
  @Test
  public void testSaveInvalidContent() {
    byte buffer[] = new byte[400 * 400 * 4];
    img = new Image(300,200,3);
    img.setContent(buffer);
    //byte testBuffer[] = new byte[buffer.length];
    //System.arraycopy(img.getContent(), 0, testBuffer, 0, buffer.length);
    //assertArrayEquals(testBuffer, buffer, "Content should be changed");
  }

  @Test
  public void testMetadata() {
    img = new Image(300,200,3);
    img.setMetadata("key1", "value1");
    assertNotNull(img.getMetadata("key1"),"Inserted metadata should not be null");
    assertEquals("value1", img.getMetadata("key1"),"Get method should return set value");
    assertFalse(img.allMetadata().isEmpty(),"Metadata should not be empty");
    assertEquals("value1", img.getMetadata("key1"),"Get method should return set value");
  }

  @Test
  public void testTwoSameImages() {
    img = new Image(300,200,3,Image.ChannelType.BYTE);
    Image img2 = new Image(300, 200, 3, Image.ChannelType.BYTE);

    double compare = img.compare(img2);
    boolean equals = img.equals(img2);
    boolean notEqual = img.notEquals(img2);

    assertEquals(0, compare,"Compare should return 0 for same images");
    assertTrue(equals,"Equals should return true for same images");
    assertFalse(notEqual,"notEquals should return false for same images");
    img2.delete();
  }

  @Test
  public void testTwoDifferentImages() {
    img = new Image(100,100,2,Image.ChannelType.BYTE);
    Image img2 = new Image(300, 200, 3, Image.ChannelType.BYTE);

    double compare = img.compare(img2);
    boolean equals = img.equals(img2);
    boolean notEqual = img.notEquals(img2);

    assertNotEquals(0, compare,"Compare should not return 0 for different images");
    assertTrue(notEqual,"notEquals should return true for different images");
    assertFalse(equals,"Equals should return false for different images");
    img2.delete();
  }
  
  @Test
  @Disabled("This test should return -1 but it is throwing exception because of how java handles null object")
  public void testNonExistentImage() {
    img = null;
    Image img2 = new Image(300, 200, 3, Image.ChannelType.BYTE);

    double compare = img.compare(img2);
    boolean equals = img.equals(img2);
    boolean notEqual = img.notEquals(img2);

    assertEquals(-1, compare,"Compare should return -1 for comparing image with non existent one");
    assertTrue(notEqual,"notEquals should return true for different images");
    assertFalse(equals,"Equals should return false for different images");
    img2.delete();
  }

  @Test
  public void testLoadFromFile() {
    Path filePath = Path.of("dev", "f3d", "src", "testing", "data", "world.png");
    String fileDst = filePath.toAbsolutePath().toString().replace('\\', '/');
    img = new Image(fileDst);
    assertNotNull(img,"Image should not be null");
  }

  // needs more testing if the file loaded is same as file on disk
  @Test
  public void testTerminalImage() {
    Path filePath = Path.of("dev", "f3d", "src", "testing", "data", "world.png");
    String fileDst = filePath.toAbsolutePath().toString().replace('\\', '/');
    img = new Image(fileDst);
    assertNotNull("Image should be loaded",img.toTerminalText());
  }

  @Test
  public void testChangeMetadataOfImage() {
    Path filePath = Path.of("dev", "f3d", "src", "testing", "data", "world.png");
    String fileDst = filePath.toAbsolutePath().toString().replace('\\', '/');
    img = new Image(fileDst);
    List<String> testPreviousMetadata = img.allMetadata();
    img.setMetadata("TestMetadata", "Author");
    List<String> testNewMetadata = img.allMetadata();
    assertNotEquals(testPreviousMetadata.size(), testNewMetadata.size(),"Size of previous and new metada should not equal");
    assertEquals("Author",img.getMetadata("TestMetadata"), "New metadata should equal to changed metadata");
  }

  @Test
  public void testNonExistentFile() {
    assertThrows(Image.ReadException.class, () -> {
      img = new Image("non_existent_path.jpg");
    }, "Opening non existent file should throw ReadException instead of crashing the JVM");
  }

  @Test
  public void testNonExistentMetaData() {
    img = new Image(300,200,3);
    assertThrows(Image.MetadataException.class, () -> {
      img.getMetadata("Non_existent");
    }, "Getting nonexistent metadata should throw MetadataException");
  }
}