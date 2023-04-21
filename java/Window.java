package app.f3d.F3D;

public class Window {

    public enum Type { NONE, NATIVE, NATIVE_OFFSCREEN, EXTERNAL }

    public Window(long nativeAddress) {
        mNativeAddress = nativeAddress;
        mCamera = new Camera(nativeAddress);
    }

    public Camera getCamera() { return mCamera; }

    public native void render();
    public native void setSize(int width, int height);

    public native int getWidth();
    public native int getHeight();

    public native double[] getDisplayFromWorld(double[] pt);
    public native double[] getWorldFromDisplay(double[] pt);

    private long mNativeAddress;

    private Camera mCamera;
}
