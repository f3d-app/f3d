import app.f3d.F3D.*;

public class TestImageStream {
  public static void main(String[] args) {
    Image img1 = new Image(300, 200, 3);

    byte[] img1Buffer = img1.saveBuffer(Image.SaveFormat.PNG);

    Image img2 = new Image(img1Buffer);

    img1.equals(img2);

    img1.delete();
    img2.delete();
  }
}
