package app.f3d.F3D;

public class Options {

    public Options(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    public native void toggle(String name);

    private long mNativeAddress;
}
