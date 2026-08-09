import app.f3d.F3D.*;

public class TestImage {

  public static void main(String[] args) {
    Engine.autoloadPlugins();

    String testDataPath = args.length > 0 ? args[0] : ".";
    String tmpPath = args.length > 1 ? args[1] : "/tmp/";

    Image.getSupportedFormats();

    Image img1 = new Image(300, 200, 3);
    img1.getWidth();
    img1.getHeight();
    img1.getChannelCount();
    img1.getChannelType();
    img1.getChannelTypeSize();

    byte[] buffer = new byte[300 * 200 * 3];
    img1.setContent(buffer);
    img1.getContent();

    img1.getNormalizedPixel(10, 10);

    img1.save(tmpPath + "test.png");
    img1.save(tmpPath + "test.jpg", Image.SaveFormat.JPG);

    img1.saveBuffer();
    img1.saveBuffer(Image.SaveFormat.PNG);

    img1.setMetadata("key1", "value1");
    img1.getMetadata("key1");
    img1.allMetadata();

    Image img2 = new Image(300, 200, 3, Image.ChannelType.BYTE);

    img1.compare(img2);
    img1.equals(img2);
    img1.notEquals(img2);

    Image img3 = new Image(testDataPath + "data/world.png");
    img3.getWidth();
    img3.toTerminalText();

    img1.setContent(buffer)
        .setMetadata("author", "F3D")
        .save(tmpPath + "chained.png");

    img1.delete();
    img2.delete();
    img3.delete();

    // --- Exception handling tests ---

    // Reading a nonexistent image file must throw ReadException instead of crashing the JVM.
    try {
      new Image("/absolutely_nonexistent_image_f3d_test.png");
      throw new RuntimeException("Expected Image.ReadException was not thrown");
    } catch (Image.ReadException e) {
    }

    // Getting nonexistent metadata must throw MetadataException.
    Image tmpImg = new Image(300, 200, 3);
    try {
      tmpImg.getMetadata("key_that_does_not_exist");
      throw new RuntimeException("Expected Image.MetadataException was not thrown");
    } catch (Image.MetadataException e) {
    } finally {
      tmpImg.delete();
    }
  }
}
