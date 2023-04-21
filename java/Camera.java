package app.f3d.F3D;

public class Camera {

    public Camera(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    public native void dolly(double val);
    public native void roll(double angle);
    public native void azimuth(double angle);
    public native void yaw(double angle);
    public native void elevation(double angle);
    public native void pitch(double angle);

    public native double[] getFocalPoint();
    public native void setFocalPoint(double[] pt);
    public native double[] getPosition();
    public native void setPosition(double[] pt);

    private long mNativeAddress;
}
