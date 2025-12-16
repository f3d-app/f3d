package app.f3d.F3D;

public class Window {

    public enum Type {
        NONE,
        EXTERNAL,
        GLX,
        WGL,
        COCOA,
        EGL,
        OSMESA,
        WASM,
        UNKNOWN
    }

    Window(long nativeAddress) {
        mNativeAddress = nativeAddress;
        mCamera = new Camera(nativeAddress);
    }

    /**
     * Get the type of the window.
     *
     * @return window type
     */
    public native Type getType();

    /**
     * Check if the window is offscreen.
     *
     * @return true if offscreen, false otherwise
     */
    public native boolean isOffscreen();

    /**
     * Get the camera provided by the window.
     *
     * @return camera instance
     */
    public Camera getCamera() {
        return mCamera;
    }

    /**
     * Perform a render of the window to the screen.
     * All dynamic options are updated if needed.
     *
     * @return true on success, false otherwise
     */
    public native boolean render();

    /**
     * Perform a render of the window to the screen and save the result in an image.
     * Set noBackground to true to have a transparent background.
     *
     * @param noBackground if true, background will be transparent
     * @return resulting image
     */
    public native Image renderToImage(boolean noBackground);

    /**
     * Perform a render of the window to the screen and save the result in an image.
     *
     * @return resulting image
     */
    public Image renderToImage() {
        return renderToImage(false);
    }

    /**
     * Set the size of the window.
     *
     * @param width window width
     * @param height window height
     * @return this window for method chaining
     */
    public native Window setSize(int width, int height);

    /**
     * Get the width of the window.
     *
     * @return window width
     */
    public native int getWidth();

    /**
     * Get the height of the window.
     *
     * @return window height
     */
    public native int getHeight();

    /**
     * Set the position of the window.
     *
     * @param x x position
     * @param y y position
     * @return this window for method chaining
     */
    public native Window setPosition(int x, int y);

    /**
     * Set the icon to be shown by a window manager.
     *
     * @param icon icon data as byte array
     * @return this window for method chaining
     */
    public native Window setIcon(byte[] icon);

    /**
     * Set the window name to be shown by a window manager.
     *
     * @param windowName window name
     * @return this window for method chaining
     */
    public native Window setWindowName(String windowName);

    /**
     * Convert a point in display coordinate to world coordinate.
     *
     * @param displayPoint array of 3 doubles [x, y, z] in display coordinates
     * @return array of 3 doubles [x, y, z] in world coordinates
     */
    public native double[] getWorldFromDisplay(double[] displayPoint);

    /**
     * Convert a point in world coordinate to display coordinate.
     *
     * @param worldPoint array of 3 doubles [x, y, z] in world coordinates
     * @return array of 3 doubles [x, y, z] in display coordinates
     */
    public native double[] getDisplayFromWorld(double[] worldPoint);

    private long mNativeAddress;
    private Camera mCamera;
}
