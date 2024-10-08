package app.f3d.F3D;

public class Loader {

    public Loader(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    public native void add(String file);
    public native void clear();

    private long mNativeAddress;
}
