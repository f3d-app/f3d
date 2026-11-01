import static org.junit.jupiter.api.Assertions.*;

import java.lang.reflect.Type;

import org.junit.jupiter.api.Disabled;
import org.junit.jupiter.api.Test;

import app.f3d.F3D.*;
import app.f3d.F3D.Types.Mesh.ValidationResult;

public class TestTypes {

  @Test
  public void testLightTypeEnum() {
    Types.LightType lightType = Types.LightType.SCENE_LIGHT;
    assertNotNull(lightType.getValue(), "LightType value should not be null");

    assertEquals(Types.LightType.SCENE_LIGHT, Types.LightType.fromValue(lightType.getValue()), "fromValue should match original LightType");
    assertNotNull(Types.LightType.fromValue(1), "LightType fromValue 1 should not be null");
    assertNotNull(Types.LightType.fromValue(2), "LightType fromValue 2 should not be null");
    assertNotNull(Types.LightType.fromValue(3), "LightType fromValue 3 should not be null");
  }

  @Test
  public void testLightTypeException() {
    assertThrows(IllegalArgumentException.class, () -> {
      Types.LightType.fromValue(-1);
    },"Inputting wrong LightType should result in IllegalArgumentException");
  }

  @Test
  public void testLightStateConstructors() {
    Types.LightState defaultLight = new Types.LightState();
    assertNotNull(defaultLight, "Default LightState should not be null");

    Types.LightState customLight = new Types.LightState(
      Types.LightType.CAMERA_LIGHT,
      new double[] { 1.0, 2.0, 3.0 },
      new double[] { 0.8, 0.8, 1.0 },
      new double[] { 0.0, 0.0, -1.0 },
      true,
      0.5,
      false
    );
    assertNotNull(customLight, "Custom LightState should not be null");
    assertFalse(customLight.equals(defaultLight), "Custom and default light states should not be equal");
    assertTrue(customLight.equals(customLight), "LightState should equal itself");
  }

  @Test
  public void testColormapConstructors() {
    Types.Colormap defaultColormap = new Types.Colormap();
    assertNotNull(defaultColormap, "Default Colormap should not be null");

    Types.Colormap customColormap = new Types.Colormap(new double[] { 0.0, 1.0, 0.0, 0.0 });
    assertNotNull(customColormap, "Custom Colormap should not be null");
  }

  @Test
  public void testTransform2DConstructorsAndFactory() {
    Transform2D defaultTransform = new Transform2D();
    assertNotNull(defaultTransform, "Default Transform2D should not be null");

    Transform2D matrixTransform = new Transform2D(new double[] { 1, 0, 0, 0, 1, 0, 0, 0, 1 });
    assertNotNull(matrixTransform, "Matrix Transform2D should not be null");

    Transform2D factoryTransform = Transform2D.create(new double[] { 1.0, 1.0 }, new double[] { 0.0, 0.0 }, 45.0);
    assertNotNull(factoryTransform, "Factory created Transform2D should not be null");
  }

  @Test
  @Disabled ("There are errors in how isValid() function is called, but the problem is with how c++ handles the java")
  public void testMeshValidation() {
    // TODO
    Types.Mesh emptyMesh = new Types.Mesh();

    float[] points = new float[] {
      0.0f, 0.0f, 0.0f,
      1.0f, 0.0f, 0.0f,
      0.0f, 1.0f, 0.0f
    };
    int[] faceSides = new int[] { 3 };
    int[] faceIndices = new int[] { 0, 1, 2 };

    Types.Mesh triangleMesh = new Types.Mesh(points, new float[0], new float[0], faceSides, faceIndices);

    float[] normals = new float[] {
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f,
      0.0f, 0.0f, 1.0f
    };
    Types.Mesh meshWithNormals = new Types.Mesh(points, normals, new float[0], faceSides, faceIndices);

    Types.Mesh pointCloud = new Types.Mesh(points, new float[0], new float[0], new int[0], new int[0]);
  }
}