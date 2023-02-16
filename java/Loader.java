package app.f3d.F3D;

public class Loader {

    public Loader(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    public native void loadFile(String file);

    private long mNativeAddress;
}
