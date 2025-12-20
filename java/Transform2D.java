package app.f3d.F3D;

public class Transform2D {
    // Load the native library
    static {
        System.loadLibrary("f3d-java");
    }

    public double[] data;

    public Transform2D() {
        data = new double[] { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
    }

    public Transform2D(double[] matrix) {
        if (matrix.length != 9) {
            throw new IllegalArgumentException("Matrix array must have exactly 9 elements");
        }
        this.data = matrix;
    }

    /**
     * Create a 2D transform from scale, translate and angle.
     *
     * @param scale array of 2 doubles for x and y scale factors
     * @param translate array of 2 doubles for x and y translation
     * @param angleDeg rotation angle in degrees
     * @return Transform2D object with computed matrix
     */
    public static native Transform2D create(double[] scale, double[] translate, double angleDeg);
}
