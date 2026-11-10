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
      false,
      false
    );
    customLight.equals(lightState);

    Types.Colormap colormap = new Types.Colormap();
    Types.Colormap colormap2 = new Types.Colormap(new double[] { 0.0, 1.0, 0.0, 0.0 });

    Transform2D transform = new Transform2D();
    Transform2D transform2 = new Transform2D(new double[] { 1, 0, 0, 0, 1, 0, 0, 0, 1 });
    Transform2D.create(new double[] { 1.0, 1.0 }, new double[] { 0.0, 0.0 }, 45.0);

    if (!transform.compare(transform2, 0.0) || transform.compare(null, 1e-6)) {
      throw new RuntimeException("Transform comparison failed");
    }
    for (int component = 0; component < 9; component++) {
      Transform2D near = new Transform2D(transform.data.clone());
      near.data[component] += 0.0000005;
      if (!transform.compare(near, 1e-6) || !near.compare(transform, 1e-6) ||
          transform.compare(near, 0.0)) {
        throw new RuntimeException("Transform comparison failed");
      }

      near.data[component] += 0.000002;
      if (transform.compare(near, 1e-6)) {
        throw new RuntimeException("Different transforms should not compare equal");
      }
    }

    Transform2D zero = new Transform2D(new double[9]);
    Transform2D boundary = new Transform2D(new double[9]);
    boundary.data[8] = 1e-6;
    if (!zero.compare(boundary, 1e-6)) {
      throw new RuntimeException("Transforms at the tolerance should compare equal");
    }

    Transform2D invalid = new Transform2D();
    invalid.data = new double[8];
    try {
      transform.compare(invalid, 1e-6);
      throw new RuntimeException("Expected IllegalArgumentException was not thrown");
    } catch (IllegalArgumentException e) {
    }
    try {
      invalid.compare(transform, 1e-6);
      throw new RuntimeException("Expected IllegalArgumentException was not thrown");
    } catch (IllegalArgumentException e) {
    }

    invalid.data = null;
    try {
      transform.compare(invalid, 1e-6);
      throw new RuntimeException("Expected IllegalArgumentException was not thrown");
    } catch (IllegalArgumentException e) {
    }
    try {
      invalid.compare(transform, 1e-6);
      throw new RuntimeException("Expected IllegalArgumentException was not thrown");
    } catch (IllegalArgumentException e) {
    }

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
