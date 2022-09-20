package io.github.f3d_app.f3d;

public class Options {

    public Options(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    public native void toggle(String name);

    private long mNativeAddress;
}
