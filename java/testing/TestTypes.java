import app.f3d.F3D.*;

public class TestTypes {

  public static void main(String[] args) {
    Types.LightType lightType = Types.LightType.SCENE_LIGHT;
    lightType.getValue();
    Types.LightType.fromValue(1);
    Types.LightType.fromValue(2);
    Types.LightType.fromValue(3);

    Types.LightState lightState = new Types.LightState();
    Types.LightState customLight = new Types.LightState(
      Types.LightType.CAMERA_LIGHT,
      new double[] { 1.0, 2.0, 3.0 },
      new double[] { 0.8, 0.8, 1.0 },
      new double[] { 0.0, 0.0, -1.0 },
      true,
      0.5,
      false
    );
    customLight.equals(lightState);

    Types.Colormap colormap = new Types.Colormap();
    Types.Colormap colormap2 = new Types.Colormap(new double[] { 0.0, 1.0, 0.0, 0.0 });

    Transform2D transform = new Transform2D();
    Transform2D transform2 = new Transform2D(new double[] { 1, 0, 0, 0, 1, 0, 0, 0, 1 });
    Transform2D.create(new double[] { 1.0, 1.0 }, new double[] { 0.0, 0.0 }, 45.0);

    Types.Mesh emptyMesh = new Types.Mesh();
    emptyMesh.isValid();

    float[] points = new float[] {
      0.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f
    };
    int[] faceSides = new int[] { 3 };
    int[] faceIndices = new int[] { 0, 1, 2 };

    Types.Mesh triangleMesh = new Types.Mesh(points, new float[0], new float[0], faceSides, faceIndices);
    triangleMesh.isValid();

    float[] normals = new float[] {
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f
    };
    Types.Mesh meshWithNormals = new Types.Mesh(points, normals, new float[0], faceSides, faceIndices);
    meshWithNormals.isValid();

    Types.Mesh pointCloud = new Types.Mesh(points, new float[0], new float[0], new int[0], new int[0]);
    pointCloud.isValid();
  }
}
