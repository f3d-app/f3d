package app.f3d.F3D;

import java.util.List;

public class VideoEncoder implements AutoCloseable {

    static {
        System.loadLibrary("f3d-java");
    }

    /** Thrown when codec initialization fails. */
    public static class CodecException extends F3DException {
        public CodecException(String message) { super(message); }
    }

    /** Thrown when data transport fails. */
    public static class TransportException extends F3DException {
        public TransportException(String message) { super(message); }
    }

    /**
   * Enumeration of codec types.
   * Supported codecs include H264, HEVC, VP8, VP9, AV1, and EXPLICIT for specifying a codec name.
   * H264 is widely supported and commonly used for video encoding.
   * HEVC (also called H265) offers better compression efficiency than H264 but may have less
   * widespread support.
   * VP8 and VP9 are commonly used for web-based video applications.
   * AV1 is a modern codec with high compression efficiency but may require
   * more computational resources.
   */
    public enum Codec {
        H264,
        HEVC,
        VP8,
        VP9,
        AV1,
        EXPLICIT
    }

    /**
   * Parameters for configuring the video stream construction.
   * ExplicitCodecName is used when the codec is set to EXPLICIT.
   * Use getAvailableEncoders() to retrieve the list of available codec names.
   * Bitrate is specified in Mbps.
   */
    public static class Params {
        public Codec codec = Codec.H264;
        public String explicitCodecName;
        public int width;
        public int height;
        public double frameRate = 30;
        public double bitrate = 5;
        public boolean lowLatency = false;
    }

    /**
     * Listener invoked for each encoded video packet.
     */
    public interface PacketListener {
        void execute(VideoPacket packet);
    }

    private long mNativeAddress;

    /**
     * Create a video encoder with the given parameters.
     * Throws CodecException if the codec cannot be initialized.
     *
     * @param params encoder parameters
     */
    public VideoEncoder(Params params) {
        mNativeAddress = nativeCreate(params.codec.ordinal(), params.explicitCodecName, params.width, params.height,
            params.frameRate, params.bitrate, params.lowLatency);
    }

    /**
     * Represents a pair of strings, used for codec name and description.
     */
    public record StringPair(String first, String second) {}

    /**
     * Get the list of available codecs.
     * Returns a list of pair of String (name, description) representing the available codecs.
     */
    public static native List<StringPair> getAvailableEncoders();

    /**
     * Get the width of the video stream.
     *
     * @return width
     */
    public native int getWidth();

    /**
     * Get the height of the video stream.
     *
     * @return height
     */
    public native int getHeight();

    /**
     * Asynchronously listen for encoded video packets and invoke the provided listener for each
     * packet. Thread safety is ensured, allowing concurrent calls to submit and listen.
     *
     * @param listener packet listener
     * @return this video encoder for method chaining
     */
    public native VideoEncoder listen(PacketListener listener);

    /**
     * Send a frame to the video stream for encoding.
     * Returns true if the frame was successfully submitted, false if the encoder is busy and
     * cannot accept new frames at the moment. Throws TransportException if there is an error
     * submitting the frame to the encoder.
     *
     * @param frame video frame to submit
     * @return true if submitted, false otherwise
     */
    public native boolean submit(VideoFrame frame);

    /**
     * Flush the video encoder, ensuring all encoded frames are processed.
     * Signals the end of the stream to the encoder, allowing it to output any remaining packets.
     *
     * @return this video encoder for method chaining
     */
    public native VideoEncoder flush();

    /**
     * Delete the video encoder and free native resources.
     * After calling this method, the video encoder should not be used anymore.
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

    private static native long nativeCreate(int codec, String explicitCodecName, int width, int height,
        double frameRate, double bitrate, boolean lowLatency);
    private static native void nativeDestroy(long nativeAddress);
}
