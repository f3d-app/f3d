import static org.junit.jupiter.api.Assertions.*;

import org.junit.jupiter.api.Test;

import app.f3d.F3D.*;

public class TestImageStream {

  @Test
  public void testSaveBufferObject(){
    Image img = new Image(300,200,3);
    byte[] imgBuffer = img.saveBuffer(Image.SaveFormat.PNG);
    assertNotNull(imgBuffer,"Buffer from image should not be null");
    img.delete();
  }
  @Test
  public void testSaveBufferToImage(){
    Image fromImg = new Image(300,200,3);
    byte[] imgBuffer = fromImg.saveBuffer(Image.SaveFormat.PNG);
    Image toImg = new Image(imgBuffer);
    assertNotNull(toImg,"New Image should not be null");
    assertTrue(fromImg.equals(toImg),"Images should be exactly equal to each other");
    fromImg.delete();
    toImg.delete();
  }
}
