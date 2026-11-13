package app.f3d.F3D;

import java.nio.ByteBuffer;

public class VideoFrame implements AutoCloseable {

    /** Thrown when video frame allocation fails. */
    public static class InvalidFrameException extends F3DException {
        public InvalidFrameException(String message) { super(message); }
    }

    private long mNativeAddress;

    /**
     * Create a new video frame.
     * Should not be called directly, use Window.getVideoFrame() instead.
     */
    public VideoFrame(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    /**
     * Set the timestamp of the video frame.
     *
     * @param timestamp timestamp value
     * @return this video frame for method chaining
     */
    public native VideoFrame setTimestamp(long timestamp);

    /**
     * Delete the video frame and free native resources.
     * After calling this method, the video frame should not be used anymore.
     */
    public void delete() {
        if (mNativeAddress != 0) {
            nativeDestroy(mNativeAddress);
            mNativeAddress = 0;
        }
    }

    @Override
    public void close() {
        delete();
    }

    private static native void nativeDestroy(long nativeAddress);
}
