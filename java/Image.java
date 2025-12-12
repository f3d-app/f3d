package app.f3d.F3D;

import java.util.List;

public class Image {

    static {
        System.loadLibrary("f3d-java");
    }

    public enum SaveFormat {
        PNG,
        JPG,
        TIF,
        BMP
    }

    public enum ChannelType {
        BYTE,
        SHORT,
        FLOAT
    }

    private long mNativeAddress;

    /**
     * Read provided file path into a new image instance.
     *
     * @param filePath path to image file
     */
    public Image(String filePath) {
        mNativeAddress = nativeCreateFromFile(filePath);
    }

    /**
     * Create an image from given width, height, and channel count.
     *
     * @param width image width
     * @param height image height
     * @param channelCount number of channels
     * @param type channel type (default BYTE)
     */
    public Image(int width, int height, int channelCount, ChannelType type) {
        mNativeAddress = nativeCreate(width, height, channelCount, type.ordinal());
    }

    /**
     * Create an image from given width, height, and channel count with BYTE type.
     *
     * @param width image width
     * @param height image height
     * @param channelCount number of channels
     */
    public Image(int width, int height, int channelCount) {
        this(width, height, channelCount, ChannelType.BYTE);
    }

    Image(long nativeAddress) {
        mNativeAddress = nativeAddress;
    }

    /**
     * Read one specific pixel and return all channel normalized values.
     *
     * @param x x coordinate
     * @param y y coordinate
     * @return normalized pixel values [0, 1]
     */
    public native double[] getNormalizedPixel(int x, int y);

    /**
     * Get the list of supported image format extensions.
     *
     * @return list of supported formats
     */
    public static native List<String> getSupportedFormats();

    /**
     * Get image width.
     *
     * @return width
     */
    public native int getWidth();

    /**
     * Get image height.
     *
     * @return height
     */
    public native int getHeight();

    /**
     * Get image channel count.
     *
     * @return channel count
     */
    public native int getChannelCount();

    /**
     * Get image channel type.
     *
     * @return channel type
     */
    public native ChannelType getChannelType();

    /**
     * Get image channel type size in bytes.
     *
     * @return channel type size
     */
    public native int getChannelTypeSize();

    /**
     * Set image buffer data.
     *
     * @param buffer byte array containing image data
     * @return this image for method chaining
     */
    public native Image setContent(byte[] buffer);

    /**
     * Get image buffer data.
     *
     * @return byte array containing image data
     */
    public native byte[] getContent();

    /**
     * Compare current image to a reference.
     *
     * @param reference reference image
     * @return error value (0.0 = identical, 1.0 = completely different)
     */
    public native double compare(Image reference);

    /**
     * Compare images for equality using a threshold of 1e-14.
     *
     * @param reference reference image
     * @return true if images are equal within threshold, false otherwise
     */
    public boolean equals(Image reference) {
        return compare(reference) < 1e-14;
    }

    /**
     * Compare images for inequality using a threshold of 1e-14.
     *
     * @param reference reference image
     * @return true if images are not equal within threshold, false otherwise
     */
    public boolean notEquals(Image reference) {
        return compare(reference) >= 1e-14;
    }

    /**
     * Save an image to the provided file path in the specified format.
     *
     * @param filePath path to save image
     * @param format save format
     * @return this image for method chaining
     */
    public native Image save(String filePath, SaveFormat format);

    /**
     * Save an image to the provided file path in PNG format.
     *
     * @param filePath path to save image
     * @return this image for method chaining
     */
    public Image save(String filePath) {
        return save(filePath, SaveFormat.PNG);
    }

    /**
     * Save an image to a memory buffer in the specified format.
     *
     * @param format save format
     * @return byte array containing saved image
     */
    public native byte[] saveBuffer(SaveFormat format);

    /**
     * Save an image to a memory buffer in PNG format.
     *
     * @return byte array containing saved image
     */
    public byte[] saveBuffer() {
        return saveBuffer(SaveFormat.PNG);
    }

    /**
     * Convert to colored text using ANSI escape sequences.
     *
     * @return ANSI colored text representation
     */
    public native String toTerminalText();

    /**
     * Set the value for a metadata key.
     *
     * @param key metadata key
     * @param value metadata value
     * @return this image for method chaining
     */
    public native Image setMetadata(String key, String value);

    /**
     * Get the value for a metadata key.
     *
     * @param key metadata key
     * @return metadata value
     */
    public native String getMetadata(String key);

    /**
     * List all metadata keys which have a value set.
     *
     * @return list of metadata keys
     */
    public native List<String> allMetadata();

    /**
     * Delete the image and free native resources.
     * After calling this method, the image should not be used anymore.
     */
    public void delete() {
        if (mNativeAddress != 0) {
            nativeDestroy(mNativeAddress);
            mNativeAddress = 0;
        }
    }

    private static native long nativeCreateFromFile(String filePath);
    private static native long nativeCreate(int width, int height, int channelCount, int type);
    private static native void nativeDestroy(long nativeAddress);
}
