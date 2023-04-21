package app.f3d.F3D;

public class Loader {

    public Loader(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    public native void loadScene(String file);
    public native void loadGeometry(String file);

    private long mNativeAddress;
}
