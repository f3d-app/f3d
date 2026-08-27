package app.f3d.F3D;

public class VideoPacket {

    private long mNativeAddress;

    VideoPacket(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    /**
     * Get the data of the video packet.
     *
     * @return packet data
     */
    public native byte[] getPacketData();

    /**
     * Get the timestamp of the video packet.
     *
     * @return timestamp value
     */
    public native long getTimestamp();

    /**
     * Check if the video packet is a key frame.
     *
     * @return true if the packet is a key frame, false otherwise
     */
    public native boolean isKeyFrame();
}
