package app.f3d.F3D;

public class Camera {

    /**
     * A class containing all information to configure a camera.
     */
    public static class CameraState {
        /**
         * Camera position [x, y, z].
         */
        public double[] position;

        /**
         * Camera focal point [x, y, z].
         */
        public double[] focalPoint;

        /**
         * Camera view up vector [x, y, z].
         */
        public double[] viewUp;

        /**
         * Camera view angle in degrees.
         */
        public double viewAngle;

        /**
         * Create a camera state with default values.
         */
        public CameraState() {
            this.position = new double[] { 0.0, 0.0, 1.0 };
            this.focalPoint = new double[] { 0.0, 0.0, 0.0 };
            this.viewUp = new double[] { 0.0, 1.0, 0.0 };
            this.viewAngle = 30.0;
        }

        /**
         * Create a camera state with specified values.
         *
         * @param position camera position [x, y, z]
         * @param focalPoint camera focal point [x, y, z]
         * @param viewUp camera view up vector [x, y, z]
         * @param viewAngle camera view angle in degrees
         */
        public CameraState(double[] position, double[] focalPoint, double[] viewUp, double viewAngle) {
            this.position = position;
            this.focalPoint = focalPoint;
            this.viewUp = viewUp;
            this.viewAngle = viewAngle;
        }
    }

    public Camera(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    /**
     * Set the position of the camera.
     *
     * @param pos array of 3 doubles [x, y, z]
     * @return this camera for method chaining
     */
    public native Camera setPosition(double[] pos);

    /**
     * Get the position of the camera.
     *
     * @return array of 3 doubles [x, y, z]
     */
    public native double[] getPosition();

    /**
     * Set the focal point of the camera.
     *
     * @param foc array of 3 doubles [x, y, z]
     * @return this camera for method chaining
     */
    public native Camera setFocalPoint(double[] foc);

    /**
     * Get the focal point of the camera.
     *
     * @return array of 3 doubles [x, y, z]
     */
    public native double[] getFocalPoint();

    /**
     * Set the view up vector of the camera.
     *
     * @param up array of 3 doubles [x, y, z]
     * @return this camera for method chaining
     */
    public native Camera setViewUp(double[] up);

    /**
     * Get the view up vector of the camera.
     *
     * @return array of 3 doubles [x, y, z]
     */
    public native double[] getViewUp();

    /**
     * Set the view angle in degrees of the camera.
     *
     * @param angle view angle in degrees
     * @return this camera for method chaining
     */
    public native Camera setViewAngle(double angle);

    /**
     * Get the view angle in degrees of the camera.
     *
     * @return view angle in degrees
     */
    public native double getViewAngle();

    /**
     * Set the complete state of the camera.
     *
     * @param state camera state to set
     * @return this camera for method chaining
     */
    public native Camera setState(CameraState state);

    /**
     * Get the complete state of the camera.
     *
     * @return current camera state
     */
    public native CameraState getState();

    /**
     * Divide the camera's distance from the focal point by the given value.
     *
     * @param val dolly factor
     * @return this camera for method chaining
     */
    public native Camera dolly(double val);

    /**
     * Move the camera along its horizontal, vertical, and forward axes.
     *
     * @param right distance to move right
     * @param up distance to move up
     * @param forward distance to move forward
     * @return this camera for method chaining
     */
    public native Camera pan(double right, double up, double forward);

    /**
     * Move the camera along its horizontal and vertical axes.
     *
     * @param right distance to move right
     * @param up distance to move up
     * @return this camera for method chaining
     */
    public Camera pan(double right, double up) {
        return pan(right, up, 0.0);
    }

    /**
     * Decrease the view angle (or the parallel scale in parallel mode) by the specified factor.
     *
     * @param factor zoom factor
     * @return this camera for method chaining
     */
    public native Camera zoom(double factor);

    /**
     * Rotate the camera about its forward axis.
     *
     * @param angle rotation angle in degrees
     * @return this camera for method chaining
     */
    public native Camera roll(double angle);

    /**
     * Rotate the camera about its vertical axis, centered at the focal point.
     *
     * @param angle rotation angle in degrees
     * @return this camera for method chaining
     */
    public native Camera azimuth(double angle);

    /**
     * Rotate the camera about its vertical axis, centered at the camera's position.
     *
     * @param angle rotation angle in degrees
     * @return this camera for method chaining
     */
    public native Camera yaw(double angle);

    /**
     * Rotate the camera about its horizontal axis, centered at the focal point.
     *
     * @param angle rotation angle in degrees
     * @return this camera for method chaining
     */
    public native Camera elevation(double angle);

    /**
     * Rotate the camera about its horizontal axis, centered at the camera's position.
     *
     * @param angle rotation angle in degrees
     * @return this camera for method chaining
     */
    public native Camera pitch(double angle);

    /**
     * Store the current camera configuration as default.
     *
     * @return this camera for method chaining
     */
    public native Camera setCurrentAsDefault();

    /**
     * Reset the camera to the stored default camera configuration.
     *
     * @return this camera for method chaining
     */
    public native Camera resetToDefault();

    /**
     * Reset the camera using the bounds of actors in the scene.
     * Provided zoomFactor will be used to position the camera.
     * A value of 1 corresponds to the bounds roughly aligned to the edges of the window.
     *
     * @param zoomFactor zoom factor (default 0.9)
     * @return this camera for method chaining
     */
    public native Camera resetToBounds(double zoomFactor);

    /**
     * Reset the camera using the bounds of actors in the scene with default zoom factor.
     *
     * @return this camera for method chaining
     */
    public Camera resetToBounds() {
        return resetToBounds(0.9);
    }

    private long mNativeAddress;
}
