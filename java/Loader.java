package app.f3d.F3D;

public class Loader {

    public enum LoadFileEnum { LOAD_FIRST, LOAD_PREVIOUS, LOAD_CURRENT, LOAD_NEXT, LOAD_LAST }

    public Loader(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    public native void loadFile(LoadFileEnum cursor);
    public native void addFile(String file);

    private long mNativeAddress;
}
