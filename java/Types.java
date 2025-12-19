package app.f3d.F3D;

public class Types {
    // Load the native library
    static {
        System.loadLibrary("f3d-java");
    }

    /**
     * Describe a colormap, which is a vector of repeated: val,r,g,b
     */
    public static class Colormap {
        public double[] data;

        public Colormap() {
            data = new double[0];
        }

        public Colormap(double[] data) {
            this.data = data;
        }
    }

    /**
     * Light type enumeration.
     */
    public enum LightType {
        HEADLIGHT(1),
        CAMERA_LIGHT(2),
        SCENE_LIGHT(3);

        private final int value;

        LightType(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public static LightType fromValue(int value) {
            for (LightType type : values()) {
                if (type.value == value) {
                    return type;
                }
            }
            throw new IllegalArgumentException("Unknown light type: " + value);
        }
    }

    /**
     * Structure describing the state of a light.
     */
    public static class LightState {
        public LightType type = LightType.SCENE_LIGHT;
        public double[] position = new double[] { 0.0, 0.0, 0.0 };
        public double[] color = new double[] { 1.0, 1.0, 1.0 };
        public double[] direction = new double[] { 1.0, 0.0, 0.0 };
        public boolean positionalLight = false;
        public double intensity = 1.0;
        public boolean switchState = true;

        public LightState() {
        }

        public LightState(LightType type, double[] position, double[] color,
                         double[] direction, boolean positionalLight,
                         double intensity, boolean switchState) {
            this.type = type;
            this.position = position;
            this.color = color;
            this.direction = direction;
            this.positionalLight = positionalLight;
            this.intensity = intensity;
            this.switchState = switchState;
        }

        @Override
        public boolean equals(Object obj) {
            if (this == obj) return true;
            if (obj == null || getClass() != obj.getClass()) return false;
            LightState other = (LightState) obj;
            return type == other.type &&
                   java.util.Arrays.equals(position, other.position) &&
                   java.util.Arrays.equals(color, other.color) &&
                   java.util.Arrays.equals(direction, other.direction) &&
                   positionalLight == other.positionalLight &&
                   intensity == other.intensity &&
                   switchState == other.switchState;
        }
    }

    /**
     * Describes a 3D surfacic mesh.
     */
    public static class Mesh {
        public float[] points = new float[0];
        public float[] normals = new float[0];
        public float[] textureCoordinates = new float[0];
        public int[] faceSides = new int[0];
        public int[] faceIndices = new int[0];

        public Mesh() {
        }

        public Mesh(float[] points, float[] normals, float[] textureCoordinates,
                   int[] faceSides, int[] faceIndices) {
            this.points = points != null ? points : new float[0];
            this.normals = normals != null ? normals : new float[0];
            this.textureCoordinates = textureCoordinates != null ? textureCoordinates : new float[0];
            this.faceSides = faceSides != null ? faceSides : new int[0];
            this.faceIndices = faceIndices != null ? faceIndices : new int[0];
        }

        /**
         * Validates the mesh.
         *
         * @return ValidationResult containing whether mesh is valid and error message if invalid
         */
        public native ValidationResult isValid();

        /**
         * Result of mesh validation.
         */
        public static class ValidationResult {
            public boolean valid;
            public String errorMessage;

            public ValidationResult(boolean valid, String errorMessage) {
                this.valid = valid;
                this.errorMessage = errorMessage;
            }
        }
    }
}
